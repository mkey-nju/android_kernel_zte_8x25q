/* drivers/input/misc/kxtik.c - KXTIK accelerometer driver
 *
 * Copyright (C) 2012 ZTE, Inc.
 * Written by Kuching Tan <kuchingtan@kionix.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kxtik.h>
#include <linux/version.h>
#include <linux/input-polldev.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#define NAME			"kxtik"
#define G_MAX			8000
/* OUTPUT REGISTERS */
#define XOUT_L			0x06
#define WHO_AM_I		0x0F
/* CONTROL REGISTERS */
#define INT_REL			0x1A
#define CTRL_REG1		0x1B
#define INT_CTRL1		0x1E
#define DATA_CTRL		0x21
/* CONTROL REGISTER 1 BITS */
#define PC1_OFF			0x7F
#define PC1_ON			(1 << 7)
/* Data ready funtion enable bit: set during probe if using irq mode */
#define DRDYE			(1 << 5)
/* INTERRUPT CONTROL REGISTER 1 BITS */
/* Set these during probe if using irq mode */
#define KXTIK_IEL		(1 << 3)
#define KXTIK_IEA		(1 << 4)
#define KXTIK_IEN		(1 << 5)
/* INPUT_ABS CONSTANTS */
#define FUZZ			3
#define FLAT			3
/* RESUME STATE INDICES */
#define RES_DATA_CTRL		0
#define RES_CTRL_REG1		1
#define RES_INT_CTRL1		2
#define RESUME_ENTRIES		3

/*
 * The following table lists the maximum appropriate poll interval for each
 * available output data rate.
 */
static const struct {
	unsigned int cutoff;
	u8 mask;
} kxtik_odr_table[] = {
	{ 3,	ODR800F },
	{ 5,	ODR400F },
	{ 10,	ODR200F },
	{ 20,	ODR100F },
	{ 40,	ODR50F  },
	{ 80,	ODR25F  },
	{ 0,	ODR12_5F},
};

struct kxtik_data {
	struct i2c_client *client;
	struct kxtik_platform_data pdata;
	struct input_dev *input_dev;
	struct workqueue_struct *workqueue;
	struct work_struct irq_work;
	struct input_polled_dev *poll_dev;
	unsigned int poll_interval;
	unsigned int poll_delay;
	int enable; 
	u8 shift;
	u8 ctrl_reg1;
	u8 data_ctrl;
	u8 int_ctrl;
};
static short accel_data[3];
static struct mutex data_lock;
#define KXTIK_IOCTL_MAGIC 0X0F
#define KXTIK_IOCTL_GET_DATA _IOR(KXTIK_IOCTL_MAGIC, 1, short[3])

static int kxtik_i2c_read(struct kxtik_data *tik, u8 addr, u8 *data, int len)
{
	struct i2c_msg msgs[] = {
		{
			.addr = tik->client->addr,
			.flags = tik->client->flags,
			.len = 1,
			.buf = &addr,
		},
		{
			.addr = tik->client->addr,
			.flags = tik->client->flags | I2C_M_RD,
			.len = len,
			.buf = data,
		},
	};

	return i2c_transfer(tik->client->adapter, msgs, 2);
}

static void kxtik_report_acceleration_data(struct kxtik_data *tik)
{
	s16 acc_data[3]; /* Data bytes from hardware xL, xH, yL, yH, zL, zH */
	s16 x, y, z;
	int err;
//	struct input_dev *input_dev = tik->input_dev;
	if (!tik->enable) {
                return;
        }  
	mutex_lock(&data_lock);
	err = kxtik_i2c_read(tik, XOUT_L, (u8 *)acc_data, 6);
	if (err < 0)
		dev_err(&tik->client->dev, "accelerometer data read failed\n");

	x = ((s16) le16_to_cpu(acc_data[tik->pdata.axis_map_x])) >> tik->shift;
	y = ((s16) le16_to_cpu(acc_data[tik->pdata.axis_map_y])) >> tik->shift;
	z = ((s16) le16_to_cpu(acc_data[tik->pdata.axis_map_z])) >> tik->shift;

#if 0
	input_report_abs(tik->input_dev, ABS_X, tik->pdata.negate_x ? -x : x);
	input_report_abs(tik->input_dev, ABS_Y, tik->pdata.negate_y ? -y : y);
	input_report_abs(tik->input_dev, ABS_Z, tik->pdata.negate_z ? -z : z);
	input_sync(tik->input_dev);
#endif
	accel_data[0] = tik->pdata.negate_x ? -x : x;
	accel_data[1] = tik->pdata.negate_y ? -y : y;
	accel_data[2] = tik->pdata.negate_z ? -z : z;
	mutex_unlock(&data_lock);
}

static irqreturn_t kxtik_isr(int irq, void *dev)
{
	struct kxtik_data *tik = dev;
	queue_work(tik->workqueue, &tik->irq_work);
	return IRQ_HANDLED;
}

static void kxtik_irq_work(struct work_struct *work)
{
	struct kxtik_data *tik = container_of(work,	struct kxtik_data, irq_work);
	int err;

	/* data ready is the only possible interrupt type */
	kxtik_report_acceleration_data(tik);

	err = i2c_smbus_read_byte_data(tik->client, INT_REL);
	if (err < 0)
		dev_err(&tik->client->dev,
			"error clearing interrupt status: %d\n", err);
}

static int kxtik_update_g_range(struct kxtik_data *tik, u8 new_g_range)
{
	switch (new_g_range) {
	case KXTIK_G_2G:
		tik->shift = 4;
		break;
	case KXTIK_G_4G:
		tik->shift = 3;
		break;
	case KXTIK_G_8G:
		tik->shift = 2;
		break;
	default:
		return -EINVAL;
	}

	tik->ctrl_reg1 &= 0xe7;
	tik->ctrl_reg1 |= new_g_range;

	return 0;
}

static int kxtik_update_odr(struct kxtik_data *tik, unsigned int poll_interval)
{
	int err;
	int i;

	/* Use the lowest ODR that can support the requested poll interval */
	for (i = 0; i < ARRAY_SIZE(kxtik_odr_table); i++) {
		tik->data_ctrl = kxtik_odr_table[i].mask;
		if (poll_interval < kxtik_odr_table[i].cutoff)
			break;
	}

	err = i2c_smbus_write_byte_data(tik->client, CTRL_REG1, 0);
	if (err < 0)
		return err;

	err = i2c_smbus_write_byte_data(tik->client, DATA_CTRL, tik->data_ctrl);
	if (err < 0)
		return err;

	err = i2c_smbus_write_byte_data(tik->client, CTRL_REG1, tik->ctrl_reg1);
	if (err < 0)
		return err;

	return 0;
}

static int kxtik_device_power_on(struct kxtik_data *tik)
{
	if (tik->pdata.power_on)
		return tik->pdata.power_on();

	return 0;
}

static void kxtik_device_power_off(struct kxtik_data *tik)
{
	int err;

	tik->ctrl_reg1 &= PC1_OFF;
	err = i2c_smbus_write_byte_data(tik->client, CTRL_REG1, tik->ctrl_reg1);
	if (err < 0)
		dev_err(&tik->client->dev, "soft power off failed\n");

	if (tik->pdata.power_off)
		tik->pdata.power_off();
//wanghaifei for debug
	err = i2c_smbus_read_byte_data(tik->client, 0x1E);
	printk("kxtik int ctl_1 0x%x\n", err);
}

static int kxtik_enable(struct kxtik_data *tik)
{
	int err;

	err = kxtik_device_power_on(tik);
	if (err < 0)
		return err;

	/* ensure that PC1 is cleared before updating control registers */
	err = i2c_smbus_write_byte_data(tik->client, CTRL_REG1, 0);
	if (err < 0)
		return err;

	/* only write INT_CTRL_REG1 if in irq mode */
	if (tik->client->irq) {
		err = i2c_smbus_write_byte_data(tik->client,
						INT_CTRL1, tik->int_ctrl);
		if (err < 0)
			return err;
	}

	err = kxtik_update_g_range(tik, tik->pdata.g_range);
	if (err < 0)
		return err;

	/* turn on outputs */
	tik->ctrl_reg1 |= PC1_ON;
	err = i2c_smbus_write_byte_data(tik->client, CTRL_REG1, tik->ctrl_reg1);
	if (err < 0)
		return err;

	err = kxtik_update_odr(tik, tik->poll_interval);
	if (err < 0)
		return err;

	/* clear initial interrupt if in irq mode */
	if (tik->client->irq) {
		err = i2c_smbus_read_byte_data(tik->client, INT_REL);
		if (err < 0) {
			dev_err(&tik->client->dev,
				"error clearing interrupt: %d\n", err);
			goto fail;
		}
	}

	return 0;

fail:
	kxtik_device_power_off(tik);
	return err;
}

static void kxtik_disable(struct kxtik_data *tik)
{
	kxtik_device_power_off(tik);
}

static int kxtik_input_open(struct input_dev *input)
{
	struct kxtik_data *tik = input_get_drvdata(input);

	return kxtik_enable(tik);
}

static void kxtik_input_close(struct input_dev *dev)
{
	struct kxtik_data *tik = input_get_drvdata(dev);

	kxtik_disable(tik);
}

static void __devinit kxtik_init_input_device(struct kxtik_data *tik,
					      struct input_dev *input_dev)
{
	__set_bit(EV_ABS, input_dev->evbit);
	input_set_abs_params(input_dev, ABS_X, -G_MAX, G_MAX, FUZZ, FLAT);
	input_set_abs_params(input_dev, ABS_Y, -G_MAX, G_MAX, FUZZ, FLAT);
	input_set_abs_params(input_dev, ABS_Z, -G_MAX, G_MAX, FUZZ, FLAT);

	input_dev->name = "kxtik_accel";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &tik->client->dev;
}

static int __devinit kxtik_setup_input_device(struct kxtik_data *tik)
{
	struct input_dev *input_dev;
	int err;

	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&tik->client->dev, "input device allocate failed\n");
		return -ENOMEM;
	}

	tik->input_dev = input_dev;

	input_dev->open = kxtik_input_open;
	input_dev->close = kxtik_input_close;
	input_set_drvdata(input_dev, tik);

	kxtik_init_input_device(tik, input_dev);

	err = input_register_device(tik->input_dev);
	if (err) {
		dev_err(&tik->client->dev,
			"unable to register input polled device %s: %d\n",
			tik->input_dev->name, err);
		input_free_device(tik->input_dev);
		return err;
	}

	return 0;
}

/*
 * When IRQ mode is selected, we need to provide an interface to allow the user
 * to change the output data rate of the part.  For consistency, we are using
 * the set_poll method, which accepts a poll interval in milliseconds, and then
 * calls update_odr() while passing this value as an argument.  In IRQ mode, the
 * data outputs will not be read AT the requested poll interval, rather, the
 * lowest ODR that can support the requested interval.  The client application
 * will be responsible for retrieving data from the input node at the desired
 * interval.
 */

/* Returns currently selected poll interval (in ms) */
static ssize_t kxtik_get_poll(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kxtik_data *tik = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", tik->poll_interval);
}

/* Allow users to select a new poll interval (in ms) */
static ssize_t kxtik_set_poll(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kxtik_data *tik = i2c_get_clientdata(client);
	struct input_dev *input_dev = tik->input_dev;
	unsigned int interval;

	interval = (unsigned int)simple_strtoul(buf, NULL, 10);

	/* Lock the device to prevent races with open/close (and itself) */
	mutex_lock(&input_dev->mutex);

	disable_irq(client->irq);

	/*
	 * Set current interval to the greater of the minimum interval or
	 * the requested interval
	 */
	tik->poll_interval = max(interval, tik->pdata.min_interval);
	tik->poll_delay = msecs_to_jiffies(tik->poll_interval);

	kxtik_update_odr(tik, tik->poll_interval);

	enable_irq(client->irq);
	mutex_unlock(&input_dev->mutex);

	return count;
}

/* Allow users to enable/disable the device */
static ssize_t kxtik_set_enable(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kxtik_data *tik = i2c_get_clientdata(client);
	struct input_dev *input_dev = tik->input_dev;
	unsigned int enable;

	enable = (unsigned int)simple_strtoul(buf, NULL, 10);

	/* Lock the device to prevent races with open/close (and itself) */
	mutex_lock(&input_dev->mutex);

	if(enable)
		kxtik_enable(tik);
	else
		kxtik_disable(tik);

	mutex_unlock(&input_dev->mutex);

	return count;
}

static DEVICE_ATTR(poll, S_IRUGO|S_IWUSR, kxtik_get_poll, kxtik_set_poll);
static DEVICE_ATTR(enable_device, S_IWUSR, NULL, kxtik_set_enable);

static struct attribute *kxtik_attributes[] = {
	&dev_attr_poll.attr,
	&dev_attr_enable_device.attr,
	NULL
};

static struct attribute_group kxtik_attribute_group = {
	.attrs = kxtik_attributes
};

static ssize_t kxtik_poll_set_enable(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count)
{
        struct input_dev *input_d = to_input_dev(dev);
        struct input_polled_dev *poll_dev = input_get_drvdata(input_d);
        struct kxtik_data *tik = poll_dev->private;
	unsigned int enable;

	enable = (unsigned int)simple_strtoul(buf, NULL, 10);

	/* Lock the device to prevent races with open/close (and itself) */
	mutex_lock(&data_lock);

	if(enable) {
		kxtik_enable(tik);
		tik->enable = 1;
	} else {
		kxtik_disable(tik);
		tik->enable = 0;
	}

	mutex_unlock(&data_lock);

	return count;
}

static DEVICE_ATTR(enable, S_IWUSR, NULL, kxtik_poll_set_enable);

static void kxtik_poll(struct input_polled_dev *dev)
{
	struct kxtik_data *tik = (struct kxtik_data *)dev->private;
	unsigned int poll_interval = dev->poll_interval;

	kxtik_report_acceleration_data(tik);

	if (poll_interval != tik->poll_interval) {
		kxtik_update_odr(tik, poll_interval);
		tik->poll_interval = poll_interval;
	}
}

/*
static void kxtik_polled_input_open(struct input_polled_dev *dev)
{
	struct kxtik_data *tik = (struct kxtik_data *)dev->private;

	kxtik_enable(tik);
}

static void kxtik_polled_input_close(struct input_polled_dev *dev)
{
	struct kxtik_data *tik = (struct kxtik_data *)dev->private;

	kxtik_disable(tik);
}
*/
static int __devinit kxtik_setup_polled_device(struct kxtik_data *tik)
{
	int err;
	struct input_polled_dev *poll_dev;
	poll_dev = input_allocate_polled_device();

	if (!poll_dev) {
		dev_err(&tik->client->dev,
			"Failed to allocate polled device\n");
		return -ENOMEM;
	}

	tik->poll_dev = poll_dev;
	tik->input_dev = poll_dev->input;

	poll_dev->private = tik;
	poll_dev->poll = kxtik_poll;
	//poll_dev->open = kxtik_polled_input_open;
	//poll_dev->close = kxtik_polled_input_close;
	//poll_dev->poll_interval_min = tik->pdata.min_interval;
	poll_dev->poll_interval = tik->pdata.poll_interval;

	kxtik_init_input_device(tik, poll_dev->input);

	err = input_register_polled_device(poll_dev);
	if (err) {
		dev_err(&tik->client->dev,
			"Unable to register polled device, err=%d\n", err);
		input_free_polled_device(poll_dev);
		return err;
	}

        err = device_create_file(&poll_dev->input->dev, &dev_attr_enable);
        if (err) {
                dev_err(&tik->client->dev,
                        "Unable to create polled device enable interface, err=%d\n", err);
                input_unregister_polled_device(poll_dev);
                input_free_polled_device(poll_dev);
                return err;
        }

	return 0;
}

//static void __devexit kxtik_teardown_polled_device(struct kxtik_data *tik)
static void kxtik_teardown_polled_device(struct kxtik_data *tik)
{
        device_remove_file(&tik->poll_dev->input->dev, &dev_attr_enable);
	input_unregister_polled_device(tik->poll_dev);
	input_free_polled_device(tik->poll_dev);
}

static int __devinit kxtik_verify(struct kxtik_data *tik)
{
	int retval;

	retval = kxtik_device_power_on(tik);
	if (retval < 0)
		return retval;

	retval = i2c_smbus_read_byte_data(tik->client, WHO_AM_I);
	if (retval < 0) {
		dev_err(&tik->client->dev, "read err int source\n");
		goto out;
	}
	printk("kxtik chip id 0x%x\n", retval);
	
	retval = (retval != 0x05) ? -EIO : 0;

out:
	kxtik_device_power_off(tik);
	return retval;
}
static int kxtik_open(struct inode *inode, struct file *file)
{
	return 0;
}
static int kxtik_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long kxtik_ioctl(struct file *file,unsigned int cmd, unsigned long arg)
{
	switch(cmd) {
		case KXTIK_IOCTL_GET_DATA:
			mutex_lock(&data_lock);
			if (copy_to_user((void __user *)arg, accel_data, sizeof(accel_data))) {
				printk("kxtik get sensor data failed\n");
				return -EFAULT;
			}
			mutex_unlock(&data_lock);
			break;
		default:
			break;
	}

	return 0;
}

static struct file_operations kxtik_fops = { 
        .owner = THIS_MODULE,
        .open = kxtik_open,
        .release = kxtik_release,
        .unlocked_ioctl = kxtik_ioctl,
};

static struct miscdevice kxtik_device = { 
        .minor = MISC_DYNAMIC_MINOR,
        .name = "kxtik_dev",
        .fops = &kxtik_fops,
};


static int __devinit kxtik_probe(struct i2c_client *client,
				 const struct i2c_device_id *id)
{
	const struct kxtik_platform_data *pdata = client->dev.platform_data;
	struct kxtik_data *tik;
	int err;

	printk("kxtik probe begin!\n");
	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_I2C | I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev, "client is not i2c capable\n");
		return -ENXIO;
	}

	if (!pdata) {
		dev_err(&client->dev, "platform data is NULL; exiting\n");
		return -EINVAL;
	}

	tik = kzalloc(sizeof(*tik), GFP_KERNEL);
	if (!tik) {
		dev_err(&client->dev,
			"failed to allocate memory for module data\n");
		return -ENOMEM;
	}

	tik->client = client;
	tik->pdata = *pdata;

	if (pdata->init) {
		err = pdata->init();
		if (err < 0)
			goto err_free_mem;
	}

	err = kxtik_verify(tik);
	if (err < 0) {
		dev_err(&client->dev, "device not recognized\n");
		goto err_pdata_exit;
	}

	i2c_set_clientdata(client, tik);

	tik->ctrl_reg1 = tik->pdata.res_12bit | tik->pdata.g_range;
	tik->poll_interval = tik->pdata.poll_interval;
	tik->poll_delay = msecs_to_jiffies(tik->poll_interval);

	if (client->irq) {
		err = kxtik_setup_input_device(tik);
		if (err)
			goto err_pdata_exit;

		tik->workqueue = create_workqueue("KXTIK Workqueue");
		INIT_WORK(&tik->irq_work, kxtik_irq_work);
		/* If in irq mode, populate INT_CTRL_REG1 and enable DRDY. */
		tik->int_ctrl |= KXTIK_IEN | KXTIK_IEA;
		tik->ctrl_reg1 |= DRDYE;

		err = request_threaded_irq(client->irq, NULL, kxtik_isr,
					   IRQF_TRIGGER_RISING | IRQF_ONESHOT,
					   "kxtik-irq", tik);
		if (err) {
			dev_err(&client->dev, "request irq failed: %d\n", err);
			goto err_destroy_input;
		}

		err = sysfs_create_group(&client->dev.kobj, &kxtik_attribute_group);
		if (err) {
			dev_err(&client->dev, "sysfs create failed: %d\n", err);
			goto err_free_irq;
		}

	} else {
		err = kxtik_setup_polled_device(tik);
		if (err)
			goto err_pdata_exit;
		mutex_init(&data_lock);
	        tik->enable = 0;
		err = misc_register(&kxtik_device);
		if (err) {
			dev_err(&client->dev, "kxtik register device failed: %d\n", err);
			kxtik_teardown_polled_device(tik);
			goto err_pdata_exit;
		}
	}

	return 0;

err_free_irq:
	free_irq(client->irq, tik);
err_destroy_input:
	input_unregister_device(tik->input_dev);
	destroy_workqueue(tik->workqueue);
err_pdata_exit:
	if (tik->pdata.exit)
		tik->pdata.exit();
err_free_mem:
	kfree(tik);
	return err;
}

static int __devexit kxtik_remove(struct i2c_client *client)
{
	struct kxtik_data *tik = i2c_get_clientdata(client);

	if (client->irq) {
		sysfs_remove_group(&client->dev.kobj, &kxtik_attribute_group);
		free_irq(client->irq, tik);
		input_unregister_device(tik->input_dev);
		destroy_workqueue(tik->workqueue);
	} else {
		misc_deregister(&kxtik_device);
		kxtik_teardown_polled_device(tik);
	}

	if (tik->pdata.exit)
		tik->pdata.exit();

	kfree(tik);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int suspend_power = 0;
static int kxtik_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kxtik_data *tik = i2c_get_clientdata(client);

        int reg;

        reg = i2c_smbus_read_byte_data(tik->client, CTRL_REG1);
        if (reg < 0)
                dev_err(&tik->client->dev, "kxtik_suspend read power failed\n");
        if (reg & PC1_ON) {
                suspend_power = 1;
                kxtik_disable(tik);
        }   

	return 0;
}

static int kxtik_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kxtik_data *tik = i2c_get_clientdata(client);
        int retval = 0;


        if (suspend_power == 1) {
                suspend_power = 0;
                kxtik_enable(tik);
        }

	return retval;
}
#endif

static SIMPLE_DEV_PM_OPS(kxtik_pm_ops, kxtik_suspend, kxtik_resume);

static const struct i2c_device_id kxtik_id[] = {
	{NAME, 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, kxtik_id);

static struct i2c_driver kxtik_driver = {
	.driver = {
		.name	= NAME,
		.owner	= THIS_MODULE,
		.pm	= &kxtik_pm_ops,
	},
	.probe		= kxtik_probe,
	.remove		= __devexit_p(kxtik_remove),
	.id_table	= kxtik_id,
};

static int __init kxtik_init(void)
{
	return i2c_add_driver(&kxtik_driver);
}
module_init(kxtik_init);

static void __exit kxtik_exit(void)
{
	i2c_del_driver(&kxtik_driver);
}
module_exit(kxtik_exit);

MODULE_DESCRIPTION("KXTIK accelerometer driver");
MODULE_AUTHOR("wang.haifei@zte.com.cn>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.6.0");

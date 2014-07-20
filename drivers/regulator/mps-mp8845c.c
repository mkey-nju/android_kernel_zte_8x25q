/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/module.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regmap.h>
#include <linux/regulator/mps-mp8845c.h>
#include <linux/platform_device.h>

#define MP8845C_REG_VSEL			0x00
#define MP8845C_REG_SYSCNTLREG1	0x01
#define MP8845C_REG_SYSCNTLREG2	0x02
#define MP8845C_REG_ID1				0x03
#define MP8845C_REG_ID2				0x04
#define MP8845C_REG_STATUS			0x05

#define MP8845C_REG_VSEL_BUCK_OUT_MASK		0x7f
#define MP8845C_REG_VSEL_BUCK_EN_MASK		BIT(7)

#define MP8845C_REG_SYSCNTLREG1_SWITCH_FREQ_MASK	BIT(5)|BIT(6)|BIT(7)
#define MP8845C_REG_SYSCNTLREG1_MODE_MASK			BIT(0)

#define MP8845C_REG_SYSCNTLREG2_GO					BIT(5)
#define MP8845C_REG_SYSCNTLREG2_DISCHARGING_MASK	BIT(4)
#define MP8845C_REG_SYSCNTLREG2_SLEW_RATE_MASK		BIT(0)|BIT(1)|BIT(2)

#define MP8845C_REG_ID1_VENDOR_MASK			BIT(4)|BIT(5)|BIT(6)|BIT(7)
#define MP8845C_REG_ID1_DIE_ID_MASK				BIT(0)|BIT(1)|BIT(2)|BIT(3)

#define MP8845C_REG_ID2_DIE_VER_MASK			BIT(0)|BIT(1)|BIT(2)|BIT(3)

#define MP8845C_SLEW_RATE_MIN		500 		//	uV/ms
#define MP8845C_SLEW_RATE_MAX		64000	//	uV/ms


struct mp8845c_info {
	struct regulator_dev *regulator;
	struct regulator_init_data *init_data;
	struct regmap *regmap;
	struct device *dev;
	int mode;
	int curr_voltage;
	int slew_rate_uV_per_us;
	int step_uV;
};

static void dump_registers(struct mp8845c_info *dd,unsigned int reg, const char *func)
{
	unsigned int val = 0;

	regmap_read(dd->regmap, reg, &val);
	dev_dbg(dd->dev, "%s: MP8845C: Reg = %x, Val = %x\n", func, reg, val);
}

static void mp8845c_slew_delay(struct mp8845c_info *dd, int prev_uV, int new_uV)
{
	int delay;

	delay = (abs(prev_uV - new_uV) /dd->slew_rate_uV_per_us) + 1;	//number of slew

	dev_dbg(dd->dev, "Slew Delay = %d\n", delay);

	udelay(delay);
}


static struct regmap_config mp8845c_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
};

static int __devinit mp8845c_init(struct mp8845c_info *dd,
			const struct mp8845c_platform_data *pdata)
{
	int rc;
	unsigned int val;

	/* get the current programmed voltage */
	rc = regmap_read(dd->regmap, MP8845C_REG_VSEL, &val);
	if (rc) {
		dev_err(dd->dev, "Unable to get volatge rc(%d)", rc);
		return rc;
	}
	dd->curr_voltage = ((val & MP8845C_REG_VSEL_BUCK_OUT_MASK) * pdata->step_uV)+ pdata->min_uV;
	
	/* set discharge */
	rc = regmap_update_bits(dd->regmap, MP8845C_REG_SYSCNTLREG2,
					MP8845C_REG_SYSCNTLREG2_DISCHARGING_MASK,
					(pdata->discharge_enable ? MP8845C_REG_SYSCNTLREG2_DISCHARGING_MASK : 0));
	if (rc) {
		dev_err(dd->dev, "Unable to set Active Discharge rc(%d)\n", rc);
		return -EINVAL;
	}

	/* set slew rate */
	if (pdata->slew_rate_uV_per_us < MP8845C_SLEW_RATE_MIN ||
			pdata->slew_rate_uV_per_us > MP8845C_SLEW_RATE_MAX) {
		dev_err(dd->dev, "Invalid slew rate %d\n", pdata->slew_rate_uV_per_us);
		return -EINVAL;
	}
	val = (0x08 - (pdata->slew_rate_uV_per_us /MP8845C_SLEW_RATE_MIN));

	dd->slew_rate_uV_per_us = (0x08 - val) * MP8845C_SLEW_RATE_MIN;

	rc = regmap_update_bits(dd->regmap, MP8845C_REG_SYSCNTLREG2,
			MP8845C_REG_SYSCNTLREG2_SLEW_RATE_MASK, val);
	if (rc)
		dev_err(dd->dev, "Unable to set slew rate rc(%d)\n", rc);

	rc = regmap_update_bits(dd->regmap, MP8845C_REG_SYSCNTLREG2,
			MP8845C_REG_SYSCNTLREG2_GO, MP8845C_REG_SYSCNTLREG2_GO);
	if (rc)
		dev_err(dd->dev, "Unable to set buck scaling rc(%d)\n", rc);
	
	dump_registers(dd, MP8845C_REG_SYSCNTLREG1, __func__);
	dump_registers(dd, MP8845C_REG_SYSCNTLREG2, __func__);

	return rc;
}

static int mp8845c_set_voltage(struct regulator_dev *rdev,int min_uV, int max_uV, unsigned *selector)
{
	int rc, set_val, new_uV;
	struct mp8845c_info *dd = rdev_get_drvdata(rdev);

	set_val = DIV_ROUND_UP(min_uV - dd->init_data->constraints.min_uV, dd->step_uV);
	
	new_uV = (set_val * dd->step_uV) + dd->init_data->constraints.min_uV;
	
	dev_dbg(dd->dev, "mp8845c_set_voltage:  set_uV = %d, set_val = 0x%x, new_uV%d\n",  min_uV, set_val, new_uV);
	
	rc = regmap_update_bits(dd->regmap, MP8845C_REG_VSEL,
		MP8845C_REG_VSEL_BUCK_OUT_MASK, (set_val & MP8845C_REG_VSEL_BUCK_OUT_MASK));
	if (rc) {
		dev_err(dd->dev, "Unable to set volatge (%d %d)\n",
							min_uV, max_uV);
	} else {
		mp8845c_slew_delay(dd, dd->curr_voltage, new_uV);
		dd->curr_voltage = new_uV;
	}

	dump_registers(dd, MP8845C_REG_VSEL, __func__);

	return rc;

}

static int mp8845c_get_voltage(struct regulator_dev *rdev)
{
	unsigned int val;
	int rc;
	struct mp8845c_info *dd = rdev_get_drvdata(rdev);

	rc = regmap_read(dd->regmap, MP8845C_REG_VSEL, &val);
	if (rc) {
		dev_err(dd->dev, "Unable to get volatge rc(%d)", rc);
		return rc;
	}
	dd->curr_voltage = ((val & MP8845C_REG_VSEL_BUCK_OUT_MASK) * dd->step_uV) + dd->init_data->constraints.min_uV;

	dump_registers(dd, MP8845C_REG_VSEL, __func__);

	return dd->curr_voltage;
}

static int mp8845c_enable(struct regulator_dev *rdev)
{
	int rc;
	struct mp8845c_info *dd = rdev_get_drvdata(rdev);

	rc = regmap_update_bits(dd->regmap, MP8845C_REG_VSEL,
				MP8845C_REG_VSEL_BUCK_EN_MASK, MP8845C_REG_VSEL_BUCK_EN_MASK);
	if (rc)
		dev_err(dd->dev, "Unable to enable regualtor rc(%d)", rc);

	dump_registers(dd, MP8845C_REG_VSEL, __func__);

	return rc;

}
static int mp8845c_disable(struct regulator_dev *rdev)
{
	int rc;
	struct mp8845c_info *dd = rdev_get_drvdata(rdev);

	rc = regmap_update_bits(dd->regmap, MP8845C_REG_VSEL,
					MP8845C_REG_VSEL_BUCK_EN_MASK, 0);
	if (rc)
		dev_err(dd->dev, "Unable to disable regualtor rc(%d)", rc);

	dump_registers(dd, MP8845C_REG_VSEL, __func__);

	return rc;

}
static int mp8845c_set_mode(struct regulator_dev *rdev,unsigned int mode)
{
	int rc;
	struct mp8845c_info *dd = rdev_get_drvdata(rdev);

	/* only FAST and NORMAL mode types are supported */
	if (mode != REGULATOR_MODE_FAST && mode != REGULATOR_MODE_NORMAL) {
		dev_err(dd->dev, "Mode %d not supported\n", mode);
		return -EINVAL;
	}

	rc = regmap_update_bits(dd->regmap, MP8845C_REG_SYSCNTLREG1, MP8845C_REG_SYSCNTLREG1_MODE_MASK,
			(mode == REGULATOR_MODE_FAST) ? 1 : 0);	//FAST MODE == PWM MODE
	if (rc) {
		dev_err(dd->dev, "Unable to set operating mode rc(%d)", rc);
		return rc;
	}

	dump_registers(dd, MP8845C_REG_SYSCNTLREG1, __func__);

	return rc;

}

static unsigned int mp8845c_get_mode(struct regulator_dev *rdev)
{
	unsigned int val;
	int rc;
	struct mp8845c_info *dd = rdev_get_drvdata(rdev);

	rc = regmap_read(dd->regmap, MP8845C_REG_SYSCNTLREG1, &val);
	if (rc) {
		dev_err(dd->dev, "Unable to get regulator mode rc(%d)\n", rc);
		return rc;
	}

	dump_registers(dd, MP8845C_REG_SYSCNTLREG1, __func__);

	if (val & MP8845C_REG_SYSCNTLREG1_MODE_MASK)
		return REGULATOR_MODE_FAST;

	return REGULATOR_MODE_NORMAL;
}

static struct regulator_ops mp8845c_ops = {
	.set_voltage = mp8845c_set_voltage,
	.get_voltage = mp8845c_get_voltage,
	.enable = mp8845c_enable,
	.disable = mp8845c_disable,
	.set_mode = mp8845c_set_mode,
	.get_mode = mp8845c_get_mode,
};

static struct regulator_desc rdesc = {
	.name = "mp8845c",
	.owner = THIS_MODULE,
	.n_voltages = 128,
	.ops = &mp8845c_ops,
};

static int __devinit mp8845c_regulator_probe(struct i2c_client *client,
					const struct i2c_device_id *id)
{	
	int rc;
	unsigned int val = 0;
	struct mp8845c_info *dd;
	struct mp8845c_platform_data *pdata;

	pdata = client->dev.platform_data;
	if (!pdata) {
		dev_err(&client->dev, "Platform data not specified\n");
		return -EINVAL;
	}

	dd = devm_kzalloc(&client->dev, sizeof(*dd), GFP_KERNEL);
	if (!dd) {
		dev_err(&client->dev, "Unable to allocate memory\n");
		return -ENOMEM;
	}

	dd->regmap = devm_regmap_init_i2c(client, &mp8845c_regmap_config);
	if (IS_ERR(dd->regmap)) {
		dev_err(&client->dev, "Error allocating regmap\n");
		return PTR_ERR(dd->regmap);
	}

	rc = regmap_read(dd->regmap, MP8845C_REG_ID1, &val);
	if (rc) {
		dev_err(&client->dev, "Unable to identify MP8845C, rc(%d)\n",rc);
		return rc;
	}
	dev_info(&client->dev, "Detected Regulator MP8845C ID1 = %d\n", val);
	
	dd->init_data = pdata->init_data;
	dd->init_data->constraints.min_uV = pdata->min_uV;
	dd->init_data->constraints.max_uV = pdata->max_uV;	
	dd->step_uV = pdata->step_uV;
	
	dd->dev = &client->dev;
	i2c_set_clientdata(client, dd);

	rc = mp8845c_init(dd, pdata);
	if (rc) {
		dev_err(&client->dev, "Unable to intialize the regulator\n");
		return -EINVAL;
	}
	
	dd->regulator = regulator_register(&rdesc, &client->dev,
					dd->init_data, dd, NULL);
	if (IS_ERR(dd->regulator)) {
		dev_err(&client->dev, "Unable to register regulator rc(%ld)",
						PTR_ERR(dd->regulator));
		return PTR_ERR(dd->regulator);
	}
	return 0;
}

static int __devexit mp8845c_regulator_remove(struct i2c_client *client)
{

	struct mp8845c_info *dd = i2c_get_clientdata(client);

	regulator_unregister(dd->regulator);

	return 0;
}

static const struct i2c_device_id mp8845c_id[] = {
	{"mp8845c", -1},
	{ },
};

static struct i2c_driver mp8845c_regulator_driver = {
	.driver = {
		.name = "mp8845c-regulator",
	},
	.probe = mp8845c_regulator_probe,
	.remove = __devexit_p(mp8845c_regulator_remove),
	.id_table = mp8845c_id,
};
extern bool vdd_cx_buck_detected;

static inline int mps8845c_read(struct i2c_client* client, u8 reg)
{
	return i2c_smbus_read_byte_data(client,	reg);
}

static int __devinit mp8845c_pdev_probe(struct platform_device *pdev)
{
	int rc = 0;
	unsigned int val = 0;
	struct mp8845c_i2c_platform_data* pdata = NULL;
	struct i2c_adapter *adapter = NULL;
	struct i2c_client *client = NULL;	
	
	if(vdd_cx_buck_detected)
		return -EINVAL;
	
	pdata = (struct mp8845c_i2c_platform_data*)pdev->dev.platform_data;
	if(pdata == NULL)
	{
		dev_err(&pdev->dev, "can't find the platform data\n");
		return -EINVAL;
	}

	adapter = i2c_get_adapter(pdata->i2c_bus_id);
	if (IS_ERR(adapter)) {
		dev_err(&pdev->dev, "can't get i2c adapter for mp8845c\n");
		rc = PTR_ERR(adapter);
		goto probe_failed;		
	}

	client = i2c_new_device(adapter, pdata->i2c_info);
	if (client ==NULL) {
		dev_err(&pdev->dev, "can't create i2c device for mp8845c\n");
		rc = -ENODEV;
		goto probe_failed;
	}
	val = mps8845c_read(client, MP8845C_REG_ID1);
	if(val != 0x11)	//default ID1 = 0x11
	{
		dev_err(&client->dev, "not MP8845C detected, ID1 = 0x%x \n", val);
		goto unreg_i2c_device;
	}
	
	vdd_cx_buck_detected = true;

	rc = i2c_add_driver(&mp8845c_regulator_driver);
	if(rc)
	{
		dev_err(&client->dev, "add mp8845c i2c driver failed\n");
		goto unreg_i2c_device;
	}
	return rc;
	
unreg_i2c_device:
	i2c_unregister_device(client);
probe_failed:
	return rc;
}

static int __devexit mp8845c_pdev_remove(struct platform_device *pdev)
{
	i2c_del_driver(&mp8845c_regulator_driver);
	return 0;
}

static struct platform_driver mp8845c_platform_driver = {
	.probe = mp8845c_pdev_probe,
	.remove = __devexit_p(mp8845c_pdev_remove),
	.driver = {
		.name = "mp8845c_ldo",
		.owner = THIS_MODULE,
	},
};

static int __init mp8845c_regulator_init(void)
{
	return platform_driver_register(&mp8845c_platform_driver);
}

fs_initcall(mp8845c_regulator_init);

static void __exit mp8845c_regulator_exit(void)
{
	return;
}
module_exit(mp8845c_regulator_exit);
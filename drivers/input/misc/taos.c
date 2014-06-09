/*******************************************************************************
*                                                                              *
*   File Name:    taos.c                                                      *
*   Description:   Linux device driver for Taos ambient light and         *
*   proximity sensors.                                     *
*   Author:         John Koshi                                             *
*   History:   09/16/2009 - Initial creation                          *
*           10/09/2009 - Triton version         *
*           12/21/2009 - Probe/remove mode                *
*           02/07/2010 - Add proximity          *
*                                                                              *
********************************************************************************
*    Proprietary to Taos Inc., 1001 Klein Road #300, Plano, TX 75074        *
*******************************************************************************/
// includes
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <asm/uaccess.h>
#include <asm/errno.h>
#include <linux/taos_common.h>
#include <linux/delay.h>
//iVIZM
#include <linux/irq.h> 
#include <linux/interrupt.h> 
#include <linux/slab.h>
#include <linux/wakelock.h>
#include <linux/input.h>
#include <linux/proc_fs.h> // added by yangze for create proc file 20121011
//extern int set_lcd_backlight(int bl);
//extern int bootup_mode;
// device name/id/address/counts
#define TAOS_DEVICE_NAME                "taos"
#define TAOS_DEVICE_ID                  "taos"
#define TAOS_ID_NAME_SIZE               10
#define TAOS_TRITON_CHIPIDVAL           0x00
#define TAOS_TRITON_MAXREGS             32
#define TAOS_DEVICE_ADDR1               0x29
#define TAOS_DEVICE_ADDR2               0x39
#define TAOS_DEVICE_ADDR3               0x49
#define TAOS_MAX_NUM_DEVICES            3
#define TAOS_MAX_DEVICE_REGS            32
#define I2C_MAX_ADAPTERS                8

// TRITON register offsets
#define TAOS_TRITON_CNTRL               0x00
#define TAOS_TRITON_ALS_TIME            0X01
#define TAOS_TRITON_PRX_TIME            0x02
#define TAOS_TRITON_WAIT_TIME           0x03
#define TAOS_TRITON_ALS_MINTHRESHLO     0X04
#define TAOS_TRITON_ALS_MINTHRESHHI     0X05
#define TAOS_TRITON_ALS_MAXTHRESHLO     0X06
#define TAOS_TRITON_ALS_MAXTHRESHHI     0X07
#define TAOS_TRITON_PRX_MINTHRESHLO     0X08
#define TAOS_TRITON_PRX_MINTHRESHHI     0X09
#define TAOS_TRITON_PRX_MAXTHRESHLO     0X0A
#define TAOS_TRITON_PRX_MAXTHRESHHI     0X0B
#define TAOS_TRITON_INTERRUPT           0x0C
#define TAOS_TRITON_PRX_CFG             0x0D
#define TAOS_TRITON_PRX_COUNT           0x0E
#define TAOS_TRITON_GAIN                0x0F
#define TAOS_TRITON_REVID               0x11
#define TAOS_TRITON_CHIPID              0x12
#define TAOS_TRITON_STATUS              0x13
#define TAOS_TRITON_ALS_CHAN0LO         0x14
#define TAOS_TRITON_ALS_CHAN0HI         0x15
#define TAOS_TRITON_ALS_CHAN1LO         0x16
#define TAOS_TRITON_ALS_CHAN1HI         0x17
#define TAOS_TRITON_PRX_LO              0x18
#define TAOS_TRITON_PRX_HI              0x19
#define TAOS_TRITON_TEST_STATUS         0x1F

// Triton cmd reg masks
#define TAOS_TRITON_CMD_REG             0X80
#define TAOS_TRITON_CMD_AUTO            0x10 //iVIZM
#define TAOS_TRITON_CMD_BYTE_RW         0x00
#define TAOS_TRITON_CMD_WORD_BLK_RW     0x20
#define TAOS_TRITON_CMD_SPL_FN          0x60
#define TAOS_TRITON_CMD_PROX_INTCLR     0X05
#define TAOS_TRITON_CMD_ALS_INTCLR      0X06
#define TAOS_TRITON_CMD_PROXALS_INTCLR  0X07
#define TAOS_TRITON_CMD_TST_REG         0X08
#define TAOS_TRITON_CMD_USER_REG        0X09

// Triton cntrl reg masks
#define TAOS_TRITON_CNTL_PROX_INT_ENBL  0X20
#define TAOS_TRITON_CNTL_ALS_INT_ENBL   0X10
#define TAOS_TRITON_CNTL_WAIT_TMR_ENBL  0X08
#define TAOS_TRITON_CNTL_PROX_DET_ENBL  0X04
#define TAOS_TRITON_CNTL_ADC_ENBL       0x02
#define TAOS_TRITON_CNTL_PWRON          0x01

// Triton status reg masks
#define TAOS_TRITON_STATUS_ADCVALID     0x01
#define TAOS_TRITON_STATUS_PRXVALID     0x02
#define TAOS_TRITON_STATUS_ADCINTR      0x10
#define TAOS_TRITON_STATUS_PRXINTR      0x20

// lux constants
#define TAOS_MAX_LUX                    10000
#define TAOS_SCALE_MILLILUX             3
#define TAOS_FILTER_DEPTH               3
#define CHIP_ID                         0x3d


// forward declarations
static int taos_probe(struct i2c_client *clientp, const struct i2c_device_id *idp);
static int taos_remove(struct i2c_client *client);

static int taos_open(struct inode *inode, struct file *file);
static int taos_release(struct inode *inode, struct file *file);
static long taos_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static int taos_get_lux(void);
static int taos_prox_poll(struct taos_prox_info *prxp);
//iVIZM
static int taos_als_threshold_set(void);
static int taos_prox_threshold_set(void);
static int taos_als_get_data(void);
static int taos_interrupts_clear(void);
static int taos_resume(struct i2c_client *client);
static int taos_suspend(struct i2c_client *client,pm_message_t mesg);


/*
DECLARE_WAIT_QUEUE_HEAD(waitqueue_read);//iVIZM

#define ALS_PROX_DEBUG //iVIZM
static unsigned int ReadEnable = 0;//iVIZM
struct ReadData { //iVIZM
    unsigned int data;
    unsigned int interrupt;
};
struct ReadData readdata[2];//iVIZM
*/


// first device number
static dev_t taos_dev_number;


// class structure for this device
static struct class *taos_class; 

// module device table
static struct i2c_device_id taos_idtable[] = {
    {TAOS_DEVICE_ID, 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, taos_idtable);


// client and device
//iVIZM
static char pro_buf[4]; //iVIZM
static char als_buf[4]; //iVIZM
static int ALS_ON;

// driver definition
static struct i2c_driver taos_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = TAOS_DEVICE_NAME,
    },
    .id_table = taos_idtable,
    .probe = taos_probe,
    .resume = taos_resume,//iVIZM
    .suspend = taos_suspend,//iVIZM
    .remove = __devexit_p(taos_remove),
};

// per-device data
struct taos_data {
    struct i2c_client *client;
    struct cdev cdev;
    struct input_dev *input_dev;//iVIZM
    struct work_struct work;//iVIZM
    struct wake_lock taos_wake_lock;//iVIZM
    char taos_name[TAOS_ID_NAME_SIZE];
//    int working;
    int open_num;
} *taos_datap;

// file operations
static struct file_operations taos_fops = {
    .owner = THIS_MODULE,
    .open = taos_open,
    .release = taos_release,
    .unlocked_ioctl = taos_unlocked_ioctl,
};

// device configuration
struct taos_cfg *taos_cfgp;
static u32 calibrate_target_param = 300000;
static u16 als_time_param = 200;        //0.2*3 = 0.6
static u16 scale_factor_param = 1;
static u16 gain_trim_param = 512;
static u8 filter_history_param = 3;
static u8 filter_count_param = 3;             //0x0c
static u8 gain_param = 1;   //modified by fanjiankang for gain
static u16 prox_threshold_hi_param = 0x230;
static u16 prox_threshold_lo_param = 0x218;
static u16 als_threshold_hi_param = 3000;//iVIZM
static u16 als_threshold_lo_param = 10;//iVIZM
static u8 prox_int_time_param = 0xEE;//50ms
static u8 prox_adc_time_param = 0xFF;
static u8 prox_wait_time_param = 0xEE;
static u8 prox_intr_filter_param = 0x23;
static u8 prox_config_param = 0x00;
static u8 prox_pulse_cnt_param = 0x04; 
static u8 prox_gain_param = 0x61;

// prox info
struct taos_prox_info prox_cal_info[20];
struct taos_prox_info prox_cur_info;
struct taos_prox_info *prox_cur_infop = &prox_cur_info;
static int prox_on = 0;
static u16 sat_als = 0;
static u16 sat_prox = 0;

// device reg init values
u8 taos_triton_reg_init[16] = {0x00,0xFF,0XFF,0XFF,0X00,0X00,0XFF,0XFF,0X00,0X00,0XFF,0XFF,0X00,0X00,0X00,0X00};

// lux time scale
struct time_scale_factor  {
    u16 numerator;
    u16 denominator;
    u16 saturation;
};
struct time_scale_factor TritonTime = {1, 0, 0};
struct time_scale_factor *lux_timep = &TritonTime;

// gain table
u8 taos_triton_gain_table[] = {1, 8, 16, 120};

// lux data
struct lux_data {
    u16 ratio;  //红外线比上全部域的光线
    u16 clear; //红外线比上全部域的光线
    u16 ir;        //光线中的红外线

};
struct lux_data TritonFN_lux_data[] = {
    { 9830,  8320,  15360 },
    { 12452, 10554, 22797 },
    { 14746, 6234,  11430 },
    { 17695, 3968,  6400  },
    { 0,     0,     0     }
};
struct lux_data *lux_tablep = TritonFN_lux_data;
static int lux_history[TAOS_FILTER_DEPTH] = {-ENODATA, -ENODATA, -ENODATA};//iVIZM
static int chip_id = 0xff;

static irqreturn_t taos_irq_handler(int irq, void *dev_id) //iVIZM
{
    schedule_work(&taos_datap->work);

    return IRQ_HANDLED;
}

static int taos_get_data(void)//iVIZM
{
    int ret = 0;
    int status; 
    int loop =10;
		while(loop){
	    if ((ret = (i2c_smbus_write_byte(taos_datap->client, (TAOS_TRITON_CMD_REG | 0x13)))) < 0) {
				mdelay(5);
        loop--;
        printk(KERN_ERR "TAOS: failed in taos_get_data(), try again number is %d\n",loop);
	    }else{
	    	loop = 0;
	    }
    }
    status = i2c_smbus_read_byte(taos_datap->client);
    if((status & 0x20) == 0x20) {
        ret = taos_prox_threshold_set();
    } else if((status & 0x10) == 0x10) {
        taos_als_threshold_set();
        taos_als_get_data();
    }
    return ret;
}

static int taos_interrupts_clear(void)//iVIZM
{
    int ret = 0;
    int loop =10;
    while(loop){
	    if ((ret = (i2c_smbus_write_byte(taos_datap->client, (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x07)))) < 0) {
        mdelay(5);
        loop--;
        printk(KERN_ERR "TAOS: failed in taos_interrupts_clear(), try again number is %d\n",loop);
	    }else{
	    	loop = 0;
	    }
    }
    return ret;
}
static void taos_work_func(struct work_struct * work) //iVIZM
{
    wake_lock(&taos_datap->taos_wake_lock);
    taos_get_data();
    taos_interrupts_clear();
    wake_lock_timeout(&taos_datap->taos_wake_lock, HZ/2);
}
static int taos_als_get_data(void)//iVIZM
{
    int ret = 0;
    u8 reg_val;
    int lux_val = 0;

    if ((ret = (i2c_smbus_write_byte(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL)))) < 0) {
        printk(KERN_ERR "TAOS: i2c_smbus_write_byte failed in ioctl als_data\n");
        return (ret);
    }
    reg_val = i2c_smbus_read_byte(taos_datap->client);
    if (((reg_val & (TAOS_TRITON_CNTL_ADC_ENBL ))| TAOS_TRITON_CNTL_PWRON) != (TAOS_TRITON_CNTL_ADC_ENBL | TAOS_TRITON_CNTL_PWRON))
    {
	return -ENODATA;
    }
    if ((ret = (i2c_smbus_write_byte(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_STATUS)))) < 0) {
        printk(KERN_ERR "TAOS: i2c_smbus_write_byte failed in ioctl als_data\n");
        return (ret);
    }
    reg_val = i2c_smbus_read_byte(taos_datap->client);
    if ((reg_val & TAOS_TRITON_STATUS_ADCVALID) != TAOS_TRITON_STATUS_ADCVALID)
    {
	return -ENODATA;
    }
    if ((lux_val = taos_get_lux()) < 0)
        printk(KERN_ERR "TAOS: call to taos_get_lux() returned error %d in ioctl als_data\n", lux_val);
	input_report_abs(taos_datap->input_dev,ABS_MISC,lux_val);
	input_sync(taos_datap->input_dev);
    return ret;
}

static int taos_als_threshold_set(void)//iVIZM
{
    int i,ret = 0;
    u8 chdata[2];
    u16 ch0;
    int mcount; 

    for (i = 0; i < 2; i++) {
        chdata[i] = (i2c_smbus_read_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_CMD_WORD_BLK_RW | (TAOS_TRITON_ALS_CHAN0LO + i))));
    }
    ch0 = chdata[0] + chdata[1]*256;
    als_threshold_hi_param = (12*ch0)/10;
	if (als_threshold_hi_param >= 65535)
        als_threshold_hi_param = 65535;	 
    als_threshold_lo_param = (8*ch0)/10;
    als_buf[0] = als_threshold_lo_param & 0x0ff;
    als_buf[1] = als_threshold_lo_param >> 8;
    als_buf[2] = als_threshold_hi_param & 0x0ff;
    als_buf[3] = als_threshold_hi_param >> 8;

    for( mcount=0; mcount<4; mcount++ ) { 
        if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x04) + mcount, als_buf[mcount]))) < 0) {
             printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in taos als threshold set\n");
             return (ret);
        }
    }
    return ret;
}

static int taos_prox_threshold_set(void)//iVIZM
{
	int i,ret = 0;
	u8 chdata[6];
	u16 proxdata = 0;
	u16 cleardata = 0;
	int data = 0;
	int mcount; 

	for (i = 0; i < 6; i++) {
		chdata[i] = (i2c_smbus_read_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_CMD_WORD_BLK_RW| (TAOS_TRITON_ALS_CHAN0LO + i))));
	}
	cleardata = chdata[0] + chdata[1]*256;
	proxdata = chdata[4] + chdata[5]*256;

	if (prox_on || proxdata < taos_cfgp->prox_threshold_lo ) {
		pro_buf[0] = 0x0;
		pro_buf[1] = 0x0;
		pro_buf[2] = taos_cfgp->prox_threshold_hi & 0x0ff;
		pro_buf[3] = taos_cfgp->prox_threshold_hi >> 8;
		data = 0;
		input_report_abs(taos_datap->input_dev,ABS_DISTANCE,data);
	} else if (proxdata > taos_cfgp->prox_threshold_hi ){
		if (cleardata > ((sat_als*80)/100))
			return -ENODATA;
		pro_buf[0] = taos_cfgp->prox_threshold_lo & 0x0ff;
		pro_buf[1] = taos_cfgp->prox_threshold_lo >> 8;
		pro_buf[2] = 0xff;
		pro_buf[3] = 0xff;
		data = 1;
		input_report_abs(taos_datap->input_dev,ABS_DISTANCE,data);
    }
    input_sync(taos_datap->input_dev);

	printk("TAOS: prox_threshold_lo = 0x%x, prox_threashold_hi = 0x%x, proxdata = 0x%x，data = %d\n", taos_cfgp->prox_threshold_lo, taos_cfgp->prox_threshold_hi, proxdata,(data?1:5));
	
    for( mcount=0; mcount<4; mcount++ ) { 
        if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x08) + mcount, pro_buf[mcount]))) < 0) {
             printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in taos prox threshold set\n");
             return (ret);
        }
    }
	
    prox_on = 0;
    return ret;
}

static int sensor_i2c_read(struct i2c_adapter *i2c_adap,
                           unsigned char address, unsigned char reg,
                           unsigned int len, unsigned char *data)
{
        struct i2c_msg msgs[2];

        if (!data || !i2c_adap) {
		dev_err(&i2c_adap->dev, "TAOS: product test sensor read invalid parameter\n");
                return -EINVAL;
        }   

        msgs[0].addr = address;
        msgs[0].flags = 0;      /* write */
        msgs[0].buf = &reg;
        msgs[0].len = 1;

        msgs[1].addr = address;
        msgs[1].flags = I2C_M_RD;
        msgs[1].buf = data;
        msgs[1].len = len;

        return i2c_transfer(i2c_adap, msgs, 2); 
}

#define MPU_I2C_SLAVE_ADDR			0x68
#define MPUREG_WHO_AM_I				0x0

#define L3G4200D_GYR_I2C_SLAVE_ADDR		0x68
#define L3G4200D_GYR_WHO_AM_I			0x0F
#define L3G4200D_ID				0xD3

#define MMA845X_I2C_SLAVE_ADDR                  0x1C
#define MMA845X_WHO_AM_I                        0x0D
#define MMA845X_ID                              0x2A

#define KXTF9_I2C_SLAVE_ADDR                    0x0F
#define KXTF9_WHO_AM_I                          0x0F
#define KXTF9_ID                                0x05

#define LIS3DH_I2C_SLAVE_ADDR                   0x18
#define LIS3DH_I2C_SLAVE_ADDR1                  0x19
#define LIS3DH_WHO_AM_I                         0x0F
#define LIS3DH_ID                               0x33

#define AK8962_I2C_SLAVE_ADDR                   0x0C
#define AK8962_I2C_SLAVE_ADDR_A10               0x0E
#define AK8962_WHO_AM_I                         0x0
#define AK8962_ID                               0x48

#define TAOS_I2C_SLAVE_ADDR          0x39
#define TAOS_WHO_AM_I                    0x12

#if defined(CONFIG_PROJECT_P825B20)||defined(CONFIG_PROJECT_P825A20)
#define TAOS_I2C_ADAPTER_NUM 1
#define COMPASS_I2C_ADAPTER_NUM 0
#define ACCEL_I2C_ADAPTER_NUM 0
#define GYRO_I2C_ADAPTER_NUM 0
#else
#define TAOS_I2C_ADAPTER_NUM 1
#define GYRO_I2C_ADAPTER_NUM 1
#define COMPASS_I2C_ADAPTER_NUM 1
#define ACCEL_I2C_ADAPTER_NUM 1
#endif

static ssize_t taos_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        unsigned char taos_id = 0xFF;
	 union i2c_smbus_data data;
        int result = -1;
       struct i2c_adapter *taos_adapter = i2c_get_adapter(TAOS_I2C_ADAPTER_NUM);

	if((result = i2c_smbus_xfer(taos_adapter, TAOS_I2C_SLAVE_ADDR, 0,
	                      I2C_SMBUS_WRITE, (0x80|TAOS_WHO_AM_I), I2C_SMBUS_BYTE, NULL))<0)
	       taos_id = 0xff;

	if((result = i2c_smbus_xfer(taos_adapter, TAOS_I2C_SLAVE_ADDR,0,
				I2C_SMBUS_READ, 0,
				I2C_SMBUS_BYTE, &data))<0){
		taos_id = 0xff;
	}else{
		taos_id = data.byte;
	}
	i2c_put_adapter(taos_adapter);

	return sprintf(buf, "0x%x\n", taos_id & 0xff);
}

static ssize_t gyro_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        unsigned char mpu_id = 0xFF;
        int result = -1;
        struct i2c_adapter *mpu_adapter = i2c_get_adapter(GYRO_I2C_ADAPTER_NUM);
	result = sensor_i2c_read(mpu_adapter, L3G4200D_GYR_I2C_SLAVE_ADDR, L3G4200D_GYR_WHO_AM_I, 1, &mpu_id);
  if (result != 2)
	result = sensor_i2c_read(mpu_adapter, MPU_I2C_SLAVE_ADDR, MPUREG_WHO_AM_I, 1, &mpu_id);
	
	i2c_put_adapter(mpu_adapter);

        if (result != 2) {
                return sprintf(buf, "0x%x\n", 0xFF);
        } else {
                return sprintf(buf, "0x%x\n", mpu_id & 0xff);
        }
}

static ssize_t accel_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        unsigned char accel_id = 0xFF;
        int result = -1;

        struct i2c_adapter *accel_adapter = i2c_get_adapter(ACCEL_I2C_ADAPTER_NUM);
	result = sensor_i2c_read(accel_adapter, LIS3DH_I2C_SLAVE_ADDR, LIS3DH_WHO_AM_I, 1, &accel_id);
	if (result != 2)
	result = sensor_i2c_read(accel_adapter, LIS3DH_I2C_SLAVE_ADDR1, LIS3DH_WHO_AM_I, 1, &accel_id);
	if (result != 2)
	result = sensor_i2c_read(accel_adapter, KXTF9_I2C_SLAVE_ADDR, KXTF9_WHO_AM_I, 1, &accel_id);
	if (result != 2)
	result = sensor_i2c_read(accel_adapter, MMA845X_I2C_SLAVE_ADDR, MMA845X_WHO_AM_I, 1, &accel_id);

	i2c_put_adapter(accel_adapter);
	if (result != 2) {
		return sprintf(buf, "0x%x\n", 0xFF);
	} else {
		return sprintf(buf, "0x%x\n", accel_id & 0xff);
	}
}

static ssize_t compass_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        unsigned char compass_id = 0xFF;
        int result = -1;

  struct i2c_adapter *compass_adapter = i2c_get_adapter(COMPASS_I2C_ADAPTER_NUM);
	result = sensor_i2c_read(compass_adapter, AK8962_I2C_SLAVE_ADDR_A10, AK8962_WHO_AM_I, 1, &compass_id);
	if (result != 2)
	result = sensor_i2c_read(compass_adapter, AK8962_I2C_SLAVE_ADDR, AK8962_WHO_AM_I, 1, &compass_id);

        i2c_put_adapter(compass_adapter);
        if (result != 2) {
                return sprintf(buf, "0x%x\n", 0xFF);
        } else {
                return sprintf(buf, "0x%x\n", compass_id & 0xff);
        }
}

static ssize_t proxdata_show(struct device *dev, struct device_attribute *attr, char *buf){
    u16 proxdata = 0x0000;
	u16 cleardata;
	u8 data[6];
	int i = 0;

	for (i = 0; i < 6; i++) {
		data[i] = (i2c_smbus_read_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_CMD_WORD_BLK_RW| (TAOS_TRITON_ALS_CHAN0LO + i))));
	}
	cleardata = data[0] + data[1]*256;
	proxdata = data[4] + data[5]*256;
	
	if (cleardata > ((sat_als*80)/100))
		return sprintf(buf, "0x%x\n", 0xffff);

	return sprintf(buf, "0x%x\n", proxdata & 0xffff);
}

static DEVICE_ATTR(gyro, S_IRUGO, gyro_id_show, NULL);
static DEVICE_ATTR(accel, S_IRUGO, accel_id_show, NULL);
static DEVICE_ATTR(compass, S_IRUGO, compass_id_show, NULL);
static DEVICE_ATTR(taos, S_IRUGO, taos_id_show, NULL);
static DEVICE_ATTR(proxdata, S_IRUGO, proxdata_show, NULL);

static struct attribute *sensor_attr[] = {
        &dev_attr_gyro.attr,
        &dev_attr_accel.attr,
        &dev_attr_compass.attr,
        &dev_attr_taos.attr,
        &dev_attr_proxdata.attr,
        NULL
};

static struct attribute_group sensor_dev_attr_grp = {
        .attrs = sensor_attr,
};

//[ECID 000000] yangze add for ic information add 20121121 begin
static ssize_t taos_info_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	//int len = strlen(wlan_info);
	return sprintf(page, "0x%x\n", chip_id);
	//return len + 1;
}
static struct proc_dir_entry *taos_info_proc_file;
static void create_taos_info_proc_file(void)
{
  taos_info_proc_file = create_proc_entry("driver/taos", 0644, NULL);
  //printk("goes to create_acc_info_proc_file\n");
  if (taos_info_proc_file) {
			taos_info_proc_file->read_proc = taos_info_read_proc;
   } else{
	printk(KERN_INFO "proc file create failed!\n");
   }
}

static void remove_taos_info_proc_file(void)
{
	//printk("goes to remove_acc_info_proc_file\n");
	if(taos_info_proc_file){
		remove_proc_entry("driver/taos", NULL);
		taos_info_proc_file = NULL;
	}
}
//[ECID 000000] yangze add for ic information add 20121121 end
// driver init
static int __init taos_init(void) {
    int ret = 0;

    if ((ret = (i2c_add_driver(&taos_driver))) < 0) {
        printk(KERN_ERR "TAOS: i2c_add_driver() failed in taos_init()\n");
    }
    if(0xff != chip_id){
			create_taos_info_proc_file();
		}
		
    return (ret);
}

// driver exit
static void __exit taos_exit(void) {

    free_irq(taos_datap->client->irq, NULL);
    kfree(taos_cfgp);
    input_unregister_device(taos_datap->input_dev);
    device_destroy(taos_class, MKDEV(MAJOR(taos_dev_number), 0));
    class_destroy(taos_class);
    cdev_del(&taos_datap->cdev);
    kfree(taos_datap);
    unregister_chrdev_region(taos_dev_number, TAOS_MAX_NUM_DEVICES);
    sysfs_remove_group(&taos_datap->client->dev.kobj, &sensor_dev_attr_grp);
    i2c_del_driver(&taos_driver);
    if(0xff != chip_id){
    	remove_taos_info_proc_file();
    }

}

// client probe
static int taos_probe(struct i2c_client *clientp, const struct i2c_device_id *idp) {
    int ret = 0;

    printk( "TAOS:  taos_probe begin\n");
	//{
	//	pr_debug("wangminrong lcd is off for charger\r\n");
//		set_lcd_backlight(0);
//	}
  
    ret = sysfs_create_group(&clientp->dev.kobj, &sensor_dev_attr_grp);
    if (ret) {
	    dev_err(&clientp->adapter->dev, "TAOS:sensor create sys file failed\n");
    }    

    if ((ret = (i2c_smbus_write_byte(clientp, (TAOS_TRITON_CMD_REG | (TAOS_TRITON_CNTRL + TAOS_TRITON_CHIPID))))) < 0) {
	    printk(KERN_ERR "TAOS: i2c_smbus_write_byte() to chipid reg failed in taos_probe\n");
	    return -ENODATA;
    }

    chip_id = i2c_smbus_read_byte(clientp);
    printk("taos chip id is:0x%x\n",chip_id);
    if((chip_id != 0x20) && (chip_id != 0x29) && (chip_id != 0x39)) {
	    printk(" error chip_id = %d\n",chip_id);
	    return -ENODEV;
    }

    if ((ret = (alloc_chrdev_region(&taos_dev_number, 0, TAOS_MAX_NUM_DEVICES, TAOS_DEVICE_NAME))) < 0) {
        printk(KERN_ERR "TAOS: alloc_chrdev_region() failed in taos_init()\n");
        return (ret);
    }
    taos_datap = kmalloc(sizeof(struct taos_data), GFP_KERNEL);
    if (!taos_datap) {
        printk(KERN_ERR "TAOS: kmalloc for struct taos_data failed in taos_init()\n");

	ret = -ENOMEM;
	goto malloc1_fail;
        //return -ENOMEM;

    }
    memset(taos_datap, 0, sizeof(struct taos_data));
    cdev_init(&taos_datap->cdev, &taos_fops);
    taos_datap->cdev.owner = THIS_MODULE;
    if ((ret = (cdev_add(&taos_datap->cdev, taos_dev_number, 1))) < 0) {
        printk(KERN_ERR "TAOS: cdev_add() failed in taos_init()\n");

	goto cdev_add_fail;
        //return (ret);

    }

    taos_class = class_create(THIS_MODULE, TAOS_DEVICE_NAME);
    device_create(taos_class, NULL, MKDEV(MAJOR(taos_dev_number), 0), &taos_driver ,"light_sensor");
    wake_lock_init(&taos_datap->taos_wake_lock, WAKE_LOCK_SUSPEND, "taos-wake-lock");


    if (!i2c_check_functionality(clientp->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        printk(KERN_ERR "TAOS: taos_probe() - i2c smbus byte data functions unsupported\n");

	ret = -EOPNOTSUPP;
	goto i2c_func_fail;
        //return -EOPNOTSUPP;

    }
    if (!i2c_check_functionality(clientp->adapter, I2C_FUNC_SMBUS_WORD_DATA)) {
        printk(KERN_ERR "TAOS: taos_probe() - i2c smbus word data functions unsupported\n");
    }
    if (!i2c_check_functionality(clientp->adapter, I2C_FUNC_SMBUS_BLOCK_DATA)) {
        printk(KERN_ERR "TAOS: taos_probe() - i2c smbus block data functions unsupported\n");
    }
    taos_datap->client = clientp;
    i2c_set_clientdata(clientp, taos_datap);
    INIT_WORK(&(taos_datap->work),taos_work_func);

   taos_datap->input_dev = input_allocate_device();//iVIZM
   if (taos_datap->input_dev == NULL) {

	ret = -EBUSY;
	goto malloc2_fail;
       //return -ENOMEM;

   }
   
   taos_datap->input_dev->name = "light_sensor";
   taos_datap->input_dev->id.bustype = BUS_I2C;


   set_bit(EV_ABS,taos_datap->input_dev->evbit);
/*
   input_set_capability(taos_datap->input_dev,EV_ABS,ABS_DISTANCE);
   input_set_capability(taos_datap->input_dev,EV_ABS,ABS_MISC);
   ret = input_register_device(taos_datap->input_dev);
*/
    input_set_abs_params(taos_datap->input_dev, ABS_DISTANCE, 0, 100, 0, 0);
    input_set_abs_params(taos_datap->input_dev, ABS_MISC, 0, 20000, 0, 0);
    if ((ret = input_register_device(taos_datap->input_dev))) {
	goto input_register_fail;
    }





    strlcpy(clientp->name, TAOS_DEVICE_ID, I2C_NAME_SIZE);
    strlcpy(taos_datap->taos_name, TAOS_DEVICE_ID, TAOS_ID_NAME_SIZE);

    /*
    taos_datap->valid = 0; 
    */
    taos_datap->open_num = 0; 

    if (!(taos_cfgp = kmalloc(sizeof(struct taos_cfg), GFP_KERNEL))) {
        printk(KERN_ERR "TAOS: kmalloc for struct taos_cfg failed in taos_probe()\n");

	ret = -ENOMEM;
	goto malloc3_fail;
       //return -ENOMEM;

    }
    taos_cfgp->calibrate_target = calibrate_target_param;
    taos_cfgp->als_time = als_time_param;
    taos_cfgp->scale_factor = scale_factor_param;
    taos_cfgp->gain_trim = gain_trim_param;
    taos_cfgp->filter_history = filter_history_param;
    taos_cfgp->filter_count = filter_count_param;
    taos_cfgp->gain = gain_param;
    taos_cfgp->als_threshold_hi = als_threshold_hi_param;//iVIZM
    taos_cfgp->als_threshold_lo = als_threshold_lo_param;//iVIZM
    taos_cfgp->prox_threshold_hi = prox_threshold_hi_param;
    taos_cfgp->prox_threshold_lo = prox_threshold_lo_param;
    taos_cfgp->prox_int_time = prox_int_time_param;
    taos_cfgp->prox_adc_time = prox_adc_time_param;
    taos_cfgp->prox_wait_time = prox_wait_time_param;
    taos_cfgp->prox_intr_filter = prox_intr_filter_param;
    taos_cfgp->prox_config = prox_config_param;
    if(0x39 == chip_id){
	taos_cfgp->prox_pulse_cnt = 0x10;//modified by yangze for TMD27723
    }else{
    	taos_cfgp->prox_pulse_cnt = prox_pulse_cnt_param;
    }
    taos_cfgp->prox_gain = prox_gain_param;
    sat_als = (256 - taos_cfgp->prox_int_time) << 10;
    sat_prox = (256 - taos_cfgp->prox_adc_time) << 10;

    /*dmobile ::power down for init ,Rambo liu*/
    printk("Rambo::light sensor will pwr down \n");
    if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x00), 0x00))) < 0) {
        printk(KERN_ERR "TAOS:Rambo, i2c_smbus_write_byte_data failed in power down\n");

	goto smbus_fail;
        //return (ret);

    }


     ret =  request_threaded_irq( clientp->irq, NULL, taos_irq_handler, 
                                  IRQ_TYPE_EDGE_FALLING, "tms2771_irq", taos_datap);
	if (ret) {
		printk("%s %s fail to request irq\n", __FILE__, __func__);
	goto req_irq_fail;
	}
   return (ret);


req_irq_fail:
smbus_fail:
   kfree(taos_cfgp);
malloc3_fail:
   input_unregister_device(taos_datap->input_dev);
   goto malloc2_fail;
input_register_fail:
   input_free_device(taos_datap->input_dev);
malloc2_fail:
i2c_func_fail:
   device_destroy(taos_class, MKDEV(MAJOR(taos_dev_number), 0));

   class_destroy(taos_class);

   cdev_del(&taos_datap->cdev);
cdev_add_fail:
   kfree(taos_datap);
malloc1_fail:
   unregister_chrdev_region(taos_dev_number, TAOS_MAX_NUM_DEVICES);
   return ret;


}
//resume  iVIZM
static int taos_resume(struct i2c_client *client) {
    return 0;
}

//suspend  iVIZM
static int taos_suspend(struct i2c_client *client, pm_message_t mesg) {
    int ret = 0;

    return ret;
}
// client remove
static int __devexit taos_remove(struct i2c_client *client) {
    int ret = 0;

    return (ret);
}
// open
static int taos_open(struct inode *inode, struct file *file) {
    int ret = 0;


    taos_datap->open_num += 1;
    printk("TAOS:  open number %d\n", taos_datap->open_num);
    if (taos_datap->open_num == 1) {
	    enable_irq_wake(taos_datap->client->irq);
    }

    return (ret);
}

// release
static int taos_release(struct inode *inode, struct file *file) {
    int ret = 0;


//    device_released = 1;
    prox_on = 0;
  taos_datap->open_num -= 1;
    printk("TAOS:  close number %d\n", taos_datap->open_num);
  if (taos_datap->open_num <= 0) {
	  taos_datap->open_num = 0;
	  disable_irq_wake(taos_datap->client->irq);
  }

    return (ret);
}

static int taos_sensors_als_on(void) {
    int  ret = 0, i = 0;
    u8 itime = 0, reg_val = 0, reg_cntrl = 0;
    //int lux_val = 0, ret = 0, i = 0, tmp = 0;

            for (i = 0; i < TAOS_FILTER_DEPTH; i++)
                lux_history[i] = -ENODATA;
            //taos_als_threshold_set();//iVIZM
            if ((ret = (i2c_smbus_write_byte(taos_datap->client, (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|TAOS_TRITON_CMD_ALS_INTCLR)))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte failed in ioctl als_on\n");
                return (ret);
            }
            itime = (((taos_cfgp->als_time/50) * 18) - 1);
            itime = (~itime);
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_ALS_TIME), itime))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl als_on\n");
                return (ret);
            }
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|TAOS_TRITON_INTERRUPT), taos_cfgp->prox_intr_filter))) < 0) {//golden
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl als_on\n");
                return (ret);
            }
            if ((ret = (i2c_smbus_write_byte(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_GAIN)))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte failed in ioctl als_on\n");
                return (ret);
            }
            reg_val = i2c_smbus_read_byte(taos_datap->client);
            reg_val = reg_val & 0xFC;
            reg_val = reg_val | (taos_cfgp->gain & 0x03);
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_GAIN), reg_val))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl als_on\n");
                return (ret);
            }
            reg_cntrl = (TAOS_TRITON_CNTL_ADC_ENBL | TAOS_TRITON_CNTL_PWRON | TAOS_TRITON_CNTL_ALS_INT_ENBL);
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL), reg_cntrl))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl als_on\n");
                return (ret);
            }
            taos_als_threshold_set();//iVIZM
			return ret;
}	


// ioctls
//static int taos_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) {
static long taos_unlocked_ioctl(struct file *fp, unsigned int cmd, unsigned long arg) {
    struct taos_data *taos_datap;
    //int prox_sum = 0, prox_mean = 0, prox_max = 0;
    int prox_sum = 0, prox_mean = 0;
    int ret = 0, i = 0;
    u8 reg_val = 0, reg_cntrl = 0;
    taos_datap = container_of(fp->f_dentry->d_inode->i_cdev, struct taos_data, cdev);
    switch (cmd) {

        case TAOS_IOCTL_ALS_ON:
		/////////////Read Process 1 /////////		
		reg_val = i2c_smbus_read_byte_data(taos_datap->client, TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL);
		//printk("TAOS: reg_ctl als on 1 = 0x%x\n", reg_val);
		
	    if ((reg_val & TAOS_TRITON_CNTL_PROX_DET_ENBL) == 0x0) {
		    taos_sensors_als_on();
	    }	

	    ALS_ON = 1;
	    return (ret);
            break;
        case TAOS_IOCTL_ALS_OFF:
            for (i = 0; i < TAOS_FILTER_DEPTH; i++)
                lux_history[i] = -ENODATA;
            if ((ret = (i2c_smbus_write_byte(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL)))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte failed in ioctl als_calibrate\n");
                return (ret);
            }
            reg_val = i2c_smbus_read_byte(taos_datap->client);
            if ((reg_val & TAOS_TRITON_CNTL_PROX_DET_ENBL) == 0x0) {
               if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL), 0x00))) < 0) {
                   printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl als_off\n");
		   return (ret);
	       }
	       cancel_work_sync(&taos_datap->work);//golden
            }
	    ALS_ON = 0;
            return (ret);
            break;
        case TAOS_IOCTL_PROX_ON:
            printk("^^^^^^^^^ TAOS IOCTL PROX ON  ^^^^^^^\n");
            prox_on = 1;

            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x01), taos_cfgp->prox_int_time))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x02), taos_cfgp->prox_adc_time))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x03), taos_cfgp->prox_wait_time))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }

            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x0C), taos_cfgp->prox_intr_filter))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x0D), taos_cfgp->prox_config))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x0E), taos_cfgp->prox_pulse_cnt))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x0F), taos_cfgp->prox_gain))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }
            reg_cntrl = TAOS_TRITON_CNTL_PROX_DET_ENBL | TAOS_TRITON_CNTL_PWRON | TAOS_TRITON_CNTL_PROX_INT_ENBL | 
				                                      TAOS_TRITON_CNTL_ADC_ENBL | TAOS_TRITON_CNTL_WAIT_TMR_ENBL  ;
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL), reg_cntrl))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }
            taos_prox_threshold_set();//iVIZM
            break;
        case TAOS_IOCTL_PROX_OFF:
            printk("^^^^^^^^^ TAOS IOCTL PROX OFF  ^^^^^^^\n");
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL), 0x00))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl als_off\n");
                return (ret);
            }
			if (ALS_ON == 1) {
                taos_sensors_als_on();
			} else {
				cancel_work_sync(&taos_datap->work);//golden
			}
            prox_on = 0;
            break;
	
        case TAOS_IOCTL_PROX_CALIBRATE:
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x01), taos_cfgp->prox_int_time))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x02), taos_cfgp->prox_adc_time))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x03), taos_cfgp->prox_wait_time))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }

            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x0D), taos_cfgp->prox_config))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }

            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x0E), taos_cfgp->prox_pulse_cnt))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|0x0F), taos_cfgp->prox_gain))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }
            reg_cntrl = reg_val | (TAOS_TRITON_CNTL_PROX_DET_ENBL | TAOS_TRITON_CNTL_PWRON | TAOS_TRITON_CNTL_ADC_ENBL);
            if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_CNTRL), reg_cntrl))) < 0) {
                printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl prox_on\n");
                return (ret);
            }

            prox_sum = 0;
           // prox_max = 0;
            for (i = 0; i < 20; i++) {
                if ((ret = taos_prox_poll(&prox_cal_info[i])) < 0) {
                    printk(KERN_ERR "TAOS: call to prox_poll failed in ioctl prox_calibrate\n");
                    return (ret);
                }
                prox_sum += prox_cal_info[i].prox_data;
		#if 0
                if (prox_cal_info[i].prox_data > prox_max)
                    prox_max = prox_cal_info[i].prox_data;
		#endif
                mdelay(100);
            }
	     #if 0		
            prox_mean = prox_sum/20;
            taos_cfgp->prox_threshold_hi = ((((prox_max - prox_mean) * 200) + 50)/100) + prox_mean;
            taos_cfgp->prox_threshold_lo = ((((prox_max - prox_mean) * 170) + 50)/100) + prox_mean;
            printk("TAOS: prox_threshold_lo = 0x%x, prox_threashold_hi = 0x%x\n", taos_cfgp->prox_threshold_lo, taos_cfgp->prox_threshold_hi);
            if ((taos_cfgp->prox_threshold_hi >= 0x350) ||(taos_cfgp->prox_threshold_hi == taos_cfgp->prox_threshold_lo)){ //modified by yangze for TMD27723
                printk("TAOS: use default threshold\n");
                taos_cfgp->prox_threshold_hi = prox_threshold_hi_param;
                taos_cfgp->prox_threshold_lo = prox_threshold_lo_param;
            }    
		#else
		prox_mean = prox_sum/19;
		if(prox_mean<=100){
			taos_cfgp->prox_threshold_hi = (prox_mean*22)/10;
			taos_cfgp->prox_threshold_lo = (prox_mean*19)/10;
		}else if((prox_mean>100)&&(prox_mean<=300)){
			taos_cfgp->prox_threshold_hi = (prox_mean*20)/10;
			taos_cfgp->prox_threshold_lo = (prox_mean*17)/10;
		}else{
			taos_cfgp->prox_threshold_hi = (prox_mean*18)/10;
			taos_cfgp->prox_threshold_lo = (prox_mean*15)/10;
		}

            printk("TAOS: prox_threshold_lo = 0x%x, prox_threashold_hi = 0x%x, prox_mean = 0x%x\n", taos_cfgp->prox_threshold_lo, taos_cfgp->prox_threshold_hi, prox_mean);
		if(taos_cfgp->prox_threshold_hi<100){
			taos_cfgp->prox_threshold_hi = 200;
			taos_cfgp->prox_threshold_lo = 150;			
		}else if(taos_cfgp->prox_threshold_hi>900){
			taos_cfgp->prox_threshold_hi = 900;
			taos_cfgp->prox_threshold_lo = 850;
		}	
		#endif
            for (i = 0; i < sizeof(taos_triton_reg_init); i++){
                if(i !=11){
                    if ((ret = (i2c_smbus_write_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG|(TAOS_TRITON_CNTRL +i)), taos_triton_reg_init[i]))) < 0) {
                        printk(KERN_ERR "TAOS: i2c_smbus_write_byte_data failed in ioctl als_on\n");
                        return (ret);
                    }
                 }
             }

            break;
        default:
            return -EINVAL;
            break;
    }
    return (ret);
}
// read/calculate lux value
static int taos_get_lux(void) {
    u16 raw_clear = 0, raw_ir = 0, raw_lux = 0;
    u32 lux = 0;
    u32 ratio = 0;
    u8 dev_gain = 0;
    u16 Tint = 0;
    struct lux_data *p;
    int ret = 0;
    u8 chdata[4];
    int tmp = 0, i = 0;

    for (i = 0; i < 4; i++) {
        if ((ret = (i2c_smbus_write_byte(taos_datap->client, (TAOS_TRITON_CMD_REG | (TAOS_TRITON_ALS_CHAN0LO + i))))) < 0) {
            printk(KERN_ERR "TAOS: i2c_smbus_write_byte() to chan0/1/lo/hi reg failed in taos_get_lux()\n");
            return (ret);
        }
        chdata[i] = i2c_smbus_read_byte(taos_datap->client);
    }
//    printk("ch0=%d\n",chdata[0]+chdata[1]*256);
//    printk("ch1=%d\n",chdata[2]+chdata[3]*256);

    tmp = (taos_cfgp->als_time + 25)/50;            //if atime =100  tmp = (atime+25)/50=2.5   tine = 2.7*(256-atime)=  412.5
    TritonTime.numerator = 1;
    TritonTime.denominator = tmp;

    tmp = 300 * taos_cfgp->als_time;               //tmp = 300*atime  400
    if(tmp > 65535)
        tmp = 65535;
    TritonTime.saturation = tmp;
    raw_clear = chdata[1];
    raw_clear <<= 8;
    raw_clear |= chdata[0];
    raw_ir    = chdata[3];
    raw_ir    <<= 8;
    raw_ir    |= chdata[2];
    //added by yangze for dark confition als is 10000 start
    if(raw_clear<=10){
	printk("ch0=%d, ch1=%d, lux=0\n",raw_clear, raw_ir);
	return 0;  // raw_clear is so small , return lux is 0
    }
    //added by yangze for dark confition als is 10000 end
    raw_clear *= (taos_cfgp->scale_factor * 11);   
    raw_ir *= (taos_cfgp->scale_factor * 3);

    if(raw_ir > raw_clear) {
        raw_lux = raw_ir;
        raw_ir = raw_clear;
        raw_clear = raw_lux;
    }
    dev_gain = taos_triton_gain_table[taos_cfgp->gain & 0x3];
    if(raw_clear >= lux_timep->saturation)
        return(TAOS_MAX_LUX);
    if(raw_ir >= lux_timep->saturation)
        return(TAOS_MAX_LUX);
    if(raw_clear == 0)
        return(0);
    if(dev_gain == 0 || dev_gain > 127) {
        printk(KERN_ERR "TAOS: dev_gain = 0 or > 127 in taos_get_lux()\n");
        return -1;
    }
    if(lux_timep->denominator == 0) {
        printk(KERN_ERR "TAOS: lux_timep->denominator = 0 in taos_get_lux()\n");
        return -1;
    }
    ratio = (raw_ir<<15)/raw_clear;
    for (p = lux_tablep; p->ratio && p->ratio < ratio; p++);
    if(!p->ratio) {//iVIZM
    	 printk("ch0=%d, ch1=%d, lux=10000\n",chdata[0]+chdata[1]*256, chdata[2]+chdata[3]*256);
    return 10000;
        if(lux_history[0] < 0)
            return 0;
        else
            return lux_history[0];
    }
    Tint = taos_cfgp->als_time;
    raw_clear = ((raw_clear*400 + (dev_gain>>1))/dev_gain + (Tint>>1))/Tint;
    raw_ir = ((raw_ir*400 +(dev_gain>>1))/dev_gain + (Tint>>1))/Tint;
    lux = ((raw_clear*(p->clear)) - (raw_ir*(p->ir)));
    lux = (lux + 32000)/64000;
    if(lux > TAOS_MAX_LUX) {
	 printk("ch0=%d, ch1=%d, lux=10000\n",chdata[0]+chdata[1]*256, chdata[2]+chdata[3]*256);
        lux = TAOS_MAX_LUX;
    }
    //return(lux)*taos_cfgp->filter_count;
    return(lux);
}

// proximity poll
static int taos_prox_poll(struct taos_prox_info *prxp) {
    int i = 0, ret = 0; 
    u8 chdata[6];
    for (i = 0; i < 6; i++) {
        chdata[i] = (i2c_smbus_read_byte_data(taos_datap->client, (TAOS_TRITON_CMD_REG | TAOS_TRITON_CMD_AUTO | (TAOS_TRITON_ALS_CHAN0LO + i))));
    }
    prxp->prox_clear = chdata[1];
    prxp->prox_clear <<= 8;
    prxp->prox_clear |= chdata[0];
    if (prxp->prox_clear > ((sat_als*80)/100))
        return -ENODATA;
    prxp->prox_data = chdata[5];
    prxp->prox_data <<= 8;
    prxp->prox_data |= chdata[4];


    return (ret);
}

MODULE_AUTHOR("John Koshi - Surya Software");
MODULE_DESCRIPTION("TAOS ambient light and proximity sensor driver");
MODULE_LICENSE("GPL");

late_initcall(taos_init);
module_exit(taos_exit);


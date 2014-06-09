/* drivers/input/keyboard/synaptics_i2c_rmi.c
 *
 * Copyright (C) 2007 Google, Inc.
 * Copyright (C) 2008 Texas Instrument Inc.
 * Copyright (C) 2009 Synaptics, Inc.
 *
 * provides device files /dev/input/event#
 * for named device files, use udev
 * 2D sensors report ABS_X_FINGER(0), ABS_Y_FINGER(0) through ABS_X_FINGER(7), ABS_Y_FINGER(7)
 * NOTE: requires updated input.h, which should be included with this driver
 * 1D/Buttons report BTN_0 through BTN_0 + button_count
 * TODO: report REL_X, REL_Y for flick, BTN_TOUCH for tap (on 1D/0D; done for 2D)
 * TODO: check ioctl (EVIOCGABS) to query 2D max X & Y, 1D button count
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/synaptics_i2c_rmi.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/earlysuspend.h>
#include <linux/proc_fs.h>
#include <linux/slab.h> 



#define TSC_PRINT(fmt, args...) pr_debug(fmt, ##args)
#define TSP_DEBUG
#define  FUNC34

#ifdef FUNC34
#include <linux/firmware.h>
struct synaptics_rmi4 *ts_f34;
static u8 fw_ver[2];
static u8 proc_fw_infomation[4]; 
#endif

//#define FUNC05

#define BTN_F19 BTN_0
#define BTN_F30 BTN_0
#define SCROLL_ORIENTATION REL_Y
static struct workqueue_struct *synaptics_wq;
static struct i2c_driver synaptics_rmi4_driver;
static int synaptics_rmi4_read_pdt(struct synaptics_rmi4 *ts);
/* Register: EGR_0 */
#define EGR_PINCH_REG		0
#define EGR_PINCH 		(1 << 6)
#define EGR_PRESS_REG 		0
#define EGR_PRESS 		(1 << 5)
#define EGR_FLICK_REG 		0
#define EGR_FLICK 		(1 << 4)
#define EGR_EARLY_TAP_REG	0
#define EGR_EARLY_TAP		(1 << 3)
#define EGR_DOUBLE_TAP_REG	0
#define EGR_DOUBLE_TAP		(1 << 2)
#define EGR_TAP_AND_HOLD_REG	0
#define EGR_TAP_AND_HOLD	(1 << 1)
#define EGR_SINGLE_TAP_REG	0
#define EGR_SINGLE_TAP		(1 << 0)
/* Register: EGR_1 */
#define EGR_PALM_DETECT_REG	1
#define EGR_PALM_DETECT		(1 << 0)

struct synaptics_function_descriptor {
	__u8 queryBase;
	__u8 commandBase;
	__u8 controlBase;
	__u8 dataBase;
	__u8 intSrc;
	__u8 functionNumber;
};
#define FUNCTION_VERSION(x) ((x >> 5) & 3)
#define INTERRUPT_SOURCE_COUNT(x) (x & 7)

#define FD_ADDR_MAX 0xE9
#define FD_ADDR_MIN 0xDD
#define FD_BYTE_COUNT 6

#define MIN_ACTIVE_SPEED 5

#define CONFIG_SYNA_MULTI_TOUCH1

extern int prop_add( struct device *dev, char *item, char *value);

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_rmi4_early_suspend(struct early_suspend *h);
static void synaptics_rmi4_late_resume(struct early_suspend *h);
#endif

#if defined(FUNC05)||defined(FUNC34)
static struct i2c_client *client_synap;
#endif

#ifdef FUNC05
static u8 report_mode;
#endif

static int synaptics_i2c_write(struct i2c_client *client, int reg, u8 data)
{
    u8 buf[2];
    int rc;
    int ret = 0;

    buf[0] = reg;
    buf[1] = data;
    rc = i2c_master_send(client, buf, 2);
    if (rc != 2)
    {
        dev_err(&client->dev, "synaptics_i2c_write FAILED: writing to reg %d\n", reg);
        ret = -1;
    }
    return ret;
}

static int synaptics_i2c_read(struct i2c_client *client, u8 addr, u8 *buf, u8 len)
{
	struct i2c_msg i2c_msg[2];
	int ret;
	int retry_count = 0;

	i2c_msg[0].addr = client->addr;
	i2c_msg[0].flags = 0;
	i2c_msg[0].buf = &addr;
	i2c_msg[0].len = 1;

	i2c_msg[1].addr = client->addr;
	i2c_msg[1].flags = I2C_M_RD;
	i2c_msg[1].buf = buf;
	i2c_msg[1].len = len;
	
retry:
	ret = i2c_transfer(client->adapter, i2c_msg, 2);
	if (ret < 0) 
	{
		if (++retry_count == 5) 
		{
			printk(KERN_ERR "[TSP] I2C read failed\n");
		       return ret;
		} else 
		{
			msleep(1);
			goto retry;
		}
	}
	return 0;
}


#ifdef FUNC34

static int synaptics_i2c_multi_write(struct i2c_client *client, int reg, u8 * buf, int count)
{
    int rc;
    int ret = 0;
	int i;
	unsigned char *txbuf;
	unsigned char txbuf_most[17]; /* Use this buffer for fast writes of 16
					bytes or less.  The first byte will
					contain the address at which to start
					the write. */
					
	if (count < sizeof(txbuf_most)) {
		/* Avoid an allocation if we can help it. */
		txbuf = txbuf_most;
	} else {
		/* over 16 bytes write we'll need to allocate a temp buffer */
		txbuf = kzalloc(count + 1, GFP_KERNEL);
		if (!txbuf)
			return -ENOMEM;
	}

	/* Yes, it stinks here that we have to copy the buffer */
	/* We copy from valp to txbuf leaving
	the first location open for the address */
	for (i = 0; i < count; i++)
		txbuf[i + 1] = buf[i];
	
    txbuf[0] = reg; /* put the address in the first byte */
    rc = i2c_master_send(client, txbuf, count+1);
    if (rc != (count+1))
    {
        dev_err(&client->dev, "synaptics_i2c_write FAILED: writing to reg %d\n", reg);
        ret = -1;
    }
    return ret;
}

static void flash_program(void)
{
     u8 buf[20];
	 int ret=0;
	 int i;
	 int blockindex;
	 u8* data = NULL;
	const struct firmware *synap_fw = NULL;

 		printk(KERN_ERR "%s enter\n", __func__);

		disable_irq(client_synap->irq);

		/* read fw file */
		ret = request_firmware(&synap_fw, "synaptics.img", (struct device*)&(client_synap->dev));
//		ret = request_firmware(&synap_fw, "syna.img", (struct device*)&(client_synap->dev));
		if (ret)
		{
			printk(KERN_CRIT " synaptics.img request failed %d\n",ret);
//			printk(KERN_CRIT " syna.img request failed %d\n",ret);
			return;
		}
		else 
		{
			data = (u8 *)synap_fw->data;
		}
		ret = synaptics_i2c_read(client_synap, (ts_f34->f01.querryBase+2), buf, 2); 
		printk(KERN_CRIT "ret=%d,old firmware version 0x%x 0x%x\n",ret,buf[0],buf[1]);


       synaptics_i2c_write(client_synap, (ts_f34->f01.controlBase+1), 0x00);       //only eanble flash INT
       printk(KERN_CRIT "enable flash Int reg = 0x%x\n",(ts_f34->f01.controlBase+1));
	   //enable flash programming
	ret = synaptics_i2c_read(client_synap, ts_f34->f34.querryBase, buf, 2); 
		printk(KERN_CRIT "ret=%d,bootloader id=( %x,%x)\n",ret,buf[0],buf[1]);

	synaptics_i2c_multi_write(client_synap, (ts_f34->f34.dataBase+2), buf,2);

		ret = synaptics_i2c_read(client_synap, (ts_f34->f34.querryBase+3), buf, 2); 
		printk(KERN_CRIT "ret=%d,block size count=( %x,%x)\n",ret,buf[0],buf[1]);
		ret = synaptics_i2c_read(client_synap,  (ts_f34->f34.querryBase+5), buf, 2); 
		printk(KERN_CRIT "ret=%d,firmware block num count=( %x,%x)\n",ret,buf[0],buf[1]);

		ret = synaptics_i2c_read(client_synap,(ts_f34->f34.querryBase+7), buf, 2); 
		printk(KERN_CRIT "ret=%d,configuration block num count=( %x,%x)\n",ret,buf[0],buf[1]);

	       synaptics_i2c_write(client_synap, (ts_f34->f34.dataBase+0x12), 0x0f);       //enable flash programming
         msleep(200);
      		i=0;
		do
		{
	               ret = synaptics_i2c_read(client_synap, (ts_f34->f34.dataBase+0x12), buf, 1); 
                      i++;
			printk(KERN_CRIT "command in progress,i=%x\n",i);
		}while(((ret!=0)||((buf[0]&0x80)==0)) &&(i<0xff));
	
	     printk(KERN_CRIT "flash cmd0x12=%x\n",buf[0]);
          synaptics_i2c_read(client_synap, (ts_f34->f01.dataBase+1), buf, 1);
	     printk(KERN_CRIT "INT status 0x14=%x\n",buf[0]);


	  //program the firmaware image 
	ret = synaptics_i2c_read(client_synap, ts_f34->f34.querryBase, buf, 2); 
		printk(KERN_CRIT "2 ret=%d,bootloader id=( %x,%x)\n",ret,buf[0],buf[1]);

	synaptics_i2c_multi_write(client_synap, (ts_f34->f34.dataBase+2), buf,2);
	       synaptics_i2c_write(client_synap, (ts_f34->f34.dataBase+0x12), 0x03);       //erase flash

      i=0;
	do
	{
	       ret = synaptics_i2c_read(client_synap, (ts_f34->f34.dataBase+0x12), buf, 1); 
              i++;
		printk(KERN_CRIT "2 command in progress,i=%x\n",i);
		}while(((ret!=0)||((buf[0]&0x80)==0)) &&(i<0xff));
	     printk(KERN_CRIT " 2 flash cmd0x12=%x\n",buf[0]);
          synaptics_i2c_read(client_synap, (ts_f34->f01.dataBase+1), buf, 1);
	     printk(KERN_CRIT "2 INT status 0x14=%x\n",buf[0]);


buf[0] = 0;
buf[1] = 0;
synaptics_i2c_multi_write(client_synap, ts_f34->f34.dataBase, buf,2);
//program firmware
//the frist 0x100 bytes is the header message
data +=0x100;
for(blockindex=0;blockindex<0xb00;blockindex++)
{

	/*printk(KERN_ALERT "blockindex(%x,%x);syna_bin:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n", \
			buf[0],buf[1],data[0],data[1], data[2], data[3],	data[4], data[5] , data[6], data[7],data[8], data[9], data[10],data[11]);
	*/
	//synaptics_i2c_multi_write(client_synap, 0x00, buf,2);

     
	synaptics_i2c_multi_write(client_synap, (ts_f34->f34.dataBase+0x02), data,16);    //??
       synaptics_i2c_write(client_synap, (ts_f34->f34.dataBase+0x12), 0x02);       //programming firmware

	        data +=16;
      msleep(20);
      i=0;
	do{
	               ret = synaptics_i2c_read(client_synap, (ts_f34->f34.dataBase+0x12), buf, 1); 
                      i++;
		printk(KERN_CRIT "3 command in progress,i=%x\n",i);
		}while(((ret!=0)||((buf[0]&0x80)==0)) &&(i<0xff));
	     printk(KERN_CRIT " 3 flash cmd0x12=%x\n",buf[0]);
//          synaptics_i2c_read(client_synap, 0x14, buf, 1);
	     printk(KERN_CRIT "3 INT status 0x14=%x\n",buf[0]);
}

buf[0] = 0;
buf[1] = 0;
synaptics_i2c_multi_write(client_synap, ts_f34->f34.dataBase, buf,2);
//program the configration image
for(blockindex=0;blockindex<0x80;blockindex++)
{
   
	/*printk(KERN_CRIT "blockindex(%x,%x);syna_bin:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n", \
			buf[0],buf[1],data[0],data[1], data[2], data[3],	data[4], data[5] , data[6], data[7],data[8], data[9], data[10],data[11]);
	*/
	//synaptics_i2c_multi_write(client_synap, 0x00, buf,2);

     
	synaptics_i2c_multi_write(client_synap, (ts_f34->f34.dataBase+0x02), data,16);    //??
       synaptics_i2c_write(client_synap, (ts_f34->f34.dataBase+0x12), 0x06);       //enable flash programming

	        data +=16;
      msleep(20);
      i=0;
	do
	{
	       ret = synaptics_i2c_read(client_synap, (ts_f34->f34.dataBase+0x12), buf, 1); 
              i++;
		printk(KERN_CRIT "31 command in progress,i=%x\n",i);
		}while(((ret!=0)||((buf[0]&0x80)==0)) &&(i<0xff));
//	     printk(KERN_CRIT " 31 flash cmd0x12=%x\n",buf[0]);
//          synaptics_i2c_read(client_synap, 0x14, buf, 1);
//	     printk(KERN_CRIT "31 INT status 0x14=%x\n",buf[0]);
}
 printk(KERN_ERR "--------blockindex=%x\n",blockindex);
//zhangqi add for test
ret = synaptics_i2c_read(client_synap, ts_f34->f01.dataBase, buf, 1); 
printk(KERN_CRIT "============== before reset,0x13=%x\n",buf[0]);
//disable program	   
      synaptics_i2c_write(client_synap, ts_f34->f01.commandlBase, 0x01);       //enable flash programming

//zhangqi add for test
ret = synaptics_i2c_read(client_synap, ts_f34->f01.dataBase, buf, 1); 
printk(KERN_CRIT "============== after reset ,0x13=%x\n",buf[0]);
	i=0;
	do{
		              msleep(30);
	               ret = synaptics_i2c_read(client_synap, ts_f34->f01.dataBase, buf, 1); 
                      i++;
		printk(KERN_CRIT "4 command in progress,i=%x,0x13=%x\n",i,buf[0]);
	}while(((ret!=0)||((buf[0]&0x40)!=0)) &&(i<0x1ff));
	
          synaptics_i2c_read(client_synap, (ts_f34->f01.dataBase+1), buf, 1);
	     printk(KERN_CRIT "4 INT status 0x14=%x\n",buf[0]);


        ret = synaptics_rmi4_read_pdt(ts_f34);

      //init--
       synaptics_i2c_write(client_synap, (ts_f34->f01.controlBase+1), 0x04);       //only eanble abs INT
//       synaptics_i2c_write(client_synap, 0x37, 0x01);       //reduced reporting mode
//       synaptics_i2c_write(client_synap, 0x41, 0x00);      
//	     synaptics_i2c_write(client_synap, 0x39, 0x3);
//       synaptics_i2c_write(client_synap, 0x3a, 0x3);

		ret = synaptics_i2c_read(client_synap,(ts_f34->f01.querryBase+2), fw_ver, 2); 
		printk(KERN_CRIT "ret=%d,new firmware version 0x%x 0x%x\n",ret,fw_ver[0],fw_ver[1]);

	   	    enable_irq(client_synap->irq);

}
#endif

#ifdef FUNC05

static void image_report(u8 mode)
{

		printk(KERN_ERR "%s enter\n", __func__);     //shihuiqin


       synaptics_i2c_write(client_synap, 0x36, 0x08);       //only eanble abs INT
       synaptics_i2c_write(client_synap, 0xff, 0x01);       //page1

      if(mode==1)
      	{
            synaptics_i2c_write(client_synap, 0x01, 0x40);       // set image  reporting mode
      	}
	 else
      	{
             synaptics_i2c_write(client_synap, 0x01, 0x80);       // set image  reporting mode
      	}
       synaptics_i2c_write(client_synap, 0x30, 0x04);       //get image
       synaptics_i2c_write(client_synap, 0xff, 0x00);       //page1


}
#endif
static int synaptics_rmi4_read_pdt(struct synaptics_rmi4 *ts)
{
	int ret = 0;
	int nFd = 0;
	int interruptCount = 0;
	__u8 data_length = 0;
	struct i2c_msg fd_i2c_msg[2];
	__u8 fd_reg;
	struct i2c_msg query_i2c_msg[2];
	__u8 query[14];
	struct synaptics_function_descriptor fd;
	__u8 *egr;

	fd_i2c_msg[0].addr = ts->client->addr;
	fd_i2c_msg[0].flags = 0;
	fd_i2c_msg[0].buf = &fd_reg;
	fd_i2c_msg[0].len = 1;

	fd_i2c_msg[1].addr = ts->client->addr;
	fd_i2c_msg[1].flags = I2C_M_RD;
	fd_i2c_msg[1].buf = (__u8 *)(&fd);
	fd_i2c_msg[1].len = FD_BYTE_COUNT;

	query_i2c_msg[0].addr = ts->client->addr;
	query_i2c_msg[0].flags = 0;
	query_i2c_msg[0].buf = &fd.queryBase;
	query_i2c_msg[0].len = 1;

	query_i2c_msg[1].addr = ts->client->addr;
	query_i2c_msg[1].flags = I2C_M_RD;
	query_i2c_msg[1].buf = query;
	query_i2c_msg[1].len = sizeof(query);

	ts->hasF11 = false;
	ts->hasF19 = false;
	ts->hasF30 = false;
	ts->data_reg = 0xff;
	ts->data_length = 0;

	for (fd_reg = FD_ADDR_MAX; fd_reg >= FD_ADDR_MIN; fd_reg -= FD_BYTE_COUNT) {
		printk("[TSP] read from address 0x%x\n", fd_reg);
		ret = i2c_transfer(ts->client->adapter, fd_i2c_msg, 2);
		if (ret < 0) {
			printk(KERN_ERR "[TSP] I2C read failed querying RMI4 $%02X capabilities\n", ts->client->addr);
			return ret;
		}
		if (!fd.functionNumber) {
			/* End of PDT */
			ret = nFd;
			printk("[TSP] Read %d functions from PDT\n", nFd);
			printk("[TSP] end of PDT with string 0x%x; 0x%x; 0x%x; 0x%x; 0x%x; 0x%x\n", \
					fd.functionNumber, fd.queryBase, fd.commandBase, fd.controlBase, fd.dataBase, fd.intSrc);
			break;
		}
                            
		++nFd;
		printk("[TSP] fd.functionNumber = 0x%x\n", fd.functionNumber);
		printk("[TSP] fd.queryBase = 0x%x\n", fd.queryBase);
		printk("[TSP] fd.commandBase = 0x%x\n", fd.commandBase);
		printk("[TSP] fd.controlBase = 0x%x\n", fd.controlBase);
		printk("[TSP] fd.dataBase = 0x%x\n", fd.dataBase);
		printk("[TSP] fd.intSrc = 0x%x\n\n", fd.intSrc);
		
		switch (fd.functionNumber) {
			case 0x01: /* Interrupt */
				ts->f01.data_offset = fd.dataBase;
				ts->f01.controlBase =  fd.controlBase;
				ts->f01.commandlBase = fd.commandBase;//ECID:0000 zhangzhao 2012-6-1 add tsc reset in probe function
				ts->f01.dataBase  = fd.dataBase;//ECID:0000 zhangzhao 2012-6-1 add tsc reset in probe function
				ts->f01.querryBase = fd.queryBase;
				
				/*
				 * Can't determine data_length
				 * until whole PDT has been read to count interrupt sources
				 * and calculate number of interrupt status registers.
				 * Setting to 0 safely "ignores" for now.
				 */
				data_length = 0;
				break;
			case 0x11: /* 2D */
				ts->hasF11 = true;
				ts->f11.data_offset = fd.dataBase;
				ts->f11.interrupt_offset = interruptCount / 8;
				ts->f11.interrupt_mask = ((1 << INTERRUPT_SOURCE_COUNT(fd.intSrc)) - 1) << (interruptCount % 8);
                                                        
				ret = i2c_transfer(ts->client->adapter, query_i2c_msg, 2);
				if (ret < 0)
					printk(KERN_ERR "[TSP] Error reading F11 query registers\n");

				ts->f11.points_supported = (query[1] & 7) + 1;
				if (ts->f11.points_supported == 6)
					ts->f11.points_supported = 10;

				ts->f11_fingers = kcalloc(ts->f11.points_supported,
				                          sizeof(*ts->f11_fingers), 0);

				printk("%d fingers\n", ts->f11.points_supported);
				ts->f11_has_gestures = (query[1] >> 5) & 1;
				ts->f11_has_relative = (query[1] >> 3) & 1;

				egr = &query[7];


				query_i2c_msg[0].buf = &fd.controlBase;
				ret = i2c_transfer(ts->client->adapter, query_i2c_msg, 2);
				if (ret < 0)
					printk(KERN_ERR "Error reading F11 control registers\n");

				query_i2c_msg[0].buf = &fd.queryBase;

				ts->f11_max_x = ((query[7] & 0x0f) * 0x100) | query[6];
				ts->f11_max_y = ((query[9] & 0x0f) * 0x100) | query[8];
                                                        
				printk("[TSP] max X: %d; max Y: %d\n", ts->f11_max_x, ts->f11_max_y);

				ts->f11.data_length = data_length =
					/* finger status, four fingers per register */
					((ts->f11.points_supported + 3) / 4)
					/* absolute data, 5 per finger */
					+ 5 * ts->f11.points_supported
					/* two relative registers */
					+ (ts->f11_has_relative ? 2 : 0)
					/* F11_2D_Data8 is only present if the egr_0 register is non-zero. */
					+ (egr[0] ? 1 : 0)
					/* F11_2D_Data9 is only present if either egr_0 or egr_1 registers are non-zero. */
					+ ((egr[0] || egr[1]) ? 1 : 0)
					/* F11_2D_Data10 is only present if EGR_PINCH or EGR_FLICK of egr_0 reports as 1. */
					+ ((ts->hasEgrPinch || ts->hasEgrFlick) ? 1 : 0)
					/* F11_2D_Data11 and F11_2D_Data12 are only present if EGR_FLICK of egr_0 reports as 1. */
					+ (ts->hasEgrFlick ? 2 : 0)
					;
				break;
				
			case 0x19: /* Cap Buttons */
				ts->hasF19 = true;
				ts->f19.data_offset = fd.dataBase;
				ts->f19.interrupt_offset = interruptCount / 8;
				ts->f19.interrupt_mask = ((1 < INTERRUPT_SOURCE_COUNT(fd.intSrc)) - 1) << (interruptCount % 8);
				//ret = i2c_transfer(ts->client->adapter, query_i2c_msg, 2);
				if (ret < 0)
					printk(KERN_ERR "[TSP] Error reading F19 query registers\n");

				ts->f19.points_supported = query[1] & 0x1F;
				ts->f19.data_length = data_length = (ts->f19.points_supported + 7) / 8;

				printk(KERN_NOTICE "[TSP] $%02X F19 has %d buttons\n", ts->client->addr, ts->f19.points_supported);

				break;
				
			case 0x30: /* GPIO */
				ts->hasF30 = true;
				ts->f30.data_offset = fd.dataBase;
				ts->f30.interrupt_offset = interruptCount / 8;
				ts->f30.interrupt_mask = ((1 < INTERRUPT_SOURCE_COUNT(fd.intSrc)) - 1) << (interruptCount % 8);

				ret = i2c_transfer(ts->client->adapter, query_i2c_msg, 2);
				if (ret < 0)
					printk(KERN_ERR "[TSP] Error reading F30 query registers\n");

				ts->f30.points_supported = query[1] & 0x1F;
				ts->f30.data_length = data_length = (ts->f30.points_supported + 7) / 8;

				break;
			case 0x34:
				ts->hasF30 = true;
				ts->f34.querryBase = fd.queryBase;
				ts->f34.dataBase = fd.dataBase;
				ts->f34.controlBase =  fd.controlBase;
				ts->f34.commandlBase = fd.commandBase;
				break;
			default:
				goto pdt_next_iter;				
		}

		// Change to end address for comparison
		// NOTE: make sure final value of ts->data_reg is subtracted
		data_length += fd.dataBase;
		if (data_length > ts->data_length) {
			ts->data_length = data_length;
		}

		if (fd.dataBase < ts->data_reg) {
			ts->data_reg = fd.dataBase;
		}

pdt_next_iter:
		interruptCount += INTERRUPT_SOURCE_COUNT(fd.intSrc);
	}

	// Now that PDT has been read, interrupt count determined, F01 data length can be determined.
	ts->f01.data_length = data_length = 1 + ((interruptCount + 7) / 8);
	// Change to end address for comparison
	// NOTE: make sure final value of ts->data_reg is subtracted
	data_length += ts->f01.data_offset;
	if (data_length > ts->data_length) {
		ts->data_length = data_length;
	}

	// Change data_length back from end address to length
	// NOTE: make sure this was an address
	ts->data_length -= ts->data_reg;

	// Change all data offsets to be relative to first register read
	//  TODO: add __u8 *data (= &ts->data[ts->f##.data_offset]) to struct rmi_function_info?
	ts->f01.data_offset -= ts->data_reg;
	ts->f11.data_offset -= ts->data_reg;
	ts->f19.data_offset -= ts->data_reg;
	ts->f30.data_offset -= ts->data_reg;

#ifdef FUNC05

	ts->data = kcalloc(ts->data_length+15, sizeof(*ts->data), GFP_KERNEL);
#else
	ts->data = kcalloc(ts->data_length, sizeof(*ts->data), GFP_KERNEL);

#endif
	if (ts->data == NULL) {
		printk( "[TSP] Not enough memory to allocate space for RMI4 data\n");
		ret = -ENOMEM;
	}
	printk("[TSP] ts->datalength:%d; F01_DataOffset:0x%x; F11_DataOffset:0x%x; F19_DataOffset:0x%x; F30_DataOffset:0x%x;\n", \
			ts->data_length, ts->f01.data_offset, ts->f11.data_offset, ts->f19.data_offset, ts->f30.data_offset);
	ts->data_i2c_msg[0].addr = ts->client->addr;
	ts->data_i2c_msg[0].flags = 0;
	ts->data_i2c_msg[0].len = 1;
	ts->data_i2c_msg[0].buf = &ts->data_reg;

	ts->data_i2c_msg[1].addr = ts->client->addr;
	ts->data_i2c_msg[1].flags = I2C_M_RD;
	ts->data_i2c_msg[1].len = ts->data_length;
	ts->data_i2c_msg[1].buf = ts->data;

	printk("[TSP] RMI4 $%02X data read: $%02X + %d\n",
		ts->client->addr, ts->data_reg, ts->data_length);

	return ret;
}

#ifdef CONFIG_SYNA_BUTTONS_SCROLL
static int first_button(__u8 *button_data, __u8 points_supported)
{
	int b, reg;

	for (reg = 0; reg < ((points_supported + 7) / 8); reg++)
		for (b = 0; b < 8; b++)
			if ((button_data[reg] >> b) & 1)
				return reg * 8 + b;

	return -1;
}

static void synaptics_report_scroll(struct input_dev *dev,
                                    __u8 *button_data,
                                    __u8 points_supported,
                                    int ev_code)
{
	int scroll = 0;
	static int last_button = -1, current_button = -1;

	// This method is slightly problematic
	// It makes no check to find if lift/touch is more likely than slide
	current_button = first_button(button_data, points_supported);

	if (current_button >= 0 && last_button >= 0) {
		scroll = current_button - last_button;
		// This filter mostly works to isolate slide motions from lift/touch
		if (abs(scroll) == 1) {
			//printk("%s(): input_report_rel(): %d\n", __func__, scroll);
			input_report_rel(dev, ev_code, scroll);
		}
	}

	last_button = current_button;
}
#endif
static int
proc_read_val(char *page, char **start, off_t off, int count, int *eof,
	  void *data)
{
	int len = 0;
	
//	len += sprintf(page + len, "%s\n", "touchscreen infomation");
	len += sprintf(page + len, "name: %s\n", "synaptics");
	len += sprintf(page + len, "i2c address: 0x%x\n", 0x22);
	len += sprintf(page + len, "IC type: %s\n", "s2202");
#ifdef FUNC34
	len += sprintf(page + len, "firmware version: 0x%02x,0x%02x\n", proc_fw_infomation[2],proc_fw_infomation[3]);
#endif
/***************************haowiewei add start 20121019***************************************/
switch (proc_fw_infomation[1]){
     
	case 0x31:
	 	len += sprintf(page + len, "module : %s\n", "TPK");
		break ;
	case 0x32:
	 	len += sprintf(page + len, "module : %s\n", "Truly");
		break ;
	case 0x33:
	 	len += sprintf(page + len, "module : %s\n", "Success");
		break ;
	case 0x34:
	 	len += sprintf(page + len, "module : %s\n", "Oflim");
		break ;
	case 0x35:
	 	len += sprintf(page + len, "module : %s\n", "Lead");
		break ;
	case 0x36:
	 	len += sprintf(page + len, "module : %s\n", "Wintek");
		break ;
	case 0x37:
	 	len += sprintf(page + len, "module : %s\n", "Laibao");
		break ;
	case 0x38:
	 	len += sprintf(page + len, "module : %s\n", "CMI");
		break ;
	case 0x39:
	 	len += sprintf(page + len, "module : %s\n", "Eely");
		break ;
	case 0x41:
	 	len += sprintf(page + len, "module : %s\n", "Goworld");
		break ;
	case 0x42:
	 	len += sprintf(page + len, "module : %s\n", "Baoming");
		break ;
	case 0x43:
	 	len += sprintf(page + len, "module : %s\n", "Eachopto");
		break ;
	case 0x44:
	 	len += sprintf(page + len, "module : %s\n", "Mutto");
		break ;
	case 0x45:
	 	len += sprintf(page + len, "module : %s\n", "Junda");
		break ;	 
	 default:
	 	len += sprintf(page + len, "module : %s\n", "I do not know");
}
/***************************haoweiwei add end 20121019***************************************/
	
	if (off + count >= len)
		*eof = 1;
	if (len < off)
	{
		return 0;

	}
	*start = page + off;
	return ((count < len - off) ? count : len - off);
}

static int proc_write_val(struct file *file, const char *buffer,
           unsigned long count, void *data)
{
		unsigned long val;
      	       uint8_t buf1[10];
		sscanf(buffer, "%lu", &val);
		if (val >= 0) {
		if(val==1)
		{
#ifdef FUNC05
		     report_mode = 1;
		     image_report(1);     //baseline data
#endif
     pr_err("++reg = %x ++++++ reset the touch panel\n",ts_f34->f01.commandlBase);
     //ts_hardware_reset();
	synaptics_i2c_read(client_synap, ts_f34->f01.dataBase, buf1, 1);
	printk("---init-- synaptics_i2c_read, commandbase=%x, database=%x,reg = %x\n",ts_f34->f01.commandlBase,ts_f34->f01.dataBase,buf1[0]);
	 
 //     synaptics_i2c_write(client_synap, ts_f34->f01.commandlBase, 0x01);       //enable flash programming
//	synaptics_i2c_read(client_synap, ts_f34->f01.dataBase, buf1, 1);
	printk("---init-- synaptics_i2c_read, commandbase=%x, database=%x,reg = %x\n",ts_f34->f01.commandlBase,ts_f34->f01.dataBase,buf1[0]);

		}
#ifdef FUNC34
		else if (val==3)
		{
			printk("flash programming write \n");
			flash_program();	
		}
#endif
		else
		{
#ifdef FUNC05
                   	printk("reporting mode 2\n");
		     report_mode = 2;
		     image_report(2);     //delta data
#endif
		}
			
			return count;
		}
		return -EINVAL;
}

static void synaptics_rmi4_work_func(struct work_struct *work)
{
	int ret;
	struct synaptics_rmi4 *ts = container_of(work, struct synaptics_rmi4, work);
#ifdef FUNC05
	u8 reg, len, index;
#endif
	//ret = i2c_transfer(ts->client->adapter, ts->data_i2c_msg, 2);
       ret = synaptics_i2c_read(ts->client, *(ts->data_i2c_msg[0].buf), ts->data_i2c_msg[1].buf, ts->data_i2c_msg[1].len);

	if (ret < 0) {
		printk( "[TSP] %s: i2c_transfer failed\n", __func__);
		//ts_hardware_reset();
	} else {
		__u8 *interrupt = &ts->data[ts->f01.data_offset + 1];
		#ifdef TSP_DEBUG_FUNC
		TSC_PRINT("[TSP] int status: 0x%x,IntMask:0x%x\n", interrupt[ts->f11.interrupt_offset], ts->f11.interrupt_mask);
		#endif
		if (ts->hasF11 && (interrupt[ts->f11.interrupt_offset] & ts->f11.interrupt_mask))     //mengzf
		{
		       /* number of touch points - fingers down in this case */
 	              int fingerDownCount = 0;
			__u8 fsr_len = (ts->f11.points_supported + 3) / 4;
			__u8 *f11_data = &ts->data[ts->f11.data_offset];
	             int f;
	             for (f = 0; f < ts->f11.points_supported; ++f) 
	             {
				/*finger status*/
				__u8 finger_status_reg = 0;
				__u8 finger_status;

				finger_status_reg = f11_data[f / 4];
				finger_status = (finger_status_reg >> ((f % 4) * 2)) & 3;
				if (finger_status == f11_finger_accurate ) 	{
					fingerDownCount++;
					ts->wasdown = true;
				}
	                   if (finger_status == f11_finger_accurate) {
			             __u8 reg;
			             __u8 *finger_reg;
			             u12 x;
			             u12 y;
					u4 wx = 0;
					u4 wy = 0;
					int z=0;
					
					reg = fsr_len + 5 * f;
				       finger_reg = &f11_data[reg];
					x = (finger_reg[0] * 0x10) | (finger_reg[2] % 0x10);	
				       y = (finger_reg[1] * 0x10) | (finger_reg[2] / 0x10);
					wx = finger_reg[3] % 0x10;
					wy = finger_reg[3] / 0x10;
					z = finger_reg[4];
					#ifdef TSP_DEBUG
					TSC_PRINT(  "[TSP] f: %d, s:%d, x: %d, y: %d\n", f,finger_status,x,y);
					#endif
					/* if this is the first finger report normal
					ABS_X, ABS_Y, PRESSURE, TOOL_WIDTH events for
					non-MT apps. Apps that support Multi-touch
					will ignore these events and use the MT events.
					Apps that don't support Multi-touch will still
					function.
					*/
	                            input_report_key(ts->input_dev,BTN_TOUCH, 1);
              			input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, f);
              			//input_report_abs(ts->input_dev, ABS_MT_PRESSURE, z);
              			//input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, max(wx, wy) );
              			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x );
              			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y );
              			input_report_abs(ts->input_dev, ABS_MT_PRESSURE, z);


					input_mt_sync(ts->input_dev);
			
				}
			}

		/* if we had a finger down before and now we don't have any send a button up. */
			if ((fingerDownCount == 0) && ts->wasdown) {
				ts->wasdown = false;
			//#ifdef CONFIG_SYNA_MULTI_TOUCH1
				//input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
				//input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
				//input_report_abs(ts->input_dev, ABS_MT_POSITION_X, ts->oldX);
				//input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, ts->oldY);
				//input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, f);
				//input_report_key(ts->input_dev, BTN_TOUCH, 0);

				input_mt_sync(ts->input_dev);
			//#endif
				//input_report_abs(ts->input_dev, ABS_X, ts->oldX);
				//input_report_abs(ts->input_dev, ABS_Y, ts->oldY);
				//ts->oldX = ts->oldY = 0;
				#ifdef TSP_DEBUG_FUNC
				TSC_PRINT( "[TSP] %s: Finger up.", __func__);
				#endif
	                     input_report_key(ts->input_dev,BTN_TOUCH, 0);
			}
	              input_sync(ts->input_dev);     /* sync after groups of events */
		} 

#ifdef FUNC05
		if(interrupt[ts->f11.interrupt_offset]&0x08) {
			reg=0x02;
			if(report_mode==1){
				len=40;
			} else {
				len=20;
			}
			ts->data_i2c_msg[0].buf = &reg;
			ts->data_i2c_msg[1].len = len;

			synaptics_i2c_write(client_synap, 0xff, 0x01);       //page1


			for(index=0;index<0x0c;index++) {
				if(report_mode==1) {
					synaptics_i2c_write(client_synap, 0x01, (0x40|index));       // set image  reporting mode
					i2c_transfer(ts->client->adapter, ts->data_i2c_msg, 2);
						
					printk(KERN_ALERT "index=%d;%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n", \
										index, \
										ts->data[0] | (ts->data[1]<<8), ts->data[2] | (ts->data[3]<<8), ts->data[4] | (ts->data[5]<<8), ts->data[6] | (ts->data[7]<<8),\
										ts->data[8] | (ts->data[9]<<8), ts->data[10] | (ts->data[11]<<8), ts->data[12] | (ts->data[13]<<8), ts->data[14] | (ts->data[15]<<8),\
										ts->data[16] | (ts->data[17]<<8), ts->data[18] | (ts->data[19]<<8), ts->data[20] | (ts->data[21]<<8), ts->data[22] | (ts->data[23]<<8),\
										ts->data[24] | (ts->data[25]<<8), ts->data[26] | (ts->data[27]<<8), ts->data[28] | (ts->data[29]<<8), ts->data[30] | (ts->data[31]<<8),\
										ts->data[32] | (ts->data[33]<<8), ts->data[34] | (ts->data[35]<<8), ts->data[36] | (ts->data[37]<<8), ts->data[38] | (ts->data[39]<<8));
				   
					} else {
						synaptics_i2c_write(client_synap, 0x01, (0x80|index));       // set image  reporting mode
						i2c_transfer(ts->client->adapter, ts->data_i2c_msg, 2);
						printk(KERN_ALERT "index=%d;%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n", \
											index, \
											ts->data[0] , (ts->data[1]), ts->data[2] , (ts->data[3]), ts->data[4] , (ts->data[5]), ts->data[6] , (ts->data[7]),\
											ts->data[8] , (ts->data[9]), ts->data[10] , (ts->data[11]), ts->data[12], (ts->data[13]), ts->data[14] , (ts->data[15]),\
											ts->data[16] , (ts->data[17]), ts->data[18], (ts->data[19]));
					}
			}

			synaptics_i2c_write(client_synap, 0xff, 0x00);       // page 0
			synaptics_i2c_write(client_synap, 0x88, 0x01);       // reset
			msleep(150);
			synaptics_i2c_write(ts->client, 0x36, 0x04);       //only eanble abs INT
			synaptics_i2c_write(ts->client, 0x37, 0x01);       //reduced reporting mode
			synaptics_i2c_write(ts->client, 0x41, 0x00);      
			synaptics_i2c_write(ts->client, 0x39, 0x3);
			synaptics_i2c_write(ts->client, 0x3a, 0x3);

			ts->data_i2c_msg[0].buf = &ts->data_reg;
			ts->data_i2c_msg[1].len = ts->data_length;
		}
#endif

	}
	if (ts->use_irq) {
		enable_irq(ts->client->irq);
	}
}

static enum hrtimer_restart synaptics_rmi4_timer_func(struct hrtimer *timer)
{
	struct synaptics_rmi4 *ts = container_of(timer, \
					struct synaptics_rmi4, timer);

	queue_work(synaptics_wq, &ts->work);

	hrtimer_start(&ts->timer, ktime_set(0, 12 * NSEC_PER_MSEC), HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}

irqreturn_t synaptics_rmi4_irq_handler(int irq, void *dev_id)
{
	struct synaptics_rmi4 *ts = dev_id;
#ifdef TSP_DEBUG
	TSC_PRINT("[TSP] %s enter\n", __func__);
#endif
	disable_irq_nosync(ts->client->irq);
	queue_work(synaptics_wq, &ts->work);

	return IRQ_HANDLED;
}

static ssize_t synaptics_show_fw_version(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", 10);
}

static DEVICE_ATTR(fw_version, S_IRUGO, synaptics_show_fw_version, NULL);

static int synaptics_rmi4_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
	int i;
	int ret = 0;
      int min_x = 0;
      int min_y = 0;
	u8 fw_version[2];
	struct synaptics_rmi4 *ts;
	struct proc_dir_entry *refresh;
              

       struct synaptics_ts_platform_data *pdata;  
	pdata = client->dev.platform_data;
	#if 0
       if(pdata->init_platform_hw != NULL)
	{
		 ret =pdata->init_platform_hw(client);
		 if(ret < 0)
		 	printk("synaptics init error %d\n",ret);
	}
	#endif
	if(pdata->power_on != NULL)
	{      
	        ret = pdata->power_on(client, 1);     //shihuiqin 20120424
               if(ret <0)
       	 {
		         goto err_check_power_failed;
	         }
	}

       if(ret <0)
       {
		goto err_check_power_failed;
	}
	printk( "[TSP] probing for Synaptics RMI4 device %s at $%02X...\n", client->name, client->addr);
	//ts_hardware_reset();
      // msleep(100);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "[TSP] %s: need I2C_FUNC_I2C\n", __func__);
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}
        	
	ts = kzalloc(sizeof(struct synaptics_rmi4), GFP_KERNEL);
	INIT_WORK(&ts->work, synaptics_rmi4_work_func);
	ts->client = client;
	i2c_set_clientdata(client, ts);
	client->driver = &synaptics_rmi4_driver;

#if defined(FUNC05)||defined(FUNC34)
	client_synap = ts->client;
#endif
#ifdef FUNC34
	ts_f34=ts;
#endif
	   
        ret = synaptics_rmi4_read_pdt(ts);
	if (ret <= 0) {
		if (ret == 0)
			printk(KERN_ERR "[TSP] Empty PDT\n");

		printk(KERN_ERR "[TSP] Error identifying device (%d)\n", ret);
		ret = -ENODEV;
		goto err_pdt_read_failed;
	}

	synaptics_i2c_read(ts->client, (ts->f01.querryBase+2), fw_version, 2);
	printk("[TSP] firmware version: 0x%x 0x%x\n", fw_version[0], fw_version[1]);
	msleep(30);
        synaptics_i2c_read(ts->client, (ts->f34.controlBase), proc_fw_infomation, 4);
	printk("[TSP] proc config ID infomation : 0x%x 0x%x 0x%x 0x%x\n", proc_fw_infomation[0], proc_fw_infomation[1],proc_fw_infomation[2], proc_fw_infomation[3]);
#ifdef FUNC34
	fw_ver[0]=fw_version[0];
	fw_ver[1]=fw_version[1];
#endif
	
	ts->input_dev = input_allocate_device();
	if (!ts->input_dev) {
		printk(KERN_ERR "[TSP] failed to allocate input device.\n");
		ret = -EBUSY;
		goto err_alloc_dev_failed;
	}

	ts->input_dev->name = "Synaptics RMI4";
	ts->input_dev->phys = client->name;
	//ts->f11_max_x = 1139;//1153;
	//ts->f11_max_y = ts->f11_max_y - 150;
	min_x = 0;
	min_y = 0;
              
	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);
	
	/* set dummy key to make driver work with virtual keys */
	input_set_capability(ts->input_dev, EV_KEY, KEY_PROG1);
	
	set_bit(ABS_MT_PRESSURE, ts->input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, ts->input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, ts->input_dev->absbit);
	//set_bit(ABS_MT_WIDTH_MAJOR, ts->input_dev->absbit);

	set_bit(BTN_TOUCH, ts->input_dev->keybit);
	set_bit(BTN_2, ts->input_dev->keybit);

	

	if (ts->hasF11) {
		printk(KERN_DEBUG "[TSP] %s: Set ranges X=[%d..%d] Y=[%d..%d].", __func__, min_x, ts->f11_max_x, min_y, ts->f11_max_y);
		input_set_abs_params(ts->input_dev, ABS_X, min_x, ts->f11_max_x,0, 0);
		input_set_abs_params(ts->input_dev, ABS_Y, min_y, ts->f11_max_y,0, 0);
		input_set_abs_params(ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);
		input_set_abs_params(ts->input_dev, ABS_TOOL_WIDTH, 0, 15, 0, 0);

	//#ifdef CONFIG_SYNA_MULTI_TOUCH1
		input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE, 0, 255, 0, 0);   /*pressure of single-touch*/
		//input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MINOR, 0, 15, 0, 0);
		//input_set_abs_params(ts->input_dev, ABS_MT_ORIENTATION, 0, 1, 0, 0);
		//input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 1, ts->f11.points_supported, 0, 0);
		//input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);   /*ABS_TOOL_WIDTH of single-touch*/

		input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, min_x, ts->f11_max_x,0, 0);
		input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, min_y, ts->f11_max_y,0, 0);
	//#endif
		for (i = 0; i < ts->f11.points_supported; ++i) 
		{
              #ifdef CONFIG_SYNA_MULTI_TOUCH
			/* Linux 2.6.31 multi-touch */
                     input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 1, ts->f11.points_supported, 0, 0);
              #endif
		}
		
		if (ts->hasEgrPalmDetect)
			set_bit(BTN_DEAD, ts->input_dev->keybit);
		if (ts->hasEgrFlick) {
			set_bit(REL_X, ts->input_dev->keybit);
			set_bit(REL_Y, ts->input_dev->keybit);
		}
		if (ts->hasEgrSingleTap)
			set_bit(BTN_TOUCH, ts->input_dev->keybit);
		if (ts->hasEgrDoubleTap)
			set_bit(BTN_TOOL_DOUBLETAP, ts->input_dev->keybit);
	}
	if (ts->hasF19) {
		set_bit(BTN_DEAD, ts->input_dev->keybit);
#ifdef CONFIG_SYNA_BUTTONS
		/* F19 does not (currently) report ABS_X but setting maximum X is a convenient way to indicate number of buttons */
		input_set_abs_params(ts->input_dev, ABS_X, 0, ts->f19.points_supported, 0, 0);
		for (i = 0; i < ts->f19.points_supported; ++i) {
			set_bit(BTN_F19 + i, ts->input_dev->keybit);
		}
#endif

#ifdef CONFIG_SYNA_BUTTONS_SCROLL
		set_bit(EV_REL, ts->input_dev->evbit);
		set_bit(SCROLL_ORIENTATION, ts->input_dev->relbit);
#endif
	}
	if (ts->hasF30) {
		for (i = 0; i < ts->f30.points_supported; ++i) {
			set_bit(BTN_F30 + i, ts->input_dev->keybit);
		}
	}

	/*
	 * Device will be /dev/input/event#
	 * For named device files, use udev
	 */
	ret = input_register_device(ts->input_dev);
	if (ret) {
		printk(KERN_ERR "[TSP] synaptics_rmi4_probe: Unable to register %s \
				input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	} else {
		printk("[TSP] synaptics input device registered\n");
	}

	if (client->irq) {
		if (request_irq(client->irq, synaptics_rmi4_irq_handler,
				IRQF_TRIGGER_LOW, client->name, ts) >= 0) {
			ts->use_irq = 1;
		} else {
			printk("[TSP] Failed to request IRQ!\n");
		}
	}
              
	if (!ts->use_irq) {
		printk(KERN_ERR "[TSP] Synaptics RMI4 device %s in polling mode\n", client->name);
		hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		ts->timer.function = synaptics_rmi4_timer_func;
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}

	ts->enable = 1;

	dev_set_drvdata(&ts->input_dev->dev, ts);

    //dir = proc_mkdir("touchscreen", NULL);
    refresh = create_proc_entry("driver/tsc_id", 0777, NULL);	   
	if (refresh) {
		refresh->data		= NULL;
		refresh->read_proc  = proc_read_val;
		refresh->write_proc = proc_write_val;
	}
	printk( "[TSP] synaptics_ts_probe: Start touchscreen %s in %s mode\n", ts->input_dev->name, ts->use_irq ? "interrupt" : "polling");

	#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = synaptics_rmi4_early_suspend;
	ts->early_suspend.resume = synaptics_rmi4_late_resume;
	register_early_suspend(&ts->early_suspend);
	#endif
	
	ret = device_create_file(&ts->input_dev->dev, &dev_attr_fw_version);
	if(ret) {
		printk(KERN_ERR "[TSP] create fw version number fail\n");
		goto err_input_register_device_failed;
	}
	

/***************************haoweiwei add softreset start***************************/
	 synaptics_i2c_write(client_synap, ts_f34->f01.commandlBase, 0x1);
     msleep(150);
/***************************haoweiwei add softreset end*****************************/
	return 0;

err_input_register_device_failed:
	input_free_device(ts->input_dev);

err_alloc_dev_failed:
err_pdt_read_failed:
                
              if(ts != NULL)
              {
                        kfree(ts);
              }
err_check_functionality_failed:
              pdata->power_on(client,0);
err_check_power_failed:
	return ret;
}

static int synaptics_rmi4_remove(struct i2c_client *client)
{
       struct synaptics_rmi4 *ts = i2c_get_clientdata(client);

      #if defined(CONFIG_TOUCHSCREEN_VIRTUAL_KEYS)
        ts_key_report_deinit();	   
      #endif
	
	unregister_early_suspend(&ts->early_suspend);
	if (ts->use_irq)
		free_irq(client->irq, ts);
	else
		hrtimer_cancel(&ts->timer);
	
	device_remove_file(&ts->input_dev->dev, &dev_attr_fw_version);

	input_unregister_device(ts->input_dev);
       if(ts != NULL)
       {
                kfree(ts);
        }
	   
	return 0;
}

static int synaptics_rmi4_suspend(struct i2c_client *client, pm_message_t mesg)
{
       int ret;
	struct synaptics_rmi4 *ts = i2c_get_clientdata(client);
	if (ts->use_irq)
       {
		disable_irq(client->irq);
	}
	else
	{
		hrtimer_cancel(&ts->timer);
	}
	ret = cancel_work_sync(&ts->work);
	if (ret && ts->use_irq) /* if work was pending disable-count is now 2 */
	{
	    printk("[TSP] cancel_work_sync ret=%d",ret);
	    enable_irq(client->irq);
	}


      synaptics_i2c_write(client, ts->f01.controlBase, 0x01);      /* deep sleep */
      synaptics_i2c_write(client, (ts->f01.controlBase)+1, 0);     /* disable interrupt, */

	ts->enable = 0;

	return 0;
}

static int synaptics_rmi4_resume(struct i2c_client *client)
{
    struct synaptics_rmi4 *ts = i2c_get_clientdata(client);

       synaptics_i2c_write(ts->client, ts->f01.controlBase, 0x0);      /* wakeup */

	if (ts->use_irq)
	{
	    enable_irq(ts->client->irq);
	    synaptics_i2c_write(ts->client, (ts->f01.controlBase)+1, 4);     /* enable interrupt, */
	}
	else
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	ts->enable = 1;

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_rmi4_early_suspend(struct early_suspend *h)
{
	struct synaptics_rmi4 *ts;
	ts = container_of(h, struct synaptics_rmi4, early_suspend);
	synaptics_rmi4_suspend(ts->client, PMSG_SUSPEND);
}

static void synaptics_rmi4_late_resume(struct early_suspend *h)
{
	struct synaptics_rmi4 *ts;
	ts = container_of(h, struct synaptics_rmi4, early_suspend);
	synaptics_rmi4_resume(ts->client);
}
#endif


static const struct i2c_device_id synaptics_rmi4_id[] = {
	{ "synaptics-rmi-ts", 0 },
	{ }
};

static struct i2c_driver synaptics_rmi4_driver = {
	.probe		= synaptics_rmi4_probe,
	.remove		= synaptics_rmi4_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= synaptics_rmi4_suspend,
	.resume		= synaptics_rmi4_resume,
#endif
	.id_table	= synaptics_rmi4_id,
	.driver = {
		.name	= "synaptics-rmi-ts",
	},
};

static int __devinit synaptics_rmi4_init(void)
{
	synaptics_wq = create_singlethread_workqueue("synaptics_wq");
	if (!synaptics_wq)
	{
		printk(KERN_ERR "[TSP] Could not create work queue synaptics_wq: no memory");
		return -ENOMEM;
	}

	return i2c_add_driver(&synaptics_rmi4_driver);
}

static void __exit synaptics_rmi4_exit(void)
{
	i2c_del_driver(&synaptics_rmi4_driver);
	if (synaptics_wq)
		destroy_workqueue(synaptics_wq);
}

module_init(synaptics_rmi4_init);
module_exit(synaptics_rmi4_exit);

MODULE_DESCRIPTION("Synaptics RMI4 Driver");
MODULE_LICENSE("GPL");

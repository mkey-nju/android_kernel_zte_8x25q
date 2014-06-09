/*
 * include/linux/synaptics_i2c_rmi.h - platform data structure for f75375s sensor
 *
 * Copyright (C) 2008 Google, Inc.
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

#ifndef _LINUX_SYNAPTICS_I2C_RMI_H
#define _LINUX_SYNAPTICS_I2C_RMI_H

#include <linux/interrupt.h>
#include <linux/earlysuspend.h>

typedef __u8 u4;
typedef __u16 u12;

struct rmi_function_info {

	/** This is the number of data points supported - for example, for
	 *  function $11 (2D sensor) the number of data points is equal to the number
	 *  of fingers - for function $19 (buttons)it is eqaul to the number of buttons
	 */
	__u8 points_supported;
	__u8 controlBase;
	/** This is the interrupt register and mask - needed for enabling the interrupts
	 *  and for checking what source had caused the attention line interrupt.
	 */
	__u8 interrupt_offset;
	__u8 interrupt_mask;

	__u8 data_offset;
	__u8 data_length;

	__u8 ctrl_base;
	__u8 commandlBase;//zhangzhao 
	__u8 dataBase;//zhangzhao 
	__u8 querryBase;//zhangzhao 
};

enum f11_finger_status {
	f11_finger_none = 0,
	f11_finger_accurate = 1,
	f11_finger_inaccurate = 2,
};

struct f11_finger_data {
	enum f11_finger_status status;

	u12 x;
	u12 y;
	u8 z;

	unsigned int speed;
	bool active;
};

struct synaptics_rmi4 {
	struct i2c_client *client;
	struct input_dev *input_dev;
	int use_irq;
	struct hrtimer timer;
	struct work_struct  work;
	struct early_suspend early_suspend;

	__u8 data_reg;
	__u8 data_length;
	__u8 *data;
	struct i2c_msg data_i2c_msg[2];

	struct rmi_function_info f01;

	int hasF11;
	struct rmi_function_info f11;
	int f11_has_gestures;
	int f11_has_relative;
	int f11_max_x, f11_max_y;
	__u8 *f11_egr;
	bool hasEgrPinch;
	bool hasEgrPress;
	bool hasEgrFlick;
	bool hasEgrEarlyTap;
	bool hasEgrDoubleTap;
	bool hasEgrTapAndHold;
	bool hasEgrSingleTap;
	bool hasEgrPalmDetect;
	struct f11_finger_data *f11_fingers;
	
	int hasF19;
	struct rmi_function_info f19;

	int hasF30;
	struct rmi_function_info f30;

	int hasF34;
	struct rmi_function_info f34;


	int enable;


	/* Last X & Y seen, needed at finger lift.  Was down indicates at least one finger was here. */
	/* TODO: Eventually we'll need to track this info on a per finger basis. */
	bool wasdown;
	unsigned int oldX;
	unsigned int oldY;

};

/*added by wangweiping  ts begin*/
struct synaptics_ts_platform_data {
              u32 maxx;
              u32 maxy;
	u16	model;
	u16	x_plate_ohms;
	unsigned long irq_flags;
	int	(*init_platform_hw)(struct i2c_client *client);
	void(* reset)(void);
	int (*power_on)(struct i2c_client *client, int on);
};
/*added by wangweiping  ts end*/

#endif /* _LINUX_SYNAPTICS_I2C_RMI_H */

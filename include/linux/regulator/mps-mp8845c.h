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

#ifndef __MP8845C_H__
#define __MP8845C_H__

struct mp8845c_platform_data {
	struct regulator_init_data *init_data;
	int discharge_enable;
	int slew_rate_uV_per_us;
	int step_uV;
	int min_uV;
	int max_uV;
};

struct mp8845c_i2c_platform_data{
	struct i2c_board_info const *i2c_info;	
	int i2c_bus_id;
};
#endif
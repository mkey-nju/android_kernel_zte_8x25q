/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "msm_sensor.h"
#include "msm.h"
#include <linux/proc_fs.h>  //wangjianping add sensor  id
#define SENSOR_NAME "jt8ev5"
#define PLATFORM_DRIVER_NAME "msm_camera_jt8ev5"
#define jt8ev5_obj jt8ev5_##obj
#define MSB                             1
#define LSB                             0

#define EFFECT_ADD

#include "jt8ev5_v4l2.h"
#include "linux/kernel.h"

DEFINE_MUTEX(jt8ev5_mut);
static struct msm_sensor_ctrl_t jt8ev5_s_ctrl;

static int jt8ev5_scene_mode = -1;
static int jt8ev5_iso_mode = -1; //zhangzhao 2012-8-24  solve iso cant save problem
static int jt8ev5_awb_mode = -1;

static int af_count = 0;

static struct msm_camera_i2c_reg_conf jt8ev5_start_settings[] = {
	{0x0706, 0x0000},
};

static struct msm_camera_i2c_reg_conf jt8ev5_stop_settings[] = {
	{0x0706, 0x0001},
};

static struct msm_camera_i2c_reg_conf jt8ev5_groupon_settings[] = {
	//{0x0104, 0x01},
};

static struct msm_camera_i2c_reg_conf jt8ev5_groupoff_settings[] = {
	//{0x0104, 0x00},
};
static struct msm_camera_i2c_conf_array jt8ev5_fw_settings[] = {


};




static struct v4l2_subdev_info jt8ev5_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};
static struct msm_camera_i2c_conf_array jt8ev5_init_conf[] = {
	{&jt8ev5_recommend_settings_init[0],
	ARRAY_SIZE(jt8ev5_recommend_settings_init), 20, MSM_CAMERA_I2C_BYTE_DATA},
	{&jt8ev5_prev_settings[0],
	ARRAY_SIZE(jt8ev5_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_i2c_conf_array jt8ev5_confs[] = {
	{&jt8ev5_snap_settings[0],
	ARRAY_SIZE(jt8ev5_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&jt8ev5_prev_settings[0],
	ARRAY_SIZE(jt8ev5_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t jt8ev5_dimensions[] = {
	{
		.x_output = 2592,	//DangXiao for T8EV5
		.y_output = 1944,
		.line_length_pclk = 2592,
		.frame_length_lines = 1948,
		.vt_pixel_clk = 41600000,
		.op_pixel_clk = 41600000,
		.binning_factor = 1,
	},
	{
		.x_output = 1280,	//DangXiao for T8EV5
		.y_output = 960,
		.line_length_pclk = 1280,
		.frame_length_lines = 960,
		.vt_pixel_clk = 41600000,
		.op_pixel_clk =41600000 ,
		.binning_factor = 1,
	},
};

static struct msm_camera_csi_params jt8ev5_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 20,//0X5,0X24,
};

static struct msm_camera_csi_params *jt8ev5_csi_params_array[] = {
	&jt8ev5_csi_params,
	&jt8ev5_csi_params,
};


#ifdef EFFECT_ADD


//contras
static struct msm_camera_i2c_conf_array jt8ev5_contrast_confs[][1] = {
	{ {jt8ev5_contrast[0],
		ARRAY_SIZE(jt8ev5_contrast[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
		
	{ {jt8ev5_contrast[1],
		ARRAY_SIZE(jt8ev5_contrast[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	{ {jt8ev5_contrast[2],
		ARRAY_SIZE(jt8ev5_contrast[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
		
	{ {jt8ev5_contrast[3],
		ARRAY_SIZE(jt8ev5_contrast[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
		
	{ {jt8ev5_contrast[4],
		ARRAY_SIZE(jt8ev5_contrast[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
		
};

static int jt8ev5_contrast_enum_map[] = {
	CAMERA_CONTRAST_LV0,
	CAMERA_CONTRAST_LV1,
	CAMERA_CONTRAST_LV2,
	CAMERA_CONTRAST_LV3,
	CAMERA_CONTRAST_LV4,	
};

static struct msm_camera_i2c_enum_conf_array jt8ev5_contrast_enum_confs = {
	.conf = &jt8ev5_contrast_confs[0][0],
	.conf_enum = jt8ev5_contrast_enum_map,
	.num_enum = ARRAY_SIZE(jt8ev5_contrast_enum_map),
	.num_index = ARRAY_SIZE(jt8ev5_contrast_confs),
	.num_conf = ARRAY_SIZE(jt8ev5_contrast_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

static struct msm_camera_i2c_conf_array jt8ev5_saturation_confs[][1] =
{	
   {{jt8ev5_saturation[0],ARRAY_SIZE(jt8ev5_saturation[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
   {{jt8ev5_saturation[1],ARRAY_SIZE(jt8ev5_saturation[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
   {{jt8ev5_saturation[2],ARRAY_SIZE(jt8ev5_saturation[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
   {{jt8ev5_saturation[3],ARRAY_SIZE(jt8ev5_saturation[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
   {{jt8ev5_saturation[4],ARRAY_SIZE(jt8ev5_saturation[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},},
};
	
static int jt8ev5_saturation_enum_map[] = 
{	
   MSM_V4L2_SATURATION_L0,	
   MSM_V4L2_SATURATION_L1,	
   MSM_V4L2_SATURATION_L2,
   MSM_V4L2_SATURATION_L3,
   MSM_V4L2_SATURATION_L4,
  
};
		
static struct msm_camera_i2c_enum_conf_array jt8ev5_saturation_enum_confs = {	
	.conf = &jt8ev5_saturation_confs[0][0],	
       .conf_enum = jt8ev5_saturation_enum_map,
       .num_enum = ARRAY_SIZE(jt8ev5_saturation_enum_map),	
       .num_index = ARRAY_SIZE(jt8ev5_saturation_confs),	
       .num_conf = ARRAY_SIZE(jt8ev5_saturation_confs[0]),	
       .data_type = MSM_CAMERA_I2C_BYTE_DATA,
};


//sharpness
static struct msm_camera_i2c_conf_array jt8ev5_sharpness_confs[][1] = {
	{
		{jt8ev5_sharpness[0],
		ARRAY_SIZE(jt8ev5_sharpness[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_sharpness[1],
		ARRAY_SIZE(jt8ev5_sharpness[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_sharpness[2],
		ARRAY_SIZE(jt8ev5_sharpness[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_sharpness[3],
		ARRAY_SIZE(jt8ev5_sharpness[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_sharpness[4],
		ARRAY_SIZE(jt8ev5_sharpness[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},	
	
	
};

static int jt8ev5_sharpness_enum_map[] = {
	MSM_V4L2_SHARPNESS_L0,
	MSM_V4L2_SHARPNESS_L1,
	MSM_V4L2_SHARPNESS_L2,
	MSM_V4L2_SHARPNESS_L3,
	MSM_V4L2_SHARPNESS_L4,	
		
};

static struct msm_camera_i2c_enum_conf_array jt8ev5_sharpness_enum_confs = {
	.conf = &jt8ev5_sharpness_confs[0][0],
	.conf_enum = jt8ev5_sharpness_enum_map,
	.num_enum = ARRAY_SIZE(jt8ev5_sharpness_enum_map),
	.num_index = ARRAY_SIZE(jt8ev5_sharpness_confs),
	.num_conf = ARRAY_SIZE(jt8ev5_sharpness_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

//brightness
static struct msm_camera_i2c_conf_array jt8ev5_brightness_confs[][1] = {
	
	{{jt8ev5_brightness[0],
		ARRAY_SIZE(jt8ev5_brightness[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
	
	{{jt8ev5_brightness[1],
		ARRAY_SIZE(jt8ev5_brightness[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
		
	{{jt8ev5_brightness[2],
		ARRAY_SIZE(jt8ev5_brightness[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
		
	{{jt8ev5_brightness[3],
		ARRAY_SIZE(jt8ev5_brightness[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
		
	{{jt8ev5_brightness[4],
		ARRAY_SIZE(jt8ev5_brightness[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},},
		
		
};

static int jt8ev5_brightness_enum_map[] = {
	CAMERA_BRIGHTNESS_LV0,
	CAMERA_BRIGHTNESS_LV1,
	CAMERA_BRIGHTNESS_LV2,
	CAMERA_BRIGHTNESS_LV3,
	CAMERA_BRIGHTNESS_LV4,	
};

static struct msm_camera_i2c_enum_conf_array jt8ev5_brightness_enum_confs = {
	.conf = &jt8ev5_brightness_confs[0][0],
	.conf_enum = jt8ev5_brightness_enum_map,
	.num_enum = ARRAY_SIZE(jt8ev5_brightness_enum_map),
	.num_index = ARRAY_SIZE(jt8ev5_brightness_confs),
	.num_conf = ARRAY_SIZE(jt8ev5_brightness_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};




static struct msm_camera_i2c_conf_array jt8ev5_exposure_confs[][1] = {
	
	{{jt8ev5_exposure[0],
		ARRAY_SIZE(jt8ev5_exposure[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
	
	{{jt8ev5_exposure[1],
		ARRAY_SIZE(jt8ev5_exposure[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
		
	{{jt8ev5_exposure[2],
		ARRAY_SIZE(jt8ev5_exposure[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
		
	{{jt8ev5_exposure[3],
		ARRAY_SIZE(jt8ev5_exposure[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
		
	{{jt8ev5_exposure[4],
		ARRAY_SIZE(jt8ev5_exposure[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},},
		
	
};

static int jt8ev5_exposure_enum_map[] = {
	MSM_V4L2_EXPOSURE_N2,
	MSM_V4L2_EXPOSURE_N1,
	MSM_V4L2_EXPOSURE_D,
	MSM_V4L2_EXPOSURE_P1,
	MSM_V4L2_EXPOSURE_P2,
};


static struct msm_camera_i2c_enum_conf_array jt8ev5_exposure_enum_confs = {
	.conf = &jt8ev5_exposure_confs[0][0],
	.conf_enum = jt8ev5_exposure_enum_map,
	.num_enum = ARRAY_SIZE(jt8ev5_exposure_enum_map),
	.num_index = ARRAY_SIZE(jt8ev5_exposure_confs),
	.num_conf = ARRAY_SIZE(jt8ev5_exposure_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};




static struct msm_camera_i2c_conf_array jt8ev5_effect_confs[][1] = {
	{
		{jt8ev5_effect[0],
		ARRAY_SIZE(jt8ev5_effect[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
		
	{
		{jt8ev5_effect[1],
		ARRAY_SIZE(jt8ev5_effect[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_effect[2],
		ARRAY_SIZE(jt8ev5_effect[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_effect[3],
		ARRAY_SIZE(jt8ev5_effect[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_effect[4],
		ARRAY_SIZE(jt8ev5_effect[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	{
		{jt8ev5_effect[5],
		ARRAY_SIZE(jt8ev5_effect[5]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	{
		{jt8ev5_effect[6],
		ARRAY_SIZE(jt8ev5_effect[6]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	{
		{jt8ev5_effect[7],
		ARRAY_SIZE(jt8ev5_effect[7]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
			
};

static int jt8ev5_effect_enum_map[] = {
	MSM_V4L2_EFFECT_OFF,
	MSM_V4L2_EFFECT_MONO,
	MSM_V4L2_EFFECT_NEGATIVE,
	MSM_V4L2_EFFECT_SOLARIZE,
	MSM_V4L2_EFFECT_SEPIA,
	MSM_V4L2_EFFECT_REDISH,
	MSM_V4L2_EFFECT_BLUEISH,
	MSM_V4L2_EFFECT_GREENISH,
};

static struct msm_camera_i2c_enum_conf_array jt8ev5_effect_enum_confs = {
	.conf = &jt8ev5_effect_confs[0][0],
	.conf_enum = jt8ev5_effect_enum_map,
	.num_enum = ARRAY_SIZE(jt8ev5_effect_enum_map),
	.num_index = ARRAY_SIZE(jt8ev5_effect_confs),
	.num_conf = ARRAY_SIZE(jt8ev5_effect_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

//awb
static struct msm_camera_i2c_conf_array jt8ev5_awb_confs[][1] = {
	{
		{jt8ev5_awb[0],
		ARRAY_SIZE(jt8ev5_awb[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_awb[1],
		ARRAY_SIZE(jt8ev5_awb[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_awb[2],
		ARRAY_SIZE(jt8ev5_awb[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_awb[3],
		ARRAY_SIZE(jt8ev5_awb[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_awb[4],
	       ARRAY_SIZE(jt8ev5_awb[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_awb[5],
		ARRAY_SIZE(jt8ev5_awb[5]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_awb[6],
	       ARRAY_SIZE(jt8ev5_awb[6]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},	
	
};

static int jt8ev5_awb_enum_map[] = {	
	MSM_V4L2_WB_OFF,//not used
	MSM_V4L2_WB_AUTO ,//= 1
	MSM_V4L2_WB_CUSTOM,  //not used
	MSM_V4L2_WB_INCANDESCENT, //°×³ã
	MSM_V4L2_WB_FLUORESCENT,   //Ó«¹â
	MSM_V4L2_WB_DAYLIGHT,
	MSM_V4L2_WB_CLOUDY_DAYLIGHT,
};

static struct msm_camera_i2c_enum_conf_array jt8ev5_awb_enum_confs = {
	.conf = &jt8ev5_awb_confs[0][0],
	.conf_enum = jt8ev5_awb_enum_map,
	.num_enum = ARRAY_SIZE(jt8ev5_awb_enum_map),
	.num_index = ARRAY_SIZE(jt8ev5_awb_confs),
	.num_conf = ARRAY_SIZE(jt8ev5_awb_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};


static struct msm_camera_i2c_conf_array jt8ev5_antibanding_confs[][1] = {
	{
		{jt8ev5_antibanding[0],
		ARRAY_SIZE(jt8ev5_antibanding[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_antibanding[1],
		ARRAY_SIZE(jt8ev5_antibanding[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_antibanding[2],
		ARRAY_SIZE(jt8ev5_antibanding[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_antibanding[3],
		ARRAY_SIZE(jt8ev5_antibanding[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
};

static int jt8ev5_antibanding_enum_map[] = {	
	MSM_V4L2_POWER_LINE_OFF,  //not used
	MSM_V4L2_POWER_LINE_60HZ,
	MSM_V4L2_POWER_LINE_50HZ,
	MSM_V4L2_POWER_LINE_AUTO,

};

static struct msm_camera_i2c_enum_conf_array jt8ev5_antibanding_enum_confs = {
	.conf = &jt8ev5_antibanding_confs[0][0],
	.conf_enum = jt8ev5_antibanding_enum_map,
	.num_enum = ARRAY_SIZE(jt8ev5_antibanding_enum_map),
	.num_index = ARRAY_SIZE(jt8ev5_antibanding_confs),
	.num_conf = ARRAY_SIZE(jt8ev5_antibanding_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};


//ISO

static struct msm_camera_i2c_conf_array jt8ev5_iso_confs[][1] = {
	{
		{jt8ev5_iso[0],
		ARRAY_SIZE(jt8ev5_iso[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_iso[1],
		ARRAY_SIZE(jt8ev5_iso[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_iso[2],
		ARRAY_SIZE(jt8ev5_iso[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_iso[3],
		ARRAY_SIZE(jt8ev5_iso[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_iso[4],
	  ARRAY_SIZE(jt8ev5_iso[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
{
		{jt8ev5_iso[5],
	  ARRAY_SIZE(jt8ev5_iso[5]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},	
	
		/*{
		{jt8ev5_iso[6],
	  ARRAY_SIZE(jt8ev5_iso[6]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},	*/
	
};

static int jt8ev5_iso_enum_map[] = {	
	MSM_V4L2_ISO_AUTO,
	//MSM_V4L2_ISO_DEBLUR,//not used
	MSM_V4L2_ISO_100,
	MSM_V4L2_ISO_200,
	MSM_V4L2_ISO_400,
	MSM_V4L2_ISO_800,
	MSM_V4L2_ISO_1600,
};

static struct msm_camera_i2c_enum_conf_array jt8ev5_iso_enum_confs = {
	.conf = &jt8ev5_iso_confs[0][0],
	.conf_enum = jt8ev5_iso_enum_map,
	.num_enum = ARRAY_SIZE(jt8ev5_iso_enum_map),
	.num_index = ARRAY_SIZE(jt8ev5_iso_confs),
	.num_conf = ARRAY_SIZE(jt8ev5_iso_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};


//scen
static struct msm_camera_i2c_conf_array jt8ev5_scene_confs[][1] = {
	{
		{jt8ev5_scene[0],
		ARRAY_SIZE(jt8ev5_scene[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_scene[1],
		ARRAY_SIZE(jt8ev5_scene[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_scene[2],
		ARRAY_SIZE(jt8ev5_scene[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	{
		{jt8ev5_scene[3],
		ARRAY_SIZE(jt8ev5_scene[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	{
		{jt8ev5_scene[4],
		ARRAY_SIZE(jt8ev5_scene[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_scene[5],
		ARRAY_SIZE(jt8ev5_scene[5]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	{
		{jt8ev5_scene[6],
		ARRAY_SIZE(jt8ev5_scene[6]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_scene[7],
		ARRAY_SIZE(jt8ev5_scene[7]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	{
		{jt8ev5_scene[8],
		ARRAY_SIZE(jt8ev5_scene[8]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_scene[9],
		ARRAY_SIZE(jt8ev5_scene[9]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{jt8ev5_scene[10],
		ARRAY_SIZE(jt8ev5_scene[10]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
};

static int jt8ev5_scene_enum_map[] = {	
	MSM_V4L2_SCENE_AUTO,
	MSM_V4L2_SCENE_LANDSCAPE,
	MSM_V4L2_SCENE_FIREWORK,
	MSM_V4L2_SCENE_BEACH,
	MSM_V4L2_SCENE_PARTY,
	MSM_V4L2_SCENE_PORTRAIT,
	MSM_V4L2_SCENE_SUNSET,
	MSM_V4L2_SCENE_SNOW,
	MSM_V4L2_SCENE_NIGHT,
	MSM_V4L2_SCENE_SPORTS,
	MSM_V4L2_SCENE_CANDLELIGHT,

};

static struct msm_camera_i2c_enum_conf_array jt8ev5_scene_enum_confs = {
	.conf = &jt8ev5_scene_confs[0][0],
	.conf_enum = jt8ev5_scene_enum_map,
	.num_enum = ARRAY_SIZE(jt8ev5_scene_enum_map),
	.num_index = ARRAY_SIZE(jt8ev5_scene_confs),
	.num_conf = ARRAY_SIZE(jt8ev5_scene_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

//cmd=0x03:single focus  ,cmd=0x08:cancel focus
#define AG_1X 0x3E
#define PV_HCOUNT 1440
#define CP_HCOUNT 2808
#define CP_MAXES   0x7B0 //0xF60




int jt8ev5_set_af_result(struct msm_sensor_ctrl_t *s_ctrl)//,uint16_t cmd)
{
   	int rc = 0;	 
	// uint16_t regVal = 0;
 	// unsigned int nCounter = 0;
	 
	 printk("+++++++++++: Enter %s: \n",__func__);


          rc =msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,&jt8ev5_snap_autofocus_settings[0],ARRAY_SIZE(jt8ev5_snap_autofocus_settings),1);


	   //AF Start cmd
	 //  rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x27ff,0x28,1);
/*
	   nCounter = 0;
		regVal = 0;
		do{
		  msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x2800,&regVal,1);
		  regVal=(regVal& 0x80);
		  nCounter++;
		  msleep(10);
		  CDBG("%s, t8ev5 sequence status is %d, nCounter = %d\n",__func__,regVal,nCounter);
		}while((regVal > 0)&& (nCounter < 200));
		// Return Value for AF_OK
		if(regVal==0x0){
		  	rc=0;
		}
		else{
		  	rc=1;
		}
	       printk("--------------:  %s: %d \n",__func__,rc);
*/		
		return rc;

}

EXPORT_SYMBOL(jt8ev5_set_af_result);

int jt8ev5_AF_proc(struct msm_sensor_ctrl_t *s_ctrl,
		                          struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	// int rc = 0;
	// int count  = 0;
	// uint16_t ack=0;	 
	 printk("+++++++++++++: Enter %s: \n",__func__);
        return (jt8ev5_set_af_result(s_ctrl));

}

int jt8ev5_emode_flashtest(struct msm_sensor_ctrl_t *s_ctrl,
                            struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
    int rc = 0;

    printk("fuyipeng---%s enter  value is === %d\n", __func__,value);
    
#ifdef CONFIG_ZTE_CAMERA_SOC_FLASH
    if(value == 2)
    {
        camera_flash_set_led_state(MSM_CAMERA_LED_HIGH, 0x0a);
        return rc;
    }
#endif

    return rc;

}

int jt8ev5_get_autofocus_status(struct msm_sensor_ctrl_t *s_ctrl,
                            struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
    int iFocusStatus = 0;
   // int rc;
    uint16_t regVal = 0,temp_reg=0;
 //   unsigned int nCounter = 0;

    CDBG("fuyipeng---%s enter-----\n", __func__);

    msleep(10);

    af_count++;
   msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x2800,&regVal,1);
    pr_err("%s, t8ev5 sequence status is %x, nCounter = %d\n",__func__,regVal,af_count);
   temp_reg = regVal;
    regVal=(regVal& 0x80);
		// Return Value for AF_OK

    if (regVal == 0)
    {
    pr_err("%s,++++++++++++= t8ev5 sequence status is +++++++++%x, nCounter = %d\n",__func__,temp_reg,af_count);
    temp_reg=(temp_reg& 0x10);
    if(temp_reg !=0)
		{
                 if (af_count > 30)
    	          {
    	              iFocusStatus = 1;
    	               af_count = 0;
                      msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x27FF,0xF7,1);
         	        msleep(50);
    	           }
                  return iFocusStatus;
		}
	else
		{
                  iFocusStatus = 1;
                  af_count = 0;
         	    msleep(50);
		}
    }
    else
    {
        if (af_count > 50)
        {
            iFocusStatus = 2;
            af_count = 0;
        }
    }
    return iFocusStatus;
}

static int jt8ev5_scene_sensor_s_ctrl_by_enum(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{

	int rc = 0;
	CDBG("%s enter\n", __func__);
	CDBG("%s:setting enum is %x,value =%d\n",__func__,ctrl_info->ctrl_id,value);
	jt8ev5_scene_mode = value;
	rc = msm_sensor_s_ctrl_by_enum(s_ctrl,ctrl_info, value);
	return rc;


}

//zhangzhao 2012-8-24  solve iso cant save problem start
static int jt8ev5_iso_sensor_s_ctrl_by_enum(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	printk("%s enter\n", __func__);
	printk("%s:setting enum is %x,value =%d\n",__func__,ctrl_info->ctrl_id,value);
	jt8ev5_iso_mode = value;
	rc = msm_sensor_s_ctrl_by_enum(s_ctrl,ctrl_info, value);
	return rc;
}
//zhangzhao 2012-8-24  solve iso cant save problem end

static int jt8ev5_awb_sensor_s_ctrl_by_enum(struct msm_sensor_ctrl_t *s_ctrl,
        struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
    int rc = 0;

    jt8ev5_awb_mode = value;
    rc = msm_sensor_s_ctrl_by_enum(s_ctrl,ctrl_info, value);
    return rc;
}

/* [ECID:0000] zhangzhao 2012-7-15 add panorama start*/
static struct msm_camera_i2c_reg_conf jt8ev5_panorama_20fps[] = {	
{0x0107,0x00},//;-/-/-/-/-/V_COUNT[10:8]
{0x0108,0xB5},//;V_COUNT[7:0]
{0x0229,0x2B},//;Cbr_MGAIN1[3:0]/Cbr_MGAIN0[3:0]
{0x022A,0x00},//;-/-/-/-/Cbr_MGAIN2[3:0]
{0x0249,0x87},//;CONT_LEV[7:0]
{0x0315,0x08},//;FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]
{0x0302,0x01},//;-/-/-/-/-/-/ALCAIM[9:8]
{0x0303,0x10},//;ALCAIM[7:0]
{0x030C,0x55},//;A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
{0x030D,0x5A},//;A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
{0x030E,0x95},//;B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
{0x030F,0x50},//;C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
{0x032C,0x2C},//;-/RYCUT0P[6:0]
{0x032F,0x28},//;-/BYCUT0N[6:0]
};

struct msm_camera_i2c_conf_array jt8ev5_panorama_reg[] = {

//ECID:0000 2012-6-18 zhangzhao optimize the camera start up time start

	{&jt8ev5_panorama_20fps[0],
	ARRAY_SIZE(jt8ev5_panorama_20fps), 0, MSM_CAMERA_I2C_BYTE_DATA},
	
};

int jt8ev5_panorama_mode(struct msm_sensor_ctrl_t *s_ctrl,
                            struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
    int rc = 0;

    printk("fuyipeng---%s enter  value is === %d\n", __func__,value);

    if(value)  
    	{
	rc = msm_sensor_write_all_conf_array(
		s_ctrl->sensor_i2c_client,
		&jt8ev5_panorama_reg[0],
		ARRAY_SIZE(jt8ev5_panorama_reg));
	if (rc<0)
	 {
	     printk("I2C write error!-------");
	     return rc;
	 }

	pr_err("---panorama 20fps----%s: %d\n", __func__, __LINE__);
	return rc;

    	}
	pr_err("---not in panorama mode 25 fps----%s: %d\n", __func__, __LINE__);
	
	return rc;

}
/* [ECID:0000] zhangzhao 2012-7-15 add panorama end*/

int jt8ev5_ctrl_cancel_autofocus(struct msm_sensor_ctrl_t *s_ctrl,
                            struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
    int rc = -1;
    af_count = 0;
   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x2401,0x15,1);
   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x2402,0x33,1);
   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x27FF,0xe1,1);
   msleep(5);
   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x27FF,0xF7,1);

    return rc;
}


struct msm_sensor_v4l2_ctrl_info_t jt8ev5_v4l2_ctrl_info[] = {
	{
		.ctrl_id = V4L2_CID_SATURATION,
		.min = MSM_V4L2_SATURATION_L0,
		.max = MSM_V4L2_SATURATION_L4,
		.step = 1,
		.enum_cfg_settings = &jt8ev5_saturation_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},
	
	{
		.ctrl_id = V4L2_CID_CONTRAST,
		.min = CAMERA_CONTRAST_LV0,
		.max = CAMERA_CONTRAST_LV4,
		.step = 1,
		.enum_cfg_settings = &jt8ev5_contrast_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},

	{
		.ctrl_id = V4L2_CID_SHARPNESS,
		.min = MSM_V4L2_SHARPNESS_L0,
		.max = MSM_V4L2_SHARPNESS_L4,
		.step = 1,
		.enum_cfg_settings = &jt8ev5_sharpness_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},

	{
		.ctrl_id = V4L2_CID_BRIGHTNESS,
		.min = CAMERA_BRIGHTNESS_LV0,
		.max = CAMERA_BRIGHTNESS_LV4,
		.step = 1,
		.enum_cfg_settings = &jt8ev5_brightness_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},
	
      { 
		.ctrl_id = V4L2_CID_EXPOSURE,
		.min = MSM_V4L2_EXPOSURE_N2,
		.max = MSM_V4L2_EXPOSURE_P2,
		.step = 1,
		.enum_cfg_settings = &jt8ev5_exposure_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},  
	
	{
		.ctrl_id = V4L2_CID_COLORFX,
		.min = CAMERA_EFFECT_OFF,
		.max = CAMERA_EFFECT_SEPIA,
		.step = 1,
		.enum_cfg_settings = &jt8ev5_effect_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},

	{
		.ctrl_id = V4L2_CID_AUTO_WHITE_BALANCE,
		.min = MSM_V4L2_WB_AUTO,
		.max = MSM_V4L2_WB_CLOUDY_DAYLIGHT,
		.step = 1,
		.enum_cfg_settings = &jt8ev5_awb_enum_confs,
        .s_v4l2_ctrl = jt8ev5_awb_sensor_s_ctrl_by_enum, 
	},	
	
	{
		.ctrl_id = V4L2_CID_POWER_LINE_FREQUENCY,//antibanding
		.min = MSM_V4L2_POWER_LINE_60HZ,
		.max = MSM_V4L2_POWER_LINE_AUTO,
		.step = 1,
		.enum_cfg_settings = &jt8ev5_antibanding_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},  

       {
		.ctrl_id = V4L2_CID_ISO,//antibanding
		.min = MSM_V4L2_ISO_AUTO,
		.max = MSM_V4L2_ISO_1600,
		.step = 1,
		.enum_cfg_settings = &jt8ev5_iso_enum_confs,
		.s_v4l2_ctrl = jt8ev5_iso_sensor_s_ctrl_by_enum,//zhangzhao 2012-8-24  solve iso cant save problem
	},  

	{
		.ctrl_id = V4L2_CID_SCENE,
		.min = MSM_V4L2_SCENE_AUTO,
		.max = MSM_V4L2_SCENE_CANDLELIGHT,
		.step = 1,
		.enum_cfg_settings = &jt8ev5_scene_enum_confs,
		.s_v4l2_ctrl = jt8ev5_scene_sensor_s_ctrl_by_enum,
	},	
	
	{
		.ctrl_id = V4L2_CID_FOCUS_AUTO,
		//.min = MSM_V4L2_FOCUS_NORMAL,
		//.max = MSM_V4L2_FOCUS_MAX,
		//.step = 1,
		//.enum_cfg_settings = &jt8ev5_awb_enum_confs,
		.s_v4l2_ctrl = jt8ev5_AF_proc,
	},	     	
/* [ECID:0000] zhangzhao 2012-7-15 add panorama start*/
    {
        .ctrl_id = V4L2_CID_PANORAMA_MODE,

        .s_v4l2_ctrl = jt8ev5_panorama_mode,
    },
/* [ECID:0000] zhangzhao 2012-7-15 add panorama end*/


    {
        .ctrl_id = V4L2_CID_GET_AUTOFOCUS_STATUS,

        .s_v4l2_ctrl = jt8ev5_get_autofocus_status,
    },

    {
        .ctrl_id = V4L2_CID_CANCEL_AUTOFOCUS,

        .s_v4l2_ctrl = jt8ev5_ctrl_cancel_autofocus,
    },

    {
        .ctrl_id = V4L2_CID_EMODE_FLASHTEST,

        .s_v4l2_ctrl = jt8ev5_emode_flashtest,
    },

};
#endif	

static struct msm_sensor_output_reg_addr_t jt8ev5_reg_addr = {
	.x_output = 0,//0x034C,
	.y_output = 0,//0x034E,
	.line_length_pclk = 0,//0x0342,
	.frame_length_lines = 0,//0x0340,
};

#if 0
static struct msm_camera_i2c_reg_conf sensor_reg_addr_ext[]={
       {0x002c,0x0000},
       {0x002e,0x0040},
};
static struct msm_camera_i2c_conf_array sensor_reg_addr_ext_i2c[] = {
	{&sensor_reg_addr_ext[0],
	ARRAY_SIZE(sensor_reg_addr_ext), 0, MSM_CAMERA_I2C_WORD_DATA}
};
#endif
static struct msm_sensor_id_info_t jt8ev5_id_info = {
	//.sensor_id_reg_addr_ext = &sensor_reg_addr_ext_i2c[0],		//DangXiao for T8EV5
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x0010,
};

static struct msm_sensor_exp_gain_info_t jt8ev5_exp_gain_info = {
	.coarse_int_time_addr =0,// 0x0202,
	.global_gain_addr = 0,//0x0204,
	.vert_offset = 4,
};





int32_t jt8ev5_sensor_set_fps(struct msm_sensor_ctrl_t *s_ctrl,
		struct fps_cfg *fps)
{
	CDBG("jt8ev5_sensor_set_fps: Not supported\n");
	return 0;
}

//ECID:0000 zhangzhao 2012-6-25 add ev algorithm end

static ssize_t camera_id_read_proc(char *page,char **start,off_t off,int count,int *eof,void* data)
{		 	
    int ret;
	
    unsigned char *camera_status = "BACK Camera ID:Jt8ev5-5.0M";	
    ret = strlen(camera_status);	 	 
    sprintf(page,"%s\n",camera_status);	 	 
    return (ret + 1);	
}
static void camera_proc_file(void)
{	
    struct proc_dir_entry *proc_file  = create_proc_entry("driver/camera_id_back",0644,NULL);	
    if(proc_file)	
     {		
  	     proc_file->read_proc = camera_id_read_proc;			
     }	
    else	
     {		
        printk(KERN_INFO "camera_proc_file error!\r\n");	
     }
}




int32_t jt8ev5_sensor_set_af_rect(struct msm_sensor_ctrl_t *s_ctrl, 
                                        struct msm_sensor_af_rect_data *rect)
{
    int32_t rc = 0;
    uint8_t reg_x,reg_y,reg_position;
    int xPoint = 0;
    int yPoint = 0;
    int temp = 0;
    
    if (NULL == s_ctrl || NULL == rect)
    {
        return rc;
    }

    printk("rect.x:%d   rect.y:%d \n", rect->x, rect->y);

    xPoint = rect->x + rect->dx / 2;
    yPoint = rect->y + rect->dy / 2;

    //xPoint = abs(640-xPoint);
    //yPoint = abs(480-yPoint);

    //printk("--------------rect.x:%d   rect.y:%d \n", xPoint, yPoint);

    if( xPoint <= 119)
    {
        reg_x = 0;
    }
    else if( xPoint >= 120 && xPoint <= 199)
    {
        reg_x = 1;
    }
    else if( xPoint >= 200 && xPoint <= 279)
    {
        reg_x = 2;
    }
    else if( xPoint >= 280 && xPoint <= 359)
    {
        reg_x = 3;
    }
    else if( xPoint >= 360 && xPoint <= 439)
    {
        reg_x = 4;
    }
    else if( xPoint >= 440 && xPoint <= 519)
    {
        reg_x = 5;
    }
    else if( xPoint >= 520)
    {
        reg_x = 6;
    }
    else
    {
        reg_x = 3;
    }

    if( yPoint < 160)
    {
        reg_y = 0;
    }
    else if (yPoint >=160 && yPoint <= 320)
    {
        reg_y = 3;
    }
    else if (yPoint > 320) 
    {
        reg_y = 6;
    }
    else
    {
        reg_y = 3;
    }

    // change x,y
    {
        temp = reg_x;
        reg_x = reg_y;
        reg_y = temp;
    }

    reg_position = (reg_x<<4)|(reg_y&0x0f);
    pr_err("reg_position ==== %x\n",reg_position);
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x2401,0x15,1);
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x2402,reg_position,1);
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x27ff,0xe1,1);

    //jt8ev5_AF_proc(s_ctrl);

    return rc;
}


#if 0
int32_t jt8ev5_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	struct msm_camera_sensor_info *s_info;
	
	s_info = client->dev.platform_data;
	if (s_info == NULL) {
		pr_err("%s %s NULL sensor data\n", __func__, client->name);
		return -EFAULT;
	}
      rc = jt8ev5_probe_init_gpio(s_info);
      if(rc < 0)
      	{
		pr_err("%s probe faild--gpio requeset faild-----\n", __func__);
		return -EFAULT;
	  }
	
	rc = msm_sensor_i2c_probe(client, id);
      if(rc < 0)
      	 {
		pr_err("%s probe faild-------\n", __func__);
		goto probe_faided;
	  }
	pr_err("%s probe OK+++++++++++\n", __func__);
	return rc;
	
probe_faided:
	jt8ev5_release_gpio();
	return rc;
}
#else
static int32_t jt8ev5_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	rc = msm_sensor_i2c_probe(client,id);
	
    if(rc == 0)
    {
        camera_proc_file();
    }
	return rc;
}
//ECID:0000 2012-6-18 zhangzhao optimize the camera start up time start
#endif
static const struct i2c_device_id jt8ev5_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&jt8ev5_s_ctrl},
	{ }
};

static struct i2c_driver jt8ev5_i2c_driver = {
	.id_table = jt8ev5_i2c_id,
	.probe  = jt8ev5_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client jt8ev5_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,		//DangXiao for T8EV5
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&jt8ev5_i2c_driver);
}

static struct v4l2_subdev_core_ops jt8ev5_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops jt8ev5_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops jt8ev5_subdev_ops = {
	.core = &jt8ev5_subdev_core_ops,
	.video  = &jt8ev5_subdev_video_ops,
};

static uint16_t pv_es;	

static int32_t jt8ev5_hw_ae_transfer(struct msm_sensor_ctrl_t *s_ctrl)
{

   	int rc = 0;	 
	 uint16_t esh,esl,agh,agl,dgh,dgl,dgain,es,ag;
	 uint16_t rh,rl,gh,gl,bh,bl,temp_awb,r_gain,b_gain;
	 uint32_t temp;
	 uint32_t pv_val, cp_val;
	// uint16_t regVal = 0;
 	// unsigned int nCounter = 0;

 #if 1
	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x300,0x03,1);//for ae start
	 rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x35f,&esh,1);
	 rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x360,&esl,1);
	 rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x361,&agh,1);
	 rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x362,&agl,1);
	 rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x363,&dgh,1);
	 rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x364,&dgl,1);
	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x300,0x00,1);

	 dgain=(((dgh&0x3)<<8)|dgl)>>2;
	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x30B,dgain,1);// DGain
	 ag=((agh&0x0F)<<8)|agl;
	 es=(esh<<8)|esl;
        pv_es = ag*es/AG_1X ;
	 pv_val = ag*es*PV_HCOUNT/AG_1X ;
	 cp_val = CP_HCOUNT*CP_MAXES;
	 if(pv_val>cp_val){
	 	pr_err("Ninja: MAXES write! \n");
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x307,(CP_MAXES&0xFF00)>>8,1);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x308,CP_MAXES&0xFF,1);
		
		temp = pv_val*AG_1X/cp_val;
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x309,0x40|((temp&0x0F00)>>8),1);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x30A,temp&0xFF,1);	
	 }
	 else{
		temp = pv_val/CP_HCOUNT;
		
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x307, (temp&0xFF00)>>8,1);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x308, temp & 0xFF,1);
		
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x309,0x40|(AG_1X&0xF00)>>8,1);
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x30A,AG_1X&0xFF,1);	
	 }
	 
	   //for awb start
	   rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x36e,&rh,1);
	   rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x36f,&rl,1);
	   rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x370,&gh,1);
	   rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x371,&gl,1);
	   rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x372,&bh,1);
	   rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x373,&bl,1);
	   temp=(rh&0x3);
	   
          r_gain = ((rh&0x03)<<8)|rl;
          b_gain = ((bh&0x03)<<8)|bl;
	   if((r_gain < 0xD0)&&(b_gain > 0x167))
	   	{
		CDBG("Ninja: temp is +++++++++++++++! \n");
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0204,0x1B,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0205,0x01,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0206,0x30,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0207,0x3C,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0208,0x2D,1);//0X1D
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0209,0x54,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0223,0x47,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0224,0x83,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0225,0x80,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0226,0x83,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0227,0x6E,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0228,0x80,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0229,0x2D,1);
	   }

	   
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x320,temp,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x321,rl,1);
	   rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x322,&temp_awb,1);
	   temp=(temp_awb&0xfc)|(gh&0x3);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x322,temp,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x323,gl,1);
	   rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x324,&temp_awb,1);				
	   temp=(temp_awb&0xfc)|(bh&0x3);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x324,temp,1);
	   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x325,bl,1);   
	   //for awb end
#endif
	return rc;
}

static int32_t jt8ev5_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;

	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		CDBG("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		msm_sensor_write_init_settings(s_ctrl);
		
		csi_config = 0;
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		 CDBG("PERIODIC : %d\n", res);
		// printk("%s:sensor_name = %s, res = %d\n",__func__, s_ctrl->sensordata->sensor_name, res);

		switch(res)
		{
			case 0://snapshot
				{
	
#ifdef CONFIG_ZTE_CAMERA_SOC_FLASH
				{
				uint16_t reg_agh,reg_agl,ag_value;
				led_mode_t  flash_mode;
				flash_mode = msm_flash_mode_get();
				printk("%s, flash mode %d\n", __func__, flash_mode);
                      	 rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x361,&reg_agh,1);
				if(rc<0)
					{
                            printk("camera read reg_value wrong\n");
                            return -EIO;
				}
                      	 rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x362,&reg_agl,1);
				if(rc<0)
					{
                            printk("camera read reg_value wrong\n");
                            return -EIO;
				}
				ag_value=((reg_agh&0x0F)<<8)|reg_agl;
				ag_value = ag_value /AG_1X;
				printk("%s, expo value %d\n", __func__, ag_value);					
				if (((ag_value >= 8) && (flash_mode == LED_MODE_AUTO)) || (flash_mode == LED_MODE_ON)) {
					camera_flash_set_led_state(MSM_CAMERA_LED_HIGH, 0x0a);	
				}
				}
#endif				

				rc=jt8ev5_hw_ae_transfer(s_ctrl);
				if(rc<0)
					{
					CDBG("jt8ev5_hw_ae_transfer ERROR\n");
					return rc;
				}
				
                    		msm_sensor_write_conf_array(
                    			s_ctrl->sensor_i2c_client,
                    			s_ctrl->msm_sensor_reg->mode_settings, res);
                    		msleep(30);
                    		if (!csi_config) {
                    			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
                    			printk("CSI config in progress\n");
                    			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                    				NOTIFY_CSIC_CFG,
                    				s_ctrl->curr_csic_params);
                    			printk("CSI config is done\n");
                    			mb();
                    			msleep(30);
                    			csi_config = 1;
                    		}
                    		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                    			NOTIFY_PCLK_CHANGE,
                    			&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);

				s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
                    		msleep(50);	
				break;			
			     }
			case 1://preview
			default :
				{
                    		msm_sensor_write_conf_array(
                    			s_ctrl->sensor_i2c_client,
                    			s_ctrl->msm_sensor_reg->mode_settings, res);

				//msm_sensor_s_ctrl_by_enum(s_ctrl,&jt8ev5_v4l2_ctrl_info[9], jt8ev5_scene_mode);
				if(jt8ev5_scene_mode>=0)
					{
				msm_sensor_write_enum_conf_array(
                           		s_ctrl->sensor_i2c_client,
                          		&jt8ev5_scene_enum_confs, jt8ev5_scene_mode);
					}
                    		msleep(10);
				if(jt8ev5_iso_mode>=0)//zhangzhao 2012-8-24  solve iso cant save problem
					{
				msm_sensor_write_enum_conf_array(
                           		s_ctrl->sensor_i2c_client,
                          		&jt8ev5_iso_enum_confs, jt8ev5_iso_mode);
					}

                if (jt8ev5_awb_mode > MSM_V4L2_WB_AUTO && jt8ev5_scene_mode <= MSM_V4L2_SCENE_AUTO)
                {
                    msm_sensor_write_enum_conf_array(s_ctrl->sensor_i2c_client, 
                                       &jt8ev5_awb_enum_confs, jt8ev5_awb_mode);
                }

							
                    		if (!csi_config) {
                    			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
                    			CDBG("CSI config in progress\n");
                    			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                    				NOTIFY_CSIC_CFG,
                    				s_ctrl->curr_csic_params);
                    			CDBG("CSI config is done\n");
                    			mb();
                    			msleep(20);
                    			csi_config = 1;
                    		}
				else
					{
                    			CDBG("CSI config is done-------------------------\n");
                         	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x307,(pv_es&0xff00)>>8,1);
                         	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x308,(pv_es&0x00ff),1);

                         	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x300,0x02,1);
							 
                         	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x351,0x00,1);
                         	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x319,0x40,1);
                         	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x310,0xdf,1);
                    		msleep(100);
                         	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x351,0xd0,1);
                         	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x319,0x00,1);
                        	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x310,0x1f,1);
				}
                    		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                    			NOTIFY_PCLK_CHANGE,
                    			&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);
                    
                    		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
                    		msleep(50);
                    		printk("init  is done\n");
                    		break;
				}
			}
	}
	return rc;
}



static struct msm_sensor_fn_t jt8ev5_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	//.sensor_group_hold_on = msm_sensor_group_hold_on,
	//.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = jt8ev5_sensor_set_fps,
	//.sensor_write_exp_gain = jt8ev5_write_prev_exp_gain,
	//.sensor_write_snapshot_exp_gain = jt8ev5_write_pict_exp_gain,
	.sensor_csi_setting = jt8ev5_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up, //msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,//msm_sensor_power_down,
    .sensor_set_af_rect = jt8ev5_sensor_set_af_rect,
	.sensor_set_af_result = NULL,//jt8ev5_set_af_result,
	.sensor_download_af_firmware = NULL,
};

static struct msm_sensor_reg_t jt8ev5_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,		//DangXiao for T8EV5
	.start_stream_conf = jt8ev5_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(jt8ev5_start_settings),
	.stop_stream_conf = jt8ev5_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(jt8ev5_stop_settings),
	.group_hold_on_conf = jt8ev5_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(jt8ev5_groupon_settings),
	.group_hold_off_conf = jt8ev5_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(jt8ev5_groupoff_settings),
	.init_settings = &jt8ev5_init_conf[0],
	.init_size = ARRAY_SIZE(jt8ev5_init_conf),
	.mode_settings = &jt8ev5_confs[0],
	.output_settings = &jt8ev5_dimensions[0],
	.num_conf = ARRAY_SIZE(jt8ev5_confs),
	.fw_download = &(jt8ev5_fw_settings[0]),
	.fw_size = ARRAY_SIZE(jt8ev5_fw_settings),
};

static struct msm_sensor_ctrl_t jt8ev5_s_ctrl = {
	.msm_sensor_reg = &jt8ev5_regs,
	.sensor_i2c_client = &jt8ev5_sensor_i2c_client,
	#ifdef EFFECT_ADD
	
	.msm_sensor_v4l2_ctrl_info = jt8ev5_v4l2_ctrl_info,   
	.num_v4l2_ctrl = ARRAY_SIZE(jt8ev5_v4l2_ctrl_info), 
	#endif
		
	.sensor_i2c_addr = 0x78,						//DangXiao for T8EV5
	.sensor_output_reg_addr = &jt8ev5_reg_addr,
	.sensor_id_info = &jt8ev5_id_info,
	.sensor_exp_gain_info = &jt8ev5_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &jt8ev5_csi_params_array[0],
	.msm_sensor_mutex = &jt8ev5_mut,
	.sensor_i2c_driver = &jt8ev5_i2c_driver,
	.sensor_v4l2_subdev_info = jt8ev5_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(jt8ev5_subdev_info),
	.sensor_v4l2_subdev_ops = &jt8ev5_subdev_ops,
	.func_tbl = &jt8ev5_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Samsung 3MP YUV sensor driver");
MODULE_LICENSE("GPL v2");



/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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
#include "ov5640af.h" //ECID:0000 zhangzhao optimize the camera start up time
#include <linux/proc_fs.h>  //wangjianping add sensor  id

//#include "ov5640_v4l2.h"
#define SENSOR_NAME "ov5640"
#define PLATFORM_DRIVER_NAME "msm_camera_ov5640"
#define ov5640_obj ov5640_##obj


#define CAMERA_FOR_OBJECTIVE   


DEFINE_MUTEX(ov5640_mut);

//ECID:0000 zhangzhao 2012-6-25 add ev algorithm start
uint16_t g_preview_u;
uint16_t g_preview_v;
uint16_t g_preview_exposure;
uint16_t g_preview_line_width;
uint16_t g_preview_gain_low;
uint16_t g_preview_gain_high;
uint16_t g_preview_gain;
uint16_t g_preview_frame_rate;
uint16_t g_preview_uv;
uint16_t YAVG; //kenxu add for improve noise.
int ov5640_effect_mode=MSM_V4L2_EFFECT_OFF;
static  int download_flag = 0;

static int af_count = 0;

#define  CAPTURE_FRAMERATE 750
#define  PREVIEW_FRAMERATE 2500    
//ECID:0000 zhangzhao 2012-6-25 add ev algorithm end
static struct msm_sensor_ctrl_t ov5640_s_ctrl;

static struct msm_camera_i2c_reg_conf ov5640_start_settings[] = {
          {0x4202, 0x00}
};

static struct msm_camera_i2c_reg_conf ov5640_stop_settings[] = {
          {0x4202, 0x0f}
};

static struct msm_camera_i2c_reg_conf ov5640_groupon_settings[] = {
//      {0x3008, 0x02},
};

static struct msm_camera_i2c_reg_conf ov5640_groupoff_settings[] = {
//	{0x3008, 0x42},
};




static struct msm_camera_i2c_reg_conf ov5640_recommend_settings[] ={

/* 1280*960 25fps*/
//{0x3103, 0x11},
//{0x3008, 0x82},
{0x3008, 0x42},  
{0x3103, 0x03},
{0x3017, 0x00},
{0x3018, 0x00},
{0x3034, 0x18},
{0x3035, 0x21},
{0x3036, 0x5d},
{0x3037, 0x13},
{0x3108, 0x01},
{0x3630, 0x36},
{0x3631, 0x0e},
{0x3632, 0xe2},
{0x3633, 0x12},
{0x3634, 0x40},
{0x3621, 0xe0},
{0x3704, 0xa0},
{0x3703, 0x5a},
{0x3715, 0x78},
{0x3717, 0x01},
{0x370b, 0x60},
{0x3705, 0x1a},
{0x3905, 0x02},
{0x3906, 0x10},
{0x3901, 0x0a},
{0x3731, 0x12},
{0x3600, 0x08},
{0x3601, 0x33},
{0x302d, 0x60},
{0x3620, 0x52},
{0x371b, 0x20},
{0x471c, 0x50},
{0x3a13, 0x43},
{0x3a18, 0x00},
{0x3a19, 0xf8},
{0x3635, 0x13},
{0x3636, 0x03},
{0x3622, 0x01},
{0x3c01, 0x34},
{0x3c04, 0x28},
{0x3c05, 0x98},
{0x3c06, 0x00},
{0x3c07, 0x07},
{0x3c08, 0x00},
{0x3c09, 0x1c},
{0x3c0a, 0x9c},
{0x3c0b, 0x40},
#if defined ( CONFIG_PROJECT_P825F01 )
{0x3820, 0x40},//0x46 yuxin modify for flip 2012.05.28
{0x3821, 0x06},//0x00
#else
{0x3820, 0x46},
{0x3821, 0x00},
#endif
{0x3814, 0x31},
{0x3815, 0x31},
{0x3800, 0x00},
{0x3801, 0x00},
{0x3802, 0x00},
{0x3803, 0x04},
{0x3804, 0x0a},
{0x3805, 0x3f},
{0x3806, 0x07},
{0x3807, 0x9b},
{0x3808, 0x05},
{0x3809, 0x00},
{0x380a, 0x03},
{0x380b, 0xc0},
{0x380c, 0x07},
{0x380d, 0x68},
{0x380e, 0x03},
{0x380f, 0xd8},
{0x3810, 0x00},
{0x3811, 0x10},
{0x3812, 0x00},
{0x3813, 0x06},
{0x3618, 0x00},
{0x3612, 0x29},
{0x3708, 0x64},
{0x3709, 0x52},
{0x370c, 0x03},
{0x3a02, 0x03},
{0x3a03, 0xd8},
{0x3a08, 0x01},
{0x3a09, 0x27},
{0x3a0a, 0x00},
{0x3a0b, 0xf6},
{0x3a0e, 0x03},
{0x3a0d, 0x04},
{0x3a14, 0x03},
{0x3a15, 0xd8},
{0x4001, 0x02},
{0x4004, 0x02},
//{0x3000, 0x00},
{0x3002, 0x1c},
{0x3004, 0xff},
{0x3006, 0xc3},
{0x300e, 0x45}, //zhangzhao 45---25
{0x302e, 0x08},
{0x4300, 0x30},
{0x501f, 0x00},
{0x4713, 0x02},
{0x4407, 0x04},
{0x440e, 0x00},
{0x460b, 0x37},
{0x460c, 0x20},
{0x3824, 0x04},
  //SDE
{0x5000, 0xa7},
{0x5001, 0x83},
{0x5309, 0x08},
{0x530a, 0x30},
{0x530b, 0x04},
{0x530c, 0x06},
{0x5480, 0x01}, 
{0x5580, 0x06},
{0x5583, 0x40},
{0x5584, 0x40},
{0x5585, 0x20},
{0x5586, 0x20},
{0x5587, 0x00},
{0x5588, 0x01},
{0x5589, 0x10},
{0x558a, 0x00},
{0x558b, 0x80},
{0x5025, 0x00},
  {0x3a0f, 0x33},
  {0x3a10, 0x2b},
  {0x3a1b, 0x33},
  {0x3a1e, 0x2b},
{0x3a11, 0x60},
{0x3a1f, 0x14},
//lens shading
  {0x5800, 0x3F},  //yuxin modify 2011.11.14 ++
  {0x5801, 0x27},
  {0x5802, 0x20},
  {0x5803, 0x20},
  {0x5804, 0x2D},
  {0x5805, 0x3F},
  {0x5806, 0x14},
  {0x5807, 0x0C},
  {0x5808, 0x07},
  {0x5809, 0x08},
  {0x580A, 0x0C},
  {0x580B, 0x18},
  {0x580C, 0x0E},
  {0x580D, 0x07},
  {0x580E, 0x02},
  {0x580F, 0x02},
  {0x5810, 0x07},
  {0x5811, 0x0E},
  {0x5812, 0x0C},
  {0x5813, 0x07},
  {0x5814, 0x02},
  {0x5815, 0x02},
  {0x5816, 0x07},
  {0x5817, 0x0F},
  {0x5818, 0x13},
  {0x5819, 0x0C},
  {0x581A, 0x07},
  {0x581B, 0x07},
  {0x581C, 0x0B},
  {0x581D, 0x15},
  {0x581E, 0x3D},
  {0x581F, 0x21},
  {0x5820, 0x19},
  {0x5821, 0x18},
  {0x5822, 0x22},
  {0x5823, 0x3F},          
  {0x5824, 0x46},
  {0x5825, 0x17},
  {0x5826, 0x17},
  {0x5827, 0x16},
  {0x5828, 0x36},
  {0x5829, 0x26},
  {0x582A, 0x33},   //copy from TD's code
  {0x582B, 0x22},
  {0x582C, 0x23},
  {0x582D ,0x15},
  {0x582E ,0x15},
  {0x582F, 0x41},  //copy from TD's code
  {0x5830, 0x41},
  {0x5831, 0x32},
  {0x5832, 0x14},
  {0x5833, 0x26},
  {0x5834, 0x22},   //copy from TD's code
  {0x5835, 0x22},
  {0x5836, 0x23},  
  
  {0x5837, 0x16},
  {0x5838, 0x36},
  {0x5839, 0x17},
  {0x583A, 0x17},
  {0x583B, 0x16},
  {0x583C, 0x47},
  {0x583d, 0xce},    //copy from TD's code //yuxin modify 2011.11.14 --
//lenc mini Q
{0x583e, 0x20},
{0x583f, 0x10},
{0x5840, 0x00},
  {0x5841, 0x0d},
  //color matrix 
  {0x5381, 0x26},
  {0x5382, 0x4E},
  {0x5383, 0x0C},
  {0x5384, 0x05},
  {0x5385, 0x71},
  {0x5386, 0x76},
  {0x5387, 0x79},
  {0x5388, 0x6D},
  {0x5389, 0x0C},
  {0x538A, 0x01},
  {0x538B, 0x98},
//de-noise sharpness
  {0x5300, 0x08},
  {0x5301, 0x20},
{0x5302,0x14},//1206
  {0x5303, 0x00},
{0x5308,0x35},
  {0x5304, 0x08},
  {0x5305, 0x20},
  {0x5306, 0x18},// {0x5306, 0x08},//yuxin modify for sdec
  {0x5307, 0x20},
//Ken Xu add 20110325 for BLC auto update
{0x4005, 0x1a}, // always update BLC


{0x3708, 0x64},//kenxu @20110707 from 0x62-->0x64 short exposure color shift 
{0x3c01, 0xb4}, // 50Hz banding
{0x3c00, 0x04},
{0x3a08, 0x01},
{0x3a09, 0xea},
{0x3a0e, 0x02},
{0x3a00, 0x78}, // 0x7c enable night mode; 0x78 disable night mode
{0x3a14, 0x03}, // max 50Hz banding step for normal mode
{0x3a15, 0xd8},
{0x5183, 0x14},
{0x4009, 0x10},//blc, modifed for SNR
  //MIPI Clock gate and send line package each line
  {0x4800, 0x34},
  //reset MIPI
  {0x3003, 0x03}, 
  {0x3003, 0x01}, 
  //release software reset
//awb
  //kenxu update @20120215
{0x5180, 0xff},
{0x5181, 0xf2},
{0x5182, 0x0 },
{0x5183, 0x14},
{0x5184, 0x25},
{0x5185, 0x24},
{0x5186, 0x13},
{0x5187, 0x30},
{0x5188, 0x15},
{0x5189, 0x77},
{0x518a, 0x5d},
{0x518b, 0xff},
{0x518c, 0xaf},
{0x518d, 0x44},
{0x518e, 0x39},
{0x518f, 0x57},
{0x5190, 0x48},
{0x5191, 0xf8},
{0x5192, 0x4 },
{0x5193, 0x70},
{0x5194, 0xf0},
{0x5195, 0xf0},
{0x5196, 0x3 },
{0x5197, 0x1 },
{0x5198, 0x6 },
{0x5199, 0x67},
{0x519a, 0x4 },
{0x519b, 0x4 },
{0x519c, 0x4 },
{0x519d, 0xf6},
{0x519e, 0x38},
//gamma
  {0x5481, 0x08},  //yuxin modify 2011.11.14 ++
  {0x5482, 0x14},
  {0x5483, 0x28},
  {0x5484, 0x51},
  {0x5485, 0x65},
  {0x5486, 0x71},
  {0x5487, 0x7d},
  {0x5488, 0x87},
  {0x5489, 0x91},
  {0x548a, 0x9a},
  {0x548b, 0xaa},
  {0x548c, 0xb8},
  {0x548d, 0xcd},
  {0x548e, 0xdd},
  {0x548f, 0xea},
  {0x5490, 0x1d},  //yuxin modify 2011.11.14 --



//auto uv
/*{0x5580, 0x02},
{0x5588, 0x00},
{0x5583, 0x40},
{0x5584, 0x20},
{0x5589, 0x40},
{0x558a, 0x00},
{0x558b, 0x80},*/  //ECID:0000] zhangzhao 2012-3-8 modified for brightness no effect
  //AEC AND AGC enable
  {0x3503, 0x00},
{0x4837, 0x10},//zhangzhao 10---2b
{0x3008, 0x02},  


};




static struct msm_camera_i2c_reg_conf ov5640_snapshot_5m_array[] ={
{0x4202, 0x0f}, //streaming off
{0x3035, 0x21},
{0x3036, 0x54}, 
#if defined ( CONFIG_PROJECT_P825F01 )
{0x3820, 0x40},//0x46 yuxin modify for flip 2012.05.28
{0x3821, 0x06},//0x00
#else
{0x3820, 0x46},
{0x3821, 0x00},
#endif
{0x3814, 0x11},
{0x3815, 0x11},
{0x3800, 0x00},
{0x3801, 0x00},
{0x3802, 0x00},
{0x3803, 0x00},
{0x3804, 0x0a},
{0x3805, 0x3f},
{0x3806, 0x07},
{0x3807, 0x9f},
{0x3808, 0x0a},
{0x3809, 0x20},
{0x380a, 0x07},
{0x380b, 0x98},
{0x380c, 0x0b},
{0x380d, 0x1c},
{0x380e, 0x07},
{0x380f, 0xb0},
{0x3810, 0x00},
{0x3811, 0x10},
{0x3812, 0x00},
  {0x3813, 0x06},
{0x3618, 0x04},              
  {0x3612, 0x4b},
{0x3709, 0x12}, 
{0x370c, 0x00}, 
{0x3a02, 0x07}, 
{0x3a03, 0xb0}, 
{0x3a0e, 0x06}, 
{0x3a0d, 0x08}, 
{0x3a14, 0x07}, 
{0x3a15, 0xb0}, 
{0x4004, 0x06}, 
  {0x4837, 0x17},   //{0x4837, 0x09},0x19,yuxin modify for capture green screen 
{0x5001, 0x83}, 
  {0x3708, 0x21},
  {0x3c01, 0xb4},
  {0x3a08, 0x01},
  {0x3a09, 0x27},  
  {0x3a0e, 0x06},  //{0x3a0e, 0x03},//yuxin copy from TD pad code 11.17
  {0x3a14, 0x07},  //yuxin copy from TD pad code 11.17
  {0x3a15, 0xb0},  //yuxin copy from TD pad code 11.17
  {0x5183, 0x14},
  {0x5302, 0x14},
  {0x5303, 0x00},
};





struct msm_camera_i2c_reg_conf ov5640_capture_to_preview_settings_array[] = {
/* 1280*960 25fps*/
{0x3035, 0x21}, 
{0x3036, 0x5d}, 
#if  defined ( CONFIG_PROJECT_P825F01 )
{0x3820, 0x40},//0x46 yuxin modify for flip 2012.05.28
{0x3821, 0x06},//0x00
#else
{0x3820, 0x46},
{0x3821, 0x00},
#endif
{0x3814, 0x31},
{0x3815, 0x31},
{0x3800, 0x00},
{0x3801, 0x00},
{0x3802, 0x00},
{0x3803, 0x04},
{0x3804, 0x0a},
{0x3805, 0x3f},
{0x3806, 0x07},
{0x3807, 0x9b},
{0x3808, 0x05},
{0x3809, 0x00},
{0x380a, 0x03},
{0x380b, 0xc0},
{0x380c, 0x07},
{0x380d, 0x68},
{0x380e, 0x03},
{0x380f, 0xd8},
{0x3810, 0x00},
{0x3811, 0x10},
{0x3812, 0x00},
{0x3813, 0x06},
{0x3618, 0x00},
{0x3612, 0x29},
{0x3709, 0x52},
{0x370c, 0x03},
{0x3a02, 0x03},
{0x3a03, 0xd8},
{0x3a0e, 0x03},
{0x3a0d, 0x04},
{0x3a14, 0x03},
{0x3a15, 0xd8},
{0x4004, 0x02},
{0x4837, 0x10},//zhangzhao 10----2b
{0x5001, 0x83},
{0x3503, 0x00},
{0x3708, 0x64},//kenxu @20110707 from 0x62-->0x64 short exposure color shift 
//{0x3c01,0xb4},
//{0x3c00,0x04},
//{0x3a08,0x01},
//{0x3a09, 0xea},
//{0x3a0e, 0x02},
{0x3a00, 0x78}, // 0x7c enable night mode; 0x78 disable night mode
{0x3a14, 0x03}, // max 50Hz banding step for normal mode
{0x3a15, 0xd8},
{0x5183, 0x14},
{0x4009,0x10},  //zhangzhao return preview the viewfinder more brighter problem
//banding
{0x3a08, 0x00},
{0x3a09, 0xf5},
{0x3a0a, 0x00},
{0x3a0b, 0xcc},
{0x3a0e, 0x04},
{0x3a0d, 0x04},
{0x3c01, 0x84},
{0x3c00, 0x04},

//{0x5588, 0x09}, //auto uv enable
//{0x5583, 0x40}, //modify by jacky_xi
//{0x5584, 0x20}, // modify by jacky_xi

//{0x4202, 0x00},
};



/* no overlay display */
//zhangzhao make the sigle file for auto focus regs ov5640af.h

//ECID:0000 2012-6-18 zhangzhao optimize the camera start up time start
struct msm_camera_i2c_reg_conf ov5640_af_settings_array_end[] = {
  {0x3022, 0x00},
  {0x3023, 0x00},
  {0x3024, 0x00},
  {0x3025, 0x00},
  {0x3026, 0x00},
  {0x3027, 0x00},
  {0x3028, 0x00},
  {0x3029, 0x7F},
  {0x3000, 0x00},
  //read the 3029 reg,if its value is not 0x70,return fail
  {0x3029,0x70,MSM_CAMERA_I2C_BYTE_DATA,MSM_CAMERA_I2C_CMD_LOAD},
};
//ECID:0000 2012-6-18 zhangzhao optimize the camera start up time end





// saturation
static struct msm_camera_i2c_reg_conf ov5640_saturation[][14] = 
{	
   	{//level 0
           {0x3212, 0x03},  //start group 3
           {0x5381, 0x1c},
           {0x5382, 0x5a},
           {0x5383, 0x06},
           {0x5384, 0x0c},
           {0x5385, 0x30},
           {0x5386, 0x3d},
           {0x5387, 0x3e},
           {0x5388, 0x3d},
           {0x5389, 0x01},
           {0x538A, 0x01},
           {0x538B, 0x98},
           {0x3212, 0x13}, //end group 3
           {0x3212, 0xa3}, //launch group 3
	},
	
	{//level 1
     	    {0x3212, 0x03},  //start group 3
           {0x5381, 0x1c},
           {0x5382, 0x5a},
           {0x5383, 0x06},
           {0x5384, 0x15},
           {0x5385, 0x52},
           {0x5386, 0x66},
           {0x5387, 0x68},
           {0x5388, 0x66},
           {0x5389, 0x02},
           {0x538A, 0x01},
           {0x538B, 0x98},
           {0x3212, 0x13}, //end group 3
           {0x3212, 0xa3}, //launch group 3
	},
	
	{//level 2 -default level
           {0x3212, 0x03},  //start group 3
           {0x5381, 0x26},
           {0x5382, 0x4E},
           {0x5383, 0x0C},
           {0x5384, 0x05},
           {0x5385, 0x71},
           {0x5386, 0x76},
           {0x5387, 0x79},
           {0x5388, 0x6D},
           {0x5389, 0x0C},
           {0x538A, 0x01},
           {0x538B, 0x98},
           {0x3212, 0x13}, //end group 3
           {0x3212, 0xa3}, //launch group 3
	},
	
	{//level 3
           {0x3212, 0x03},  //start group 3
           {0x5381, 0x1c},
           {0x5382, 0x5a},
           {0x5383, 0x06},
           {0x5384, 0x1f},
           {0x5385, 0x7a},
           {0x5386, 0x9a},
           {0x5387, 0x9c},
           {0x5388, 0x9a},
           {0x5389, 0x02},
           {0x538A, 0x01},
           {0x538B, 0x98},
           {0x3212, 0x13}, //end group 3
           {0x3212, 0xa3}, //launch group 3
	},
	
	{//level 4
           {0x3212, 0x03},  //start group 3
           {0x5381, 0x1c},
           {0x5382, 0x5a},
           {0x5383, 0x06},
           {0x5384, 0x2b},
           {0x5385, 0xab},
           {0x5386, 0xd6},
           {0x5387, 0xda},
           {0x5388, 0xd6},
           {0x5389, 0x04},
           {0x538A, 0x01},
           {0x538B, 0x98},
           {0x3212, 0x13}, //end group 3
           {0x3212, 0xa3}, //launch group 3
	},
};

static struct msm_camera_i2c_conf_array ov5640_saturation_confs[][1] =
{	
   {{ov5640_saturation[0],		ARRAY_SIZE(ov5640_saturation[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov5640_saturation[1],		ARRAY_SIZE(ov5640_saturation[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov5640_saturation[2],		ARRAY_SIZE(ov5640_saturation[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov5640_saturation[3],		ARRAY_SIZE(ov5640_saturation[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov5640_saturation[4],		ARRAY_SIZE(ov5640_saturation[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
 
};
	
static int ov5640_saturation_enum_map[] = 
{	
   MSM_V4L2_SATURATION_L0,	
   MSM_V4L2_SATURATION_L1,	
   MSM_V4L2_SATURATION_L2,
   MSM_V4L2_SATURATION_L3,
   MSM_V4L2_SATURATION_L4,
  
};
		
static struct msm_camera_i2c_enum_conf_array ov5640_saturation_enum_confs = {	
	.conf = &ov5640_saturation_confs[0][0],	
       .conf_enum = ov5640_saturation_enum_map,
       .num_enum = ARRAY_SIZE(ov5640_saturation_enum_map),	
       .num_index = ARRAY_SIZE(ov5640_saturation_confs),	
       .num_conf = ARRAY_SIZE(ov5640_saturation_confs[0]),	
       .data_type = MSM_CAMERA_I2C_BYTE_DATA,
};


// contrast
static struct msm_camera_i2c_reg_conf ov5640_contrast[][5] = {
      	{//level 0
         {0x3212,0x03},
         {0x5586, 0x28},
         {0x5585, 0x18},
         {0x3212, 0x13},
         {0x3212, 0xa3},
	},

       {//level 1
         {0x3212,0x03},
         {0x5586, 0x24},
         {0x5585, 0x10},
         {0x3212, 0x13},
         {0x3212, 0xa3},
       },

       {//level 2
         {0x3212,0x03},
         {0x5586, 0x20},
         {0x5585, 0x00},
         {0x3212, 0x13},
         {0x3212, 0xa3},
       },

       {//level 3
         {0x3212,0x03},
         {0x5586, 0x1c},
         {0x5585, 0x1c},
         {0x3212, 0x13},
         {0x3212, 0xa3},
       },

       {//level 4
         {0x3212,0x03},
         {0x5586, 0x18},
         {0x5585, 0x18},
         {0x3212, 0x13},
         {0x3212, 0xa3},
       },
};


static struct msm_camera_i2c_reg_conf ov5640_enable_contrast[] = {	
  {0x5580, 0x04, MSM_CAMERA_I2C_BYTE_DATA, MSM_CAMERA_I2C_CMD_READ},
};


static struct msm_camera_i2c_conf_array ov5640_contrast_confs[][2] = {
	{ {ov5640_contrast[0],
		ARRAY_SIZE(ov5640_contrast[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	   {ov5640_enable_contrast,
		ARRAY_SIZE(ov5640_enable_contrast), 0, MSM_CAMERA_I2C_BYTE_DATA},
	  
	},
		
	{ {ov5640_contrast[1],
		ARRAY_SIZE(ov5640_contrast[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	   {ov5640_enable_contrast,
		ARRAY_SIZE(ov5640_enable_contrast), 0, MSM_CAMERA_I2C_BYTE_DATA},
	   
	},
	{ {ov5640_contrast[2],
		ARRAY_SIZE(ov5640_contrast[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	   {ov5640_enable_contrast,
		ARRAY_SIZE(ov5640_enable_contrast), 0, MSM_CAMERA_I2C_BYTE_DATA},
	 
	},
		
	{ {ov5640_contrast[3],
		ARRAY_SIZE(ov5640_contrast[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	   {ov5640_enable_contrast,
		ARRAY_SIZE(ov5640_enable_contrast), 0, MSM_CAMERA_I2C_BYTE_DATA},
	   
	},
		
	{ {ov5640_contrast[4],
		ARRAY_SIZE(ov5640_contrast[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	   {ov5640_enable_contrast,
		ARRAY_SIZE(ov5640_enable_contrast), 0, MSM_CAMERA_I2C_BYTE_DATA},
	   
	},
		

};

static int ov5640_contrast_enum_map[] = {
	MSM_V4L2_CONTRAST_L0,
	MSM_V4L2_CONTRAST_L1,
	MSM_V4L2_CONTRAST_L2,
	MSM_V4L2_CONTRAST_L3,
	MSM_V4L2_CONTRAST_L4,	
};

static struct msm_camera_i2c_enum_conf_array ov5640_contrast_enum_confs = {
	.conf = &ov5640_contrast_confs[0][0],
	.conf_enum = ov5640_contrast_enum_map,
	.num_enum = ARRAY_SIZE(ov5640_contrast_enum_map),
	.num_index = ARRAY_SIZE(ov5640_contrast_confs),
	.num_conf = ARRAY_SIZE(ov5640_contrast_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

// sharpness
static struct msm_camera_i2c_reg_conf ov5640_sharpness[][9] = {
	{//level 0
          {0x5308, 0x65,},
          {0x5302, 0x02,},
          {0x5308, 0x65,},
          {0x5302, 0x02,},
          {0x5308, 0x65,},
          {0x5302, 0x02,},
          {0x5308, 0x65,},
          {0x5302, 0x02,},
          {0x5302, 0x02,},
	},
       {//level 1
          {0x5308, 0x65,},
          {0x5302, 0x08,},
          {0x5308, 0x65,},
          {0x5302, 0x08,},
          {0x5308, 0x65,},
          {0x5302, 0x08,},
          {0x5308, 0x65,},
          {0x5302, 0x08,},
          {0x5302, 0x08,},
	 },
	 {//level 2 -auto mode
          {0x5308, 0x25},
          {0x5300, 0x08},
          {0x5301, 0x20},
          {0x5302, 0x14},
          {0x5303, 0x00},
          {0x5309, 0x08},
          {0x530a, 0x30},
          {0x530b, 0x04},
          {0x530c, 0x06},
	 },
       {//level 3
          {0x5308, 0x65},
          {0x5302, 0x20},
          {0x5308, 0x65},
          {0x5302, 0x20},
          {0x5308, 0x65},
          {0x5302, 0x20},
          {0x5308, 0x65},
          {0x5302, 0x20},
          {0x5302, 0x20},
       },
       {//level 4
          {0x5308, 0x65},
          {0x5302, 0x2c},
          {0x5308, 0x65},
          {0x5302, 0x2c},
          {0x5308, 0x65},
          {0x5302, 0x2c},
          {0x5308, 0x65},
          {0x5302, 0x2c},
          {0x5302, 0x2c},
       },
	
};

static struct msm_camera_i2c_conf_array ov5640_sharpness_confs[][1] = {
	{
		{ov5640_sharpness[0],
		ARRAY_SIZE(ov5640_sharpness[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_sharpness[1],
		ARRAY_SIZE(ov5640_sharpness[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_sharpness[2],
		ARRAY_SIZE(ov5640_sharpness[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_sharpness[3],
		ARRAY_SIZE(ov5640_sharpness[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_sharpness[4],
		ARRAY_SIZE(ov5640_sharpness[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},	
	
	
};

static int ov5640_sharpness_enum_map[] = {
	MSM_V4L2_SHARPNESS_L0,
	MSM_V4L2_SHARPNESS_L1,
	MSM_V4L2_SHARPNESS_L2,
	MSM_V4L2_SHARPNESS_L3,
	MSM_V4L2_SHARPNESS_L4,	
		
};

static struct msm_camera_i2c_enum_conf_array ov5640_sharpness_enum_confs = {
	.conf = &ov5640_sharpness_confs[0][0],
	.conf_enum = ov5640_sharpness_enum_map,
	.num_enum = ARRAY_SIZE(ov5640_sharpness_enum_map),
	.num_index = ARRAY_SIZE(ov5640_sharpness_confs),
	.num_conf = ARRAY_SIZE(ov5640_sharpness_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};
//

// brightness
static struct msm_camera_i2c_reg_conf ov5640_brightness[][2] = {
       {//level 0
          //{0x3212, 0x03},
          {0x5587, 0x40},
          {0x5588, 0x09},
          //{0x3212, 0x13},
          //{0x3212, 0xa3},
	 },
	 
     	{//level 1
          //{0x3212, 0x03},
          {0x5587, 0x20},
          {0x5588, 0x09},
          //{0x3212, 0x13},
          //{0x3212, 0xa3},
	 },
	 {//level 2
          //{0x3212, 0x03},
          {0x5587, 0x00},
          {0x5588, 0x01},
          //{0x3212, 0x13},
          //{0x3212, 0xa3},
	 },
	 {//level 3
          //{0x3212, 0x03},
          {0x5587, 0x20},
          {0x5588, 0x01},
          //{0x3212, 0x13},
          //{0x3212, 0xa3},
	 },
	 {//level 4
          //{0x3212, 0x03},
          {0x5587, 0x40},
          {0x5588, 0x01},
          //{0x3212, 0x13},
          //{0x3212, 0xa3},
	 },
};



static struct msm_camera_i2c_conf_array ov5640_brightness_confs[][1] = {
	
	{{ov5640_brightness[0],
		ARRAY_SIZE(ov5640_brightness[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
	
	{{ov5640_brightness[1],
		ARRAY_SIZE(ov5640_brightness[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
		
	{{ov5640_brightness[2],
		ARRAY_SIZE(ov5640_brightness[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
		
	{{ov5640_brightness[3],
		ARRAY_SIZE(ov5640_brightness[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
		
	{{ov5640_brightness[4],
		ARRAY_SIZE(ov5640_brightness[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},},
		
		
};

static int ov5640_brightness_enum_map[] = {
	MSM_V4L2_BRIGHTNESS_L0,
	MSM_V4L2_BRIGHTNESS_L1,
	MSM_V4L2_BRIGHTNESS_L2,
	MSM_V4L2_BRIGHTNESS_L3,
	MSM_V4L2_BRIGHTNESS_L4,	
};

static struct msm_camera_i2c_enum_conf_array ov5640_brightness_enum_confs = {
	.conf = &ov5640_brightness_confs[0][0],
	.conf_enum = ov5640_brightness_enum_map,
	.num_enum = ARRAY_SIZE(ov5640_brightness_enum_map),
	.num_index = ARRAY_SIZE(ov5640_brightness_confs),
	.num_conf = ARRAY_SIZE(ov5640_brightness_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};
//

// exposure
static struct msm_camera_i2c_reg_conf ov5640_exposure[][6] = {
	{// -1.7EV    -2
           {0x3A0F, 0x20}, 
           {0x3A10, 0x18},
           {0x3A11, 0x41},   
           {0x3A1B, 0x20}, 
           {0x3A1E, 0x18}, 
           {0x3A1F, 0x10},
       },
       
	{ // -1.0EV   -1
           {0x3A0F, 0x30}, 
           {0x3A10, 0x28},     
           {0x3A11, 0x61}, 
           {0x3A1B, 0x30}, 
           {0x3A1E, 0x28}, 
           {0x3A1F, 0x10}, 
       },
       
	{//  0
           {0x3A0F, 0x38}, 
           {0x3A10, 0x30},     
           {0x3A11, 0x61}, 
           {0x3A1B, 0x38}, 
           {0x3A1E, 0x30}, 
           {0x3A1F, 0x10}, 
       },
       
	{//+1
    	    {0x3A0F, 0x50}, 
           {0x3A10, 0x48},     
           {0x3A11, 0x90}, 
           {0x3A1B, 0x50}, 
           {0x3A1E, 0x48}, 
           {0x3A1F, 0x20}, 
	},
	
	{ //+2
           {0x3A0F, 0x60}, 
           {0x3A10, 0x58},     
           {0x3A11, 0xA0}, 
           {0x3A1B, 0x60}, 
           {0x3A1E, 0x58}, 
           {0x3A1F, 0x20}, 
	},

};


static struct msm_camera_i2c_conf_array ov5640_exposure_confs[][1] = {
	
	{{ov5640_exposure[0],
		ARRAY_SIZE(ov5640_exposure[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
	
	{{ov5640_exposure[1],
		ARRAY_SIZE(ov5640_exposure[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
		
	{{ov5640_exposure[2],
		ARRAY_SIZE(ov5640_exposure[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
		
	{{ov5640_exposure[3],
		ARRAY_SIZE(ov5640_exposure[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},},	
		
	{{ov5640_exposure[4],
		ARRAY_SIZE(ov5640_exposure[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},},
		
	
};

static int ov5640_exposure_enum_map[] = {
	MSM_V4L2_EXPOSURE_N2,
	MSM_V4L2_EXPOSURE_N1,
	MSM_V4L2_EXPOSURE_D,
	MSM_V4L2_EXPOSURE_P1,
	MSM_V4L2_EXPOSURE_P2,
};

static struct msm_camera_i2c_enum_conf_array ov5640_exposure_enum_confs = {
	.conf = &ov5640_exposure_confs[0][0],
	.conf_enum = ov5640_exposure_enum_map,
	.num_enum = ARRAY_SIZE(ov5640_exposure_enum_map),
	.num_index = ARRAY_SIZE(ov5640_exposure_confs),
	.num_conf = ARRAY_SIZE(ov5640_exposure_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};
//

// special  effect
static struct msm_camera_i2c_reg_conf ov5640_effect[][7] = {
    { //effect_off
       {0x3212, 0x03}, 
       {0x5580, 0x06}, 
       {0x5583, 0x40}, 
       {0x5584, 0x10},//0x10 
       {0x5003, 0x08},
       {0x3212, 0x13}, 
       {0x3212, 0xa3},  
    },

    {  //effect_mono
       {0x3212, 0x03}, 
       {0x5580, 0x1e}, 
       {0x5583, 0x80}, 
       {0x5584, 0x80}, 
       {0x5003, 0x08},
       {0x3212, 0x13}, 
       {0x3212, 0xa3},        
    },

    { //effect_negative
       {0x3212, 0x03}, 
       {0x5580, 0x40}, 
       {0x5003, 0x08},
       {0x5583, 0x40}, 
       {0x5584, 0x10}, 
       {0x3212, 0x13}, 
       {0x3212, 0xa3},  
    },
  
    {//SOLARIZE
       {0x3212, 0x03}, 
       {0x5580, 0x06}, 
       {0x5583, 0x40}, 
       {0x5584, 0x10},
       {0x5003, 0x09},  
       {0x3212, 0x13}, 
       {0x3212, 0xa3},
    },

    {//SEPIA
       {0x3212, 0x03}, 
       {0x5580, 0x1e}, 
       {0x5583, 0x40}, 
       {0x5584, 0xa0},
       {0x5003, 0x08},  
       {0x3212, 0x13}, 
       {0x3212, 0xa3},
    },

    { 
    //redish
       {0x3212, 0x03}, 
       {0x5580, 0x1e}, 
       {0x5583, 0x80}, 
       {0x5584, 0xc0}, 
       {0x5003, 0x08},
       {0x3212, 0x13}, 
       {0x3212, 0xa3},  
    },

   { 
    //blueish
       {0x3212, 0x03}, 
       {0x5580, 0x1e}, 
       {0x5583, 0xa0}, 
       {0x5584, 0x40}, 
       {0x5003, 0x08},
       {0x3212, 0x13}, 
       {0x3212, 0xa3},  
    },
    
    { 
    //greenish
       {0x3212, 0x03}, 
       {0x5580, 0x1e}, 
       {0x5583, 0x60}, 
       {0x5584, 0x60}, 
       {0x5003, 0x08},
       {0x3212, 0x13}, 
       {0x3212, 0xa3},  
    },
    

  
};

static struct msm_camera_i2c_conf_array ov5640_effect_confs[][1] = {
	{
		{ov5640_effect[0],
		ARRAY_SIZE(ov5640_effect[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
		
	{
		{ov5640_effect[1],
		ARRAY_SIZE(ov5640_effect[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_effect[2],
		ARRAY_SIZE(ov5640_effect[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_effect[3],
		ARRAY_SIZE(ov5640_effect[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_effect[4],
		ARRAY_SIZE(ov5640_effect[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
		
	{
		{ov5640_effect[5],
		ARRAY_SIZE(ov5640_effect[5]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_effect[6],
		ARRAY_SIZE(ov5640_effect[6]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_effect[7],
		ARRAY_SIZE(ov5640_effect[7]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
		
};

static int ov5640_effect_enum_map[] = {
	MSM_V4L2_EFFECT_OFF,
	MSM_V4L2_EFFECT_MONO,
	MSM_V4L2_EFFECT_NEGATIVE,
	MSM_V4L2_EFFECT_SOLARIZE,
	MSM_V4L2_EFFECT_SEPIA,
	MSM_V4L2_EFFECT_REDISH,
	MSM_V4L2_EFFECT_BLUEISH,
	MSM_V4L2_EFFECT_GREENISH,
};

static struct msm_camera_i2c_enum_conf_array ov5640_effect_enum_confs = {
	.conf = &ov5640_effect_confs[0][0],
	.conf_enum = ov5640_effect_enum_map,
	.num_enum = ARRAY_SIZE(ov5640_effect_enum_map),
	.num_index = ARRAY_SIZE(ov5640_effect_confs),
	.num_conf = ARRAY_SIZE(ov5640_effect_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};
//

// awb
static struct msm_camera_i2c_reg_conf ov5640_awb[][10] = {	
	{ //MSM_V4L2_WB_MIN_MINUS_1,not used
	  {0x3212, 0x03}, 
         {0x3406, 0x00},     
         {0x3400, 0x04}, 
         {0x3401, 0x00}, 
         {0x3402, 0x04}, 
         {0x3403, 0x00}, 
         {0x3404, 0x04}, 
         {0x3405, 0x00}, 
         {0x3212, 0x13}, 
         {0x3212, 0xa3}, 
	}, 
	
	{ // wb_auto
	  {0x3212, 0x03}, 
         {0x3406, 0x00},     
         {0x3400, 0x04}, 
         {0x3401, 0x00}, 
         {0x3402, 0x04}, 
         {0x3403, 0x00}, 
         {0x3404, 0x04}, 
         {0x3405, 0x00}, 
         {0x3212, 0x13}, 
         {0x3212, 0xa3}, 
	}, 
       {//MSM_V4L2_WB_CUSTOM,not used
         {0x3212, 0x03}, 
         {0x3406, 0x01},     
         {0x3400, 0x04}, 
         {0x3401, 0x10}, 
         {0x3402, 0x04}, 
         {0x3403, 0x00}, 
         {0x3404, 0x08}, 
         {0x3405, 0x40}, 
         {0x3212, 0x13}, 
         {0x3212, 0xa3}, 
	},

	{//INCANDESCENT,  //白炽
         {0x3212, 0x03}, 
         {0x3406, 0x01},     
         {0x3400, 0x04}, 
         {0x3401, 0x10}, 
         {0x3402, 0x04}, 
         {0x3403, 0x00}, 
         {0x3404, 0x08}, 
         {0x3405, 0x40}, 
         {0x3212, 0x13}, 
         {0x3212, 0xa3}, 
	},

       {//FLUORESCENT,    //荧光
         {0x3212, 0x03}, 
         {0x3406, 0x01},     
         {0x3400, 0x05}, 
         {0x3401, 0x48}, 
         {0x3402, 0x04}, 
         {0x3403, 0x00}, 
         {0x3404, 0x07}, 
         {0x3405, 0xcf}, 
         {0x3212, 0x13}, 
         {0x3212, 0xa3}, 
       },

       { //daylight
          {0x3212, 0x03}, 
          {0x3406, 0x01},     
          {0x3400, 0x06}, 
          {0x3401, 0x1c}, 
          {0x3402, 0x04}, 
          {0x3403, 0x00}, 
          {0x3404, 0x04}, 
          {0x3405, 0xf3}, 
          {0x3212, 0x13}, 
          {0x3212, 0xa3}, 
       },

       { //cloudy
           {0x3212, 0x03}, 
           {0x3406, 0x01},     
           {0x3400, 0x06}, 
           {0x3401, 0x48}, 
           {0x3402, 0x04}, 
           {0x3403, 0x00}, 
           {0x3404, 0x04}, 
           {0x3405, 0x03}, //ECID:0000 zhangzhao 2012-7-2 solve cloudy no effect  0xd3 --->0x03
           {0x3212, 0x13}, 
           {0x3212, 0xa3},
       },
};

static struct msm_camera_i2c_conf_array ov5640_awb_confs[][1] = {
	{
		{ov5640_awb[0],
		ARRAY_SIZE(ov5640_awb[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_awb[1],
		ARRAY_SIZE(ov5640_awb[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_awb[2],
		ARRAY_SIZE(ov5640_awb[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_awb[3],
		ARRAY_SIZE(ov5640_awb[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_awb[4],
	       ARRAY_SIZE(ov5640_awb[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_awb[5],
		ARRAY_SIZE(ov5640_awb[5]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_awb[6],
	       ARRAY_SIZE(ov5640_awb[6]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},	
	
};

static int ov5640_awb_enum_map[] = {	
	MSM_V4L2_WB_OFF,//not used
	MSM_V4L2_WB_AUTO ,//= 1
	MSM_V4L2_WB_CUSTOM,  //not used
	MSM_V4L2_WB_INCANDESCENT, //白炽
	MSM_V4L2_WB_FLUORESCENT,   //荧光
	MSM_V4L2_WB_DAYLIGHT,
	MSM_V4L2_WB_CLOUDY_DAYLIGHT,
};

static struct msm_camera_i2c_enum_conf_array ov5640_awb_enum_confs = {
	.conf = &ov5640_awb_confs[0][0],
	.conf_enum = ov5640_awb_enum_map,
	.num_enum = ARRAY_SIZE(ov5640_awb_enum_map),
	.num_index = ARRAY_SIZE(ov5640_awb_confs),
	.num_conf = ARRAY_SIZE(ov5640_awb_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

//ISO
static struct msm_camera_i2c_reg_conf ov5640_iso[][2] = {	
	
	{ //auto
	   {0x3a18,0x00},
          {0x3a19,0xf8},  //0xfc  yuxin modify 0628
	}, 
	
       { //MSM_V4L2_ISO_DEBLUR ,not used
	   {0x3a18,0x00},
          {0x3a19,0xfc},
	}, 
	
	{//iso_100
   	   {0x3a18,0x00},
          {0x3a19,0x20},
	},

       {//iso200
          {0x3a18,0x00},
          {0x3a19,0x40},
       },

       { //iso400
          {0x3a18,0x00},
          {0x3a19,0x80},
       },

       { //iso800
          {0x3a18,0x00},
          {0x3a19,0xfc},
       },
       
       { //iso1600
          {0x3a18,0x01},
          {0x3a19,0xfc},
       },
};

static struct msm_camera_i2c_conf_array ov5640_iso_confs[][1] = {
	{
		{ov5640_iso[0],
		ARRAY_SIZE(ov5640_iso[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_iso[1],
		ARRAY_SIZE(ov5640_iso[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_iso[2],
		ARRAY_SIZE(ov5640_iso[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_iso[3],
		ARRAY_SIZE(ov5640_iso[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_iso[4],
	  ARRAY_SIZE(ov5640_iso[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_iso[5],
	  ARRAY_SIZE(ov5640_iso[5]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},	
	
	{
		{ov5640_iso[6],
	  ARRAY_SIZE(ov5640_iso[6]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},	
	
};

static int ov5640_iso_enum_map[] = {	
	MSM_V4L2_ISO_AUTO,
	MSM_V4L2_ISO_DEBLUR,//not used
	MSM_V4L2_ISO_100,
	MSM_V4L2_ISO_200,
	MSM_V4L2_ISO_400,
	MSM_V4L2_ISO_800,
	MSM_V4L2_ISO_1600,
};

static struct msm_camera_i2c_enum_conf_array ov5640_iso_enum_confs = {
	.conf = &ov5640_iso_confs[0][0],
	.conf_enum = ov5640_iso_enum_map,
	.num_enum = ARRAY_SIZE(ov5640_iso_enum_map),
	.num_index = ARRAY_SIZE(ov5640_iso_confs),
	.num_conf = ARRAY_SIZE(ov5640_iso_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};


//anti-banding
static struct msm_camera_i2c_reg_conf ov5640_antibanding[][14] = {		
       {//off.not used
          {0x3c00, 0x04},
          {0x3c01, 0x80},
          {0x3a0a, 0x01},
          {0x3a0b, 0x98},
          {0x3a0d, 0x03},
          {0x3c00, 0x04},
          {0x3c01, 0x80},
          {0x3a0a, 0x01},
          {0x3a0b, 0x98},
          {0x3a0d, 0x03},
          {0x3c00, 0x04},
          {0x3c01, 0x80},
          {0x3a0a, 0x01},
          {0x3a0b, 0x98},
       },

       {//60Hz
          {0x3c00, 0x04},
          {0x3c01, 0x80},
          {0x3a0a, 0x01},
          {0x3a0b, 0x98},
          {0x3a0d, 0x03},
          {0x3c00, 0x04},
          {0x3c01, 0x80},
          {0x3a0a, 0x01},
          {0x3a0b, 0x98},
          {0x3a0d, 0x03},
          {0x3c00, 0x04},
          {0x3c01, 0x80},
          {0x3a0a, 0x01},
          {0x3a0b, 0x98},
       },
       
	{//50Hz
         {0x3c00, 0x00},
         {0x3c01, 0x80},
         {0x3a08, 0x01},
         {0x3a09, 0xec},
         {0x3a0e, 0x02},
         {0x3c00, 0x00},
         {0x3c01, 0x80},
         {0x3a08, 0x01},
         {0x3a09, 0xec},
         {0x3a0e, 0x02},
         {0x3c00, 0x00},
         {0x3c01, 0x80},
         {0x3a08, 0x01},
         {0x3a09, 0xec},
	},

       { //auto
         {0x3622, 0x01},
         {0x3635, 0x1c},
         {0x3634, 0x40},
         {0x3c01, 0x34},
         {0x3c00, 0x00},
         {0x3c04, 0x28},
         {0x3c05, 0x98},
         {0x3c06, 0x00},
         {0x3c07, 0x08},
         {0x3c08, 0x00},
         {0x3c09, 0x1c},
         {0x300c, 0x22},
         {0x3c0a, 0x9c},
         {0x3c0b, 0x40},
	}, 
};

static struct msm_camera_i2c_conf_array ov5640_antibanding_confs[][1] = {
	{
		{ov5640_antibanding[0],
		ARRAY_SIZE(ov5640_antibanding[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_antibanding[1],
		ARRAY_SIZE(ov5640_antibanding[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_antibanding[2],
		ARRAY_SIZE(ov5640_antibanding[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_antibanding[3],
		ARRAY_SIZE(ov5640_antibanding[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
};

static int ov5640_antibanding_enum_map[] = {	
	MSM_V4L2_POWER_LINE_OFF,  //not used
	MSM_V4L2_POWER_LINE_60HZ,
	MSM_V4L2_POWER_LINE_50HZ,
	MSM_V4L2_POWER_LINE_AUTO,

};

static struct msm_camera_i2c_enum_conf_array ov5640_antibanding_enum_confs = {
	.conf = &ov5640_antibanding_confs[0][0],
	.conf_enum = ov5640_antibanding_enum_map,
	.num_enum = ARRAY_SIZE(ov5640_antibanding_enum_map),
	.num_index = ARRAY_SIZE(ov5640_antibanding_confs),
	.num_conf = ARRAY_SIZE(ov5640_antibanding_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

//scene
static struct msm_camera_i2c_reg_conf ov5640_scene[][58]={
      {//SCENE_MODE_AUTO  自动
        {0x3a00 ,0x00}, //04        //Night mode disable
        {0x3a02 ,0x0f},  //Night mode(Half) 1/4
        {0x3a03 ,0x60},
        {0x3a14 ,0x0f},
        {0x3a15 ,0x60},
        {0x5381 ,0x1e},//CMX
        {0x5382 ,0x5b},
        {0x5383 ,0x08},
        {0x5384 ,0x0a},
        {0x5385 ,0x7e},
        {0x5386 ,0x88},
        {0x5387 ,0x7c},
        {0x5388 ,0x6c},
        {0x5389 ,0x10},
        {0x538a ,0x01},
        {0x538b ,0x98},
        {0x3a0f ,0x30},//EV default
        {0x3a10 ,0x28},
        {0x3a1b ,0x30},
        {0x3a1e ,0x26},
        {0x3a11 ,0x60},
        {0x3a1f ,0x14},
        {0x5490 ,0x1d},//Gamma
        {0x5481 ,0x08},
        {0x5482 ,0x14},
        {0x5483 ,0x28},
        {0x5484 ,0x51},
        {0x5485 ,0x65},
        {0x5486 ,0x71},
        {0x5487 ,0x7d},
        {0x5488 ,0x87},
        {0x5489 ,0x91},
        {0x548a ,0x9a},
        {0x548b ,0xaa},
        {0x548c ,0xb8},
        {0x548d ,0xcd},
        {0x548e ,0xdd},
        {0x548f ,0xea},
        {0x5308 ,0x00}, //40        //Sharpness Auto default
        {0x5300 ,0x08},
        {0x5301 ,0x30},
        {0x5302 ,0x10},
        {0x5303 ,0x00},
        {0x5309 ,0x08},
        {0x530a ,0x30},
        {0x530b ,0x04},
        {0x530c ,0x06},
        {0x3a18 ,0x00},        //ISO Auto - 16x
        {0x3a19 ,0xf8},   //0xfc  yuxin modify 0628
        {0x501d ,0x00},//AE Weight - Average
        {0x5688 ,0x11},
        {0x5689 ,0x11}, 
        {0x568a ,0x11}, 
        {0x568b ,0x11}, 
        {0x568c ,0x11},
        {0x568d ,0x11}, 
        {0x568e ,0x11}, 
        {0x568f ,0x11},
	},
	
      {   //SCENE_MODE_LANDSCAPE，风景
         {0x3a00 ,0x00}, //04         //Night mode disable
         {0x3a02 ,0x0f},//Night mode(Half) 1/4
         {0x3a03 ,0x60},
         {0x3a14 ,0x0f},
         {0x3a15 ,0x60},        
         {0x5381 ,0x1c}, //CMX
         {0x5382 ,0x5a},
         {0x5383 ,0x06},
         {0x5384 ,0x0d},
         {0x5385 ,0xa4},
         {0x5386 ,0xb1},
         {0x5387 ,0xa1},
         {0x5388 ,0x8c},
         {0x5389 ,0x15},
         {0x538a ,0x01},
         {0x538b ,0x98},
         {0x3a0f ,0x30},//EV default
         {0x3a10 ,0x28},
         {0x3a1b ,0x30},
         {0x3a1e ,0x26},
         {0x3a11 ,0x60},
         {0x3a1f ,0x14},
         {0x5490 ,0x24},         //Gamma
         {0x5481 ,0x04},
         {0x5482 ,0x07},
         {0x5483 ,0x10},
         {0x5484 ,0x28},
         {0x5485 ,0x36},
         {0x5486 ,0x44},
         {0x5487 ,0x52},
         {0x5488 ,0x60},
         {0x5489 ,0x6c},
         {0x548a ,0x78},
         {0x548b ,0x8c},
         {0x548c ,0x9e},
         {0x548d ,0xbb},
         {0x548e ,0xd2},
         {0x548f ,0xe5},
         {0x5308 ,0x00}, //40 //Sharpness Auto default
         {0x5300 ,0x08},
         {0x5301 ,0x30},
         {0x5302 ,0x10},
         {0x5303 ,0x00},
         {0x5309 ,0x08},
         {0x530a ,0x30},
         {0x530b ,0x04},
         {0x530c ,0x06},
         {0x3a18 ,0x00},//ISO Auto - 16x
         {0x3a19 ,0xfc},
         {0x501d ,0x00},//AE Weight - Average
         {0x5688 ,0x62},
         {0x5689 ,0x26}, 
         {0x568a ,0xe6}, 
         {0x568b ,0x6e}, 
         {0x568c ,0xea},
         {0x568d ,0xae}, 
         {0x568e ,0xa6}, 
         {0x568f ,0x6a}, 

      },
      {//SCENE_MODE_FIREWORKS 篝火
          {0x3a00 ,0x00}, //04//Night mode disable
          {0x3a02 ,0x0f},          //Night mode(Half) 1/4
          {0x3a03 ,0x60},
          {0x3a14 ,0x0f},
          {0x3a15 ,0x60},          
          {0x5381 ,0x1e},          //CMX
          {0x5382 ,0x5b},
          {0x5383 ,0x08},
          {0x5384 ,0x0a},
          {0x5385 ,0x7e},
          {0x5386 ,0x88},
          {0x5387 ,0x7c},
          {0x5388 ,0x6c},
          {0x5389 ,0x10},
          {0x538a ,0x01},
          {0x538b ,0x98},
          {0x3a0f ,0x10}, //EV default
          {0x3a10 ,0x08}, 
          {0x3a1b ,0x10}, 
          {0x3a1e ,0x08}, 
          {0x3a11 ,0x20}, 
          {0x3a1f ,0x10}, 
          {0x5490 ,0x1d}, //Gamma
          {0x5481 ,0x08},
          {0x5482 ,0x14},
          {0x5483 ,0x28},
          {0x5484 ,0x51},
          {0x5485 ,0x65},
          {0x5486 ,0x71},
          {0x5487 ,0x7d},
          {0x5488 ,0x87},
          {0x5489 ,0x91},
          {0x548a ,0x9a},
          {0x548b ,0xaa},
          {0x548c ,0xb8},
          {0x548d ,0xcd},
          {0x548e ,0xdd},
          {0x548f ,0xea},  
          {0x5308 ,0x00}, //40//Sharpness Auto default
          {0x5300 ,0x08},
          {0x5301 ,0x30},
          {0x5302 ,0x10},
          {0x5303 ,0x00},
          {0x5309 ,0x08},
          {0x530a ,0x30},
          {0x530b ,0x04},
          {0x530c ,0x06},          
          {0x3a18 ,0x00},//ISO Auto - 16x
          {0x3a19 ,0xfc},          
          {0x501d ,0x00},//AE Weight - Average
          {0x5688 ,0x11},
          {0x5689 ,0x11}, 
          {0x568a ,0x11}, 
          {0x568b ,0x11}, 
          {0x568c ,0x11},
          {0x568d ,0x11}, 
          {0x568e ,0x11}, 
          {0x568f ,0x11}, 
      	},
       {//SCENE_MODE_BEACH   海滩
         {0x3a00 ,0x00}, //04 //Night mode disable
         {0x3a02 ,0x0f},         //Night mode(Half) 1/4
         {0x3a03 ,0x60},
         {0x3a14 ,0x0f},
         {0x3a15 ,0x60},
         {0x5381 ,0x0b},//CMX
         {0x5382 ,0x5f},
         {0x5383 ,0x18},
         {0x5384 ,0x14},
         {0x5385 ,0x95},
         {0x5386 ,0xab},
         {0x5387 ,0x65},
         {0x5388 ,0x5c},
         {0x5389 ,0x09},
         {0x538a ,0x01},
         {0x538b ,0x98},
         {0x3a0f ,0x30},         //EV default
         {0x3a10 ,0x28},
         {0x3a1b ,0x30},
         {0x3a1e ,0x26},
         {0x3a11 ,0x60},
         {0x3a1f ,0x14},
         {0x5490 ,0x1d},         //Gamma
         {0x5481 ,0x08},
         {0x5482 ,0x14},
         {0x5483 ,0x28},
         {0x5484 ,0x51},
         {0x5485 ,0x65},
         {0x5486 ,0x71},
         {0x5487 ,0x7d},
         {0x5488 ,0x87},
         {0x5489 ,0x91},
         {0x548a ,0x9a},
         {0x548b ,0xaa},
         {0x548c ,0xb8},
         {0x548d ,0xcd},
         {0x548e ,0xdd},
         {0x548f ,0xea},
         {0x5308 ,0x00}, //40 //Sharpness Auto default
         {0x5300 ,0x08},
         {0x5301 ,0x30},
         {0x5302 ,0x10},
         {0x5303 ,0x00},
         {0x5309 ,0x08},
         {0x530a ,0x30},
         {0x530b ,0x04},
         {0x530c ,0x06},      
         {0x3a18 ,0x00},  //ISO 100
         {0x3a19 ,0x6c},
         {0x501d ,0x00},//AE Weight - Average
         {0x5688 ,0x11},
         {0x5689 ,0x11}, 
         {0x568a ,0x11}, 
         {0x568b ,0x11}, 
         {0x568c ,0x11},
         {0x568d ,0x11}, 
         {0x568e ,0x11}, 
         {0x568f ,0x11},
       },

       {//SCENE_MODE_PARTY - Take indoor low-light shot.派对
           {0x3a00 ,0x04}, //04//Night mode enable
           {0x3a02 ,0x0f},//Night mode(Half) 1/4
           {0x3a03 ,0x60},
           {0x3a14 ,0x0f},
           {0x3a15 ,0x60},
           {0x5381 ,0x1e},    //CMX
           {0x5382 ,0x5b},
           {0x5383 ,0x08},
           {0x5384 ,0x0a},
           {0x5385 ,0x7e},
           {0x5386 ,0x88},
           {0x5387 ,0x7c},
           {0x5388 ,0x6c},
           {0x5389 ,0x10},
           {0x538a ,0x01},
           {0x538b ,0x98}, 
           {0x3a0f ,0x30},//EV default
           {0x3a10 ,0x28},
           {0x3a1b ,0x30},
           {0x3a1e ,0x26},
           {0x3a11 ,0x60},
           {0x3a1f ,0x14},
           {0x5490 ,0x20},//Gamma
           {0x5481 ,0x08},
           {0x5482 ,0x16},
           {0x5483 ,0x2c},
           {0x5484 ,0x48},
           {0x5485 ,0x55},
           {0x5486 ,0x6a},
           {0x5487 ,0x76},
           {0x5488 ,0x80},
           {0x5489 ,0x8c},
           {0x548a ,0x96},
           {0x548b ,0xa3},
           {0x548c ,0xaf},
           {0x548d ,0xc4},
           {0x548e ,0xd7},
           {0x548f ,0xe8}, 
           {0x5308 ,0x00}, //40//Sharpness Auto default
           {0x5300 ,0x08},
           {0x5301 ,0x30},
           {0x5302 ,0x10},
           {0x5303 ,0x00},
           {0x5309 ,0x08},
           {0x530a ,0x30},
           {0x530b ,0x04},
           {0x530c ,0x06},
           {0x3a18 ,0x00},//ISO Auto - 16x
           {0x3a19 ,0xfc},
           {0x501d ,0x00},//AE Weight - Average
           {0x5688 ,0x11},
           {0x5689 ,0x11}, 
           {0x568a ,0x11}, 
           {0x568b ,0x11}, 
           {0x568c ,0x11},
           {0x568d ,0x11}, 
           {0x568e ,0x11}, 
           {0x568f ,0x11}, 
       },

       {//SCENE_MODE_PORTRAIT - Take people pictures.人物
          {0x3a00 ,0x00},// 04//Night mode disable
          {0x3a02 ,0x0f},//Night mode(Half) 1/4
          {0x3a03 ,0x60},
          {0x3a14 ,0x0f},
          {0x3a15 ,0x60}, 
          {0x5381 ,0x1c},//CMX
          {0x5382 ,0x5a},
          {0x5383 ,0x06},
          {0x5384 ,0x08},
          {0x5385 ,0x65},
          {0x5386 ,0x6d},
          {0x5387 ,0x63},
          {0x5388 ,0x56},
          {0x5389 ,0x0d},
          {0x538a ,0x01},
          {0x538b ,0x98},
          {0x3a0f ,0x30},//EV default
          {0x3a10 ,0x28},
          {0x3a1b ,0x30},
          {0x3a1e ,0x26},
          {0x3a11 ,0x60},
          {0x3a1f ,0x14},
          {0x5490 ,0x1d},//Gamma
          {0x5481 ,0x08},
          {0x5482 ,0x14},
          {0x5483 ,0x28},
          {0x5484 ,0x51},
          {0x5485 ,0x65},
          {0x5486 ,0x71},
          {0x5487 ,0x7d},
          {0x5488 ,0x87},
          {0x5489 ,0x91},
          {0x548a ,0x9a},
          {0x548b ,0xaa},
          {0x548c ,0xb8},
          {0x548d ,0xcd},
          {0x548e ,0xdd},
          {0x548f ,0xea},
          {0x5308 ,0x40}, //40//Sharpness manual 0
          {0x5302 ,0x00},
          {0x5302 ,0x00},
          {0x5302 ,0x00},
          {0x5302 ,0x00},
          {0x5302 ,0x00},
          {0x5302 ,0x00},
          {0x5302 ,0x00},
          {0x5302 ,0x00},
          {0x3a18 ,0x00}, //ISO Auto - 16x
          {0x3a19 ,0xfc}, 
          {0x501d ,0x00},//AE Weight - CenterAverage
          {0x5688 ,0x00},
          {0x5689 ,0x00}, 
          {0x568a ,0x10}, 
          {0x568b ,0x01}, 
          {0x568c ,0x10},
          {0x568d ,0x01}, 
          {0x568e ,0x00}, 
          {0x568f ,0x00}, 
       },
       
       {//SCENE_MODE_SUNSET 日落
          {0x3a00 ,0x00}, //04 //Night mode disable
          {0x3a02 ,0x0f},          //Night mode(Half) 1/4
          {0x3a03 ,0x60},
          {0x3a14 ,0x0f},
          {0x3a15 ,0x60},
          {0x5381 ,0x42},//CMX
          {0x5382 ,0x5a},
          {0x5383 ,0x08},
          {0x5384 ,0x20},
          {0x5385 ,0x7e},
          {0x5386 ,0x48},
          {0x5387 ,0xbc},
          {0x5388 ,0x6c},
          {0x5389 ,0x06},
          {0x538a ,0x01},
          {0x538b ,0x9c},
          {0x3a0f ,0x20},//EV default
          {0x3a10 ,0x18}, 
          {0x3a1b ,0x20}, 
          {0x3a1e ,0x18}, 
          {0x3a11 ,0x41},
          {0x3a1f ,0x10},
          {0x5490 ,0x1d},//Gamma
          {0x5481 ,0x08},
          {0x5482 ,0x14},
          {0x5483 ,0x28},
          {0x5484 ,0x51},
          {0x5485 ,0x65},
          {0x5486 ,0x71},
          {0x5487 ,0x7d},
          {0x5488 ,0x87},
          {0x5489 ,0x91},
          {0x548a ,0x9a},
          {0x548b ,0xaa},
          {0x548c ,0xb8},
          {0x548d ,0xcd},
          {0x548e ,0xdd},
          {0x548f ,0xea},
          {0x5308 ,0x00}, //40//Sharpness Auto default
          {0x5300 ,0x08},
          {0x5301 ,0x30},
          {0x5302 ,0x10},
          {0x5303 ,0x00},
          {0x5309 ,0x08},
          {0x530a ,0x30},
          {0x530b ,0x04},
          {0x530c ,0x06},
          {0x3a18 ,0x00},//ISO Auto - 16x
          {0x3a19 ,0xfc},
          {0x501d ,0x00},//AE Weight - Average
          {0x5688 ,0x11},
          {0x5689 ,0x11}, 
          {0x568a ,0x11}, 
          {0x568b ,0x11}, 
          {0x568c ,0x11},
          {0x568d ,0x11}, 
          {0x568e ,0x11}, 
          {0x568f ,0x11}, 
       },

      { //SCENE_MODE_SNOW - Take pictures on the snow. 下雪
         {0x3a00 ,0x00}, //04   //Night mode disable
         {0x3a02 ,0x0f},         //Night mode(Half) 1/4
         {0x3a03 ,0x60},
         {0x3a14 ,0x0f},
         {0x3a15 ,0x60},
         {0x5381 ,0x1e}, //CMX
         {0x5382 ,0x5b},
         {0x5383 ,0x08},
         {0x5384 ,0x0a},
         {0x5385 ,0x7e},
         {0x5386 ,0x88},
         {0x5387 ,0x7c},
         {0x5388 ,0x6c},
         {0x5389 ,0x10},
         {0x538a ,0x01},
         {0x538b ,0x98},
         {0x3a0f ,0x50}, //EV default
         {0x3a10 ,0x48}, 
         {0x3a11 ,0x90}, 
         {0x3a1b ,0x50}, 
         {0x3a1e ,0x48}, 
         {0x3a1f ,0x20}, 
         {0x5490 ,0x1d},  //Gamma
         {0x5481 ,0x08},
         {0x5482 ,0x14},
         {0x5483 ,0x28},
         {0x5484 ,0x51},
         {0x5485 ,0x65},
         {0x5486 ,0x71},
         {0x5487 ,0x7d},
         {0x5488 ,0x87},
         {0x5489 ,0x91},
         {0x548a ,0x9a},
         {0x548b ,0xaa},
         {0x548c ,0xb8},
         {0x548d ,0xcd},
         {0x548e ,0xdd},
         {0x548f ,0xea},
         {0x5308 ,0x00}, //40 //Sharpness Auto default
         {0x5300 ,0x08},
         {0x5301 ,0x30},
         {0x5302 ,0x10},
         {0x5303 ,0x00},
         {0x5309 ,0x08},
         {0x530a ,0x30},
         {0x530b ,0x04},
         {0x530c ,0x06},
         {0x3a18 ,0x00}, //ISO 100
         {0x3a19 ,0x6c},
         {0x501d ,0x00},//AE Weight - Average
         {0x5688 ,0x11},
         {0x5689 ,0x11}, 
         {0x568a ,0x11}, 
         {0x568b ,0x11}, 
         {0x568c ,0x11},
         {0x568d ,0x11}, 
         {0x568e ,0x11}, 
         {0x568f ,0x11}, 
      },

		 
      {//SCENE_MODE_NIGHT    夜景
          {0x3a00 ,0x04}, //04 //Night mode enable
          {0x3a02 ,0x0f}, //Night mode(Half) 1/4
          {0x3a03 ,0x60},
          {0x3a14 ,0x0f},
          {0x3a15 ,0x60},
          {0x5381 ,0x1e},//CMX
          {0x5382 ,0x5b},
          {0x5383 ,0x08},
          {0x5384 ,0x0a},
          {0x5385 ,0x7e},
          {0x5386 ,0x88},
          {0x5387 ,0x7c},
          {0x5388 ,0x6c},
          {0x5389 ,0x10},
          {0x538a ,0x01},
          {0x538b ,0x98},
          {0x3a0f ,0x30},//EV default
          {0x3a10 ,0x28},
          {0x3a1b ,0x30},
          {0x3a1e ,0x26},
          {0x3a11 ,0x60},
          {0x3a1f ,0x14},
          {0x5490 ,0x20},//Gamma - low light gamma
          {0x5481 ,0x1b},
          {0x5482 ,0x26},
          {0x5483 ,0x3c},
          {0x5484 ,0x5a},
          {0x5485 ,0x68},
          {0x5486 ,0x76},
          {0x5487 ,0x80},
          {0x5488 ,0x88},
          {0x5489 ,0x8f},
          {0x548a ,0x96},
          {0x548b ,0xa3},
          {0x548c ,0xaf},
          {0x548d ,0xc4},
          {0x548e ,0xd7},
          {0x548f ,0xe8},
          {0x5308 ,0x00}, //40//Sharpness Auto default
          {0x5300 ,0x08},
          {0x5301 ,0x30},
          {0x5302 ,0x10},
          {0x5303 ,0x00},
          {0x5309 ,0x08},
          {0x530a ,0x30},
          {0x530b ,0x04},
          {0x530c ,0x06},
          {0x3a18 ,0x00},//ISO Auto - 16x
          {0x3a19 ,0xfc}, 
          {0x501d ,0x00},//AE Weight - Average
          {0x5688 ,0x11},
          {0x5689 ,0x11}, 
          {0x568a ,0x11}, 
          {0x568b ,0x11}, 
          {0x568c ,0x11},
          {0x568d ,0x11}, 
          {0x568e ,0x11}, 
          {0x568f ,0x11}, 

      	},

      { //SCENE_MODE_SPORTS  运动
           {0x3a00 ,0x00}, //04//Night mode disable
           {0x3a02 ,0x0f},//Night mode(Half) 1/4
           {0x3a03 ,0x60},
           {0x3a14 ,0x0f},
           {0x3a15 ,0x60},
           {0x5381 ,0x1e},//CMX
           {0x5382 ,0x5b},
           {0x5383 ,0x08},
           {0x5384 ,0x0a},
           {0x5385 ,0x7e},
           {0x5386 ,0x88},
           {0x5387 ,0x7c},
           {0x5388 ,0x6c},
           {0x5389 ,0x10},
           {0x538a ,0x01},
           {0x538b ,0x98},
           {0x3a0f ,0x30},//EV default
           {0x3a10 ,0x28},
           {0x3a1b ,0x30},
           {0x3a1e ,0x26},
           {0x3a11 ,0x60},
           {0x3a1f ,0x14},
           {0x5490 ,0x1d},//Gamma
           {0x5481 ,0x08},
           {0x5482 ,0x14},
           {0x5483 ,0x28},
           {0x5484 ,0x51},
           {0x5485 ,0x65},
           {0x5486 ,0x71},
           {0x5487 ,0x7d},
           {0x5488 ,0x87},
           {0x5489 ,0x91},
           {0x548a ,0x9a},
           {0x548b ,0xaa},
           {0x548c ,0xb8},
           {0x548d ,0xcd},
           {0x548e ,0xdd},
           {0x548f ,0xea},
           {0x5308 ,0x00}, //40   //Sharpness Auto default
           {0x5300 ,0x08},
           {0x5301 ,0x30},
           {0x5302 ,0x10},
           {0x5303 ,0x00},
           {0x5309 ,0x08},
           {0x530a ,0x30},
           {0x530b ,0x04},
           {0x530c ,0x06},
           {0x3a18 ,0x01},//ISO Auto - 32x
           {0x3a19 ,0xfc},
           {0x501d ,0x00}, //AE Weight - Average
           {0x5688 ,0x11},
           {0x5689 ,0x11}, 
           {0x568a ,0x11}, 
           {0x568b ,0x11}, 
           {0x568c ,0x11},
           {0x568d ,0x11}, 
           {0x568e ,0x11}, 
           {0x568f ,0x11},

	},

       {//SCENE_MODE_CANDLELIGHT 烛光
         {0x3a00 ,0x00}, //04  //Night mode disable
         {0x3a02 ,0x0f},         //Night mode(Half) 1/4
         {0x3a03 ,0x60},
         {0x3a14 ,0x0f},
         {0x3a15 ,0x60},
         {0x5381 ,0x42},//CMX
         {0x5382 ,0x5a},
         {0x5383 ,0x08},
         {0x5384 ,0x20},
         {0x5385 ,0x7e},
         {0x5386 ,0x48},
         {0x5387 ,0xe0},
         {0x5388 ,0x6c},
         {0x5389 ,0x06},
         {0x538a ,0x01},
         {0x538b ,0x9c},
         {0x3a0f ,0x30},//EV default
         {0x3a10 ,0x28},
         {0x3a1b ,0x30},
         {0x3a1e ,0x26},
         {0x3a11 ,0x60},
         {0x3a1f ,0x14},
         {0x5490 ,0x1d},//Gamma
         {0x5481 ,0x08},
         {0x5482 ,0x14},
         {0x5483 ,0x28},
         {0x5484 ,0x51},
         {0x5485 ,0x65},
         {0x5486 ,0x71},
         {0x5487 ,0x7d},
         {0x5488 ,0x87},
         {0x5489 ,0x91},
         {0x548a ,0x9a},
         {0x548b ,0xaa},
         {0x548c ,0xb8},
         {0x548d ,0xcd},
         {0x548e ,0xdd},
         {0x548f ,0xea},
         {0x5308 ,0x00}, //40  //Sharpness Auto default
         {0x5300 ,0x08},
         {0x5301 ,0x30},
         {0x5302 ,0x10},
         {0x5303 ,0x00},
         {0x5309 ,0x08},
         {0x530a ,0x30},
         {0x530b ,0x04},
         {0x530c ,0x06},
         {0x3a18 ,0x00},  //ISO Auto - 16x
         {0x3a19 ,0xfc},
         {0x501d ,0x00},//AE Weight - Average
         {0x5688 ,0x11},
         {0x5689 ,0x11}, 
         {0x568a ,0x11}, 
         {0x568b ,0x11}, 
         {0x568c ,0x11},
         {0x568d ,0x11}, 
         {0x568e ,0x11}, 
         {0x568f ,0x11}, 
         
       },
        
};

static struct msm_camera_i2c_conf_array ov5640_scene_confs[][1] = {
	{
		{ov5640_scene[0],
		ARRAY_SIZE(ov5640_scene[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_scene[1],
		ARRAY_SIZE(ov5640_scene[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_scene[2],
		ARRAY_SIZE(ov5640_scene[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	{
		{ov5640_scene[3],
		ARRAY_SIZE(ov5640_scene[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_scene[4],
		ARRAY_SIZE(ov5640_scene[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_scene[5],
		ARRAY_SIZE(ov5640_scene[5]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	{
		{ov5640_scene[6],
		ARRAY_SIZE(ov5640_scene[6]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_scene[7],
		ARRAY_SIZE(ov5640_scene[7]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	{
		{ov5640_scene[8],
		ARRAY_SIZE(ov5640_scene[8]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_scene[9],
		ARRAY_SIZE(ov5640_scene[9]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov5640_scene[10],
		ARRAY_SIZE(ov5640_scene[10]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
};

static int ov5640_scene_enum_map[] = {	
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

static struct msm_camera_i2c_enum_conf_array ov5640_scene_enum_confs = {
	.conf = &ov5640_scene_confs[0][0],
	.conf_enum = ov5640_scene_enum_map,
	.num_enum = ARRAY_SIZE(ov5640_scene_enum_map),
	.num_index = ARRAY_SIZE(ov5640_scene_confs),
	.num_conf = ARRAY_SIZE(ov5640_scene_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};
//
//cmd=0x03:single focus  ,cmd=0x08:cancel focus
int ov5640_set_af_result(struct msm_sensor_ctrl_t *s_ctrl, uint16_t cmd)
{
   int rc = 0;	 
//	 uint16_t	 ack;	
//	 int count = 30;    

       printk("%s: cmd is 0x%x\n",__func__,cmd);
	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3023, 0x01, 1);
	 if (rc<0)
	 {
	     printk("%s:AF reg I2C write 0x3023 error!-------",__func__);
	     return rc;
	 }
	 
	 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3022, cmd, 1);
	 if (rc<0)
	 {
	     printk("%s:AF reg I2C write 0x3022 error!-------",__func__);
	     return rc;
	 } 
	 
/*
	 while(count) 
	 {  
	    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x3023, &ack, 1);
	 	  if (rc<0)
	 	  {
	 	     printk("%s:msm_camera_i2c_read 0x3023 reg error!\n", __func__); 	      
	 	      return rc;
	 	  } 
	 	  	 	  
	 	  if (ack == 0x0)
	 	  {
	 	      printk("%s:i2c_read:ack = 0x%x\n", __func__,ack);
	 	      break;
	 	  }
	 	  count--;	 	    
	 	  msleep(50);
	 	  
	 }	
	 if (!count)
	 {
	 	  printk("%s:failed!!\n",__func__);
		  return -1;
	 }
	 */
	 printk("%s:success!\n", __func__);
	 return 0;	 
}
EXPORT_SYMBOL(ov5640_set_af_result);
//
//ECID:0000 2012-6-18 zhangzhao optimize the camera start up time start
#define BURST_LENGTH  128 //256  //ECID:0000 zhangzhao optimize f/w download function
static int32_t ov5640_download_af_firmware(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
//ECID:0000 zhangzhao optimize f/w download function	
	int32_t i, j;
	int32_t array_length;
	uint16_t length;
	uint8_t buffer[BURST_LENGTH];
//ECID:0000 zhangzhao optimize f/w download function	
	//struct msm_camera_sensor_info *info = NULL;
	pr_err("++++++%s: %d\n", __func__, __LINE__);


        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3000, 0x20, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
	      pr_err("----ov5640_sensor_power_up: msm_camera_i2c_write FAILS!\n");
        }
//ECID:0000 zhangzhao optimize f/w download function
	array_length = sizeof(ov5640_af_settings_start) /sizeof(ov5640_af_settings_start[0]);	
	for(i = 0; i < (array_length/BURST_LENGTH + 1); i++){
		if(ov5640_af_settings_start[i*BURST_LENGTH].reg_addr != 0x8f80 /*0x8e00*/){//zhangzhao 2012-9-18 for touch focus
			length = BURST_LENGTH;
		}else{
			length = array_length % BURST_LENGTH;
		}
		for(j = 0; j < length; j++){
			buffer[j] = ov5640_af_settings_start[i*BURST_LENGTH + j].reg_data;
		}
		
             rc = msm_camera_i2c_write_seq(s_ctrl->sensor_i2c_client,
      			ov5640_af_settings_start[i*BURST_LENGTH].reg_addr,buffer, length);
        if (rc < 0)
        {
	      pr_err("----ov5640  AF FW WRITE SEQ FAIL: msm_camera_i2c_write FAILS!\n");
        }
	}
//ECID:0000 zhangzhao optimize f/w download function

	rc = msm_sensor_write_all_conf_array(
		s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->fw_download,
		s_ctrl->msm_sensor_reg->fw_size);

	 if (rc < 0)
        {
	      pr_err("----ov5640 AF END ARRAY WRITE FAIL  : msm_camera_i2c_write FAILS!\n");
        }

	pr_err("-------%s: %d\n", __func__, __LINE__);
		return rc;
}
//ECID:0000 2012-6-18 zhangzhao optimize the camera start up time end
int ov5640_AF_proc(struct msm_sensor_ctrl_t *s_ctrl,
		                          struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	 int rc = 0;
	// int count  = 0;
	// uint16_t ack=0;	 

     af_count = 0;
	 // trig af
	 rc = ov5640_set_af_result(s_ctrl, 0x03);
	 if (rc!=0)
	 {
	     printk("trig af error!-------\n");
		 return rc;
	 }
	 /*
	 while((ack != 0x00) &&(count < 100) ){

	     rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x3029, &ack, 1);
	 if (rc<0)
	 {
	     printk("I2C write error!-------");
	     return rc;
	 }
	count ++;
	msleep(10);
	if (count>=100)  
			return -EINVAL;
      } */
	 return 0;
}

static struct msm_camera_i2c_reg_conf ov5640_panorama_17fps[] = {	

          {0x3034, 0x18},
          {0x3035, 0x31},
          {0x3036, 0x5d},
          {0x3037, 0x13},
          {0x3108, 0x01},
	   {0x3824, 0x04},
	   {0x460c, 0x20},
          {0x4837, 0x19,},//--------------------
	   {0x300e, 0x45}, //zhangzhao 45---25
          {0x380c, 0x07},
          {0x380d, 0x68},
          {0x380e, 0x03},
          {0x380f, 0xd8},		  	
};

struct msm_camera_i2c_conf_array ov5640_panorama_reg[] = {

//ECID:0000 2012-6-18 zhangzhao optimize the camera start up time start

	{&ov5640_panorama_17fps[0],
	ARRAY_SIZE(ov5640_panorama_17fps), 0, MSM_CAMERA_I2C_BYTE_DATA},
	
};

int ov5640_panorama_mode(struct msm_sensor_ctrl_t *s_ctrl,
                            struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
    int rc = 0;

    printk("fuyipeng---%s enter  value is === %d\n", __func__,value);

    if(value)  
    	{
	rc = msm_sensor_write_all_conf_array(
		s_ctrl->sensor_i2c_client,
		&ov5640_panorama_reg[0],
		ARRAY_SIZE(ov5640_panorama_reg));
	if (rc<0)
	 {
	     printk("I2C write error!-------");
	     return rc;
	 }

	pr_err("---panorama 17fps----%s: %d\n", __func__, __LINE__);
	return rc;

    	}
	pr_err("---not in panorama mode 25 fps----%s: %d\n", __func__, __LINE__);
	
	return rc;

}

//zhangzhao 2012-9-18 for touch focus start
static int32_t ov5640_set_sensor_touch_point(struct msm_sensor_ctrl_t *s_ctrl,  int xPoint, int yPoint)
{
	int rc;
	int16_t ack=11;
	int count = 0;
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3023, 0x1, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
	      pr_err("----ov5640_sensor_power_up: msm_camera_i2c_write FAILS!\n");
		return rc;	  
        }
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3022, 0x8, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
	      pr_err("----ov5640_sensor_power_up: msm_camera_i2c_write FAILS!\n");
		return rc;	  
        }
        do{
               rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x3023, &ack, 1);
	      CDBG("----the ack is ======%d\n",ack);
               if (rc<0)
               {
                   printk("I2C write error!-------");
			return rc;	   
               }
		count++;
		msleep(5);		
		}while((ack != 0)&&(count <10));
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3024, xPoint, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
	      pr_err("----ov5640_sensor_power_up: msm_camera_i2c_write FAILS!\n");
		return rc;	  
        }
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3025, yPoint ,MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
	      pr_err("----ov5640_sensor_power_up: msm_camera_i2c_write FAILS!\n");
		return rc;	  
        }
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3026, 16, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
	      pr_err("----ov5640_sensor_power_up: msm_camera_i2c_write FAILS!\n");
		return rc;	  
        }
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3027, 16 ,MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
	      pr_err("----ov5640_sensor_power_up: msm_camera_i2c_write FAILS!\n");
		return rc;	  
        }
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3023, 0x1, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
	      pr_err("----ov5640_sensor_power_up: msm_camera_i2c_write FAILS!\n");
		return rc;	  
        }
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3022, 0x81, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
	      pr_err("----ov5640_sensor_power_up: msm_camera_i2c_write FAILS!\n");
		return rc;	  
        }
	count =0;	
        do{
               rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x3023, &ack, 1);
	      CDBG("--222222--the ack is ======%d\n",ack);
               if (rc<0)
               {
                   printk("I2C write error!-------");
			return rc;	   
               }
		count++;	 
		msleep(5);
		}while((ack != 0)&&(count <10));
	return 0;
}
//zhangzhao 2012-9-18 for touch focus end
int ov5640_ctrl_cancel_autofocus(struct msm_sensor_ctrl_t *s_ctrl,
                            struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc;
    af_count = 0;
    rc= ov5640_set_sensor_touch_point(s_ctrl,40,30);//zhangzhao 2012-9-18 for touch focus
    if(rc)
	return rc;	

    return ov5640_set_af_result(s_ctrl, 0x08);
}

int ov5640_get_autofocus_status(struct msm_sensor_ctrl_t *s_ctrl,
                            struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
    int iFocusStatus = 0;
    int rc;
    uint16_t ack=11;

    printk("fuyipeng---%s enter-----\n", __func__);


    af_count++;
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x3023, &ack, 1);
    if (rc<0)
    {
        printk("I2C write error!-------");
    }
     
    if (ack == 0)
    {
         iFocusStatus = 1;
         af_count = 0;
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

//ECID:0000 zhangzhao 2012-6-25 add ev algorithm start
int ov5640_set_effect(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	printk("%s enter\n", __func__);
	printk("%s:setting enum is %x,value =%d\n",__func__,ctrl_info->ctrl_id,value);
	ov5640_effect_mode = value;
	rc = msm_sensor_s_ctrl_by_enum(s_ctrl,ctrl_info, value);
	return rc;
}
//ECID:0000 zhangzhao 2012-6-25 add ev algorithm end

struct msm_sensor_v4l2_ctrl_info_t ov5640_v4l2_ctrl_info[] = {
	{
		.ctrl_id = V4L2_CID_SATURATION,
		.min = MSM_V4L2_SATURATION_L0,
		.max = MSM_V4L2_SATURATION_L4,
		.step = 1,
		.enum_cfg_settings = &ov5640_saturation_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},
	
	{
		.ctrl_id = V4L2_CID_CONTRAST,
		.min = MSM_V4L2_CONTRAST_L0,
		.max = MSM_V4L2_CONTRAST_L4,
		.step = 1,
		.enum_cfg_settings = &ov5640_contrast_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},

	{
		.ctrl_id = V4L2_CID_SHARPNESS,
		.min = MSM_V4L2_SHARPNESS_L0,
		.max = MSM_V4L2_SHARPNESS_L4,
		.step = 1,
		.enum_cfg_settings = &ov5640_sharpness_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},

	{
		.ctrl_id = V4L2_CID_BRIGHTNESS,
		.min = MSM_V4L2_BRIGHTNESS_L0,
		.max = MSM_V4L2_BRIGHTNESS_L4,
		.step = 1,
		.enum_cfg_settings = &ov5640_brightness_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},
	
      { 
		.ctrl_id = V4L2_CID_EXPOSURE,
		.min = MSM_V4L2_EXPOSURE_N2,
		.max = MSM_V4L2_EXPOSURE_P2,
		.step = 1,
		.enum_cfg_settings = &ov5640_exposure_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},  
	
	{
		.ctrl_id = V4L2_CID_COLORFX,
		.min = MSM_V4L2_EFFECT_OFF,
		.max = MSM_V4L2_EFFECT_POSTERAIZE,
		.step = 1,
		.enum_cfg_settings = &ov5640_effect_enum_confs,
		.s_v4l2_ctrl = ov5640_set_effect,//ECID:0000 zhangzhao 2012-6-25 add ev algorithm start

	},

	{
		.ctrl_id = V4L2_CID_AUTO_WHITE_BALANCE,
		.min = MSM_V4L2_WB_AUTO,
		.max = MSM_V4L2_WB_CLOUDY_DAYLIGHT,
		.step = 1,
		.enum_cfg_settings = &ov5640_awb_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},	
	
	{
		.ctrl_id = V4L2_CID_POWER_LINE_FREQUENCY,//antibanding
		.min = MSM_V4L2_POWER_LINE_60HZ,
		.max = MSM_V4L2_POWER_LINE_AUTO,
		.step = 1,
		.enum_cfg_settings = &ov5640_antibanding_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},  

       {
		.ctrl_id = V4L2_CID_ISO,//antibanding
		.min = MSM_V4L2_ISO_AUTO,
		.max = MSM_V4L2_ISO_1600,
		.step = 1,
		.enum_cfg_settings = &ov5640_iso_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},  

	{
		.ctrl_id = V4L2_CID_SCENE,
		.min = MSM_V4L2_SCENE_AUTO,
		.max = MSM_V4L2_SCENE_PORTRAIT,
		.step = 1,
		.enum_cfg_settings = &ov5640_scene_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},
	
	{
		.ctrl_id = V4L2_CID_FOCUS_AUTO,
		//.min = MSM_V4L2_FOCUS_NORMAL,
		//.max = MSM_V4L2_FOCUS_MAX,
		//.step = 1,
		//.enum_cfg_settings = &ov5640_awb_enum_confs,
		.s_v4l2_ctrl = ov5640_AF_proc,
	},		     	

    {
        .ctrl_id = V4L2_CID_PANORAMA_MODE,

        .s_v4l2_ctrl = ov5640_panorama_mode,
    },

    {
        .ctrl_id = V4L2_CID_GET_AUTOFOCUS_STATUS,

        .s_v4l2_ctrl = ov5640_get_autofocus_status,
    },

    {
        .ctrl_id = V4L2_CID_CANCEL_AUTOFOCUS,

        .s_v4l2_ctrl = ov5640_ctrl_cancel_autofocus,
    },
    
};



static struct v4l2_subdev_info ov5640_subdev_info[] = {
	{
	.code       = V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt        = 1,
	.order      = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array ov5640_init_conf[] = {
	{&ov5640_recommend_settings[0],
	ARRAY_SIZE(ov5640_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_i2c_conf_array ov5640_confs[] = {	
{&ov5640_snapshot_5m_array[0],
	ARRAY_SIZE(ov5640_snapshot_5m_array), 0, MSM_CAMERA_I2C_BYTE_DATA},//////////////snapshot
{&ov5640_capture_to_preview_settings_array[0],
	ARRAY_SIZE(ov5640_capture_to_preview_settings_array), 0, MSM_CAMERA_I2C_BYTE_DATA},///////////////preview
};


struct msm_camera_i2c_conf_array af_firmware_download[] = {

//ECID:0000 2012-6-18 zhangzhao optimize the camera start up time start

	{&ov5640_af_settings_array_end[0],
	ARRAY_SIZE(ov5640_af_settings_array_end), 0, MSM_CAMERA_I2C_BYTE_DATA},
	
};


static struct msm_sensor_output_info_t ov5640_dimensions[] = {
	{
		//////////////////////////////snapshot
		.x_output = 2592,
		.y_output = 1944,
		.line_length_pclk = 2592,
		.frame_length_lines = 1944,
		.vt_pixel_clk = 96000000,
		.op_pixel_clk = 96000000,
		.binning_factor = 1,
	},
	
	{
	//////////////////////////////preview
		.x_output = 1280,
		.y_output = 960,
		.line_length_pclk = 1280,
		.frame_length_lines = 960,
		.vt_pixel_clk = 96000000,
		.op_pixel_clk = 96000000,
		.binning_factor = 1,
	},
	
};


/*static struct msm_camera_csid_vc_cfg ov5640_cid_cfg[] = {
	{0, CSI_YUV422_8, CSI_DECODE_8BIT},   //0x1E
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};*/

static struct msm_camera_csi_params ov5640_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x10,
};

static struct msm_camera_csi_params *ov5640_csi_params_array[] = {
	&ov5640_csi_params,
	&ov5640_csi_params,
};


static struct msm_sensor_output_reg_addr_t ov5640_reg_addr = {
	.x_output = 0x3808,
	.y_output = 0x380a,
	.line_length_pclk = 0x380c,
	.frame_length_lines = 0x380e,
};

static struct msm_sensor_id_info_t ov5640_id_info = {
	.sensor_id_reg_addr = 0x300A,
	.sensor_id = 0x56, //jidewei 2012-01-10  
};


static struct msm_sensor_exp_gain_info_t ov5640_exp_gain_info = {
	.coarse_int_time_addr = 0x3501,
	.global_gain_addr = 0x350a,
	.vert_offset = 6,
};


//ECID:0000 zhangzhao 2012-6-25 add ev algorithm start
static int32_t ov5640_hw_ae_parameter_record(struct msm_sensor_ctrl_t *s_ctrl)
{
    uint16_t ret_l, ret_m, ret_h;
    uint16_t temp_l, temp_m, temp_h;
    uint32_t extra_line;
    int32_t rc = 0;
    uint16_t night;
    uint16_t LencQ_H, LencQ_L;//zhangzhao	

    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3503, 0x07,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3500, &ret_h,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;   
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3501, &ret_m,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3502, &ret_l,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x558c, &g_preview_uv,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;   

    printk("ov5640_parameter_record:ret_h=%x,ret_m=%x,ret_l=%x\n",ret_h,ret_m,ret_l);

    temp_l = ret_l & 0x00FF;
    temp_m = ret_m & 0x00FF;
    temp_h = ret_h & 0x00FF;
    g_preview_exposure = (temp_h << 12) + (temp_m << 4) + (temp_l >> 4);

    printk("ov5640_parameter_record:ret_l=%x\n",ret_l);
    printk("ov5640_parameter_record:ret_m=%x\n",ret_m);
    printk("ov5640_parameter_record:ret_h=%x\n",ret_h);
   /****************neil add for night mode ************************/ 
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x350c, &ret_h,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x350d, &ret_l,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;
	
    extra_line = (ret_h <<8) + ret_l;

    g_preview_exposure = g_preview_exposure + extra_line;
   /******************************************************************/
    /*
      * Set as global metering mode
      */
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x380e, &ret_h,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x380f, &ret_l,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;
	
    printk("ov5640_parameter_record:0x380e=%x  0x380f=%x\n",ret_h,ret_l);

    g_preview_line_width = ret_h & 0xff;
    g_preview_line_width = g_preview_line_width * 256 + ret_l;
    
    g_preview_line_width = g_preview_line_width + extra_line; // neil add for night mode
    
    //Read back AGC gain for preview
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x350b, &g_preview_gain_low,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x350a, &g_preview_gain_high,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;
	
    g_preview_gain_low = g_preview_gain_low & 0xff;
    g_preview_gain = g_preview_gain_high * 256 + g_preview_gain_low;

      LencQ_H=0;
	   LencQ_L=0;
	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x350a, &LencQ_H,MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0)
		return rc;
	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x350b, &LencQ_L,MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0)
		return rc;	   

	//WriteSCCB(0x78, 0x5054, LencQ_H); 
	//WriteSCCB(0x78, 0x5055, LencQ_L); 
	//WriteSCCB(0x78, 0x504d, 0x02); // Manual mini Q	

	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5054, LencQ_H,MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0)
		return rc;	
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5055, LencQ_L,MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0)
		return rc;	
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x504d, 0x02,MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0)
		return rc;	


    YAVG = 0;
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x56a1, &YAVG,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
       return rc;

    if (0 == g_preview_frame_rate){
        g_preview_frame_rate = PREVIEW_FRAMERATE;
    }

    /****************************disable night mode**********************/
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3a00, &night,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;
    
    night = night & 0xfb;
    
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3a00, night,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x350c, 0x00,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x350d, 0x00,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;
   /***********************************************************/    
    return 0;
}
static int preview_switch_to_snapshot = 0;//ECID:0000 zhangzhao 2012-6-25 add ev algorithm start
static int32_t ov5640_hw_ae_transfer(struct msm_sensor_ctrl_t *s_ctrl)
{
    long rc = 0;
    uint16_t exposure_low;
    uint16_t exposure_mid;
    uint16_t exposure_high;
    uint32_t capture_exposure;
    uint32_t capture_gain;
    uint16_t lines_10ms;
    uint32_t m_60Hz = 0;
    uint16_t reg_l,reg_h;
    uint16_t preview_lines_max = g_preview_line_width;
    uint16_t gain = g_preview_gain;
    uint32_t capture_lines_max;
  //  int array_length = 0;
   // int i;
//	int err;

    printk("1ov5640_hw_ae_transfer:gain=%x\n",gain);
	

    
   //change resolution from VGA to 5M here
    /*
      * Set as global metering mode
      */

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x380e, &reg_h,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x380f, &reg_l,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        return rc;
	
    capture_lines_max = reg_h & 0xff;
    capture_lines_max = (capture_lines_max << 8) + reg_l;

    printk("ov5640_hw_ae_transfer:0x380e=%x  0x380f=%x\n",reg_h,reg_l);
	
    if(m_60Hz== 1){
        lines_10ms = 2 * CAPTURE_FRAMERATE * capture_lines_max / 12000;
    }
    else{
        lines_10ms = 2 * CAPTURE_FRAMERATE * capture_lines_max / 10000;
    }

    printk("ov5640_hw_ae_transfer:lines_10ms=%x\n",lines_10ms);

    if(preview_lines_max == 0){
        preview_lines_max = 1;
    }

    capture_exposure = g_preview_exposure * 84 / 93;
    capture_exposure = capture_exposure * 1896 / 2844;
	
    #if 0
    //kenxu add for reduce noise under dark condition
    if(gain > 32) //gain > 2x, gasin / 2, exposure * 2;
    {
        gain = gain / 2;
        capture_exposure = capture_exposure * 2;
    }
    if(gain > 32) //gain > 2x, gain / 2, exposure * 2;
    {
        gain = gain / 2;
        capture_exposure = capture_exposure * 2;
    }
    #endif
	
    //kenxu add for reduce noise under dark condition
    if(gain > 32) //gain > 2x, gain / 2, exposure * 2;
    {
        gain = gain / 2;
        capture_exposure = capture_exposure * 2;
		  
        if(gain > 64) //reach to max gain 16x, then improve contrast to pass SNR test under low light
        {
          	gain = gain / 2;
          	capture_exposure = capture_exposure * 2;	
          	
          	//extend vts to set more exposure time
            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x380e, 0x0a,MSM_CAMERA_I2C_BYTE_DATA);
            if (rc < 0)
              return rc;
            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x380f, 0x40,MSM_CAMERA_I2C_BYTE_DATA);
            if (rc < 0)
              return rc;
		  
          	printk("ov5640_hw_ae_transfer:gain33333 =%x \n",gain);     	
          	if(YAVG < 9){
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x4009, 0x40,MSM_CAMERA_I2C_BYTE_DATA);
	            if (rc < 0)
	              return rc;
          	}
          	else if(YAVG == 9){
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x4009, 0x3c,MSM_CAMERA_I2C_BYTE_DATA);
	            if (rc < 0)
	              return rc;
          	}			
          	else if(YAVG == 10){
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x4009, 0x38,MSM_CAMERA_I2C_BYTE_DATA);
	            if (rc < 0)
	              return rc;
          	}	
          	else if(YAVG == 11){
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x4009, 0x34,MSM_CAMERA_I2C_BYTE_DATA);
	            if (rc < 0)
	              return rc;
          	}			
          	else if(YAVG == 12){
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x4009, 0x30,MSM_CAMERA_I2C_BYTE_DATA);
	            if (rc < 0)
	              return rc;
          	}	
          	else if(YAVG == 13){
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x4009, 0x2c,MSM_CAMERA_I2C_BYTE_DATA);
	            if (rc < 0)
	              return rc;
          	}			
          	else if(YAVG == 14){
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x4009, 0x28,MSM_CAMERA_I2C_BYTE_DATA);
	            if (rc < 0)
	              return rc;
          	}	
          	else if(YAVG == 15){
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x4009, 0x24,MSM_CAMERA_I2C_BYTE_DATA);
	            if (rc < 0)
	              return rc;
          	}			
          	else if(YAVG == 16){
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x4009, 0x20,MSM_CAMERA_I2C_BYTE_DATA);
	            if (rc < 0)
	              return rc;
          	}	
          	else if(YAVG == 17){
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x4009, 0x1c,MSM_CAMERA_I2C_BYTE_DATA);
	            if (rc < 0)
	              return rc;
          	}			
          	else if(YAVG == 18){
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x4009, 0x18,MSM_CAMERA_I2C_BYTE_DATA);
	            if (rc < 0)
	              return rc;
          	}	
          	else if(YAVG == 19){
	            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x4009, 0x14,MSM_CAMERA_I2C_BYTE_DATA);
	            if (rc < 0)
	              return rc;
            }
        }
    }
    capture_gain = gain;
    exposure_low = ((unsigned char)capture_exposure) << 4;
    exposure_mid = (unsigned char)(capture_exposure >> 4) & 0xff;
    exposure_high = (unsigned char)(capture_exposure >> 12);
 
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x350b, (capture_gain & 0x00ff),MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0){
        return rc;
    }    
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x350a, ((capture_gain & 0xff00)>>8),MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0){
        return rc;
    }    
	
    //m_iWrite0x3502=exposure_low;
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3502, exposure_low,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0){
        return rc;
    }    
    printk("exposure_low  =%x \n",exposure_low);

    //m_iWrite0x3501=exposure_mid;
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3501, exposure_mid,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0){
        return rc;
    }    
    printk("exposure_mid  =%x \n",exposure_mid);

    //m_iWrite0x3500=exposure_high;
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3500, exposure_high,MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0){
        return rc;
    }
    printk("exposure_high  =%x  \n",exposure_high);

    if(ov5640_effect_mode == MSM_V4L2_EFFECT_OFF)
    {
       CDBG("ov5640 write ov5640_effect_mode----snapshot!%s\n", __func__);

	    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x5583, &g_preview_u,MSM_CAMERA_I2C_BYTE_DATA);
	    if (rc < 0)
	        return rc;
	    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x5584, &g_preview_v,MSM_CAMERA_I2C_BYTE_DATA);
	    if (rc < 0)
	        return rc;	    
	    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x5588, &reg_h,MSM_CAMERA_I2C_BYTE_DATA);
	    if (rc < 0)
	        return rc;
	    reg_h |= 0x41;
	    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5588, reg_h,MSM_CAMERA_I2C_BYTE_DATA);
	    if (rc < 0)
	        return rc;
	    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5583,g_preview_uv,MSM_CAMERA_I2C_BYTE_DATA);
	    if (rc < 0)
	        return rc;
	    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5584, g_preview_uv,MSM_CAMERA_I2C_BYTE_DATA);
	    if (rc < 0)
	        return rc;
	preview_switch_to_snapshot =1;	//ECID:0000 zhangzhao 2012-6-25 add ev algorithm start
    }
    mdelay(250); //inoder to deduce snapshot time 
    return 0;
}



int32_t ov5640_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
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
		
		
		if (NULL != s_ctrl->func_tbl->sensor_download_af_firmware)
			{

		      rc=s_ctrl->func_tbl->sensor_download_af_firmware(s_ctrl);
        		if(!rc)
        		{
        		  printk("%s:ov5640 AF FW download success\n",__func__);
        		  download_flag=1;   //download success is 1
        		}
		}
		
		csi_config = 0;
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		 printk("PERIODIC : %d\n", res);
		 printk("%s:sensor_name = %s, res = %d\n",__func__, s_ctrl->sensordata->sensor_name, res);

		switch(res)
		{
			case 0://snapshot
				{

#ifdef CONFIG_ZTE_CAMERA_SOC_FLASH
				uint16_t reg1;
				led_mode_t  flash_mode;
				flash_mode = msm_flash_mode_get();
				printk("%s, flash mode %d\n", __func__, flash_mode);
                            if (msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x56a1, &reg1,MSM_CAMERA_I2C_BYTE_DATA) < 0) {
                            printk("ov5642 read reg_value wrong\n");
                            return -EIO;
                            }
				printk("%s, expo value %d\n", __func__, reg1);					
				if ((reg1 <0x24 && (flash_mode == LED_MODE_AUTO)) || (flash_mode == LED_MODE_ON)) {
					camera_flash_set_led_state(MSM_CAMERA_LED_HIGH, 0x0a);	
				}
#endif					
				rc=ov5640_hw_ae_parameter_record(s_ctrl);
				if(rc<0)
				{
					CDBG("ov5640_hw_ae_parameter_record ERROR\n");
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

				rc=ov5640_hw_ae_transfer(s_ctrl);
				if(rc<0)
					{
					CDBG("ov5640_hw_ae_transfer ERROR\n");
					return rc;
				}

	                    	msleep(150);				
				s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
				break;			
			     }
			case 1://preview
			default :
				{
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
				if (preview_switch_to_snapshot)	//ECID:0000 zhangzhao 2012-6-25 add ev algorithm start						
				{
     			         uint16_t reg_value;
				     preview_switch_to_snapshot =0;	//ECID:0000 zhangzhao 2012-6-25 add ev algorithm start	 
                                 if(ov5640_effect_mode == MSM_V4L2_EFFECT_OFF){
                                 CDBG("ov5640 write ov5640_effect_mode----preview!%s\n", __func__);
                                 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5583, g_preview_u,MSM_CAMERA_I2C_BYTE_DATA);
                                 if (rc < 0) {
                                 CDBG("ov5640 write reg 0x5583 error!%s\n", __func__);
                                 return rc;
                                 }
                                 rc =msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5584, g_preview_v,MSM_CAMERA_I2C_BYTE_DATA);
                                 if (rc < 0) {
                                 CDBG("ov5640 write reg 0x5584 error!%s\n", __func__);
                                 return rc;
                                 }
                                 if (msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x5588, &reg_value,MSM_CAMERA_I2C_BYTE_DATA) < 0) {
                                 CDBG("ov5642 read reg_value wrong\n");
                                 return -EIO;
                                 }
                                 reg_value &= 0xbf;
                                 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5588, reg_value,MSM_CAMERA_I2C_BYTE_DATA);
                                 if (rc < 0) {
                                 CDBG("ov5640 write reg 0x5588 error!%s\n", __func__);
                                 return rc;
                                 }
                                 }
                                 msleep(5);
                    		}
                                   msleep(100);
                    		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
                                   msleep(250);//zhangzhao 2012-8-17 optimize green screen problem
                    		
                                     // cancel af when snapshot switch to preview     
                                   if ((1 == res)&&(1 == download_flag))
                                   {
                    			if(NULL !=s_ctrl->func_tbl->sensor_set_af_result )	  	
                    			 rc=s_ctrl->func_tbl->sensor_set_af_result(s_ctrl);			
                                   }  
                    		
                    		break;
				}
			}
	}
	return rc;
}
//ECID:0000 zhangzhao 2012-6-25 add ev algorithm end
//ECID:0000 zhangzhao 2012-6-25 add ev algorithm end

static ssize_t camera_id_read_proc(char *page,char **start,off_t off,int count,int *eof,void* data)
{		 	
    int ret;
	
    unsigned char *camera_status = "BACK Camera ID:OV5640-5.0M";	
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


#if 0

static int ov5640_pwdn_gpio;
static int ov5640_reset_gpio;

static void ov5640_release_gpio(void)
{

	pr_err("%s release gpio %d, %d\n", __func__,ov5640_pwdn_gpio,ov5640_reset_gpio);

	gpio_free(ov5640_pwdn_gpio);
	gpio_free(ov5640_reset_gpio);
	
}

static void ov5640_hw_reset(void)
{
	CDBG("--CAMERA-- %s ... (Start...)\n", __func__);
	gpio_set_value(ov5640_reset_gpio, 1);   /*reset camera reset pin*/
	usleep_range(5000, 5100);
	gpio_set_value(ov5640_reset_gpio, 0);
	usleep_range(5000, 5100);
	gpio_set_value(ov5640_reset_gpio, 1);
	msleep(25);//ECID:0000 2012-6-18 zhangzhao optimize the camera start up time start
	CDBG("--CAMERA-- %s ... (End...)\n", __func__);
}

static int ov5640_probe_init_gpio(const struct msm_camera_sensor_info *data)
{
	int rc = 0;
	CDBG("%s: entered\n", __func__);

	ov5640_pwdn_gpio = data->sensor_pwd;
	ov5640_reset_gpio = data->sensor_reset ;

	CDBG("%s: pwdn_gpio:%d, reset_gpio:%d\n", __func__,
			ov5640_pwdn_gpio, ov5640_reset_gpio);

	rc = gpio_request(ov5640_pwdn_gpio, "ov5640-pwd");
	if (rc < 0)
		{
		pr_err("%s: gpio_request ov5640_pwdn_gpio failed!",
			 __func__);
		goto gpio_request_failed;
		}

	pr_debug("gpio_tlmm_config %d\r\n", ov5640_pwdn_gpio);
	rc = gpio_tlmm_config(GPIO_CFG(ov5640_pwdn_gpio, 0,
		GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN,
		GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("%s:unable to enable Powr Dwn gpio for main camera!\n",
			 __func__);
		gpio_free(ov5640_pwdn_gpio);
		goto gpio_request_failed;
	}

	gpio_direction_output(ov5640_pwdn_gpio, 1);

	rc = gpio_request(ov5640_reset_gpio, "ov5640-reset");
	if (rc < 0)
		{
		pr_err("%s: gpio_request ov5640_reset_gpio failed!",
			 __func__);
		goto gpio_request_failed2;
		}

	pr_debug("gpio_tlmm_config %d\r\n", ov5640_reset_gpio);
	rc = gpio_tlmm_config(GPIO_CFG(ov5640_reset_gpio, 0,
		GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN,
		GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("%s: unable to enable reset gpio for main camera!\n",
			 __func__);
		gpio_free(ov5640_reset_gpio);
		goto gpio_request_failed2;
	}

	gpio_direction_output(ov5640_reset_gpio, 1);

	if (data->sensor_reset_enable)
		gpio_direction_output(data->sensor_reset, 1);

	gpio_direction_output(data->sensor_pwd, 1);

	return rc;
gpio_request_failed2:
		gpio_free(ov5640_pwdn_gpio);
gpio_request_failed:
	return rc;
}
static void ov5640_power_on(void)
{
	CDBG("%s\n", __func__);
#if ((defined CONFIG_PROJECT_P865E01)||(defined CONFIG_PROJECT_P825A20))
	gpio_set_value(39, 1);//power down 7692
	usleep_range(5000, 8000);
	gpio_set_value(49, 0);//zhangzhao switch mclk
#endif	
	usleep_range(9000, 9100);
	gpio_set_value(ov5640_pwdn_gpio, 0); //power up
	usleep_range(5000, 8000);
}

static void ov5640_power_down(void)
{
	CDBG("%s\n", __func__);
	gpio_set_value(ov5640_pwdn_gpio, 1);//power down 
	usleep_range(5000, 8000);
#if ((defined CONFIG_PROJECT_P865E01)||(defined CONFIG_PROJECT_P825A20))	
	gpio_set_value(49, 1);//zhangzhao
	usleep_range(5000, 8000);
	gpio_set_value(39, 1);//zhangzhao
#endif	
	usleep_range(5000, 8000);
}

int32_t ov5640_sensor_set_af_rect(struct msm_sensor_ctrl_t *s_ctrl, 
                                        struct msm_sensor_af_rect_data *rect)
{
//zhangzhao 2012-9-18 for touch focus start
    int32_t rc = 0;
    int xPoint = 0;
    int yPoint = 0;

    if (NULL == s_ctrl || NULL == rect)
    {
        return rc;
    }

    CDBG("rect.x:%d   rect.y:%d \n", rect->x, rect->y);

    xPoint = (rect->x + rect->dx / 2)/8;
    yPoint = (rect->y + rect->dy / 2)/8;

    CDBG("-[------------]-----rect.x:%d   rect.y:%d \n", xPoint, yPoint);
   rc= ov5640_set_sensor_touch_point(s_ctrl,xPoint,yPoint);
//zhangzhao 2012-9-18 for touch focus end
    return rc;
}

int32_t ov5640_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	struct msm_camera_sensor_info *info = NULL;

	printk("%s: %d\n", __func__, __LINE__);
  
	info = s_ctrl->sensordata;

	rc = msm_sensor_power_up(s_ctrl);
	if (rc < 0) {
		printk("%s: msm_sensor_power_up failed\n", __func__);
		return rc;
	}

	
	/* turn on LDO for PVT */
	//if (info->pmic_gpio_enable)
	//	lcd_camera_power_onoff(1);

	ov5640_power_down();
	ov5640_power_on();
	usleep_range(5000, 5100);

	if (info->sensor_reset_enable)
		ov5640_hw_reset();
	//else
	//	ov5640_sw_reset(s_ctrl);

        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x300e, 0x0045, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
	      CDBG("ov5640_sensor_power_up: msm_camera_i2c_write FAILS!\n");
		goto power_up_fail;
        }

	return rc;

power_up_fail:
	msm_sensor_power_down(s_ctrl);
	CDBG("ov5640_sensor_power_up: OV5640 SENSOR POWER UP FAILS!\n");
	//if (info->pmic_gpio_enable)
	//	lcd_camera_power_onoff(0);
	return rc;
}


int32_t ov5640_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	//struct msm_camera_sensor_info *info = NULL;

	CDBG("%s: %d\n", __func__, __LINE__);

	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x300e, 0x005d, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
	      CDBG("ov5640_sensor_power_down: msm_camera_i2c_write FAILS!\n");
		return rc;
        }

	ov5640_power_down();

	rc = msm_sensor_power_down(s_ctrl);
	if (rc < 0) {
		CDBG("%s: msm_sensor_power_down failed\n", __func__);
		return rc;
	}
	//info = s_ctrl->sensordata;

		return rc;


}
//ECID:0000 2012-6-18 zhangzhao optimize the camera start up time start


int32_t ov5640_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	struct msm_camera_sensor_info *s_info;
	
	s_info = client->dev.platform_data;
	if (s_info == NULL) {
		pr_err("%s %s NULL sensor data\n", __func__, client->name);
		return -EFAULT;
	}
      rc = ov5640_probe_init_gpio(s_info);
      if(rc < 0)
      	{
		pr_err("%s probe faild--gpio requeset faild-----\n", __func__);
		return -EFAULT;
	  }
	
	rc = msm_sensor_i2c_probe(client, id);
      if(rc < 0)
      	 {
		pr_err("%s probe faild-------\n", __func__);
		goto probe_failed;
	  }
	pr_err("%s probe OK+++++++++++\n", __func__);
	return rc;

probe_failed:
	ov5640_release_gpio();
	return rc;
}
#else
static int32_t ov5640_sensor_i2c_probe(struct i2c_client *client,
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
#endif
int32_t ov5640_sensor_set_af_rect(struct msm_sensor_ctrl_t *s_ctrl, 
                                        struct msm_sensor_af_rect_data *rect)
{
//zhangzhao 2012-9-18 for touch focus start
    int32_t rc = 0;
    int xPoint = 0;
    int yPoint = 0;

    if (NULL == s_ctrl || NULL == rect)
    {
        return rc;
    }

    CDBG("rect.x:%d   rect.y:%d \n", rect->x, rect->y);

    xPoint = (rect->x + rect->dx / 2)/8;
    yPoint = (rect->y + rect->dy / 2)/8;

    CDBG("-[------------]-----rect.x:%d   rect.y:%d \n", xPoint, yPoint);
   rc= ov5640_set_sensor_touch_point(s_ctrl,xPoint,yPoint);
//zhangzhao 2012-9-18 for touch focus end
    return rc;
}
static const struct i2c_device_id ov5640_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&ov5640_s_ctrl},
	{ }
};

static struct i2c_driver ov5640_i2c_driver = {
	.id_table = ov5640_i2c_id,
	.probe  = ov5640_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov5640_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};
static int __init msm_sensor_init_module(void)
{
       printk(KERN_ERR "%s():cam here!-----%d\n", __func__,  __LINE__); 
	//return platform_driver_register(&ov5640_driver);
	return i2c_add_driver(&ov5640_i2c_driver);
}

static struct v4l2_subdev_core_ops ov5640_subdev_core_ops ={
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

	static struct v4l2_subdev_video_ops ov5640_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops ov5640_subdev_ops = {
	.core = &ov5640_subdev_core_ops,
	.video  = &ov5640_subdev_video_ops,
};



static int ov5640_sensor_cancel_af(struct msm_sensor_ctrl_t *s_ctrl)
{
//zhangzhao 2012-9-18 for touch focus start
	int rc =0;
	 CDBG("%s:++++++++++++++++++++!\n", __func__);
    af_count = 0;
    rc= ov5640_set_sensor_touch_point(s_ctrl,40,30);
    if(rc)
	return rc;	
//zhangzhao 2012-9-18 for touch focus end

        return ov5640_set_af_result(s_ctrl, 0x08);
}
//ECID:0000 2012-6-18 zhangzhao optimize the camera start up time end

/*[617001744332]modified by zhangzhao 2012-12-4 for camera flash*/
static int32_t ov5640_sensor_flash_auto_state(struct msm_sensor_ctrl_t *s_ctrl, 
                                        uint8_t *AutoFlash)
{
    int32_t rc = 0;
    uint16_t reg_value;


    if (msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x56a1, &reg_value,MSM_CAMERA_I2C_BYTE_DATA) < 0) {
      pr_err("ov5642 read reg_value wrong\n");
      return -EIO;
     }

    printk("%s, expo value %d\n", __func__, reg_value);

    if (reg_value < 0x24)
    {
        *AutoFlash = 1;
    }
    else
    {
        *AutoFlash = 0;
    }

    return rc;
}
/*[617001744332]modified by zhangzhao 2012-12-4 for camera flash*/



static struct msm_sensor_fn_t ov5640_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	//.sensor_group_hold_on = msm_sensor_group_hold_on,
	//.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	//.sensor_write_exp_gain = ov5640_write_exp_gain,
	//.sensor_write_snapshot_exp_gain = ov5640_write_exp_gain,
	.sensor_csi_setting = ov5640_sensor_setting,////ECID:0000 zhangzhao 2012-6-25 add ev algorithm 
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	//.sensor_open_init = ov5640_sensor_open_init,
	//.sensor_release = ov5640_sensor_release,
	.sensor_power_up = msm_sensor_power_up,//msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,//msm_sensor_power_down,
    .sensor_set_af_rect = ov5640_sensor_set_af_rect,
	//.sensor_probe = msm_sensor_probe,
	//.sensor_test =  msm_sensor_test,
	.sensor_download_af_firmware = ov5640_download_af_firmware,//ECID:0000 2012-6-18 zhangzhao optimize the camera start up time start
	.sensor_set_af_result = ov5640_sensor_cancel_af,//ECID:0000 2012-6-18 zhangzhao optimize the camera start up time start
        .sensor_flash_auto_state = ov5640_sensor_flash_auto_state,/*[617001744332]modified by zhangzhao 2012-12-4 for camera flash*/
};

static struct msm_sensor_reg_t ov5640_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = ov5640_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(ov5640_start_settings),
	.stop_stream_conf = ov5640_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(ov5640_stop_settings),
	.group_hold_on_conf = ov5640_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(ov5640_groupon_settings),
	.group_hold_off_conf = ov5640_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(ov5640_groupoff_settings),
	.init_settings = &ov5640_init_conf[0],
	.init_size = ARRAY_SIZE(ov5640_init_conf),

	
	.fw_download = &af_firmware_download[0],
	.fw_size = ARRAY_SIZE(af_firmware_download),		
	
	
	.mode_settings = &ov5640_confs[0],
	.output_settings = &ov5640_dimensions[0],
	.num_conf = ARRAY_SIZE(ov5640_confs),
};

static struct msm_sensor_ctrl_t ov5640_s_ctrl = {
	.msm_sensor_reg = &ov5640_regs,
	
	.msm_sensor_v4l2_ctrl_info = ov5640_v4l2_ctrl_info,   
	.num_v4l2_ctrl = ARRAY_SIZE(ov5640_v4l2_ctrl_info), 
	
	.sensor_i2c_client = &ov5640_sensor_i2c_client,
	.sensor_i2c_addr = 0x78,
	.sensor_output_reg_addr = &ov5640_reg_addr,
	.sensor_id_info = &ov5640_id_info,
	.sensor_exp_gain_info =&ov5640_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &ov5640_csi_params_array[0],
	.msm_sensor_mutex = &ov5640_mut,
	.sensor_i2c_driver = &ov5640_i2c_driver,
	.sensor_v4l2_subdev_info = ov5640_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov5640_subdev_info),
	.sensor_v4l2_subdev_ops = &ov5640_subdev_ops,
	.func_tbl = &ov5640_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,  
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Omnivision 5MP YUV sensor driver");
MODULE_LICENSE("GPL v2");



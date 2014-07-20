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
#define SENSOR_NAME "ov2675"

static unsigned int ov2675_preview_exposure;
static unsigned int ov2675_gain;
static unsigned short ov2675_preview_maxlines;
static int m_60Hz = FALSE;
#define Capture_Framerate 750     //7.5fps capture frame rate
#define g_Preview_FrameRate 2000  //15fps preview frame rate //3000

DEFINE_MUTEX(ov2675_mut);
static struct msm_sensor_ctrl_t ov2675_s_ctrl;


static struct msm_camera_i2c_reg_conf ov2675_start_settings[] = {
		{0x3008,0x02},
};

static struct msm_camera_i2c_reg_conf ov2675_stop_settings[] = {
		{0x3008,0x42},
};

static struct msm_camera_i2c_reg_conf ov2675_recommend_settings[] = 
{
     //@@initial
     //99 640 480
     //98 0 0
     //;IO & Clock & Analog Setup
     {0x308c ,0x80},
     {0x308d ,0x0e},
     {0x360b ,0x00},
     {0x30b0 ,0xff},
     {0x30b1 ,0xff},
     {0x30b2 ,0x04},
     
     {0x3082 ,0x01},
     {0x30f4 ,0x01},
     {0x3090 ,0x43},
     {0x3091 ,0xc0},
     {0x30ac ,0x42},
     
     {0x30d1 ,0x08},
     {0x30a8 ,0x54},
     {0x3015 ,0x02},
     {0x3093 ,0x00},
     {0x307e ,0xe5},
     {0x3079 ,0x00},
     {0x30aa ,0x52},
     {0x3017 ,0x40},
     {0x30f3 ,0x83},
     {0x306a ,0x0c},
     {0x306d ,0x00},
     {0x336a ,0x3c},
     {0x3076 ,0x6a},
     {0x30d9 ,0x95},
     {0x3016 ,0x52},
     {0x3601 ,0x30},
     {0x304e ,0x88},
     {0x30f1 ,0x82},
     {0x306f ,0x14},
     {0x302a ,0x02},
     {0x302b ,0x6a},
     
     //;D5060
     {0x30af ,0x00},
     {0x3048 ,0x1f}, 
     {0x3049 ,0x4e},  
     {0x304a ,0x20},  
     {0x304f ,0x20},  
     {0x304b ,0x02}, 
     {0x304c ,0x00},  
     {0x304d ,0x02},  
     {0x304f ,0x20},  
     {0x30a3 ,0x10},  
     {0x3013 ,0xf7}, 
     {0x3014 ,0x64},  
     {0x3071 ,0x00},
     {0x3070 ,0x5d},
     {0x3073 ,0x00},
     {0x3072 ,0x4d},
     {0x301c ,0x05},
     {0x301d ,0x06}, 
     {0x304d ,0x42},     
     {0x304a ,0x40},  
     {0x304f ,0x40},  
     {0x3095 ,0x07},  
     {0x3096 ,0x16}, 
     {0x3097 ,0x1d}, 
     
     //;Window Setup
     {0x3020 ,0x01},
     {0x3021 ,0x18},
     {0x3022 ,0x00},
     {0x3023 ,0x06},
     {0x3024 ,0x06},
     {0x3025 ,0x58},
     {0x3026 ,0x02},
     {0x3027 ,0x61},
     {0x3088 ,0x02},
     {0x3089 ,0x80},
     {0x308a ,0x01},
     {0x308b ,0xe0},
     {0x3316 ,0x64},
     {0x3317 ,0x25},
     {0x3318 ,0x80},
     {0x3319 ,0x08},
     {0x331a ,0x28},
     {0x331b ,0x1e},
     {0x331c ,0x00},
     {0x331d ,0x38},
     {0x3100 ,0x00},
     
     //;UVadjust
     {0x3301 ,0xff},//
     {0x338B ,0x0d},//10
     {0x338c ,0x10},//1f
     {0x338d ,0x50},//40
     
     //;Sharpness/De-noise
     {0x3370 ,0xd0},
     {0x3371 ,0x00},
     {0x3372 ,0x00},
     {0x3373 ,0x40},
     {0x3374 ,0x10},
     {0x3375 ,0x10},
     {0x3376 ,0x04},
     {0x3377 ,0x00},
     {0x3378 ,0x04},
     {0x3379 ,0x80},
     
     //;BLC
     {0x3069 ,0x86},
     {0x307c ,0x10},
     {0x3087 ,0x02},
     
     //;black sun 
     //;Avdd 2.55~3.0V
     {0x3090 ,0x03},
     {0x30a8 ,0x54},
     {0x30aa ,0x82},
     {0x30a3 ,0x91},
     {0x30a1 ,0x41},      
     
     //;Other functions
     {0x3300 ,0xfc},//
     {0x3302 ,0x11},//
     {0x3400 ,0x00},//02
     {0x3606 ,0x20},//
     {0x3601 ,0x30},//
     //{0x300e ,0x34},//
     {0x30f3 ,0x83},//
     {0x304e ,0x88},//
     
     //;ov2675 image quality
     //;cmx
     {0x3380 ,0x39}, 
     {0x3381 ,0x43}, 
     {0x3382 ,0x0d}, 
     {0x3383 ,0x32}, 
     {0x3384 ,0xab}, 
     {0x3385 ,0xdd}, 
     {0x3386 ,0xd7}, 
     {0x3387 ,0xd1}, 
     {0x3388 ,0x05}, 
     {0x3389 ,0x98}, 
     {0x338a ,0x01},
     //;awb
     {0x3320 ,0xfa},    
     {0x3321 ,0x11},    
     {0x3322 ,0x92},    
     {0x3323 ,0x01},   
     {0x3324 ,0x97},    
     {0x3325 ,0x02},   
     {0x3326 ,0xff},    
     {0x3327 ,0x0c},   
     {0x3328 ,0x13},    
     {0x3329 ,0x16},    
     {0x332a ,0x56},   
     {0x332b ,0x60},    
     {0x332c ,0xb4},    
     {0x332d ,0x9c},    
     {0x332e ,0x36},    
     {0x332f ,0x31},    
     {0x3330 ,0x4c},    
     {0x3331 ,0x48},    
     {0x3332 ,0xf0},    
     {0x3333 ,0x00},   
     {0x3334 ,0xf0},    
     {0x3335 ,0xf0},    
     {0x3336 ,0xf0},    
     {0x3337 ,0x40},    
     {0x3338 ,0x40},    
     {0x3339 ,0x40},    
     {0x333a ,0x00},   
     {0x333b ,0x00},   
     //;lens correction
     {0x3300 ,0xFC},
     //;R
     {0x3350 ,0x2c},
     {0x3351 ,0x26},
     {0x3352 ,0x83}, 
     {0x3353 ,0x27},
     {0x3354 ,0x00},
     {0x3355 ,0x85},
     //;G
     {0x3356 ,0x2d},
     {0x3357 ,0x26},
     {0x3358 ,0xf0},
     {0x3359 ,0x23},
     {0x335a ,0x00},
     {0x335b ,0x85},
     //;B
     {0x335c ,0x2b},
     {0x335d ,0x26},
     {0x335e ,0x08}, 
     {0x335f ,0x20},
     {0x3360 ,0x00},
     {0x3361 ,0x85},
      
     {0x307c ,0x10},//  ; mirror&flip --> 13
     {0x3090 ,0x33},//  ; mirror&flip --> 3b   
      
     {0x3363 ,0xff},
     {0x3364 ,0xff},
     {0x3365 ,0x00},
     {0x3366 ,0x00},
                   
     //;gamma
     {0x334f ,0x20}, 
     {0x3340 ,0x06},
     {0x3341 ,0x14}, 
     {0x3342 ,0x2b}, 
     {0x3343 ,0x42}, 
     {0x3344 ,0x55}, 
     {0x3345 ,0x65},
     {0x3346 ,0x70}, 
     {0x3347 ,0x7c}, 
     {0x3348 ,0x86}, 
     {0x3349 ,0x96},  
     {0x334a ,0xa3},   
     {0x334b ,0xaf},   
     {0x334c ,0xc4},   
     {0x334d ,0xd7},  
     {0x334e ,0xe8}, 
     //;aec
     {0x3018 ,0x70},
     {0x3019 ,0x60},
     {0x301a ,0x95},
      
     {0x3306 ,0x00},  
     {0x3376 ,0x05}, 
     {0x3377 ,0x00}, 
     {0x3378 ,0x04},
     {0x3379 ,0x40},
     {0x3373 ,0x50},
     {0x3303 ,0x00},
     {0x3308 ,0x00},
     {0x3069 ,0x80},
     {0x338b ,0x10},
     {0x3391 ,0x06},
};


static struct msm_camera_i2c_reg_conf ov2675_preview_settings[] = 
{
      //@@vga preview 20fps
    //99 640 480
    //98 0 0
    //{0x3013 ,0x05},//yuxin add for prev and snap brightness not th same 2012.11.02-3
    {0x300e ,0x38},
    {0x3010 ,0x81},
    {0x3012 ,0x10},
    {0x3015 ,0x01},
    {0x3016 ,0x82},
    {0x3023 ,0x06},
    {0x3026 ,0x02},
    {0x3027 ,0x5e},
    {0x302a ,0x02},
    {0x302b ,0x6a},
    {0x330c ,0x00},
    {0x3301 ,0xff},
    {0x3069 ,0x80},
    {0x306f ,0x14},
    {0x3088 ,0x03},
    {0x3089 ,0x20},
    {0x308a ,0x02},
    {0x308b ,0x58},
    {0x308e ,0x00},
    {0x30a1 ,0x41},
    {0x30a3 ,0x80},
    {0x30d9 ,0x95},
    {0x3302 ,0x11},
    {0x3317 ,0x25},
    {0x3318 ,0x80},
    {0x3319 ,0x08},
    {0x331d ,0x38},
    {0x3373 ,0x30},
    {0x3376 ,0x05},
    {0x3362 ,0x90},
    //;svga->vga
    {0x3302 ,0x11},
    {0x3088 ,0x02},
    {0x3089 ,0x80},
    {0x308a ,0x01},
    {0x308b ,0xe0},
    {0x331a ,0x28},
    {0x331b ,0x1e},
    {0x331c ,0x00},
    {0x3302 ,0x11},
    //;mipi
    {0x363b ,0x01},
    {0x309e ,0x08},
    {0x3606 ,0x00},
    {0x3630 ,0x35},
    {0x3086 ,0x0f},
    {0x3086 ,0x00},
    {0x304e ,0x04},
    {0x363b ,0x01},
    {0x309e ,0x08},
    {0x3606 ,0x00},
    {0x3084 ,0x01},
    {0x3010 ,0x81},
    {0x3011 ,0x00},
    {0x300e ,0x38},
    {0x3634 ,0x26},
    {0x3086 ,0x0f},
    {0x3086 ,0x00},
    //;avoid black screen slash
    {0x3000 ,0x15},
    {0x3002 ,0x02},
    {0x3003 ,0x28},
};

static struct msm_camera_i2c_reg_conf ov2675_snapshot_settings[] = 
{
    //@@uxga capture 7.5fps 
    //99 1600 1200
    //98 0 0
    //{0x3013 ,0x00},//yuxin add for prev and snap brightness not the same 2012.11.02-3
    {0x300e ,0x34},
    {0x3011 ,0x01},
    {0x3010 ,0x81},
    {0x3012 ,0x00},
    
    {0x3015 ,0x02},
    {0x3016 ,0xc2},
    {0x3023 ,0x0c},
    {0x3026 ,0x04},
    {0x3027 ,0xbc},
    {0x302a ,0x04},
    {0x302b ,0xd4},
    {0x3069 ,0x80},
    {0x306f ,0x54},
    {0x3088 ,0x06},
    {0x3089 ,0x40},
    {0x308a ,0x04},
    {0x308b ,0xb0},
    {0x308e ,0x64},
    {0x30a1 ,0x41},
    {0x30a3 ,0x80},
    {0x30d9 ,0x95},
    {0x3302 ,0x01},
    {0x3317 ,0x4b},
    {0x3318 ,0x00},
    {0x3319 ,0x4c},
    {0x331d ,0x6c},
    {0x3362 ,0x80},
    {0x3373 ,0x40},
    {0x3376 ,0x03},
};

static struct v4l2_subdev_info ov2675_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
	/* more can be supported, to be added later */
};


static struct msm_camera_i2c_conf_array ov2675_init_conf[] = {
	{&ov2675_recommend_settings[0],
	ARRAY_SIZE(ov2675_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array ov2675_confs[] = {
	{&ov2675_snapshot_settings[0],
	ARRAY_SIZE(ov2675_snapshot_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov2675_preview_settings[0],
	ARRAY_SIZE(ov2675_preview_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	
};


static struct msm_sensor_output_info_t ov2675_dimensions[] = {
      //snapshot
	{
		.x_output = 1600,
		.y_output = 1200,
		.line_length_pclk = 1939,//1600
		.frame_length_lines = 1236,//1200
		.vt_pixel_clk = 36000000,//14400000,  //line_length_pclk*frame_length_lines*fps
		.op_pixel_clk = 36000000,//14400000,
		.binning_factor = 0,
	},
	//preview
	{
		.x_output = 640,//800,
		.y_output = 480,//600,
		.line_length_pclk = 1939,
		.frame_length_lines = 618,
		.vt_pixel_clk =18000000,  //line_length_pclk*frame_length_lines*fps
		.op_pixel_clk = 18000000,
		.binning_factor = 0,
	},
};





static int ov2675_get_preview_exposure_gain(struct msm_sensor_ctrl_t *s_ctrl)
{
     int rc = 0;
     uint16_t ret_l,ret_m,ret_h, temp_reg;
     uint16_t temp_AE_reg = 0;

    
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3400, 
		                                    &temp_AE_reg,MSM_CAMERA_I2C_BYTE_DATA);
    pr_err("--0x3400 = 0x%x\n", temp_AE_reg);
    //turn off AEC/AGC     
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3013, 
                                                  &temp_AE_reg,MSM_CAMERA_I2C_BYTE_DATA);
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3013, 
		                                    (temp_AE_reg&(~0x05)), MSM_CAMERA_I2C_BYTE_DATA);  
            
    //get preview exp & gain
    ret_h = ret_m = ret_l = 0;
    ov2675_preview_exposure = 0;
    //shutter
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3002, 
                                                 &ret_h,MSM_CAMERA_I2C_BYTE_DATA);
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3003, 
		                                    &ret_l,MSM_CAMERA_I2C_BYTE_DATA);
    ov2675_preview_exposure = (ret_h << 8) | (ret_l & 0xff) ;
//    printk("preview_exposure=%d\n", ov2675_preview_exposure);
#if 0
    //dummy line
    ov2675_i2c_read_byte(ov2675_client->addr,0x302d, &ret_h);
    ov2675_i2c_read_byte(ov2675_client->addr,0x302e, &ret_l);
    ov2675_preview_exposure = ov2675_preview_exposure + (ret_h << 8) | (ret_l & 0xff) ;
//    printk("preview_exposure=%d\n", ov2675_preview_exposure);
#endif

    //vts
    ret_h = ret_m = ret_l = 0;
    ov2675_preview_maxlines = 0;
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x302a, 
                                                &ret_h,MSM_CAMERA_I2C_BYTE_DATA);
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x302b, 
		                                   &ret_l,MSM_CAMERA_I2C_BYTE_DATA);
    ov2675_preview_maxlines = (ret_h << 8) | (ret_l & 0xff);
//    printk("Preview_Maxlines=%d\n", ov2675_preview_maxlines);

    //Read back AGC Gain for preview
    ov2675_gain = 0;
    temp_reg = 0;
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3000, 
		                              &temp_reg,MSM_CAMERA_I2C_BYTE_DATA);
 
    ov2675_gain=(16+(temp_reg&0x0F));
    
    if(temp_reg&0x10)
        ov2675_gain<<=1;
    if(temp_reg&0x20)
        ov2675_gain<<=1;
      
    if(temp_reg&0x40)
        ov2675_gain<<=1;
      
    if(temp_reg&0x80)
        ov2675_gain<<=1;
            
//    printk("Gain,0x350b=0x%x\n", ov2675_gain);

    return rc;
}


static int ov2675_set_capture_exposure_gain(struct msm_sensor_ctrl_t *s_ctrl)
{
    int rc = 0;
   //calculate capture exp & gain
   
    unsigned char ExposureLow,ExposureHigh;
    uint16_t ret_l,ret_m,ret_h,Lines_10ms, temp_reg;
    unsigned short ulCapture_Exposure,iCapture_Gain;
    unsigned int ulCapture_Exposure_Gain,Capture_MaxLines;
    //unsigned int ulCapture_Exposure_Gain;

    ret_h = ret_m = ret_l = 0;
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x302a, 
		                            &ret_h,MSM_CAMERA_I2C_BYTE_DATA);
    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x302b, 
		                            &ret_l,MSM_CAMERA_I2C_BYTE_DATA);
    Capture_MaxLines = (ret_h << 8) | (ret_l & 0xff);
    printk("Capture_MaxLines=%d\n", Capture_MaxLines);

    if(m_60Hz == TRUE)
    {
       Lines_10ms = Capture_Framerate * Capture_MaxLines/12000;
    }
    else
    {
       Lines_10ms = Capture_Framerate * Capture_MaxLines/10000;
    }
    pr_err("Lines_10ms=%d\n", Lines_10ms);
    if(ov2675_preview_maxlines == 0)
    {
         ov2675_preview_maxlines = 1;
    }

#if 0
    ulCapture_Exposure = (ov2675_preview_exposure*(Capture_Framerate)*(Capture_MaxLines))/(((ov2675_preview_maxlines)*(g_Preview_FrameRate)));
	pr_err("ulCapture_Exposure=%d 1=%d,2=%d,3=%d,4=%d,5=%d,\n", ulCapture_Exposure,ov2675_preview_exposure,Capture_Framerate,ov2675_preview_maxlines,g_Preview_FrameRate,ov2675_gain);
    iCapture_Gain = ov2675_gain;
#endif
   // ulCapture_Exposure = (ov2675_preview_exposure*(Capture_Framerate)*(Capture_MaxLines))/(((ov2675_preview_maxlines)*(g_Preview_FrameRate)));
   // iCapture_Gain = (ov2675_gain*(Capture_Framerate)*(Capture_MaxLines))/(((ov2675_preview_maxlines)*(g_Preview_FrameRate)));
   // iCapture_Gain = ov2675_gain*(2400)/(3600);
        iCapture_Gain = ov2675_gain*(750)/(2000);  //yuxin modify for prev and snap brightness ot the same 2012.11.02-2
	pr_err("ulCapture_Exposure=%d 1=%d,2=%d,3=%d,4=%d,5=%d,\n", iCapture_Gain,ov2675_gain,Capture_Framerate,ov2675_preview_maxlines,g_Preview_FrameRate,ov2675_gain);
   // iCapture_Gain = ov2675_gain;
	//pr_err("ulCapture_Exposure=%d 1=%d,2=%d,3=%d,4=%d,5=%d,\n", ulCapture_Exposure,ov2675_preview_exposure,Capture_Framerate,ov2675_preview_maxlines,g_Preview_FrameRate,ov2675_gain);
    ulCapture_Exposure = ov2675_preview_exposure;

    ulCapture_Exposure_Gain = ulCapture_Exposure * iCapture_Gain; 

    if(ulCapture_Exposure_Gain < Capture_MaxLines*16)
    {
    ulCapture_Exposure = ulCapture_Exposure_Gain/16;

    if (ulCapture_Exposure > Lines_10ms)
    {
      ulCapture_Exposure /= Lines_10ms;
      ulCapture_Exposure *= Lines_10ms;
    }
    }
    else
    {
    ulCapture_Exposure = Capture_MaxLines;
    }

    if(ulCapture_Exposure == 0)
    {
    ulCapture_Exposure = 1;
    }

    iCapture_Gain = (ulCapture_Exposure_Gain/ulCapture_Exposure + 1)/2;

    ExposureLow = (unsigned char)(ulCapture_Exposure & 0xff);
    ExposureHigh = (unsigned char)((ulCapture_Exposure & 0xff00) >> 8);
  
    // set capture exp & gain
    temp_reg = 0;
    if (iCapture_Gain > 31)
    {
        temp_reg |= 0x10;
        iCapture_Gain = iCapture_Gain >> 1;
    }
    if (iCapture_Gain > 31)
    {
        temp_reg |= 0x20;
        iCapture_Gain = iCapture_Gain >> 1;
    }

    if (iCapture_Gain > 31)
    {
        temp_reg |= 0x40;
        iCapture_Gain = iCapture_Gain >> 1;
    }
    if (iCapture_Gain > 31)
    {
        temp_reg |= 0x80;
        iCapture_Gain = iCapture_Gain >> 1;
    }
    
    if (iCapture_Gain > 16)
    {
        temp_reg |= ((iCapture_Gain -16) & 0x0f);
    }   

	#if 1
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3000,
                                iCapture_Gain + 1, MSM_CAMERA_I2C_BYTE_DATA);
    msleep(50);//delay 1 frame    
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3000, 
		                    iCapture_Gain, MSM_CAMERA_I2C_BYTE_DATA);
	#endif

    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3003, 
                                 ExposureLow, MSM_CAMERA_I2C_BYTE_DATA);
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3002, 
		                    ExposureHigh, MSM_CAMERA_I2C_BYTE_DATA);
  
//    printk("iCapture_Gain=%d\n", iCapture_Gain);
//    printk("ExposureLow=%d\n", ExposureLow);
//    printk("ExposureHigh=%d\n", ExposureHigh);

    msleep(50);

    return rc;
}


int32_t ov2675_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;

	if (update_type == MSM_SENSOR_REG_INIT) {
		printk("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3012, 0x80, MSM_CAMERA_I2C_BYTE_DATA);
		msleep(5);
		msm_sensor_write_init_settings(s_ctrl);
		csi_config = 0;
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		printk("PERIODIC : %d\n", res);
		 printk("%s:sensor_name = %s, res = %d\n",__func__, s_ctrl->sensordata->sensor_name, res);

              switch(res)
	       {
	          case 0://snapshot
                 	{
				uint16_t temp_1, temp_2, temp_3;
	                    uint16_t avrg,bns_lv,bns;
				s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);

				ov2675_get_preview_exposure_gain(s_ctrl);//Step1: get preview exposure and gain
				
				rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x330c, 0x3e, 
					                                        MSM_CAMERA_I2C_BYTE_DATA);	
			 	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x330f, &temp_1,
					                                        MSM_CAMERA_I2C_BYTE_DATA);
			 	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3301, &temp_2,
					                                        MSM_CAMERA_I2C_BYTE_DATA);
				rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3301, ((temp_2)&0xbf), 
					                                        MSM_CAMERA_I2C_BYTE_DATA);
				temp_1 = temp_1 & 0x1f;
				temp_1 = temp_1 * 2;
				rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3391, &temp_3,
					                                        MSM_CAMERA_I2C_BYTE_DATA);
				rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3391, ((temp_3)|0x02),
					                                        MSM_CAMERA_I2C_BYTE_DATA);	
				rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3394, temp_1,
					                                        MSM_CAMERA_I2C_BYTE_DATA);	
				rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3395, temp_1, 
					                                        MSM_CAMERA_I2C_BYTE_DATA);	

  			      rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3390, &bns_lv,
				  	                                        MSM_CAMERA_I2C_BYTE_DATA);
  			      rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x339a, &bns,    
				  	                                        MSM_CAMERA_I2C_BYTE_DATA);
  			      rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x301b, &avrg,   
				  	                                        MSM_CAMERA_I2C_BYTE_DATA);
                            #if 0 //yuxin del for preview and snapshot brightness not the same,2012.11.02-1
				if((avrg <= 0x11)&&(ov2675_gain >= 0x1e)) //low light environment(<30lux)
				{
	   				if(bns_lv == 0x41) //brightness default & positive
	   			      {
	   			   	   bns+= 0x10; //enhance brightness level
	   				   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x339a,  bns,
					   	                                  MSM_CAMERA_I2C_BYTE_DATA);
	   				}
	   				else //brightness negtive
	   				{
	   				   bns-= 0x10; //enhance brightness level
	   				   rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x339a, bns,
					   	                                   MSM_CAMERA_I2C_BYTE_DATA);
	   			       }
				}
				else //normal environment
				{
				        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,  0x339a, bns, 
							                             MSM_CAMERA_I2C_BYTE_DATA);
				}
                          #endif
						  
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

				ov2675_set_capture_exposure_gain(s_ctrl);
				s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
				                          msleep(20);				

				break;		
			}


			
		   case 1://preview
		   default :
		   	{
				
				s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
				msm_sensor_write_conf_array(
			                                    s_ctrl->sensor_i2c_client,
			                                    s_ctrl->msm_sensor_reg->mode_settings, res);
				
				//yuxin add for prev and snap brightness not the same 2012.11.02-4
		            //  rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3013, 0xf7, 
					  	//                   MSM_CAMERA_I2C_BYTE_DATA);
				
                          if (!csi_config) {
		      			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
		      			printk("CSI config in progress\n");
		      			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
		      				NOTIFY_CSIC_CFG,
		      				s_ctrl->curr_csic_params);
		      			printk("CSI config is done\n");
		      			msleep(30);
		      			csi_config = 1;
		               }

				 v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
		    			NOTIFY_PCLK_CHANGE,
		    			&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);
		            	s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		    		msleep(100);
		    		break;
			}

	       }
		
		
	}
	return rc;
}


// saturation
static struct msm_camera_i2c_reg_conf ov2675_saturation[][3] = 
{	
   	{//level 0
		{0x3301, 0x80}, 
		//{0x3391, 0x02},
		{0x3394, 0x20},
		{0x3395, 0x20},
	},
	
	{//level 1
		{0x3301, 0x80}, 
		//{0x3391, 0x02},
		{0x3394, 0x30},
		{0x3395, 0x30},

	},
	
	{//level 2 -default level
		//Saturation x1 (Default)              
		{0x3301, 0x80}, 
		//{0x3391, 0x02},
		{0x3394, 0x40},
		{0x3395, 0x40},
	},
	
	{//level 3
		{0x3301, 0x80}, 
		//{0x3391, 0x02},
		{0x3394, 0x50},
		{0x3395, 0x50},
	},
	
	{//level 4
       	{0x3301, 0x80}, 
		//{0x3391, 0x02},
       	{0x3394, 0x60},
       	{0x3395, 0x60},
	},
};

static struct msm_camera_i2c_conf_array ov2675_saturation_confs[][1] =
{	
   {{ov2675_saturation[0],		ARRAY_SIZE(ov2675_saturation[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_saturation[1],		ARRAY_SIZE(ov2675_saturation[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_saturation[2],		ARRAY_SIZE(ov2675_saturation[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_saturation[3],		ARRAY_SIZE(ov2675_saturation[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_saturation[4],		ARRAY_SIZE(ov2675_saturation[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
 
};
	
static int ov2675_saturation_enum_map[] = 
{	
   MSM_V4L2_SATURATION_L0,	
   MSM_V4L2_SATURATION_L1,	
   MSM_V4L2_SATURATION_L2,
   MSM_V4L2_SATURATION_L3,
   MSM_V4L2_SATURATION_L4,
  
};
		
static struct msm_camera_i2c_enum_conf_array ov2675_saturation_enum_confs = {	
	.conf = &ov2675_saturation_confs[0][0],	
       .conf_enum = ov2675_saturation_enum_map,
       .num_enum = ARRAY_SIZE(ov2675_saturation_enum_map),	
       .num_index = ARRAY_SIZE(ov2675_saturation_confs),	
       .num_conf = ARRAY_SIZE(ov2675_saturation_confs[0]),	
       .data_type = MSM_CAMERA_I2C_BYTE_DATA,
};


// contrast
static struct msm_camera_i2c_reg_conf ov2675_contrast[][3] = 
{	
   	{
		//Contrast -2                           
		//{0x3391, 0x04},
		{0x3390, 0x45},
		{0x3398, 0x14},
		{0x3399, 0x14}, 
	},
	
	{
		//Contrast -1                             
		//{0x3391, 0x04},
		{0x3390, 0x45},
		{0x3398, 0x1c},
		{0x3399, 0x1c}, 

	},
	
	{
		//Contrast (Default)                             
		//{0x3391, 0x04},
		{0x3390, 0x41},
		{0x3398, 0x20},
		{0x3399, 0x20},
	},
	
	{
		//Contrast +1                            
		//{0x3391, 0x04},
		{0x3390, 0x45},
		{0x3398, 0x28},
		{0x3399, 0x28},
	},
	
	{
		//Contrast +2                             
		//{0x3391, 0x04},
		{0x3390, 0x45},
		{0x3398, 0x30},
		{0x3399, 0x30},
	},
};

static struct msm_camera_i2c_conf_array ov2675_contrast_confs[][1] =
{	
   {{ov2675_contrast[0],		ARRAY_SIZE(ov2675_contrast[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_contrast[1],		ARRAY_SIZE(ov2675_contrast[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_contrast[2],		ARRAY_SIZE(ov2675_contrast[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_contrast[3],		ARRAY_SIZE(ov2675_contrast[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_contrast[4],		ARRAY_SIZE(ov2675_contrast[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
 
};
	
static int ov2675_contrast_enum_map[] = 
{	
   MSM_V4L2_CONTRAST_L0,	
   MSM_V4L2_CONTRAST_L1,	
   MSM_V4L2_CONTRAST_L2,
   MSM_V4L2_CONTRAST_L3,
   MSM_V4L2_CONTRAST_L4,
  
};
		
static struct msm_camera_i2c_enum_conf_array ov2675_contrast_enum_confs = {	
	.conf = &ov2675_contrast_confs[0][0],	
       .conf_enum = ov2675_contrast_enum_map,
       .num_enum = ARRAY_SIZE(ov2675_contrast_enum_map),	
       .num_index = ARRAY_SIZE(ov2675_contrast_confs),	
       .num_conf = ARRAY_SIZE(ov2675_contrast_confs[0]),	
       .data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

// sharpness
static struct msm_camera_i2c_reg_conf ov2675_sharpness[][5] = 
{	
   	{
		//Sharpness 0                             
		{0x3306, 0x00},
		{0x3376, 0x01}, //8x
		{0x3377, 0x00},
		{0x3378, 0x10},
		{0x3379, 0x80},
	},
	
	{
		//Sharpness 1                             
		{0x3306, 0x00},
		{0x3376, 0x02}, 
		{0x3377, 0x00},
		{0x3378, 0x08},
		{0x3379, 0x80},  


	},
	
	{
		//Sharpness_Auto (Default)
		{0x3306, 0x00},
		{0x3376, 0x04}, //8x
		{0x3377, 0x00},
		{0x3378, 0x04},
		{0x3379, 0x80},
	},
	
	{
		//Sharpness 3                             
		{0x3306, 0x00},
		{0x3376, 0x06}, //8x
		{0x3377, 0x00},
		{0x3378, 0x04},
		{0x3379, 0x80}, 
	},
	
	{
		//Sharpness 4                            
		{0x3306, 0x00},
		{0x3376, 0x08}, //8x
		{0x3377, 0x00},
		{0x3378, 0x04},
		{0x3379, 0x80},
	},
};

static struct msm_camera_i2c_conf_array ov2675_sharpness_confs[][1] =
{	
   {{ov2675_sharpness[0],		ARRAY_SIZE(ov2675_sharpness[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_sharpness[1],		ARRAY_SIZE(ov2675_sharpness[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_sharpness[2],		ARRAY_SIZE(ov2675_sharpness[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_sharpness[3],		ARRAY_SIZE(ov2675_sharpness[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_sharpness[4],		ARRAY_SIZE(ov2675_sharpness[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
 
};
	
static int ov2675_sharpness_enum_map[] = 
{	
   MSM_V4L2_SHARPNESS_L0,	
   MSM_V4L2_SHARPNESS_L1,	
   MSM_V4L2_SHARPNESS_L2,
   MSM_V4L2_SHARPNESS_L3,
   MSM_V4L2_SHARPNESS_L4,
};
		
static struct msm_camera_i2c_enum_conf_array ov2675_sharpness_enum_confs = {	
	.conf = &ov2675_sharpness_confs[0][0],	
       .conf_enum = ov2675_sharpness_enum_map,
       .num_enum = ARRAY_SIZE(ov2675_sharpness_enum_map),	
       .num_index = ARRAY_SIZE(ov2675_sharpness_confs),	
       .num_conf = ARRAY_SIZE(ov2675_sharpness_confs[0]),	
       .data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

// brightness
static struct msm_camera_i2c_reg_conf ov2675_brightness[][2] = 
{	
   	{
		//Brightness -2
		//{0x3391, 0x04},
		{0x3390, 0x49},  
		{0x339a, 0x30},  
	},
	
	{
		//Brightness -1
		//{0x3391, 0x04},
		{0x3390, 0x49},  
		{0x339a, 0x18},    


	},
	
	{
		//Brightness 0 (Default)
		//{0x3391, 0x04},
		{0x3390, 0x41},  
		{0x339a, 0x00},
	},
	
	{
		//Brightness +1
		//{0x3391, 0x04},
		{0x3390, 0x41},  
		{0x339a, 0x18}, 
	},
	
	{
		//Brightness +2
		//{0x3391, 0x04},
		{0x3390, 0x41},  
		{0x339a, 0x30},
	},
};

static struct msm_camera_i2c_conf_array ov2675_brightness_confs[][1] =
{	
   {{ov2675_brightness[0],		ARRAY_SIZE(ov2675_brightness[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_brightness[1],		ARRAY_SIZE(ov2675_brightness[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_brightness[2],		ARRAY_SIZE(ov2675_brightness[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_brightness[3],		ARRAY_SIZE(ov2675_brightness[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_brightness[4],		ARRAY_SIZE(ov2675_brightness[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},	},
 
};
	
static int ov2675_brightness_enum_map[] = 
{	
   MSM_V4L2_BRIGHTNESS_L0,	
   MSM_V4L2_BRIGHTNESS_L1,	
   MSM_V4L2_BRIGHTNESS_L2,
   MSM_V4L2_BRIGHTNESS_L3,
   MSM_V4L2_BRIGHTNESS_L4,
};
		
static struct msm_camera_i2c_enum_conf_array ov2675_brightness_enum_confs = {	
	.conf = &ov2675_brightness_confs[0][0],	
       .conf_enum = ov2675_brightness_enum_map,
       .num_enum = ARRAY_SIZE(ov2675_brightness_enum_map),	
       .num_index = ARRAY_SIZE(ov2675_brightness_confs),	
       .num_conf = ARRAY_SIZE(ov2675_brightness_confs[0]),	
       .data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

// Exposure Compensation
static struct msm_camera_i2c_reg_conf ov2675_exposure_compensation[][3] = 
{	
   	{
		//@@ -1.7EV 
		{0x3018,0x5a},
		{0x3019,0x4a},
		{0x301a,0xc2}, 
	},
	
	{
		//@@ -1.0EV
		{0x3018,0x6a},
		{0x3019,0x5a},
		{0x301a,0xd4}, 
	},
	
	{
		//@@ default 
		{0x3018,0x70},//0x78
		{0x3019,0x60},//0x68
		{0x301a,0xd4},
	},
	
	{
		//@@ +1.0EV
		{0x3018,0x88},
		{0x3019,0x78},
		{0x301a,0xd4}, 
	},
	
	{
		//@@ +1.7EV
		{0x3018,0x98},
		{0x3019,0x88},
		{0x301a,0xd4},  
	},
};

static struct msm_camera_i2c_conf_array ov2675_exposure_compensation_confs[][1] =
{	
   {{ov2675_exposure_compensation[0],ARRAY_SIZE(ov2675_exposure_compensation[0]), 
								0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_exposure_compensation[1],ARRAY_SIZE(ov2675_exposure_compensation[1]), 
   								0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_exposure_compensation[2],ARRAY_SIZE(ov2675_exposure_compensation[2]),
   								0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_exposure_compensation[3],ARRAY_SIZE(ov2675_exposure_compensation[3]), 
   								0, MSM_CAMERA_I2C_BYTE_DATA},	},
   {{ov2675_exposure_compensation[4],ARRAY_SIZE(ov2675_exposure_compensation[4]), 
   								0, MSM_CAMERA_I2C_BYTE_DATA},	},
 
};
	
static int ov2675_exposure_compensation_enum_map[] = 
{	
	MSM_V4L2_EXPOSURE_N2,
	MSM_V4L2_EXPOSURE_N1,
	MSM_V4L2_EXPOSURE_D,
	MSM_V4L2_EXPOSURE_P1,
	MSM_V4L2_EXPOSURE_P2,
};
		
static struct msm_camera_i2c_enum_conf_array ov2675_exposure_compensation_enum_confs = {	
	.conf = &ov2675_exposure_compensation_confs[0][0],	
       .conf_enum = ov2675_exposure_compensation_enum_map,
       .num_enum = ARRAY_SIZE(ov2675_exposure_compensation_enum_map),	
       .num_index = ARRAY_SIZE(ov2675_exposure_compensation_confs),	
       .num_conf = ARRAY_SIZE(ov2675_exposure_compensation_confs[0]),	
       .data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

// special  effect
static struct msm_camera_i2c_reg_conf ov2675_effect[][4] = {
    { 
		//effect_off
		{0x3391, 0x00},
		{0x3391, 0x00},
		{0x3391, 0x00},
		{0x3391, 0x00},

    },

    { 
		//effect_mono
		{0x3391, 0x24},
		{0x3391, 0x24},
		{0x3391, 0x24},
		{0x3391, 0x24},

    },

    { 
		//effect_negative
		{0x3391, 0x40},
		{0x3391, 0x40},
		{0x3391, 0x40},
		{0x3391, 0x40},

    },

    { 
		//effect_solative,has no params,can't use
		{0x3391, 0x40},
		{0x3391, 0x40},
		{0x3391, 0x40},
		{0x3391, 0x40},

    },
  

    {
		//sepia
		{0x3391, 0x00},
		{0x3391, 0x18},
		{0x3396, 0x40},
		{0x3397, 0xa6},
    },

    { 
		//redish
		{0x3391, 0x00},
		{0x3391, 0x18},
		{0x3396, 0x80},
		{0x3397, 0xc0},
    },

   { 
		//blueish
		{0x3391, 0x00},
		{0x3391, 0x18},
		{0x3396, 0xa0},
		{0x3397, 0x40},
    },
    
    { 
		//greenish
		{0x3391, 0x00},
		{0x3391, 0x18},
		{0x3396, 0x60},
		{0x3397, 0x60},
    },
    

  
};

static struct msm_camera_i2c_conf_array ov2675_effect_confs[][1] = {
	{
		{ov2675_effect[0],
		ARRAY_SIZE(ov2675_effect[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
		
	{
		{ov2675_effect[1],
		ARRAY_SIZE(ov2675_effect[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_effect[2],
		ARRAY_SIZE(ov2675_effect[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_effect[3],
		ARRAY_SIZE(ov2675_effect[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_effect[4],
		ARRAY_SIZE(ov2675_effect[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
		
	{
		{ov2675_effect[5],
		ARRAY_SIZE(ov2675_effect[5]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_effect[6],
		ARRAY_SIZE(ov2675_effect[6]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_effect[7],
		ARRAY_SIZE(ov2675_effect[7]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
		
};

static int ov2675_effect_enum_map[] = {
	MSM_V4L2_EFFECT_OFF,
	MSM_V4L2_EFFECT_MONO,
	MSM_V4L2_EFFECT_NEGATIVE,
	MSM_V4L2_EFFECT_SOLARIZE,
	MSM_V4L2_EFFECT_SEPIA,
	MSM_V4L2_EFFECT_REDISH,
	MSM_V4L2_EFFECT_BLUEISH,
	MSM_V4L2_EFFECT_GREENISH,
};

static struct msm_camera_i2c_enum_conf_array ov2675_effect_enum_confs = {
	.conf = &ov2675_effect_confs[0][0],
	.conf_enum = ov2675_effect_enum_map,
	.num_enum = ARRAY_SIZE(ov2675_effect_enum_map),
	.num_index = ARRAY_SIZE(ov2675_effect_confs),
	.num_conf = ARRAY_SIZE(ov2675_effect_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};
//

// awb
static struct msm_camera_i2c_reg_conf ov2675_awb[][4] = {	
	{ //MSM_V4L2_WB_MIN_MINUS_1,not used
		{0x3306,0x00},
		{0x3306,0x00},
		{0x3306,0x00},
		{0x3306,0x00},
	}, 
	
	{ // wb_auto
		{0x3306,0x00},
		{0x3306,0x00},
		{0x3306,0x00},
		{0x3306,0x00},

	}, 
       {//MSM_V4L2_WB_CUSTOM
		{0x3306, 0x02},
		{0x3337, 0x44},
		{0x3338, 0x40},
		{0x3339, 0x70},
	},

	{//INCANDESCENT,  //°×³ã
		{0x3306, 0x02},
		{0x3337, 0x52},
		{0x3338, 0x40},
		{0x3339, 0x58},  
	},

       {//FLUORESCENT,    //Ó«¹â
             {0x3306, 0x02},
		{0x3337, 0x54},
		{0x3338, 0x40},
		{0x3339, 0x70},  
       },

       { //daylight
		{0x3306, 0x02},
		{0x3337, 0x5e},
		{0x3338, 0x40},
		{0x3339, 0x46}, 
       },

       { //cloudy
		{0x3306, 0x02},
		{0x3337, 0x68},
		{0x3338, 0x40},
		{0x3339, 0x4e},  
       },
};

static struct msm_camera_i2c_conf_array ov2675_awb_confs[][1] = {
	{
		{ov2675_awb[0],
		ARRAY_SIZE(ov2675_awb[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_awb[1],
		ARRAY_SIZE(ov2675_awb[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_awb[2],
		ARRAY_SIZE(ov2675_awb[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_awb[3],
		ARRAY_SIZE(ov2675_awb[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_awb[4],
	       ARRAY_SIZE(ov2675_awb[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_awb[5],
		ARRAY_SIZE(ov2675_awb[5]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_awb[6],
	       ARRAY_SIZE(ov2675_awb[6]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},	
	
};

static int ov2675_awb_enum_map[] = {	
	MSM_V4L2_WB_OFF,//not used
	MSM_V4L2_WB_AUTO ,//= 1
	MSM_V4L2_WB_CUSTOM,  //not used
	MSM_V4L2_WB_INCANDESCENT, //°×³ã
	MSM_V4L2_WB_FLUORESCENT,   //Ó«¹â
	MSM_V4L2_WB_DAYLIGHT,
	MSM_V4L2_WB_CLOUDY_DAYLIGHT,
};

static struct msm_camera_i2c_enum_conf_array ov2675_awb_enum_confs = {
	.conf = &ov2675_awb_confs[0][0],
	.conf_enum = ov2675_awb_enum_map,
	.num_enum = ARRAY_SIZE(ov2675_awb_enum_map),
	.num_index = ARRAY_SIZE(ov2675_awb_confs),
	.num_conf = ARRAY_SIZE(ov2675_awb_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

//anti-banding
static struct msm_camera_i2c_reg_conf ov2675_antibanding[][7] = {		
       {//off.not used
		{0x3014, 0xc0},
		{0x3014, 0xc0},
		{0x3014, 0xc0},
		{0x3014, 0xc0},
		{0x3014, 0xc0},
		{0x3014, 0xc0},
		{0x3014, 0xc0},
       },

       {//60Hz
		//Band 60Hz   
		{0x3014, 0x00},
		{0x3070, 0x5c},
		{0x3071, 0x00},
		{0x3072, 0x4d},
		{0x3073, 0x00},
		{0x301c, 0x06},
		{0x301d, 0x07},
       },
       
	{//50Hz
		//Band 50Hz   
		{0x3014, 0x80},
		{0x3070, 0x5c},
		{0x3071, 0x00},
		{0x3072, 0x4d},
		{0x3073, 0x00},
		{0x301c, 0x06},
		{0x301d, 0x07},
	},

       { //auto
		//Auto-XCLK24MHz                  
		{0x3014, 0xc0},
		{0x3014, 0xc0},
		{0x3014, 0xc0},
		{0x3014, 0xc0},
		{0x3014, 0xc0},
		{0x3014, 0xc0},
		{0x3014, 0xc0},
	}, 
};

static struct msm_camera_i2c_conf_array ov2675_antibanding_confs[][1] = {
	{
		{ov2675_antibanding[0],
		ARRAY_SIZE(ov2675_antibanding[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_antibanding[1],
		ARRAY_SIZE(ov2675_antibanding[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_antibanding[2],
		ARRAY_SIZE(ov2675_antibanding[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_antibanding[3],
		ARRAY_SIZE(ov2675_antibanding[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
};

static int ov2675_antibanding_enum_map[] = {	
	MSM_V4L2_POWER_LINE_OFF,  //not used
	MSM_V4L2_POWER_LINE_60HZ,
	MSM_V4L2_POWER_LINE_50HZ,
	MSM_V4L2_POWER_LINE_AUTO,

};

static struct msm_camera_i2c_enum_conf_array ov2675_antibanding_enum_confs = {
	.conf = &ov2675_antibanding_confs[0][0],
	.conf_enum = ov2675_antibanding_enum_map,
	.num_enum = ARRAY_SIZE(ov2675_antibanding_enum_map),
	.num_index = ARRAY_SIZE(ov2675_antibanding_confs),
	.num_conf = ARRAY_SIZE(ov2675_antibanding_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

//ISO
static struct msm_camera_i2c_reg_conf ov2675_iso[][1] = {	
	
	{ //auto
	   {0x3015,0x02},
	}, 
	
       { //MSM_V4L2_ISO_DEBLUR ,not used
	   {0x3015,0x02},
	}, 
	
	{//iso_100
	   {0x3015,0x00},
	},

       {//iso200
	   {0x3015,0x01},
       },

       { //iso400
	   {0x3015,0x02},
       },

       { //iso800
	   {0x3015,0x03},
       },
       
       { //iso1600
	   {0x3015,0x04},
       },
};

static struct msm_camera_i2c_conf_array ov2675_iso_confs[][1] = {
	{
		{ov2675_iso[0],
		ARRAY_SIZE(ov2675_iso[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_iso[1],
		ARRAY_SIZE(ov2675_iso[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_iso[2],
		ARRAY_SIZE(ov2675_iso[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_iso[3],
		ARRAY_SIZE(ov2675_iso[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_iso[4],
	  ARRAY_SIZE(ov2675_iso[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},
	
	{
		{ov2675_iso[5],
	  ARRAY_SIZE(ov2675_iso[5]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},	
	
	{
		{ov2675_iso[6],
	  ARRAY_SIZE(ov2675_iso[6]), 0, MSM_CAMERA_I2C_BYTE_DATA},
	},	
	
};

static int ov2675_iso_enum_map[] = {	
	MSM_V4L2_ISO_AUTO,
	MSM_V4L2_ISO_DEBLUR,//not used
	MSM_V4L2_ISO_100,
	MSM_V4L2_ISO_200,
	MSM_V4L2_ISO_400,
	MSM_V4L2_ISO_800,
	MSM_V4L2_ISO_1600,
};

static struct msm_camera_i2c_enum_conf_array ov2675_iso_enum_confs = {
	.conf = &ov2675_iso_confs[0][0],
	.conf_enum = ov2675_iso_enum_map,
	.num_enum = ARRAY_SIZE(ov2675_iso_enum_map),
	.num_index = ARRAY_SIZE(ov2675_iso_confs),
	.num_conf = ARRAY_SIZE(ov2675_iso_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};



struct msm_sensor_v4l2_ctrl_info_t ov2675_v4l2_ctrl_info[] = {
	{
		.ctrl_id = V4L2_CID_SATURATION,
		.min = MSM_V4L2_SATURATION_L0,
		.max = MSM_V4L2_SATURATION_L4,
		.step = 1,
		.enum_cfg_settings = &ov2675_saturation_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},
	
	{
		.ctrl_id = V4L2_CID_CONTRAST,
		.min = MSM_V4L2_CONTRAST_L0,
		.max = MSM_V4L2_CONTRAST_L4,
		.step = 1,
		.enum_cfg_settings = &ov2675_contrast_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},

	{
		.ctrl_id = V4L2_CID_SHARPNESS,
		.min = MSM_V4L2_SHARPNESS_L0,
		.max = MSM_V4L2_SHARPNESS_L4,
		.step = 1,
		.enum_cfg_settings = &ov2675_sharpness_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},

	{
		.ctrl_id = V4L2_CID_BRIGHTNESS,
		.min = MSM_V4L2_BRIGHTNESS_L0,
		.max = MSM_V4L2_BRIGHTNESS_L4,
		.step = 1,
		.enum_cfg_settings = &ov2675_brightness_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},
	
      { 
		.ctrl_id = V4L2_CID_EXPOSURE,
		.min = MSM_V4L2_EXPOSURE_N2,
		.max = MSM_V4L2_EXPOSURE_P2,
		.step = 1,
		.enum_cfg_settings = &ov2675_exposure_compensation_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},  
	
	{
		.ctrl_id = V4L2_CID_COLORFX,
		.min = MSM_V4L2_EFFECT_OFF,
		.max = MSM_V4L2_EFFECT_GREENISH,
		.step = 1,
		.enum_cfg_settings = &ov2675_effect_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,

	},

	{
		.ctrl_id = V4L2_CID_AUTO_WHITE_BALANCE,
		.min = MSM_V4L2_WB_AUTO,
		.max = MSM_V4L2_WB_CLOUDY_DAYLIGHT,
		.step = 1,
		.enum_cfg_settings = &ov2675_awb_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},	
	
	{
		.ctrl_id = V4L2_CID_POWER_LINE_FREQUENCY,//antibanding
		.min = MSM_V4L2_POWER_LINE_60HZ,
		.max = MSM_V4L2_POWER_LINE_AUTO,
		.step = 1,
		.enum_cfg_settings = &ov2675_antibanding_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},  

       {
		.ctrl_id = V4L2_CID_ISO,//antibanding
		.min = MSM_V4L2_ISO_AUTO,
		.max = MSM_V4L2_ISO_1600,
		.step = 1,
		.enum_cfg_settings = &ov2675_iso_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},  


    
};


static struct msm_camera_csi_params ov2675_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x6,
};

static struct msm_camera_csi_params *ov2675_csi_params_array[] = {
	&ov2675_csi_params,
	&ov2675_csi_params,
};

static struct msm_sensor_output_reg_addr_t ov2675_reg_addr = {
	.x_output = 0x3088,
	.y_output = 0x308a,
	.line_length_pclk = 0x3028, ///?????
	.frame_length_lines = 0x302a,//??????
	
};

static struct msm_sensor_id_info_t ov2675_id_info = {
	.sensor_id_reg_addr = 0x300A,
	.sensor_id = 0x26,
};

static const struct i2c_device_id ov2675_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&ov2675_s_ctrl},
	{ }
};

static struct i2c_driver ov2675_i2c_driver = {
	.id_table = ov2675_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov2675_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	int rc = 0;

	rc = i2c_add_driver(&ov2675_i2c_driver);

	return rc;
}

static struct v4l2_subdev_core_ops ov2675_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops ov2675_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops ov2675_subdev_ops = {
	.core = &ov2675_subdev_core_ops,
	.video  = &ov2675_subdev_video_ops,
};

static struct msm_sensor_fn_t ov2675_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_csi_setting = ov2675_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
};

static struct msm_sensor_reg_t ov2675_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = ov2675_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(ov2675_start_settings),
	.stop_stream_conf = ov2675_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(ov2675_stop_settings),
	.init_settings = &ov2675_init_conf[0],
	.init_size = ARRAY_SIZE(ov2675_init_conf),
	.mode_settings = &ov2675_confs[0],
	//.no_effect_settings = &ov7692_no_effect_confs[0],
	.output_settings = &ov2675_dimensions[0],
	.num_conf = ARRAY_SIZE(ov2675_confs),
};

static struct msm_sensor_ctrl_t ov2675_s_ctrl = {
	.msm_sensor_reg = &ov2675_regs,
	.msm_sensor_v4l2_ctrl_info = ov2675_v4l2_ctrl_info,
	.num_v4l2_ctrl = ARRAY_SIZE(ov2675_v4l2_ctrl_info), 
	.sensor_i2c_client = &ov2675_sensor_i2c_client,
	.sensor_i2c_addr = 0x60,
	.sensor_output_reg_addr = &ov2675_reg_addr,
	.sensor_id_info = &ov2675_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &ov2675_csi_params_array[0],
	.msm_sensor_mutex = &ov2675_mut,
	.sensor_i2c_driver = &ov2675_i2c_driver,
	.sensor_v4l2_subdev_info = ov2675_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov2675_subdev_info),
	.sensor_v4l2_subdev_ops = &ov2675_subdev_ops,
	.func_tbl = &ov2675_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Omnivision 2M YUV sensor driver");
MODULE_LICENSE("GPL v2");


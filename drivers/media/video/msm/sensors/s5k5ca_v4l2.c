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
#define SENSOR_NAME "s5k5ca"
#define PLATFORM_DRIVER_NAME "msm_camera_s5k5ca"
#define s5k5ca_obj s5k5ca_##obj
#define MSB                             1
#define LSB                             0

#include "s5k5ca_v4l2.h"


DEFINE_MUTEX(s5k5ca_mut);
static struct msm_sensor_ctrl_t s5k5ca_s_ctrl;

static struct msm_camera_i2c_reg_conf s5k5ca_start_settings[] = {
	//{0x007c, 0x0001},
};

static struct msm_camera_i2c_reg_conf s5k5ca_stop_settings[] = {
	//{0x007c, 0x0000},
};

static struct msm_camera_i2c_reg_conf s5k5ca_groupon_settings[] = {
	//{0x0104, 0x01},
};

static struct msm_camera_i2c_reg_conf s5k5ca_groupoff_settings[] = {
	//{0x0104, 0x00},
};

static struct msm_camera_i2c_reg_conf s5k5ca_prev_settings[] = {
{0x0028,0x7000},
{0x002A,0x0252},
{0x0F12,0x0003}, //af init 

{0x002A,0x023C},
{0x0F12,0x0000},//0000 //REG_TC_GP_ActivePrevConfig

{0x002A,0x0240},
{0x0F12,0x0001}, //REG_TC_GP_PrevOpenAfterChange
{0x002A,0x0230},
{0x0F12,0x0001}, //REG_TC_GP_NewConfigSync
{0x002A,0x023E},
{0x0F12,0x0001}, //REG_TC_GP_PrevConfigChanged
{0x002A,0x0220},
{0x0F12,0x0001}, //REG_TC_GP_EnablePreview
{0x0028,0xD000},
{0x002A,0xB0A0},
{0x0F12,0x0000}, // Clear cont. clock befor config change
{0x0028,0x7000},
{0x002A,0x0222},
{0x0F12,0x0001}, //REG_TC_GP_EnablePreviewChanged
};

static struct msm_camera_i2c_reg_conf s5k5ca_snap_settings[] = {
{0x0028,0x7000},
{0x002A,0x0244},
{0x0F12,0x0000}, //REG_TC_GP_ActiveCapConfig
{0x002A,0x0230},
{0x0F12,0x0001}, //REG_TC_GP_NewConfigSync
{0x002A,0x0246},
{0x0F12,0x0001}, //REG_TC_GP_CapConfigChanged
{0x002A,0x0224},
{0x0F12,0x0001}, //REG_TC_GP_EnableCapture
{0x0028,0xD000},
{0x002A,0xB0A0},
{0x0F12,0x0000}, // Clear cont. clock befor config change
{0x0028,0x7000},
{0x002A,0x0226},//0224
{0x0F12,0x0001}, //REG_TC_GP_EnableCaptureChanged
};


static struct v4l2_subdev_info s5k5ca_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array s5k5ca_init_conf[] = {
	{&s5k5ca_recommend_settings_0[0],
	ARRAY_SIZE(s5k5ca_recommend_settings_0), 100, MSM_CAMERA_I2C_WORD_DATA},
	{&s5k5ca_recommend_settings_1[0],
	ARRAY_SIZE(s5k5ca_recommend_settings_1), 120, MSM_CAMERA_I2C_WORD_DATA},
	{&s5k5ca_recommend_settings_2[0],
	ARRAY_SIZE(s5k5ca_recommend_settings_2), 120, MSM_CAMERA_I2C_WORD_DATA},
	{&s5k5ca_recommend_settings_3[0],
	ARRAY_SIZE(s5k5ca_recommend_settings_3), 1000, MSM_CAMERA_I2C_WORD_DATA}
};

static struct msm_camera_i2c_conf_array s5k5ca_confs[] = {
	{&s5k5ca_snap_settings[0],
	ARRAY_SIZE(s5k5ca_snap_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{&s5k5ca_prev_settings[0],
	ARRAY_SIZE(s5k5ca_prev_settings), 80, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_sensor_output_info_t s5k5ca_dimensions[] = {
	{
		.x_output = 2048,
		.y_output = 1536,
		.line_length_pclk = 2048,
		.frame_length_lines = 1536,
		.vt_pixel_clk = 36000000,
		.op_pixel_clk = 52000000,
		.binning_factor = 1,
	},
	{
		.x_output = 1024,
		.y_output = 768,
		.line_length_pclk = 1024,
		.frame_length_lines = 768,
		.vt_pixel_clk = 36000000,
		.op_pixel_clk =52000000 ,
		.binning_factor = 1,
	},
};

static struct msm_camera_csi_params s5k5ca_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 20,//0X5,0X24,
};

static struct msm_camera_csi_params *s5k5ca_csi_params_array[] = {
	&s5k5ca_csi_params,
	&s5k5ca_csi_params,
};

/*
static struct msm_camera_csid_vc_cfg s5k5ca_cid_cfg[] = {	
	{0, CSI_YUV422_8, CSI_DECODE_8BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params s5k5ca_csi_params = 
{
	.csid_params = {
		.lane_assign = 0xe4,
		.lane_cnt = 1,
		.lut_params = {
			.num_cid = 2,
			.vc_cfg = s5k5ca_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 1,
		.settle_cnt = 0x7,
		//.lane_mask = 0xf,
	},
};

static struct msm_camera_csi2_params *s5k5ca_csi_params_array[] = {
	&s5k5ca_csi_params,
	&s5k5ca_csi_params,	
};
*/

static struct msm_sensor_output_reg_addr_t s5k5ca_reg_addr = {
	.x_output = 0,//0x034C,
	.y_output = 0,//0x034E,
	.line_length_pclk = 0,//0x0342,
	.frame_length_lines = 0,//0x0340,
};

static struct msm_camera_i2c_reg_conf sensor_reg_addr_ext[]={
       {0x002c,0x0000},
       {0x002e,0x0040},
};

static struct msm_camera_i2c_conf_array sensor_reg_addr_ext_i2c[] = {
	{&sensor_reg_addr_ext[0],
	ARRAY_SIZE(sensor_reg_addr_ext), 0, MSM_CAMERA_I2C_WORD_DATA}
};

static struct msm_sensor_id_info_t s5k5ca_id_info = {
	.sensor_id_reg_addr_ext = &sensor_reg_addr_ext_i2c[0],
	.sensor_id_reg_addr = 0x0f12,
	.sensor_id = 0x05ca,
};

static struct msm_sensor_exp_gain_info_t s5k5ca_exp_gain_info = {
	.coarse_int_time_addr =0,// 0x0202,
	.global_gain_addr = 0,//0x0204,
	.vert_offset = 4,
};

#if 0
static inline uint8_t s5k5ca_byte(uint16_t word, uint8_t offset)
{
	return word >> (offset * BITS_PER_BYTE);
}
#endif
#if 0

static int32_t s5k5ca_write_prev_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
						uint16_t gain, uint32_t line)
{
	uint16_t max_legal_gain = 0x0200;
	int32_t rc = 0;
	static uint32_t fl_lines, offset;

	pr_info("s5k5ca_write_prev_exp_gain :%d %d\n", gain, line);
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;
	if (gain > max_legal_gain) {
		pr_err("Max legal gain Line:%d\n", __LINE__);
		gain = max_legal_gain;
	}

	/* Analogue Gain */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr,
		s5k5ca_byte(gain, MSB),
		MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr + 1,
		s5k5ca_byte(gain, LSB),
		MSM_CAMERA_I2C_WORD_DATA);

	if (line > (s_ctrl->curr_frame_length_lines - offset)) {
		fl_lines = line + offset;
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			s5k5ca_byte(fl_lines, MSB),
			MSM_CAMERA_I2C_WORD_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
			s5k5ca_byte(fl_lines, LSB),
			MSM_CAMERA_I2C_WORD_DATA);
		/* Coarse Integration Time */
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
			s5k5ca_byte(line, MSB),
			MSM_CAMERA_I2C_WORD_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
			s5k5ca_byte(line, LSB),
			MSM_CAMERA_I2C_WORD_DATA);
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	} else if (line < (fl_lines - offset)) {
		fl_lines = line + offset;
		if (fl_lines < s_ctrl->curr_frame_length_lines)
			fl_lines = s_ctrl->curr_frame_length_lines;

		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
		/* Coarse Integration Time */
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
			s5k5ca_byte(line, MSB),
			MSM_CAMERA_I2C_WORD_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
			s5k5ca_byte(line, LSB),
			MSM_CAMERA_I2C_WORD_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			s5k5ca_byte(fl_lines, MSB),
			MSM_CAMERA_I2C_WORD_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
			s5k5ca_byte(fl_lines, LSB),
			MSM_CAMERA_I2C_WORD_DATA);
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	} else {
		fl_lines = line+4;
		s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
		/* Coarse Integration Time */
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
			s5k5ca_byte(line, MSB),
			MSM_CAMERA_I2C_WORD_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
			s5k5ca_byte(line, LSB),
			MSM_CAMERA_I2C_WORD_DATA);
		s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	}
	return rc;
       return 0;
}

static int32_t s5k5ca_write_pict_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	uint16_t max_legal_gain = 0x0200;
	uint16_t min_ll_pck = 0x0AB2;
	uint32_t ll_pck, fl_lines;
	uint32_t ll_ratio;
	uint8_t gain_msb, gain_lsb;
	uint8_t intg_time_msb, intg_time_lsb;
	uint8_t ll_pck_msb, ll_pck_lsb;

	if (gain > max_legal_gain) {
		CDBG("Max legal gain Line:%d\n", __LINE__);
		gain = max_legal_gain;
	}

	pr_info("s5k5ca_write_exp_gain : gain = %d line = %d\n", gain, line);
	line = (uint32_t) (line * s_ctrl->fps_divider);
	fl_lines = s_ctrl->curr_frame_length_lines * s_ctrl->fps_divider / Q10;
	ll_pck = s_ctrl->curr_line_length_pclk;

	if (fl_lines < (line / Q10))
		ll_ratio = (line / (fl_lines - 4));
	else
		ll_ratio = Q10;

	ll_pck = ll_pck * ll_ratio / Q10;
	line = line / ll_ratio;
	if (ll_pck < min_ll_pck)
		ll_pck = min_ll_pck;

	gain_msb = (uint8_t) ((gain & 0xFF00) >> 8);
	gain_lsb = (uint8_t) (gain & 0x00FF);

	intg_time_msb = (uint8_t) ((line & 0xFF00) >> 8);
	intg_time_lsb = (uint8_t) (line & 0x00FF);

	ll_pck_msb = (uint8_t) ((ll_pck & 0xFF00) >> 8);
	ll_pck_lsb = (uint8_t) (ll_pck & 0x00FF);

	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr,
		gain_msb,
		MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr + 1,
		gain_lsb,
		MSM_CAMERA_I2C_WORD_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->line_length_pclk,
		ll_pck_msb,
		MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->line_length_pclk + 1,
		ll_pck_lsb,
		MSM_CAMERA_I2C_WORD_DATA);

	/* Coarse Integration Time */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		intg_time_msb,
		MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
		intg_time_lsb,
		MSM_CAMERA_I2C_WORD_DATA);
	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	return 0;
}
#endif


int32_t s5k5ca_sensor_set_fps(struct msm_sensor_ctrl_t *s_ctrl,
		struct fps_cfg *fps)
{
	CDBG("s5k5ca_sensor_set_fps: Not supported\n");
	return 0;
}


static const struct i2c_device_id s5k5ca_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&s5k5ca_s_ctrl},
	{ }
};

static struct i2c_driver s5k5ca_i2c_driver = {
	.id_table = s5k5ca_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client s5k5ca_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&s5k5ca_i2c_driver);
}

static struct v4l2_subdev_core_ops s5k5ca_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops s5k5ca_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops s5k5ca_subdev_ops = {
	.core = &s5k5ca_subdev_core_ops,
	.video  = &s5k5ca_subdev_video_ops,
};

static int s5k5ca_pwdn_gpio;
static int s5k5ca_reset_gpio;

static void s5k5ca_hw_reset(void)
{
	CDBG("--CAMERA-- %s ... (Start...)\n", __func__);
	gpio_set_value(s5k5ca_reset_gpio, 1);   /*reset camera reset pin*/
	//usleep_range(5000, 5100);
	msleep(5);
	gpio_set_value(s5k5ca_reset_gpio, 0);
	msleep(15);
	gpio_set_value(s5k5ca_reset_gpio, 1);
	msleep(10);
	CDBG("--CAMERA-- %s ... (End...)\n", __func__);
}

static int s5k5ca_probe_init_gpio(const struct msm_camera_sensor_info *data)
{
	int rc = 0;
	CDBG("%s: entered\n", __func__);

	s5k5ca_pwdn_gpio = data->sensor_pwd;
	s5k5ca_reset_gpio = data->sensor_reset ;

	CDBG("%s: pwdn_gpio:%d, reset_gpio:%d\n", __func__,
			s5k5ca_pwdn_gpio, s5k5ca_reset_gpio);


	gpio_direction_output(data->sensor_pwd, 1);

	usleep_range(1500, 1600);

	if (data->sensor_reset_enable)
		gpio_direction_output(data->sensor_reset, 1);

	return rc;

}
static void s5k5ca_power_on(void)
{
	pr_err("%s\n", __func__);
	gpio_set_value(39, 1);//zhangzhao
	//usleep_range(5000, 5100);
	msleep(20);
	gpio_set_value(49, 0);//zhangzhao
	msleep(100);
	gpio_set_value(s5k5ca_pwdn_gpio, 0);
	msleep(50);
	//gpio_set_value(s5k5ca_reset_gpio, 1);

	msleep(20);
}

static void s5k5ca_power_down(void)
{
	pr_err("%s\n", __func__);
	gpio_set_value(s5k5ca_pwdn_gpio, 1);
	msleep(300);
	//gpio_set_value(s5k5ca_reset_gpio, 0);
	msleep(20);
	//gpio_set_value(39, 1);//zhangzhao
	gpio_set_value(49, 1);//zhangzhao
	msleep(20);
}

int32_t s5k5ca_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	struct msm_camera_sensor_info *info = NULL;

	pr_err("%s: %d\n", __func__, __LINE__);

	rc = msm_sensor_power_up(s_ctrl);
	if (rc < 0) {
		pr_err("%s: msm_sensor_power_up failed\n", __func__);
		return rc;
	}
	info = s_ctrl->sensordata;

	rc = s5k5ca_probe_init_gpio(info);
	if (rc < 0) {
		pr_err("%s: gpio init failed\n", __func__);
		goto power_up_fail;
	}
	/* turn on LDO for PVT */
	if (info->pmic_gpio_enable)
		lcd_camera_power_onoff(1);

	s5k5ca_power_down();

	usleep_range(5000, 5100);

	s5k5ca_power_on();
	usleep_range(5000, 5100);

	if (info->sensor_reset_enable)
		s5k5ca_hw_reset();
	//else
	//	s5k5ca_sw_reset(s_ctrl);
	usleep_range(5000, 5100);	   
	return rc;

power_up_fail:
	pr_err("s5k5ca_sensor_power_up: s5k5ca SENSOR POWER UP FAILS!\n");
	if (info->pmic_gpio_enable)
		lcd_camera_power_onoff(0);
	return rc;
}


int32_t s5k5ca_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	//struct msm_camera_sensor_info *info = NULL;

	pr_err("%s: %d\n", __func__, __LINE__);

{
        /*
         * ZTE_CAM_LJ_20110602
         * fix bug of high current in sleep mode
         */
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0028, 0x7000, MSM_CAMERA_I2C_WORD_DATA);
        if(rc < 0)
        {
            return rc;
        }            
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x002A, 0x2824, MSM_CAMERA_I2C_WORD_DATA);
        if (rc < 0)
        {
            return rc;
        }
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0F12, 0x0000, MSM_CAMERA_I2C_WORD_DATA);
        if (rc < 0)
        {
            return rc;
        }
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0028, 0x7000, MSM_CAMERA_I2C_WORD_DATA);
        if(rc < 0)
        {
            return rc;
        }            
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x002A, 0x0254, MSM_CAMERA_I2C_WORD_DATA);
        if (rc < 0)
        {
            return rc;
        }
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0F12, 0x0001, MSM_CAMERA_I2C_WORD_DATA);
        if (rc < 0)
        {
            return rc;
        }
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x002A, 0x0252, MSM_CAMERA_I2C_WORD_DATA);
        if(rc < 0)
        {
            return rc;
        }            
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0F12, 0x0004, MSM_CAMERA_I2C_WORD_DATA);
        if (rc < 0)
        {
            return rc;
        }
        msleep(133);
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x002A, 0x0252, MSM_CAMERA_I2C_WORD_DATA);
        if (rc < 0)
        {
            return rc;
        }
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0F12, 0x0002, MSM_CAMERA_I2C_WORD_DATA);
        if (rc < 0)
        {
            return rc;
        }
        msleep(200);
            
    }
	s5k5ca_power_down();

	rc = msm_sensor_power_down(s_ctrl);
	if (rc < 0) {
		pr_err("%s: msm_sensor_power_down failed\n", __func__);
		return rc;
	}
	//info = s_ctrl->sensordata;

		return rc;


}

static struct msm_sensor_fn_t s5k5ca_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	//.sensor_group_hold_on = msm_sensor_group_hold_on,
	//.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = s5k5ca_sensor_set_fps,
	//.sensor_write_exp_gain = s5k5ca_write_prev_exp_gain,
	//.sensor_write_snapshot_exp_gain = s5k5ca_write_pict_exp_gain,
	.sensor_csi_setting = msm_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = s5k5ca_sensor_power_up, //msm_sensor_power_up,
	.sensor_power_down = s5k5ca_sensor_power_down,//msm_sensor_power_down,
};

static struct msm_sensor_reg_t s5k5ca_regs = {
	.default_data_type = MSM_CAMERA_I2C_WORD_DATA,
	.start_stream_conf = s5k5ca_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(s5k5ca_start_settings),
	.stop_stream_conf = s5k5ca_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(s5k5ca_stop_settings),
	.group_hold_on_conf = s5k5ca_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(s5k5ca_groupon_settings),
	.group_hold_off_conf = s5k5ca_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(s5k5ca_groupoff_settings),
	.init_settings = &s5k5ca_init_conf[0],
	.init_size = ARRAY_SIZE(s5k5ca_init_conf),
	.mode_settings = &s5k5ca_confs[0],
	.output_settings = &s5k5ca_dimensions[0],
	.num_conf = ARRAY_SIZE(s5k5ca_confs),
};

static struct msm_sensor_ctrl_t s5k5ca_s_ctrl = {
	.msm_sensor_reg = &s5k5ca_regs,
	.sensor_i2c_client = &s5k5ca_sensor_i2c_client,
	.sensor_i2c_addr = 0x5A,
	.sensor_output_reg_addr = &s5k5ca_reg_addr,
	.sensor_id_info = &s5k5ca_id_info,
	.sensor_exp_gain_info = &s5k5ca_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &s5k5ca_csi_params_array[0],
	.msm_sensor_mutex = &s5k5ca_mut,
	.sensor_i2c_driver = &s5k5ca_i2c_driver,
	.sensor_v4l2_subdev_info = s5k5ca_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(s5k5ca_subdev_info),
	.sensor_v4l2_subdev_ops = &s5k5ca_subdev_ops,
	.func_tbl = &s5k5ca_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Samsung 3MP YUV sensor driver");
MODULE_LICENSE("GPL v2");



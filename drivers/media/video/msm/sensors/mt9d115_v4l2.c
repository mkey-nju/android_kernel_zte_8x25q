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
#define SENSOR_NAME "mt9d115"
#define PLATFORM_DRIVER_NAME "msm_camera_mt9d115"
#define mt9d115_obj mt9d115_##obj


DEFINE_MUTEX(mt9d115_mut);
static struct msm_sensor_ctrl_t mt9d115_s_ctrl;

//#define _LEN_CORRECTION_85_
//#define _LEN_CORRECTION_90_
#define _LEN_CORRECTION_100_

//24M  -250M mipi clk
//12M  -125M mipi clk
static struct msm_camera_i2c_reg_conf reset_sensor_setting1[] = {
    {0x001A, 0x0051,}, // RESET_AND_MISC_CONTROL
    {0x001A, 0x0050,}, // RESET_AND_MISC_CONTROL
};

static struct msm_camera_i2c_reg_conf reset_sensor_setting2[] = {
    {0x001A, 0x0058,}, // RESET_AND_MISC_CONTROL
};

static struct msm_camera_i2c_reg_conf  mt9d115_pll_settings[] = {
    {0x0014, 0x21F9,}, 	// PLL_CONTROL
    {0x0010, 0x0115,}, 	// PLL_DIVIDERS
    {0x0012, 0x00F5,}, 	// PLL_P_DIVIDERS
    {0x0014, 0x2545,}, 	// PLL_CONTROL
    {0x0014, 0x2547,}, 	// PLL_CONTROL
    {0x0014, 0x2447,}, 	// PLL_CONTROL
};
static struct msm_camera_i2c_reg_conf  mt9d115_settings[] = {
    {0x0014, 0x2047,}, 	// PLL_CONTROL
    {0x0014, 0x2046,}, 	// PLL_CONTROL
    {0x0018, 0x402D,}, 	// STANDBY_CONTROL
    {0x0018, 0x402C,}, 	// STANDBY_CONTROL
};

static struct msm_camera_i2c_reg_conf preview_mode_setting[] = {
    {0x098C, 0xA115,},     // MCU_ADDRESS [SEQ_CAP_MODE]
    {0x0990, 0x0000,},     // MCU_DATA_0
    {0x098C, 0xA103,},     // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0001,},     // MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf capture_mode_setting[] = {
    {0x098C, 0xA115,},     // MCU_ADDRESS [SEQ_CAP_MODE]
    {0x0990, 0x0002,},     // MCU_DATA_0
    {0x098C, 0xA103,},     // MCU_ADDRESS [SEQ_CMD]
    {0x0990, 0x0002,},     // MCU_DATA_0
};

static struct msm_camera_i2c_reg_conf software_standby_disable[] = {
   {0x0018, 0x0028,},
};

static struct msm_camera_i2c_reg_conf refresh_setting1[] = {
    {0x098C, 0xA103,},
    {0x0990, 0x0006,},     
};

static struct msm_camera_i2c_reg_conf mt9d115_refresh[] = {
    {0x098C, 0xA103,},          
    {0x0990, 0x0005,},		
};

static struct msm_camera_i2c_reg_conf mt9d115_start_settings[] = {
    {0x0018, 0x0028,},
};

static struct msm_camera_i2c_reg_conf mt9d115_stop_settings[] = {
    {0x0018, 0x0029,},
};

static struct msm_camera_i2c_reg_conf mt9d115_init_fine_tune[] = {
    {0x098C, 0x2306,}, 	// MCU_ADDRESS [AWB_CCM_L_0]
    {0x0990, 0x0163,}, 	// MCU_DATA_0
    {0x098C, 0x231C,}, 	// MCU_ADDRESS [AWB_CCM_RL_0]
    {0x0990, 0x0040,}, 	// MCU_DATA_0
    {0x098C, 0x2308,}, 	// MCU_ADDRESS [AWB_CCM_L_1]
    {0x0990, 0xFFD8,}, 	// MCU_DATA_0
    {0x098C, 0x231E,}, 	// MCU_ADDRESS [AWB_CCM_RL_1]
    {0x0990, 0xFFD4,}, 	// MCU_DATA_0
    {0x098C, 0x230A,}, 	// MCU_ADDRESS [AWB_CCM_L_2]
    {0x0990, 0xFFC5,}, 	// MCU_DATA_0
    {0x098C, 0x2320,}, 	// MCU_ADDRESS [AWB_CCM_RL_2]
    {0x0990, 0xFFEC,}, 	// MCU_DATA_0
    {0x098C, 0x230C,}, 	// MCU_ADDRESS [AWB_CCM_L_3]
    {0x0990, 0xFFB2,}, 	// MCU_DATA_0
    {0x098C, 0x2322,}, 	// MCU_ADDRESS [AWB_CCM_RL_3]
    {0x0990, 0xFFE7,}, 	// MCU_DATA_0
    {0x098C, 0x230E,}, 	// MCU_ADDRESS [AWB_CCM_L_4]
    {0x0990, 0x0150,}, 	// MCU_DATA_0
    {0x098C, 0x2324,}, 	// MCU_ADDRESS [AWB_CCM_RL_4]
    {0x0990, 0x0035,}, 	// MCU_DATA_0
    {0x098C, 0x2310,}, 	// MCU_ADDRESS [AWB_CCM_L_5]
    {0x0990, 0xFFFE,}, 	// MCU_DATA_0
    {0x098C, 0x2326,}, 	// MCU_ADDRESS [AWB_CCM_RL_5]
    {0x0990, 0xFFE4,}, 	// MCU_DATA_0
    {0x098C, 0x2312,}, 	// MCU_ADDRESS [AWB_CCM_L_6]
    {0x0990, 0xFFDD,}, 	// MCU_DATA_0
    {0x098C, 0x2328,}, 	// MCU_ADDRESS [AWB_CCM_RL_6]
    {0x0990, 0x000D,}, 	// MCU_DATA_0
    {0x098C, 0x2314,}, 	// MCU_ADDRESS [AWB_CCM_L_7]
    {0x0990, 0xFF95,}, 	// MCU_DATA_0
    {0x098C, 0x232A,}, 	// MCU_ADDRESS [AWB_CCM_RL_7]
    {0x0990, 0xFFF5,}, 	// MCU_DATA_0
    {0x098C, 0x2316,}, 	// MCU_ADDRESS [AWB_CCM_L_8]
    {0x0990, 0x018C,}, 	// MCU_DATA_0
    {0x098C, 0x232C,}, 	// MCU_ADDRESS [AWB_CCM_RL_8]
    {0x0990, 0x0000,}, 	// MCU_DATA_0
                   
    {0x098C, 0xA366,}, 	// MCU_ADDRESS [AWB_KR_L]
    {0x0990, 0x0080,}, 	// MCU_DATA_0
    {0x098C, 0xA368,}, 	// MCU_ADDRESS [AWB_KB_L]
    {0x0990, 0x008B,}, 	// MCU_DATA_0
                   
//[Day light]      
    {0x098C, 0xA369,}, 	// MCU_ADDRESS [AWB_KR_R]
    {0x0990, 0x007A,}, 	// MCU_DATA_0
    {0x098C, 0xAB20,}, 	// MCU_ADDRESS [HG_LL_SAT1]
    {0x0990, 0x00B0,}, 	// MCU_DATA_0
                
// for CWF        
    {0x098C, 0xA36E,}, 	// MCU_ADDRESS [AWB_EDGETH_MAX]
    {0x0990, 0x0028,}, 	// MCU_DATA_0
    {0x098C, 0xA363,}, 	// MCU_ADDRESS [AWB_TG_MIN0]
    {0x0990, 0x00CF,}, 	// MCU_DATA_0
// End finetune. 2011-7-7
                   
		               
//[Lens Correction 85% 07/08/11 21:50:50]
#ifdef _LEN_CORRECTION_85_
    {0x3210, 0x01B0,}, 	// COLOR_PIPELINE_CONTROL
    {0x364E, 0x0390,}, 	// P_GR_P0Q0
    {0x3650, 0x04ED,}, 	// P_GR_P0Q1
    {0x3652, 0x1871,}, 	// P_GR_P0Q2
    {0x3654, 0x0D0D,}, 	// P_GR_P0Q3
    {0x3656, 0x90B2,}, 	// P_GR_P0Q4
    {0x3658, 0x7EEF,}, 	// P_RD_P0Q0
    {0x365A, 0x3B6C,}, 	// P_RD_P0Q1
    {0x365C, 0x1411,}, 	// P_RD_P0Q2
    {0x365E, 0x474E,}, 	// P_RD_P0Q3
    {0x3660, 0xB891,}, 	// P_RD_P0Q4
    {0x3662, 0x7F0F,}, 	// P_BL_P0Q0
    {0x3664, 0x05CD,}, 	// P_BL_P0Q1
    {0x3666, 0x0151,}, 	// P_BL_P0Q2
    {0x3668, 0x2ECE,}, 	// P_BL_P0Q3
    {0x366A, 0xFB91,}, 	// P_BL_P0Q4
    {0x366C, 0x7E8F,}, 	// P_GB_P0Q0
    {0x366E, 0x138C,}, 	// P_GB_P0Q1
    {0x3670, 0x1DD1,}, 	// P_GB_P0Q2
    {0x3672, 0x1CEE,}, 	// P_GB_P0Q3
    {0x3674, 0x9F72,}, 	// P_GB_P0Q4
    {0x3676, 0x190C,}, 	// P_GR_P1Q0
    {0x3678, 0xCE8E,}, 	// P_GR_P1Q1
    {0x367A, 0xE7CE,}, 	// P_GR_P1Q2
    {0x367C, 0x0091,}, 	// P_GR_P1Q3
    {0x367E, 0x586A,}, 	// P_GR_P1Q4
    {0x3680, 0x78AB,}, 	// P_RD_P1Q0
    {0x3682, 0x35AE,}, 	// P_RD_P1Q1
    {0x3684, 0x58CF,}, 	// P_RD_P1Q2
    {0x3686, 0xCA50,}, 	// P_RD_P1Q3
    {0x3688, 0x9E71,}, 	// P_RD_P1Q4
    {0x368A, 0xFD8C,}, 	// P_BL_P1Q0
    {0x368C, 0xEAAD,}, 	// P_BL_P1Q1
    {0x368E, 0xB86E,}, 	// P_BL_P1Q2
    {0x3690, 0x4270,}, 	// P_BL_P1Q3
    {0x3692, 0x9CEC,}, 	// P_BL_P1Q4
    {0x3694, 0x99CD,}, 	// P_GB_P1Q0
    {0x3696, 0x07EF,}, 	// P_GB_P1Q1
    {0x3698, 0x542F,}, 	// P_GB_P1Q2
    {0x369A, 0xBEF0,}, 	// P_GB_P1Q3
    {0x369C, 0xD770,}, 	// P_GB_P1Q4
    {0x369E, 0x0CB2,}, 	// P_GR_P2Q0
    {0x36A0, 0x462E,}, 	// P_GR_P2Q1
    {0x36A2, 0x4512,}, 	// P_GR_P2Q2
    {0x36A4, 0x1393,}, 	// P_GR_P2Q3
    {0x36A6, 0x8297,}, 	// P_GR_P2Q4
    {0x36A8, 0x7CF1,}, 	// P_RD_P2Q0
    {0x36AA, 0x3CB0,}, 	// P_RD_P2Q1
    {0x36AC, 0x6C92,}, 	// P_RD_P2Q2
    {0x36AE, 0x136F,}, 	// P_RD_P2Q3
    {0x36B0, 0xE1B6,}, 	// P_RD_P2Q4
    {0x36B2, 0x7811,}, 	// P_BL_P2Q0
    {0x36B4, 0x09D0,}, 	// P_BL_P2Q1
    {0x36B6, 0xC8B0,}, 	// P_BL_P2Q2
    {0x36B8, 0x7F11,}, 	// P_BL_P2Q3
    {0x36BA, 0x8876,}, 	// P_BL_P2Q4
    {0x36BC, 0x10F2,}, 	// P_GB_P2Q0
    {0x36BE, 0x03D0,}, 	// P_GB_P2Q1
    {0x36C0, 0x04F2,}, 	// P_GB_P2Q2
    {0x36C2, 0x5692,}, 	// P_GB_P2Q3
    {0x36C4, 0xFE96,}, 	// P_GB_P2Q4
    {0x36C6, 0xB9AE,}, 	// P_GR_P3Q0
    {0x36C8, 0x2CD0,}, 	// P_GR_P3Q1
    {0x36CA, 0x704F,}, 	// P_GR_P3Q2
    {0x36CC, 0x9713,}, 	// P_GR_P3Q3
    {0x36CE, 0x1554,}, 	// P_GR_P3Q4
    {0x36D0, 0x176E,}, 	// P_RD_P3Q0
    {0x36D2, 0xB8AF,}, 	// P_RD_P3Q1
    {0x36D4, 0xB411,}, 	// P_RD_P3Q2
    {0x36D6, 0x4AF2,}, 	// P_RD_P3Q3
    {0x36D8, 0x9A12,}, 	// P_RD_P3Q4
    {0x36DA, 0x44EF,}, 	// P_BL_P3Q0
    {0x36DC, 0x2691,}, 	// P_BL_P3Q1
    {0x36DE, 0x9F73,}, 	// P_BL_P3Q2
    {0x36E0, 0xC433,}, 	// P_BL_P3Q3
    {0x36E2, 0x0A56,}, 	// P_BL_P3Q4
    {0x36E4, 0x58EE,}, 	// P_GB_P3Q0
    {0x36E6, 0xF790,}, 	// P_GB_P3Q1
    {0x36E8, 0xE8B1,}, 	// P_GB_P3Q2
    {0x36EA, 0xE1CC,}, 	// P_GB_P3Q3
    {0x36EC, 0x3B14,}, 	// P_GB_P3Q4
    {0x36EE, 0x94D3,}, 	// P_GR_P4Q0
    {0x36F0, 0x1B53,}, 	// P_GR_P4Q1
    {0x36F2, 0xE217,}, 	// P_GR_P4Q2
    {0x36F4, 0x83B7,}, 	// P_GR_P4Q3
    {0x36F6, 0x3B9A,}, 	// P_GR_P4Q4
    {0x36F8, 0x8B53,}, 	// P_RD_P4Q0
    {0x36FA, 0xA592,}, 	// P_RD_P4Q1
    {0x36FC, 0xA437,}, 	// P_RD_P4Q2
    {0x36FE, 0x2734,}, 	// P_RD_P4Q3
    {0x3700, 0x39F9,}, 	// P_RD_P4Q4
    {0x3702, 0xD1B3,}, 	// P_BL_P4Q0
    {0x3704, 0x8F32,}, 	// P_BL_P4Q1
    {0x3706, 0xD796,}, 	// P_BL_P4Q2
    {0x3708, 0xB1F6,}, 	// P_BL_P4Q3
    {0x370A, 0x67B9,}, 	// P_BL_P4Q4
    {0x370C, 0xA013,}, 	// P_GB_P4Q0
    {0x370E, 0x6192,}, 	// P_GB_P4Q1
    {0x3710, 0xD8B7,}, 	// P_GB_P4Q2
    {0x3712, 0xD7D6,}, 	// P_GB_P4Q3
    {0x3714, 0x3ABA,}, 	// P_GB_P4Q4
    {0x3644, 0x02BC,}, 	// POLY_ORIGIN_C
    {0x3642, 0x024C,}, 	// POLY_ORIGIN_R
    {0x3210, 0x01B8,}, 	// COLOR_PIPELINE_CONTROL
#endif             
                   
                   
//[Lens Correction 90% 07/08/11 21:50:57]
#ifdef _LEN_CORRECTION_90_
    {0x3210, 0x01B0,}, 	// COLOR_PIPELINE_CONTROL
    {0x364E, 0x0370,}, 	// P_GR_P0Q0
    {0x3650, 0x462C,}, 	// P_GR_P0Q1
    {0x3652, 0x2FD1,}, 	// P_GR_P0Q2
    {0x3654, 0x3C6B,}, 	// P_GR_P0Q3
    {0x3656, 0xE931,}, 	// P_GR_P0Q4
    {0x3658, 0x7F8F,}, 	// P_RD_P0Q0
    {0x365A, 0x75EB,}, 	// P_RD_P0Q1
    {0x365C, 0x2AB1,}, 	// P_RD_P0Q2
    {0x365E, 0x19CE,}, 	// P_RD_P0Q3
    {0x3660, 0xF7B0,}, 	// P_RD_P0Q4
    {0x3662, 0x7F2F,}, 	// P_BL_P0Q0
    {0x3664, 0x454C,}, 	// P_BL_P0Q1
    {0x3666, 0x17B1,}, 	// P_BL_P0Q2
    {0x3668, 0x17EE,}, 	// P_BL_P0Q3
    {0x366A, 0xC971,}, 	// P_BL_P0Q4
    {0x366C, 0x7E6F,}, 	// P_GB_P0Q0
    {0x366E, 0x21CB,}, 	// P_GB_P0Q1
    {0x3670, 0x34D1,}, 	// P_GB_P0Q2
    {0x3672, 0x4C6D,}, 	// P_GB_P0Q3
    {0x3674, 0x83F2,}, 	// P_GB_P0Q4
    {0x3676, 0x124C,}, 	// P_GR_P1Q0
    {0x3678, 0xD04E,}, 	// P_GR_P1Q1
    {0x367A, 0xDFAE,}, 	// P_GR_P1Q2
    {0x367C, 0x7B50,}, 	// P_GR_P1Q3
    {0x367E, 0x98CE,}, 	// P_GR_P1Q4
    {0x3680, 0x674B,}, 	// P_RD_P1Q0
    {0x3682, 0x304E,}, 	// P_RD_P1Q1
    {0x3684, 0x4A6F,}, 	// P_RD_P1Q2
    {0x3686, 0xB710,}, 	// P_RD_P1Q3
    {0x3688, 0xF770,}, 	// P_RD_P1Q4
    {0x368A, 0x862D,}, 	// P_BL_P1Q0
    {0x368C, 0xDA6D,}, 	// P_BL_P1Q1
    {0x368E, 0xC70E,}, 	// P_BL_P1Q2
    {0x3690, 0x31B0,}, 	// P_BL_P1Q3
    {0x3692, 0xC2AE,}, 	// P_BL_P1Q4
    {0x3694, 0xA18D,}, 	// P_GB_P1Q0
    {0x3696, 0x09EF,}, 	// P_GB_P1Q1
    {0x3698, 0x6E6F,}, 	// P_GB_P1Q2
    {0x369A, 0xA6F0,}, 	// P_GB_P1Q3
    {0x369C, 0xC111,}, 	// P_GB_P1Q4
    {0x369E, 0x1892,}, 	// P_GR_P2Q0
    {0x36A0, 0x4D6D,}, 	// P_GR_P2Q1
    {0x36A2, 0x1C13,}, 	// P_GR_P2Q2
    {0x36A4, 0x1533,}, 	// P_GR_P2Q3
    {0x36A6, 0x8A17,}, 	// P_GR_P2Q4
    {0x36A8, 0x0912,}, 	// P_RD_P2Q0
    {0x36AA, 0x18B0,}, 	// P_RD_P2Q1
    {0x36AC, 0x2AD3,}, 	// P_RD_P2Q2
    {0x36AE, 0x6F70,}, 	// P_RD_P2Q3
    {0x36B0, 0xEAF6,}, 	// P_RD_P2Q4
    {0x36B2, 0x0752,}, 	// P_BL_P2Q0
    {0x36B4, 0x0170,}, 	// P_BL_P2Q1
    {0x36B6, 0x1810,}, 	// P_BL_P2Q2
    {0x36B8, 0x6171,}, 	// P_BL_P2Q3
    {0x36BA, 0x8C96,}, 	// P_BL_P2Q4
    {0x36BC, 0x1C12,}, 	// P_GB_P2Q0
    {0x36BE, 0x65AF,}, 	// P_GB_P2Q1
    {0x36C0, 0x70F2,}, 	// P_GB_P2Q2
    {0x36C2, 0x6172,}, 	// P_GB_P2Q3
    {0x36C4, 0x8657,}, 	// P_GB_P2Q4
    {0x36C6, 0xC5CE,}, 	// P_GR_P3Q0
    {0x36C8, 0x1890,}, 	// P_GR_P3Q1
    {0x36CA, 0xB5CE,}, 	// P_GR_P3Q2
    {0x36CC, 0x8473,}, 	// P_GR_P3Q3
    {0x36CE, 0x6B34,}, 	// P_GR_P3Q4
    {0x36D0, 0x146E,}, 	// P_RD_P3Q0
    {0x36D2, 0xD1AE,}, 	// P_RD_P3Q1
    {0x36D4, 0x9391,}, 	// P_RD_P3Q2
    {0x36D6, 0x37F1,}, 	// P_RD_P3Q3
    {0x36D8, 0xD952,}, 	// P_RD_P3Q4
    {0x36DA, 0x3F2F,}, 	// P_BL_P3Q0
    {0x36DC, 0x1C31,}, 	// P_BL_P3Q1
    {0x36DE, 0xB5F3,}, 	// P_BL_P3Q2
    {0x36E0, 0xA2D3,}, 	// P_BL_P3Q3
    {0x36E2, 0x1BF6,}, 	// P_BL_P3Q4
    {0x36E4, 0x452E,}, 	// P_GB_P3Q0
    {0x36E6, 0xE450,}, 	// P_GB_P3Q1
    {0x36E8, 0xCD72,}, 	// P_GB_P3Q2
    {0x36EA, 0x9D31,}, 	// P_GB_P3Q3
    {0x36EC, 0x3815,}, 	// P_GB_P3Q4
    {0x36EE, 0xFA32,}, 	// P_GR_P4Q0
    {0x36F0, 0x2793,}, 	// P_GR_P4Q1
    {0x36F2, 0xF0D7,}, 	// P_GR_P4Q2
    {0x36F4, 0xF056,}, 	// P_GR_P4Q3
    {0x36F6, 0x3F7A,}, 	// P_GR_P4Q4
    {0x36F8, 0xE252,}, 	// P_RD_P4Q0
    {0x36FA, 0xD271,}, 	// P_RD_P4Q1
    {0x36FC, 0xAAF7,}, 	// P_RD_P4Q2
    {0x36FE, 0x6313,}, 	// P_RD_P4Q3
    {0x3700, 0x3299,}, 	// P_RD_P4Q4
    {0x3702, 0xBEF3,}, 	// P_BL_P4Q0
    {0x3704, 0x9452,}, 	// P_BL_P4Q1
    {0x3706, 0xDF76,}, 	// P_BL_P4Q2
    {0x3708, 0x9E76,}, 	// P_BL_P4Q3
    {0x370A, 0x5B19,}, 	// P_BL_P4Q4
    {0x370C, 0x8753,}, 	// P_GB_P4Q0
    {0x370E, 0x5492,}, 	// P_GB_P4Q1
    {0x3710, 0xDFF7,}, 	// P_GB_P4Q2
    {0x3712, 0xC1D6,}, 	// P_GB_P4Q3
    {0x3714, 0x3A1A,}, 	// P_GB_P4Q4
    {0x3644, 0x02BC,}, 	// POLY_ORIGIN_C
    {0x3642, 0x024C,}, 	// POLY_ORIGIN_R
    {0x3210, 0x01B8,}, 	// COLOR_PIPELINE_CONTROL
#endif             
                   
                   
                   
//[Lens Correction 100% 07/08/11 21:51:09]
#ifdef _LEN_CORRECTION_100_
    {0x3210, 0x01B0,}, 	// COLOR_PIPELINE_CONTROL
    {0x364E, 0x03B0,}, 	// P_GR_P0Q0
    {0x3650, 0x74EA,}, 	// P_GR_P0Q1
    {0x3652, 0x5C91,}, 	// P_GR_P0Q2
    {0x3654, 0x9E0D,}, 	// P_GR_P0Q3
    {0x3656, 0xD4F0,}, 	// P_GR_P0Q4
    {0x3658, 0x7FAF,}, 	// P_RD_P0Q0
    {0x365A, 0x99E8,}, 	// P_RD_P0Q1
    {0x365C, 0x5611,}, 	// P_RD_P0Q2
    {0x365E, 0x454C,}, 	// P_RD_P0Q3
    {0x3660, 0x066D,}, 	// P_RD_P0Q4
    {0x3662, 0x7FCF,}, 	// P_BL_P0Q0
    {0x3664, 0x028B,}, 	// P_BL_P0Q1
    {0x3666, 0x42D1,}, 	// P_BL_P0Q2
    {0x3668, 0x120D,}, 	// P_BL_P0Q3
    {0x366A, 0xABB0,}, 	// P_BL_P0Q4
    {0x366C, 0x7EAF,}, 	// P_GB_P0Q0
    {0x366E, 0xCF8A,}, 	// P_GB_P0Q1
    {0x3670, 0x6151,}, 	// P_GB_P0Q2
    {0x3672, 0x9189,}, 	// P_GB_P0Q3
    {0x3674, 0x9251,}, 	// P_GB_P0Q4
    {0x3676, 0x7E4B,}, 	// P_GR_P1Q0
    {0x3678, 0xCE8E,}, 	// P_GR_P1Q1
    {0x367A, 0xBE8E,}, 	// P_GR_P1Q2
    {0x367C, 0x6490,}, 	// P_GR_P1Q3
    {0x367E, 0xD86F,}, 	// P_GR_P1Q4
    {0x3680, 0x456B,}, 	// P_RD_P1Q0
    {0x3682, 0x3B2E,}, 	// P_RD_P1Q1
    {0x3684, 0x52AF,}, 	// P_RD_P1Q2
    {0x3686, 0xCA50,}, 	// P_RD_P1Q3
    {0x3688, 0xFDD0,}, 	// P_RD_P1Q4
    {0x368A, 0x8D8D,}, 	// P_BL_P1Q0
    {0x368C, 0xDB0D,}, 	// P_BL_P1Q1
    {0x368E, 0xF5CE,}, 	// P_BL_P1Q2
    {0x3690, 0x3D90,}, 	// P_BL_P1Q3
    {0x3692, 0xE0AF,}, 	// P_BL_P1Q4
    {0x3694, 0xA64D,}, 	// P_GB_P1Q0
    {0x3696, 0x0D6F,}, 	// P_GB_P1Q1
    {0x3698, 0x1B6F,}, 	// P_GB_P1Q2
    {0x369A, 0x8E10,}, 	// P_GB_P1Q3
    {0x369C, 0xD1B0,}, 	// P_GB_P1Q4
    {0x369E, 0x2DF2,}, 	// P_GR_P2Q0
    {0x36A0, 0xB2CD,}, 	// P_GR_P2Q1
    {0x36A2, 0x1454,}, 	// P_GR_P2Q2
    {0x36A4, 0x4753,}, 	// P_GR_P2Q3
    {0x36A6, 0xA777,}, 	// P_GR_P2Q4
    {0x36A8, 0x1EF2,}, 	// P_RD_P2Q0
    {0x36AA, 0x6DCF,}, 	// P_RD_P2Q1
    {0x36AC, 0x0A14,}, 	// P_RD_P2Q2
    {0x36AE, 0x37B1,}, 	// P_RD_P2Q3
    {0x36B0, 0xFAD6,}, 	// P_RD_P2Q4
    {0x36B2, 0x1D92,}, 	// P_BL_P2Q0
    {0x36B4, 0x366F,}, 	// P_BL_P2Q1
    {0x36B6, 0x7192,}, 	// P_BL_P2Q2
    {0x36B8, 0x4D52,}, 	// P_BL_P2Q3
    {0x36BA, 0xB096,}, 	// P_BL_P2Q4
    {0x36BC, 0x31B2,}, 	// P_GB_P2Q0
    {0x36BE, 0x5E6E,}, 	// P_GB_P2Q1
    {0x36C0, 0x7533,}, 	// P_GB_P2Q2
    {0x36C2, 0x12D3,}, 	// P_GB_P2Q3
    {0x36C4, 0x9A37,}, 	// P_GB_P2Q4
    {0x36C6, 0xBF6E,}, 	// P_GR_P3Q0
    {0x36C8, 0x694F,}, 	// P_GR_P3Q1
    {0x36CA, 0x9071,}, 	// P_GR_P3Q2
    {0x36CC, 0xDB32,}, 	// P_GR_P3Q3
    {0x36CE, 0x2CD5,}, 	// P_GR_P3Q4
    {0x36D0, 0x1D4E,}, 	// P_RD_P3Q0
    {0x36D2, 0x8FAF,}, 	// P_RD_P3Q1
    {0x36D4, 0x8251,}, 	// P_RD_P3Q2
    {0x36D6, 0x4D72,}, 	// P_RD_P3Q3
    {0x36D8, 0xA2D2,}, 	// P_RD_P3Q4
    {0x36DA, 0x170F,}, 	// P_BL_P3Q0
    {0x36DC, 0x1C11,}, 	// P_BL_P3Q1
    {0x36DE, 0xC2F3,}, 	// P_BL_P3Q2
    {0x36E0, 0x89F3,}, 	// P_BL_P3Q3
    {0x36E2, 0x27B6,}, 	// P_BL_P3Q4
    {0x36E4, 0x1F0D,}, 	// P_GB_P3Q0
    {0x36E6, 0xC270,}, 	// P_GB_P3Q1
    {0x36E8, 0xFDD0,}, 	// P_GB_P3Q2
    {0x36EA, 0x9E71,}, 	// P_GB_P3Q3
    {0x36EC, 0x7B53,}, 	// P_GB_P3Q4
    {0x36EE, 0x8632,}, 	// P_GR_P4Q0
    {0x36F0, 0x47F3,}, 	// P_GR_P4Q1
    {0x36F2, 0x9058,}, 	// P_GR_P4Q2
    {0x36F4, 0x9997,}, 	// P_GR_P4Q3
    {0x36F6, 0x64BA,}, 	// P_GR_P4Q4
    {0x36F8, 0x8D92,}, 	// P_RD_P4Q0
    {0x36FA, 0x8732,}, 	// P_RD_P4Q1
    {0x36FC, 0xB457,}, 	// P_RD_P4Q2
    {0x36FE, 0x7E14,}, 	// P_RD_P4Q3
    {0x3700, 0x1C99,}, 	// P_RD_P4Q4
    {0x3702, 0x97D3,}, 	// P_BL_P4Q0
    {0x3704, 0xDDB1,}, 	// P_BL_P4Q1
    {0x3706, 0x8B97,}, 	// P_BL_P4Q2
    {0x3708, 0xE0F6,}, 	// P_BL_P4Q3
    {0x370A, 0x059A,}, 	// P_BL_P4Q4
    {0x370C, 0xA5F2,}, 	// P_GB_P4Q0
    {0x370E, 0x1373,}, 	// P_GB_P4Q1
    {0x3710, 0x80F8,}, 	// P_GB_P4Q2
    {0x3712, 0xDA36,}, 	// P_GB_P4Q3
    {0x3714, 0x4D3A,}, 	// P_GB_P4Q4
    {0x3644, 0x02BC,}, 	// POLY_ORIGIN_C
    {0x3642, 0x024C,}, 	// POLY_ORIGIN_R
    {0x3210, 0x01B8,}, 	// COLOR_PIPELINE_CONTROL
#endif             
                   
};                  

static struct msm_camera_i2c_reg_conf mt9d115_recommend_settings[] = {
    
//800x600
    {0x098C, 0x2703,}, 	// MCU_ADDRESS [MODE_OUTPUT_WIDTH_A]
    {0x0990, 0x0320,}, 	// MCU_DATA_0
    {0x098C, 0x2705,}, 	// MCU_ADDRESS [MODE_OUTPUT_HEIGHT_A]
    {0x0990, 0x0258,}, 	// MCU_DATA_0

//1024x768
//    {0x098C, 0x2703,}, 	// MCU_ADDRESS [MODE_OUTPUT_WIDTH_A]
//    {0x0990, 0x0400,}, 	// MCU_DATA_0
//    {0x098C, 0x2705,}, 	// MCU_ADDRESS [MODE_OUTPUT_HEIGHT_A]
//    {0x0990, 0x0300,}, 	// MCU_DATA_0

//640x480
//    {0x098C, 0x2703,}, 	// MCU_ADDRESS [MODE_OUTPUT_WIDTH_A]
//    {0x0990, 0x0280,}, 	// MCU_DATA_0
//    {0x098C, 0x2705,}, 	// MCU_ADDRESS [MODE_OUTPUT_HEIGHT_A]
//    {0x0990, 0x01E0,}, 	// MCU_DATA_0

    {0x098C, 0x2707,}, 	// MCU_ADDRESS [MODE_OUTPUT_WIDTH_B]
    {0x0990, 0x0640,}, 	// MCU_DATA_0
    {0x098C, 0x2709,}, 	// MCU_ADDRESS [MODE_OUTPUT_HEIGHT_B]
    {0x0990, 0x04B0,}, 	// MCU_DATA_0
    {0x098C, 0x270D,}, 	// MCU_ADDRESS [MODE_SENSOR_ROW_START_A]
    {0x0990, 0x0000,}, 	// MCU_DATA_0
    {0x098C, 0x270F,}, 	// MCU_ADDRESS [MODE_SENSOR_COL_START_A]
    {0x0990, 0x0000,}, 	// MCU_DATA_0
    {0x098C, 0x2711,}, 	// MCU_ADDRESS [MODE_SENSOR_ROW_END_A]
    {0x0990, 0x04BD,}, 	// MCU_DATA_0
    {0x098C, 0x2713,}, 	// MCU_ADDRESS [MODE_SENSOR_COL_END_A]
    {0x0990, 0x064D,}, 	// MCU_DATA_0
    {0x098C, 0x2715,}, 	// MCU_ADDRESS [MODE_SENSOR_ROW_SPEED_A]
    {0x0990, 0x0111,}, 	// MCU_DATA_0
    {0x098C, 0x2717,}, 	// MCU_ADDRESS [MODE_SENSOR_READ_MODE_A]
    {0x0990, 0x046C,}, 	// MCU_DATA_0
    {0x098C, 0x2719,}, 	// MCU_ADDRESS [MODE_SENSOR_FINE_CORRECTION_A]
    {0x0990, 0x005A,}, 	// MCU_DATA_0
    {0x098C, 0x271B,}, 	// MCU_ADDRESS [MODE_SENSOR_FINE_IT_MIN_A]
    {0x0990, 0x01BE,}, 	// MCU_DATA_0
    {0x098C, 0x271D,}, 	// MCU_ADDRESS [MODE_SENSOR_FINE_IT_MAX_MARGIN_A]
    {0x0990, 0x0131,}, 	// MCU_DATA_0
    {0x098C, 0x271F,}, 	// MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_A]
    {0x0990, 0x02BB,}, 	// MCU_DATA_0
    {0x098C, 0x2721,}, 	// MCU_ADDRESS [MODE_SENSOR_LINE_LENGTH_PCK_A]
    {0x0990, 0x0888,}, 	// MCU_DATA_0
    {0x098C, 0x2723,}, 	// MCU_ADDRESS [MODE_SENSOR_ROW_START_B]
    {0x0990, 0x0004,}, 	// MCU_DATA_0
    {0x098C, 0x2725,}, 	// MCU_ADDRESS [MODE_SENSOR_COL_START_B]
    {0x0990, 0x0004,}, 	// MCU_DATA_0
    {0x098C, 0x2727,}, 	// MCU_ADDRESS [MODE_SENSOR_ROW_END_B]
    {0x0990, 0x04BB,}, 	// MCU_DATA_0
    {0x098C, 0x2729,}, 	// MCU_ADDRESS [MODE_SENSOR_COL_END_B]
    {0x0990, 0x064B,}, 	// MCU_DATA_0
    {0x098C, 0x272B,}, 	// MCU_ADDRESS [MODE_SENSOR_ROW_SPEED_B]
    {0x0990, 0x0111,}, 	// MCU_DATA_0
    {0x098C, 0x272D,}, 	// MCU_ADDRESS [MODE_SENSOR_READ_MODE_B]
    {0x0990, 0x0024,}, 	// MCU_DATA_0
    {0x098C, 0x272F,}, 	// MCU_ADDRESS [MODE_SENSOR_FINE_CORRECTION_B]
    {0x0990, 0x003A,}, 	// MCU_DATA_0
    {0x098C, 0x2731,}, 	// MCU_ADDRESS [MODE_SENSOR_FINE_IT_MIN_B]
    {0x0990, 0x00F6,}, 	// MCU_DATA_0
    {0x098C, 0x2733,}, 	// MCU_ADDRESS [MODE_SENSOR_FINE_IT_MAX_MARGIN_B]
    {0x0990, 0x008B,}, 	// MCU_DATA_0
    {0x098C, 0x2735,}, 	// MCU_ADDRESS [MODE_SENSOR_FRAME_LENGTH_B]
    {0x0990, 0x0521,}, 	// MCU_DATA_0
    {0x098C, 0x2737,}, 	// MCU_ADDRESS [MODE_SENSOR_LINE_LENGTH_PCK_B]
    {0x0990, 0x0888,}, 	// MCU_DATA_0
    {0x098C, 0x2739,}, 	// MCU_ADDRESS [MODE_CROP_X0_A]
    {0x0990, 0x0000,}, 	// MCU_DATA_0
    {0x098C, 0x273B,}, 	// MCU_ADDRESS [MODE_CROP_X1_A]
    {0x0990, 0x031F,}, 	// MCU_DATA_0
    {0x098C, 0x273D,}, 	// MCU_ADDRESS [MODE_CROP_Y0_A]
    {0x0990, 0x0000,}, 	// MCU_DATA_0
    {0x098C, 0x273F,}, 	// MCU_ADDRESS [MODE_CROP_Y1_A]
    {0x0990, 0x0257,}, 	// MCU_DATA_0
    {0x098C, 0x2747,}, 	// MCU_ADDRESS [MODE_CROP_X0_B]
    {0x0990, 0x0000,}, 	// MCU_DATA_0
    {0x098C, 0x2749,}, 	// MCU_ADDRESS [MODE_CROP_X1_B]
    {0x0990, 0x063F,}, 	// MCU_DATA_0
    {0x098C, 0x274B,}, 	// MCU_ADDRESS [MODE_CROP_Y0_B]
    {0x0990, 0x0000,}, 	// MCU_DATA_0
    {0x098C, 0x274D,}, 	// MCU_ADDRESS [MODE_CROP_Y1_B]
    {0x0990, 0x04AF,}, 	// MCU_DATA_0
    {0x098C, 0x2222,}, 	// MCU_ADDRESS [AE_R9]
    {0x0990, 0x00A0,}, 	// MCU_DATA_0
    {0x098C, 0xA408,}, 	// MCU_ADDRESS [FD_SEARCH_F1_50]
    {0x0990, 0x0026,}, 	// MCU_DATA_0
    {0x098C, 0xA409,}, 	// MCU_ADDRESS [FD_SEARCH_F2_50]
    {0x0990, 0x0029,}, 	// MCU_DATA_0
    {0x098C, 0xA40A,}, 	// MCU_ADDRESS [FD_SEARCH_F1_60]
    {0x0990, 0x002E,}, 	// MCU_DATA_0
    {0x098C, 0xA40B,}, 	// MCU_ADDRESS [FD_SEARCH_F2_60]
    {0x0990, 0x0031,}, 	// MCU_DATA_0
    {0x098C, 0x2411,}, 	// MCU_ADDRESS [FD_R9_STEP_F60_A]
    {0x0990, 0x00A0,}, 	// MCU_DATA_0
    {0x098C, 0x2413,}, 	// MCU_ADDRESS [FD_R9_STEP_F50_A]
    {0x0990, 0x00C0,}, 	// MCU_DATA_0
    {0x098C, 0x2415,}, 	// MCU_ADDRESS [FD_R9_STEP_F60_B]
    {0x0990, 0x00A0,}, 	// MCU_DATA_0
    {0x098C, 0x2417,}, 	// MCU_ADDRESS [FD_R9_STEP_F50_B]
    {0x0990, 0x00C0,}, 	// MCU_DATA_0
    {0x098C, 0xA404,}, 	// MCU_ADDRESS [FD_MODE]
    {0x0990, 0x0010,}, 	// MCU_DATA_0
    {0x098C, 0xA40D,}, 	// MCU_ADDRESS [FD_STAT_MIN]
    {0x0990, 0x0002,}, 	// MCU_DATA_0
    {0x098C, 0xA40E,}, 	// MCU_ADDRESS [FD_STAT_MAX]
    {0x0990, 0x0003,}, 	// MCU_DATA_0
    {0x098C, 0xA410,}, 	// MCU_ADDRESS [FD_MIN_AMPLITUDE]
    {0x0990, 0x000A,}, 	// MCU_DATA_0
    {0x098C, 0xA117,}, 	// MCU_ADDRESS [SEQ_PREVIEW_0_AE]
    {0x0990, 0x0002,}, 	// MCU_DATA_0
    {0x098C, 0xA11D,}, 	// MCU_ADDRESS [SEQ_PREVIEW_1_AE]
    {0x0990, 0x0002,}, 	// MCU_DATA_0
    {0x098C, 0xA129,}, 	// MCU_ADDRESS [SEQ_PREVIEW_3_AE]
    {0x0990, 0x0002,}, 	// MCU_DATA_0
    {0x098C, 0xA24F,}, 	// MCU_ADDRESS [AE_BASETARGET]
    {0x0990, 0x0032,}, 	// MCU_DATA_0
    {0x098C, 0xA20C,}, 	// MCU_ADDRESS [AE_MAX_INDEX]
    {0x0990, 0x0010,}, 	// MCU_DATA_0
    {0x098C, 0xA216,}, 	// MCU_ADDRESS
    {0x0990, 0x0091,}, 	// MCU_DATA_0
    {0x098C, 0xA20E,}, 	// MCU_ADDRESS [AE_MAX_VIRTGAIN]
    {0x0990, 0x0091,}, 	// MCU_DATA_0
    {0x098C, 0x2212,}, 	// MCU_ADDRESS [AE_MAX_DGAIN_AE1]
    {0x0990, 0x00A4,}, 	// MCU_DATA_0
    {0x3210, 0x01B8,}, 	// COLOR_PIPELINE_CONTROL
    {0x098C, 0xAB36,}, 	// MCU_ADDRESS [HG_CLUSTERDC_TH]
    {0x0990, 0x0014,}, 	// MCU_DATA_0
    {0x098C, 0x2B66,}, 	// MCU_ADDRESS [HG_CLUSTER_DC_BM]
    {0x0990, 0x2AF8,}, 	// MCU_DATA_0
    {0x098C, 0xAB20,}, 	// MCU_ADDRESS [HG_LL_SAT1]
    {0x0990, 0x0080,}, 	// MCU_DATA_0
    {0x098C, 0xAB24,}, 	// MCU_ADDRESS [HG_LL_SAT2]
    {0x0990, 0x0000,}, 	// MCU_DATA_0
    {0x098C, 0xAB21,}, 	// MCU_ADDRESS [HG_LL_INTERPTHRESH1]
    {0x0990, 0x000A,}, 	// MCU_DATA_0
    {0x098C, 0xAB25,}, 	// MCU_ADDRESS [HG_LL_INTERPTHRESH2]
    {0x0990, 0x002A,}, 	// MCU_DATA_0
    {0x098C, 0xAB22,}, 	// MCU_ADDRESS [HG_LL_APCORR1]
    {0x0990, 0x0007,}, 	// MCU_DATA_0
    {0x098C, 0xAB26,}, 	// MCU_ADDRESS [HG_LL_APCORR2]
    {0x0990, 0x0001,}, 	// MCU_DATA_0
    {0x098C, 0xAB23,}, 	// MCU_ADDRESS [HG_LL_APTHRESH1]
    {0x0990, 0x0004,}, 	// MCU_DATA_0
    {0x098C, 0xAB27,}, 	// MCU_ADDRESS [HG_LL_APTHRESH2]
    {0x0990, 0x0009,}, 	// MCU_DATA_0
    {0x098C, 0x2B28,}, 	// MCU_ADDRESS [HG_LL_BRIGHTNESSSTART]
    {0x0990, 0x0BB8,}, 	// MCU_DATA_0
    {0x098C, 0x2B2A,}, 	// MCU_ADDRESS [HG_LL_BRIGHTNESSSTOP]
    {0x0990, 0x2968,}, 	// MCU_DATA_0
    {0x098C, 0xAB2C,}, 	// MCU_ADDRESS [HG_NR_START_R]
    {0x0990, 0x00FF,}, 	// MCU_DATA_0
    {0x098C, 0xAB30,}, 	// MCU_ADDRESS [HG_NR_STOP_R]
    {0x0990, 0x00FF,}, 	// MCU_DATA_0
    {0x098C, 0xAB2D,}, 	// MCU_ADDRESS [HG_NR_START_G]
    {0x0990, 0x00FF,}, 	// MCU_DATA_0
    {0x098C, 0xAB31,}, 	// MCU_ADDRESS [HG_NR_STOP_G]
    {0x0990, 0x00FF,}, 	// MCU_DATA_0
    {0x098C, 0xAB2E,}, 	// MCU_ADDRESS [HG_NR_START_B]
    {0x0990, 0x00FF,}, 	// MCU_DATA_0
    {0x098C, 0xAB32,}, 	// MCU_ADDRESS [HG_NR_STOP_B]
    {0x0990, 0x00FF,}, 	// MCU_DATA_0
    {0x098C, 0xAB2F,}, 	// MCU_ADDRESS [HG_NR_START_OL]
    {0x0990, 0x000A,}, 	// MCU_DATA_0
    {0x098C, 0xAB33,}, 	// MCU_ADDRESS [HG_NR_STOP_OL]
    {0x0990, 0x0006,}, 	// MCU_DATA_0
    {0x098C, 0xAB34,}, 	// MCU_ADDRESS [HG_NR_GAINSTART]
    {0x0990, 0x0020,}, 	// MCU_DATA_0
    {0x098C, 0xAB35,}, 	// MCU_ADDRESS [HG_NR_GAINSTOP]
    {0x0990, 0x0091,}, 	// MCU_DATA_0
    {0x098C, 0xA765,}, 	// MCU_ADDRESS [MODE_COMMONMODESETTINGS_FILTER_MODE]
    {0x0990, 0x0006,}, 	// MCU_DATA_0
    {0x098C, 0xAB37,}, 	// MCU_ADDRESS [HG_GAMMA_MORPH_CTRL]
    {0x0990, 0x0003,}, 	// MCU_DATA_0
    {0x098C, 0x2B38,}, 	// MCU_ADDRESS [HG_GAMMASTARTMORPH]
    {0x0990, 0x2968,}, 	// MCU_DATA_0
    {0x098C, 0x2B3A,}, 	// MCU_ADDRESS [HG_GAMMASTOPMORPH]
    {0x0990, 0x2D50,}, 	// MCU_DATA_0
    {0x098C, 0x2B62,}, 	// MCU_ADDRESS [HG_FTB_START_BM]
    {0x0990, 0xFFFE,}, 	// MCU_DATA_0
    {0x098C, 0x2B64,}, 	// MCU_ADDRESS [HG_FTB_STOP_BM]
    {0x0990, 0xFFFF,}, 	// MCU_DATA_0
    {0x098C, 0xAB4F,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_0]
    {0x0990, 0x0000,}, 	// MCU_DATA_0
    {0x098C, 0xAB50,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_1]
    {0x0990, 0x0013,}, 	// MCU_DATA_0
    {0x098C, 0xAB51,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_2]
    {0x0990, 0x0027,}, 	// MCU_DATA_0
    {0x098C, 0xAB52,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_3]
    {0x0990, 0x0043,}, 	// MCU_DATA_0
    {0x098C, 0xAB53,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_4]
    {0x0990, 0x0068,}, 	// MCU_DATA_0
    {0x098C, 0xAB54,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_5]
    {0x0990, 0x0081,}, 	// MCU_DATA_0
    {0x098C, 0xAB55,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_6]
    {0x0990, 0x0093,}, 	// MCU_DATA_0
    {0x098C, 0xAB56,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_7]
    {0x0990, 0x00A3,}, 	// MCU_DATA_0
    {0x098C, 0xAB57,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_8]
    {0x0990, 0x00B0,}, 	// MCU_DATA_0
    {0x098C, 0xAB58,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_9]
    {0x0990, 0x00BC,}, 	// MCU_DATA_0
    {0x098C, 0xAB59,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_10]
    {0x0990, 0x00C7,}, 	// MCU_DATA_0
    {0x098C, 0xAB5A,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_11]
    {0x0990, 0x00D1,}, 	// MCU_DATA_0
    {0x098C, 0xAB5B,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_12]
    {0x0990, 0x00DA,}, 	// MCU_DATA_0
    {0x098C, 0xAB5C,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_13]
    {0x0990, 0x00E2,}, 	// MCU_DATA_0
    {0x098C, 0xAB5D,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_14]
    {0x0990, 0x00E9,}, 	// MCU_DATA_0
    {0x098C, 0xAB5E,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_15]
    {0x0990, 0x00EF,}, 	// MCU_DATA_0
    {0x098C, 0xAB5F,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_16]
    {0x0990, 0x00F4,}, 	// MCU_DATA_0
    {0x098C, 0xAB60,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_17]
    {0x0990, 0x00FA,}, 	// MCU_DATA_0
    {0x098C, 0xAB61,}, 	// MCU_ADDRESS [HG_GAMMA_TABLE_B_18]
    {0x0990, 0x00FF,}, 	// MCU_DATA_0
    {0x098C, 0x2306,}, 	// MCU_ADDRESS [AWB_CCM_L_0]
    {0x0990, 0x01D6,}, 	// MCU_DATA_0
    {0x098C, 0x2308,}, 	// MCU_ADDRESS [AWB_CCM_L_1]
    {0x0990, 0xFF89,}, 	// MCU_DATA_0
    {0x098C, 0x230A,}, 	// MCU_ADDRESS [AWB_CCM_L_2]
    {0x0990, 0xFFA1,}, 	// MCU_DATA_0
    {0x098C, 0x230C,}, 	// MCU_ADDRESS [AWB_CCM_L_3]
    {0x0990, 0xFF73,}, 	// MCU_DATA_0
    {0x098C, 0x230E,}, 	// MCU_ADDRESS [AWB_CCM_L_4]
    {0x0990, 0x019C,}, 	// MCU_DATA_0
    {0x098C, 0x2310,}, 	// MCU_ADDRESS [AWB_CCM_L_5]
    {0x0990, 0xFFF1,}, 	// MCU_DATA_0
    {0x098C, 0x2312,}, 	// MCU_ADDRESS [AWB_CCM_L_6]
    {0x0990, 0xFFB0,}, 	// MCU_DATA_0
    {0x098C, 0x2314,}, 	// MCU_ADDRESS [AWB_CCM_L_7]
    {0x0990, 0xFF2D,}, 	// MCU_DATA_0
    {0x098C, 0x2316,}, 	// MCU_ADDRESS [AWB_CCM_L_8]
    {0x0990, 0x0223,}, 	// MCU_DATA_0
    {0x098C, 0x2318,}, 	// MCU_ADDRESS [AWB_CCM_L_9]
    {0x0990, 0x001C,}, 	// MCU_DATA_0
    {0x098C, 0x231A,}, 	// MCU_ADDRESS [AWB_CCM_L_10]
    {0x0990, 0x0048,}, 	// MCU_DATA_0
    {0x098C, 0x2318,}, 	// MCU_ADDRESS [AWB_CCM_L_9]
    {0x0990, 0x001C,}, 	// MCU_DATA_0
    {0x098C, 0x231A,}, 	// MCU_ADDRESS [AWB_CCM_L_10]
    {0x0990, 0x0038,}, 	// MCU_DATA_0
    {0x098C, 0x2318,}, 	// MCU_ADDRESS [AWB_CCM_L_9]
    {0x0990, 0x001E,}, 	// MCU_DATA_0
    {0x098C, 0x231A,}, 	// MCU_ADDRESS [AWB_CCM_L_10]
    {0x0990, 0x0038,}, 	// MCU_DATA_0
    {0x098C, 0x2318,}, 	// MCU_ADDRESS [AWB_CCM_L_9]
    {0x0990, 0x0022,}, 	// MCU_DATA_0
    {0x098C, 0x231A,}, 	// MCU_ADDRESS [AWB_CCM_L_10]
    {0x0990, 0x0038,}, 	// MCU_DATA_0
    {0x098C, 0x2318,}, 	// MCU_ADDRESS [AWB_CCM_L_9]
    {0x0990, 0x002C,}, 	// MCU_DATA_0
    {0x098C, 0x231A,}, 	// MCU_ADDRESS [AWB_CCM_L_10]
    {0x0990, 0x0038,}, 	// MCU_DATA_0
    {0x098C, 0x2318,}, 	// MCU_ADDRESS [AWB_CCM_L_9]
    {0x0990, 0x0024,}, 	// MCU_DATA_0
    {0x098C, 0x231A,}, 	// MCU_ADDRESS [AWB_CCM_L_10]
    {0x0990, 0x0038,}, 	// MCU_DATA_0
    {0x098C, 0x231C,}, 	// MCU_ADDRESS [AWB_CCM_RL_0]
    {0x0990, 0xFFCD,}, 	// MCU_DATA_0
    {0x098C, 0x231E,}, 	// MCU_ADDRESS [AWB_CCM_RL_1]
    {0x0990, 0x0023,}, 	// MCU_DATA_0
    {0x098C, 0x2320,}, 	// MCU_ADDRESS [AWB_CCM_RL_2]
    {0x0990, 0x0010,}, 	// MCU_DATA_0
    {0x098C, 0x2322,}, 	// MCU_ADDRESS [AWB_CCM_RL_3]
    {0x0990, 0x0026,}, 	// MCU_DATA_0
    {0x098C, 0x2324,}, 	// MCU_ADDRESS [AWB_CCM_RL_4]
    {0x0990, 0xFFE9,}, 	// MCU_DATA_0
    {0x098C, 0x2326,}, 	// MCU_ADDRESS [AWB_CCM_RL_5]
    {0x0990, 0xFFF1,}, 	// MCU_DATA_0
    {0x098C, 0x2328,}, 	// MCU_ADDRESS [AWB_CCM_RL_6]
    {0x0990, 0x003A,}, 	// MCU_DATA_0
    {0x098C, 0x232A,}, 	// MCU_ADDRESS [AWB_CCM_RL_7]
    {0x0990, 0x005D,}, 	// MCU_DATA_0
    {0x098C, 0x232C,}, 	// MCU_ADDRESS [AWB_CCM_RL_8]
    {0x0990, 0xFF69,}, 	// MCU_DATA_0
    {0x098C, 0x232E,}, 	// MCU_ADDRESS [AWB_CCM_RL_9]
    {0x0990, 0x000C,}, 	// MCU_DATA_0
    {0x098C, 0x2330,}, 	// MCU_ADDRESS [AWB_CCM_RL_10]
    {0x0990, 0xFFE4,}, 	// MCU_DATA_0
    {0x098C, 0x232E,}, 	// MCU_ADDRESS [AWB_CCM_RL_9]
    {0x0990, 0x000C,}, 	// MCU_DATA_0
    {0x098C, 0x2330,}, 	// MCU_ADDRESS [AWB_CCM_RL_10]
    {0x0990, 0xFFF4,}, 	// MCU_DATA_0
    {0x098C, 0x232E,}, 	// MCU_ADDRESS [AWB_CCM_RL_9]
    {0x0990, 0x000A,}, 	// MCU_DATA_0
    {0x098C, 0x2330,}, 	// MCU_ADDRESS [AWB_CCM_RL_10]
    {0x0990, 0xFFF4,}, 	// MCU_DATA_0
    {0x098C, 0x232E,}, 	// MCU_ADDRESS [AWB_CCM_RL_9]
    {0x0990, 0x0006,}, 	// MCU_DATA_0
    {0x098C, 0x2330,}, 	// MCU_ADDRESS [AWB_CCM_RL_10]
    {0x0990, 0xFFF4,}, 	// MCU_DATA_0
    {0x098C, 0x232E,}, 	// MCU_ADDRESS [AWB_CCM_RL_9]
    {0x0990, 0xFFFC,}, 	// MCU_DATA_0
    {0x098C, 0x2330,}, 	// MCU_ADDRESS [AWB_CCM_RL_10]
    {0x0990, 0xFFF4,}, 	// MCU_DATA_0
    {0x098C, 0x232E,}, 	// MCU_ADDRESS [AWB_CCM_RL_9]
    {0x0990, 0x0004,}, 	// MCU_DATA_0
    {0x098C, 0x2330,}, 	// MCU_ADDRESS [AWB_CCM_RL_10]
    {0x0990, 0xFFF4,}, 	// MCU_DATA_0
    {0x098C, 0x0415,}, 	// MCU_ADDRESS
    {0x0990, 0xF601,},
    {0x0992, 0x42C1,},
    {0x0994, 0x0326,},
    {0x0996, 0x11F6,},
    {0x0998, 0x0143,},
    {0x099A, 0xC104,},
    {0x099C, 0x260A,},
    {0x099E, 0xCC04,},
    {0x098C, 0x0425,},     // MCU_ADDRESS
    {0x0990, 0x33BD,},
    {0x0992, 0xA362,},
    {0x0994, 0xBD04,},
    {0x0996, 0x3339,},
    {0x0998, 0xC6FF,},
    {0x099A, 0xF701,},
    {0x099C, 0x6439,},
    {0x099E, 0xFE01,},
    {0x098C, 0x0435,},     // MCU_ADDRESS
    {0x0990, 0x6918,},
    {0x0992, 0xCE03,},
    {0x0994, 0x25CC,},
    {0x0996, 0x0013,},
    {0x0998, 0xBDC2,},
    {0x099A, 0xB8CC,},
    {0x099C, 0x0489,},
    {0x099E, 0xFD03,},
    {0x098C, 0x0445,},     // MCU_ADDRESS
    {0x0990, 0x27CC,},
    {0x0992, 0x0325,},
    {0x0994, 0xFD01,},
    {0x0996, 0x69FE,},
    {0x0998, 0x02BD,},
    {0x099A, 0x18CE,},
    {0x099C, 0x0339,},
    {0x099E, 0xCC00,},
    {0x098C, 0x0455,},     // MCU_ADDRESS
    {0x0990, 0x11BD,},
    {0x0992, 0xC2B8,},
    {0x0994, 0xCC04,},
    {0x0996, 0xC8FD,},
    {0x0998, 0x0347,},
    {0x099A, 0xCC03,},
    {0x099C, 0x39FD,},
    {0x099E, 0x02BD,},
    {0x098C, 0x0465,},     // MCU_ADDRESS
    {0x0990, 0xDE00,},
    {0x0992, 0x18CE,},
    {0x0994, 0x00C2,},
    {0x0996, 0xCC00,},
    {0x0998, 0x37BD,},
    {0x099A, 0xC2B8,},
    {0x099C, 0xCC04,},
    {0x099E, 0xEFDD,},
    {0x098C, 0x0475,},     // MCU_ADDRESS
    {0x0990, 0xE6CC,},
    {0x0992, 0x00C2,},
    {0x0994, 0xDD00,},
    {0x0996, 0xC601,},
    {0x0998, 0xF701,},
    {0x099A, 0x64C6,},
    {0x099C, 0x03F7,},
    {0x099E, 0x0165,},
    {0x098C, 0x0485,},     // MCU_ADDRESS
    {0x0990, 0x7F01,},
    {0x0992, 0x6639,},
    {0x0994, 0x3C3C,},
    {0x0996, 0x3C34,},
    {0x0998, 0xCC32,},
    {0x099A, 0x3EBD,},
    {0x099C, 0xA558,},
    {0x099E, 0x30ED,},
    {0x098C, 0x0495,},     // MCU_ADDRESS
    {0x0990, 0x04BD,},
    {0x0992, 0xB2D7,},
    {0x0994, 0x30E7,},
    {0x0996, 0x06CC,},
    {0x0998, 0x323E,},
    {0x099A, 0xED00,},
    {0x099C, 0xEC04,},
    {0x099E, 0xBDA5,},
    {0x098C, 0x04A5,},     // MCU_ADDRESS
    {0x0990, 0x44CC,},
    {0x0992, 0x3244,},
    {0x0994, 0xBDA5,},
    {0x0996, 0x585F,},
    {0x0998, 0x30ED,},
    {0x099A, 0x02CC,},
    {0x099C, 0x3244,},
    {0x099E, 0xED00,},
    {0x098C, 0x04B5,},     // MCU_ADDRESS
    {0x0990, 0xF601,},
    {0x0992, 0xD54F,},
    {0x0994, 0xEA03,},
    {0x0996, 0xAA02,},
    {0x0998, 0xBDA5,},
    {0x099A, 0x4430,},
    {0x099C, 0xE606,},
    {0x099E, 0x3838,},
    {0x098C, 0x04C5,},     // MCU_ADDRESS
    {0x0990, 0x3831,},
    {0x0992, 0x39BD,},
    {0x0994, 0xD661,},
    {0x0996, 0xF602,},
    {0x0998, 0xF4C1,},
    {0x099A, 0x0126,},
    {0x099C, 0x0BFE,},
    {0x099E, 0x02BD,},
    {0x098C, 0x04D5,},     // MCU_ADDRESS
    {0x0990, 0xEE10,},
    {0x0992, 0xFC02,},
    {0x0994, 0xF5AD,},
    {0x0996, 0x0039,},
    {0x0998, 0xF602,},
    {0x099A, 0xF4C1,},
    {0x099C, 0x0226,},
    {0x099E, 0x0AFE,},
    {0x098C, 0x04E5,},     // MCU_ADDRESS
    {0x0990, 0x02BD,},
    {0x0992, 0xEE10,},
    {0x0994, 0xFC02,},
    {0x0996, 0xF7AD,},
    {0x0998, 0x0039,},
    {0x099A, 0x3CBD,},
    {0x099C, 0xB059,},
    {0x099E, 0xCC00,},
    {0x098C, 0x04F5,},     // MCU_ADDRESS
    {0x0990, 0x28BD,},
    {0x0992, 0xA558,},
    {0x0994, 0x8300,},
    {0x0996, 0x0027,},
    {0x0998, 0x0BCC,},
    {0x099A, 0x0026,},
    {0x099C, 0x30ED,},
    {0x099E, 0x00C6,},
    {0x098C, 0x0505,},     // MCU_ADDRESS
    {0x0990, 0x03BD,},
    {0x0992, 0xA544,},
    {0x0994, 0x3839,},
    {0x098C, 0x2006,},     // MCU_ADDRESS [MON_ARG1]
    {0x0990, 0x0415,},     // MCU_DATA_0
    {0x098C, 0xA005,},     // MCU_ADDRESS [MON_CMD]
    {0x0990, 0x0001,},     // MCU_DATA_0    

    {0x098C, 0x2755,},     // MCU_ADDRESS [MODE_OUTPUT_FORMAT_A]
    {0x0990, 0x0002,},     // MCU_DATA_0
    {0x098C, 0x2757,},     // MCU_ADDRESS [MODE_OUTPUT_FORMAT_B]
    {0x0990, 0x0002,},     // MCU_DATA_0

    #ifdef CONFIG_CAMERA_MT9D115_MIRROR_ZTE
    {0x098C, 0x2717,},
    {0x0990, 0x046D,},
    {0x098C, 0x272D,},
    {0x0990, 0x0025,},
    {0x098C, 0xA103,},
    {0x0990, 0x0006,},
    #endif 
    #ifdef CONFIG_CAMERA_MT9D115_FLIP_ZTE 
    {0x098C, 0x2717,},
    {0x0990, 0x046E,},
    {0x098C, 0x272D,},
    {0x0990, 0x0026,},
    {0x098C, 0xA103,},
    {0x0990, 0x0006,},
    #endif
    #ifdef CONFIG_CAMERA_MT9D115_FLIP_MIRROR_ZTE
    {0x098C, 0x2717,},
    {0x0990, 0x046F,},
    {0x098C, 0x272D,},
    {0x0990, 0x0027,},
    {0x098C, 0xA103,},
    {0x0990, 0x0006,},
    #endif    
	
};

static struct msm_camera_i2c_reg_conf mt9d115_saturation[][2] = {
    {{0x098C, 0xAB20,},{0x0990, 0x003e,}},        // saturation -2
    {{0x098C, 0xAB20,},{0x0990, 0x004e,}},        // saturation -1
    {{0x098C, 0xAB20,},{0x0990, 0x005e,}},        // saturation 0
    {{0x098C, 0xAB20,},{0x0990, 0x006e,}},        // saturation +1
    {{0x098C, 0xAB20,},{0x0990, 0x007e,}}         // saturation +2
};


static struct v4l2_subdev_info mt9d115_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array mt9d115_init_conf[] = {
	{&reset_sensor_setting1[0],
	ARRAY_SIZE(reset_sensor_setting1), 2, MSM_CAMERA_I2C_WORD_DATA},
	{&reset_sensor_setting2[0],
	ARRAY_SIZE(reset_sensor_setting2), 10, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9d115_pll_settings[0],
	ARRAY_SIZE(mt9d115_pll_settings), 10, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9d115_settings[0],
	ARRAY_SIZE(mt9d115_settings), 50, MSM_CAMERA_I2C_WORD_DATA},	
	{&mt9d115_recommend_settings[0],
	ARRAY_SIZE(mt9d115_recommend_settings), 10, MSM_CAMERA_I2C_WORD_DATA},
	{&mt9d115_init_fine_tune[0],
	ARRAY_SIZE(mt9d115_init_fine_tune), 50, MSM_CAMERA_I2C_WORD_DATA},	
	{&software_standby_disable[0],
	ARRAY_SIZE(software_standby_disable), 10, MSM_CAMERA_I2C_WORD_DATA},	
	{&refresh_setting1[0],
	ARRAY_SIZE(refresh_setting1), 10, MSM_CAMERA_I2C_WORD_DATA},	
#if 0		
	{&preview_mode_setting[0],
	ARRAY_SIZE(preview_mode_setting), 10, MSM_CAMERA_I2C_WORD_DATA},	
#endif	
};

static struct msm_camera_i2c_conf_array mt9d115_confs[] = {
	{&capture_mode_setting[0],
	ARRAY_SIZE(capture_mode_setting), 0, MSM_CAMERA_I2C_WORD_DATA},	   
	{&preview_mode_setting[0],
	ARRAY_SIZE(preview_mode_setting), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9d115_saturation_confs[][2] = {
	{{mt9d115_saturation[0],
		ARRAY_SIZE(mt9d115_saturation[0]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9d115_refresh,
		ARRAY_SIZE(mt9d115_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9d115_saturation[1],
		ARRAY_SIZE(mt9d115_saturation[1]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9d115_refresh,
		ARRAY_SIZE(mt9d115_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9d115_saturation[2],
		ARRAY_SIZE(mt9d115_saturation[2]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9d115_refresh,
		ARRAY_SIZE(mt9d115_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9d115_saturation[3],
		ARRAY_SIZE(mt9d115_saturation[3]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9d115_refresh,
		ARRAY_SIZE(mt9d115_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9d115_saturation[4],
		ARRAY_SIZE(mt9d115_saturation[4]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9d115_refresh,
		ARRAY_SIZE(mt9d115_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9d115_saturation[5],
		ARRAY_SIZE(mt9d115_saturation[5]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9d115_refresh,
		ARRAY_SIZE(mt9d115_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9d115_saturation[6],
		ARRAY_SIZE(mt9d115_saturation[6]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9d115_refresh,
		ARRAY_SIZE(mt9d115_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9d115_saturation[7],
		ARRAY_SIZE(mt9d115_saturation[7]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9d115_refresh,
		ARRAY_SIZE(mt9d115_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9d115_saturation[8],
		ARRAY_SIZE(mt9d115_saturation[8]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9d115_refresh,
		ARRAY_SIZE(mt9d115_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9d115_saturation[9],
		ARRAY_SIZE(mt9d115_saturation[9]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9d115_refresh,
		ARRAY_SIZE(mt9d115_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9d115_saturation[10],
		ARRAY_SIZE(mt9d115_saturation[10]),
		0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9d115_refresh,
		ARRAY_SIZE(mt9d115_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
};

static int mt9d115_saturation_enum_map[] = {
	MSM_V4L2_SATURATION_L0,
	MSM_V4L2_SATURATION_L1,
	MSM_V4L2_SATURATION_L2,
	MSM_V4L2_SATURATION_L3,
	MSM_V4L2_SATURATION_L4,
	MSM_V4L2_SATURATION_L5,
	MSM_V4L2_SATURATION_L6,
	MSM_V4L2_SATURATION_L7,
	MSM_V4L2_SATURATION_L8,
	MSM_V4L2_SATURATION_L9,
	MSM_V4L2_SATURATION_L10,
};

static struct msm_camera_i2c_enum_conf_array mt9d115_saturation_enum_confs = {
	.conf = &mt9d115_saturation_confs[0][0],
	.conf_enum = mt9d115_saturation_enum_map,
	.num_enum = ARRAY_SIZE(mt9d115_saturation_enum_map),
	.num_index = ARRAY_SIZE(mt9d115_saturation_confs),
	.num_conf = ARRAY_SIZE(mt9d115_saturation_confs[0]),
	.data_type = MSM_CAMERA_I2C_WORD_DATA,
};

struct msm_sensor_v4l2_ctrl_info_t mt9d115_v4l2_ctrl_info[] = {
	{
		.ctrl_id = V4L2_CID_SATURATION,
		.min = MSM_V4L2_SATURATION_L0,
		//.max = MSM_V4L2_SATURATION_L10,
		.max = MSM_V4L2_SATURATION_L10,
		.step = 1,
		.enum_cfg_settings = &mt9d115_saturation_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},
};

static struct msm_sensor_output_info_t mt9d115_dimensions[] = {
	{
		.x_output = 0x640,  //1600
		.y_output = 0x4b0,  //1200
		.line_length_pclk = 0x640,
		.frame_length_lines = 0x4b0,
		.vt_pixel_clk = 48000000,//96000000,
		.op_pixel_clk = 128000000,//96000000,
		.binning_factor = 1,
	},	
	{
		.x_output = 0x320,  //800
		.y_output = 0x258,  //600
		.line_length_pclk = 0x320,
		.frame_length_lines = 0x258,
		.vt_pixel_clk = 48000000,//96000000,
		.op_pixel_clk = 128000000,//96000000,
		.binning_factor = 1,
	},
};

/*static struct msm_camera_csid_vc_cfg mt9d115_cid_cfg[] = {	
	{0, CSI_YUV422_8, CSI_DECODE_8BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params mt9d115_csi_params = {
	.csid_params = {
		.lane_assign = 0xe4,
		.lane_cnt = 1,
		.lut_params = {
			.num_cid = 2,
			.vc_cfg = mt9d115_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 1,
		.settle_cnt = 0x14,
	},
};*/
static struct msm_camera_csi_params mt9d115_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 20,//0X5,0X24,
};

static struct msm_camera_csi_params *mt9d115_csi_params_array[] = {
	&mt9d115_csi_params,
	&mt9d115_csi_params,
};

static struct msm_sensor_output_reg_addr_t mt9d115_reg_addr = {
	.x_output = 0x2703,
	.y_output = 0x2705,
	.line_length_pclk = 0x2703,
	.frame_length_lines = 0x2705,
};

static struct msm_sensor_id_info_t mt9d115_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x2580,
};


static const struct i2c_device_id mt9d115_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&mt9d115_s_ctrl},
	{ }
};

static struct i2c_driver mt9d115_i2c_driver = {
	.id_table = mt9d115_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client mt9d115_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};
static int __init msm_sensor_init_module(void)
{
	//return platform_driver_register(&mt9d115_driver);  
	return i2c_add_driver(&mt9d115_i2c_driver);
}

static struct v4l2_subdev_core_ops mt9d115_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops mt9d115_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops mt9d115_subdev_ops = {
	.core = &mt9d115_subdev_core_ops,
	.video  = &mt9d115_subdev_video_ops,
};

int32_t mt9d115_sensor_setting1(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;
	static int flag_init;
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		printk("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		flag_init =0;//msm_sensor_write_init_settings(s_ctrl);		
		csi_config = 0;
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		printk("PERIODIC : %d\n", res);
		 printk("%s:sensor_name = %s, res = %d\n",__func__, s_ctrl->sensordata->sensor_name, res);
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
		if (!flag_init) {
			msleep(30);
			printk("init----- config is done\n");
			msm_sensor_write_init_settings(s_ctrl);
			msleep(30);
			flag_init = 1;
		}
		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		msleep(50);
	}
	return rc;
}
static struct msm_sensor_fn_t mt9d115_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream, 
	.sensor_csi_setting = mt9d115_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	//.sensor_open_init = mt9d115_sensor_open_init,
	//.sensor_release = mt9d115_sensor_release,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
};

static struct msm_sensor_reg_t mt9d115_regs = {
	.default_data_type = MSM_CAMERA_I2C_WORD_DATA,
	.start_stream_conf = mt9d115_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(mt9d115_start_settings),
	.stop_stream_conf = mt9d115_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(mt9d115_stop_settings),	
	.init_settings = &mt9d115_init_conf[0],
	.init_size = ARRAY_SIZE(mt9d115_init_conf),
	.mode_settings = &mt9d115_confs[0],
	.output_settings = &mt9d115_dimensions[0],
	.num_conf = ARRAY_SIZE(mt9d115_confs),
};

static struct msm_sensor_ctrl_t mt9d115_s_ctrl = {
	.msm_sensor_reg = &mt9d115_regs,
	.msm_sensor_v4l2_ctrl_info = mt9d115_v4l2_ctrl_info,
	.num_v4l2_ctrl = ARRAY_SIZE(mt9d115_v4l2_ctrl_info),
	.sensor_i2c_client = &mt9d115_sensor_i2c_client,
	.sensor_i2c_addr = 0x78,
	.sensor_output_reg_addr = &mt9d115_reg_addr,
	.sensor_id_info = &mt9d115_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &mt9d115_csi_params_array[0],
	.msm_sensor_mutex = &mt9d115_mut,
	.sensor_i2c_driver = &mt9d115_i2c_driver,
	.sensor_v4l2_subdev_info = mt9d115_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9d115_subdev_info),
	.sensor_v4l2_subdev_ops = &mt9d115_subdev_ops,
	.func_tbl = &mt9d115_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,  
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Aptina 2MP YUV sensor driver");
MODULE_LICENSE("GPL v2");

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


#include <linux/module.h>
#include "msm_sensor.h"
#include "msm.h"
#define SENSOR_NAME "t4k04"
#define PLATFORM_DRIVER_NAME "msm_camera_t4k04"
#define t4k04_obj t4k04_##obj

#ifdef CDBG
#undef CDBG
#endif
#ifdef CDBG_HIGH
#undef CDBG_HIGH
#endif

#define T4K04_DGB

#ifdef T4K04_DGB
#define CDBG(fmt, args...) printk(fmt, ##args)
#define CDBG_HIGH(fmt, args...) printk(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#define CDBG_HIGH(fmt, args...) printk(fmt, ##args)
#endif



DEFINE_MUTEX(t4k04_mut);
static struct msm_sensor_ctrl_t t4k04_s_ctrl;


static struct msm_camera_i2c_reg_conf t4k04_start_settings[] = {
	{0x0100,0x01},//-/-/-/-/-/-/-/MODE_SELECT 

};

static struct msm_camera_i2c_reg_conf t4k04_stop_settings[] = {
	{0x0100,0x00},//-/-/-/-/-/-/-/MODE_SELECT 

};

static struct msm_camera_i2c_reg_conf t4k04_groupon_settings[] = {
	{0x0104,0x01},  //hold
};

static struct msm_camera_i2c_reg_conf t4k04_groupoff_settings[] = {
	{0x0104,0x00},  //normal

};

static struct msm_camera_i2c_reg_conf t4k04_prev_settings[] = {
	    //pv_1640x1232
		{0x0340,0x09},// FR_LENGTH_LINES[15:8]
		{0x0341,0x74},// FR_LENGTH_LINES[7:0]
		{0x0342,0x07},// LINE_LENGTH_PCK[15:8]
		{0x0343,0x70},// LINE_LENGTH_PCK[7:0]
		{0x0344,0x00},// X_ADDR_START[15:8]
		{0x0345,0x00},// X_ADDR_START[7:0]
		{0x0346,0x00},// Y_ADDR_START[15:8]
		{0x0347,0x00},// Y_ADDR_START[7:0]
		{0x034C,0x06},// X_OUTPUT_SIZE[15:8]
		{0x034D,0x68},// X_OUTPUT_SIZE[7:0]
		{0x034E,0x04},// Y_OUTPUT_SIZE[15:8]
		{0x034F,0xd0},// Y_OUTPUT_SIZE[7:0]
		{0x0401,0x02},// -/-/-/-/-/-/SCALING_MODE[1:0]
		{0x0405,0x10},// SCALE_M[7:0]
		{0x0901,0x22},//BINNING_TYPE[7:0] 
};

static struct msm_camera_i2c_reg_conf t4k04_snap_settings[] = {
	  //snap_3280x2464
		{0x0340,0x09},// FR_LENGTH_LINES[15:8]
		{0x0341,0xc0},// FR_LENGTH_LINES[7:0]
		{0x0342,0x0E},// LINE_LENGTH_PCK[15:8]
		{0x0343,0x68},// LINE_LENGTH_PCK[7:0]
		{0x0344,0x00},// X_ADDR_START[15:8]
		{0x0345,0x00},// X_ADDR_START[7:0]
		{0x0346,0x00},// Y_ADDR_START[15:8]
		{0x0347,0x00},// Y_ADDR_START[7:0]
		{0x034C,0x0C},// X_OUTPUT_SIZE[15:8]
		{0x034D,0xD0},// X_OUTPUT_SIZE[7:0]
		{0x034E,0x09},// Y_OUTPUT_SIZE[15:8]
		{0x034F,0xA0},// Y_OUTPUT_SIZE[7:0]
		{0x0401,0x00},// -/-/-/-/-/-/SCALING_MODE[1:0]
		{0x0405,0x10},// SCALE_M[7:0]
		{0x0901,0x11},//BINNING_TYPE[7:0] 
};



static struct msm_camera_i2c_reg_conf t4k04_recommend_settings[] = {
		{0x0101,0x03},//-/-/-/-/-/-/IMAGE_ORIENT[1:0] 
		{0x0103,0x00},//-/-/-/-/-/-/-/SOFTWARE_RESET 
		{0x0104,0x00},//-/-/-/-/-/-/-/GROUP_PARA_HOLD 
		{0x0105,0x00},//-/-/-/-/-/-/-/MSK_CORRUPT_FR 
		{0x0110,0x00},//-/-/-/-/-/CSI_CHAN_IDNTF[2:0] 
		{0x0111,0x02},//-/-/-/-/-/-/CSI_SIG_MODE[1:0] 
		{0x0112,0x0A},//CSI_DATA_FORMAT[15:8] 
		{0x0113,0x0A},//CSI_DATA_FORMAT[7:0] 
		{0x0114,0x01},//-/-/-/-/-/CSI_LANE_MODE[2:0] 
		{0x0115,0x30},//-/-/CSI_10TO8_DT[5:0] 
		{0x0117,0x32},//-/-/CSI_10TO6_DT[5:0] 
		{0x0118,0x33},//-/-/CSI_12TO8_DT[5:0] 
		{0x0202,0x01},//COAR_INTEGR_TIM[15:8] 
		{0x0203,0xb0},//COAR_INTEGR_TIM[7:0] 
		{0x0204,0x00},//ANA_GA_CODE_GL[15:8] 
		{0x0205,0x41},//ANA_GA_CODE_GL[7:0] 
		{0x020E,0x01},//-/-/-/-/-/-/DG_GA_GREENR[9:8] 
		{0x020F,0x00},//DG_GA_GREENR[7:0] 
		{0x0210,0x01},//-/-/-/-/-/-/DG_GA_RED[9:8] 
		{0x0211,0x00},//DG_GA_RED[7:0] 
		{0x0212,0x01},//-/-/-/-/-/-/DG_GA_BLUE[9:8] 
		{0x0213,0x00},//DG_GA_BLUE[7:0] 
		{0x0214,0x01},//-/-/-/-/-/-/DG_GA_GREENB[9:8] 
		{0x0215,0x00},//DG_GA_GREENB[7:0] 
		{0x0301,0x05},//-/-/-/-/VT_PIX_CLK_DIV[3:0] 
		{0x0303,0x03},//-/-/-/-/VT_SYS_CLK_DIV[3:0] 
		{0x0305,0x05},//-/-/-/-/-/PRE_PLL_CLK_DIV[2:0] 
		{0x0306,0x00},//-/-/-/-/-/-/-/PLL_MULTIPLIER[8] 
		{0x0307,0xE6},//PLL_MULTIPLIER[7:0] 
		{0x0309,0x08},//-/-/-/-/OP_PIX_CLK_DIV[3:0] 
		{0x030B,0x01},//-/-/-/-/OP_SYS_CLK_DIV[3:0] 
		{0x0340,0x09},//FR_LENGTH_LINES[15:8] 
		{0x0341,0xC0},//FR_LENGTH_LINES[7:0] 
		{0x0342,0x0E},//LINE_LENGTH_PCK[15:8] 
		{0x0343,0x68},//LINE_LENGTH_PCK[7:0] 
		{0x0344,0x00},//X_ADDR_START[15:8] 
		{0x0345,0x00},//X_ADDR_START[7:0] 
		{0x0346,0x00},//Y_ADDR_START[15:8] 
		{0x0347,0x00},//Y_ADDR_START[7:0] 
		{0x0348,0x0C},//X_ADDR_END[15:8] 
		{0x0349,0xCF},//X_ADDR_END[7:0] 
		{0x034A,0x09},//Y_ADDR_END[15:8] 
		{0x034B,0x9F},//Y_ADDR_END[7:0] 
		{0x034C,0x0C},//X_OUTPUT_SIZE[15:8] 
		{0x034D,0xD0},//X_OUTPUT_SIZE[7:0] 
		{0x034E,0x09},//Y_OUTPUT_SIZE[15:8] 
		{0x034F,0xA0},//Y_OUTPUT_SIZE[7:0] 
		{0x0381,0x01},//-/-/-/-/X_EVEN_INC[3:0] 
		{0x0383,0x01},//-/-/-/-/X_ODD_INC[3:0] 
		{0x0401,0x00},//-/-/-/-/-/-/SCALING_MODE[1:0] 
		{0x0403,0x00},//-/-/-/-/-/-/-/SPATIAL_SAMPLING 
		{0x0405,0x10},//SCALE_M[7:0] 
		{0x0408,0x00},//-/-/-/DCROP_XOFS[12:8] 
		{0x0409,0x00},//DCROP_XOFS[7:0] 
		{0x040A,0x00},//-/-/-/-/DCROP_YOFS[11:8] 
		{0x040B,0x00},//DCROP_YOFS[7:0] 
		{0x040C,0x0C},//-/-/-/DCROP_WIDTH[12:8] 
		{0x040D,0xD0},//DCROP_WIDTH[7:0] 
		{0x040E,0x09},//-/-/-/-/DCROP_HIGT[11:8] 
		{0x040F,0xA0},//DCROP_HIGT[7:0] 
		{0x0601,0x00},//TEST_PATT_MODE[7:0] 
		{0x0800,0x80},//TCLK_POST[7:3]/-/-/- 
		{0x0801,0x28},//THS_PREPARE[7:3]/-/-/- 
		{0x0802,0x68},//THS_ZERO[7:3]/-/-/- 
		{0x0803,0x48},//THS_TRAIL[7:3]/-/-/- 
		{0x0804,0x40},//TCLK_TRAIL[7:3]/-/-/- 
		{0x0805,0x28},//TCLK_PREPARE[7:3]/-/-/- 
		{0x0806,0xF8},//TCLK_ZERO[7:3]/-/-/- 
		{0x0807,0x38},//TLPX[7:3]/-/-/- 
		{0x0808,0x01},//-/-/-/-/-/-/DPHY_CTRL[1:0] 
		{0x0820,0x0C},//MSB_LBRATE[31:24] 
		{0x0821,0x60},//MSB_LBRATE[23:16] 
		{0x0900,0x01},//-/-/-/-/-/-/-/BINNING_MODE 
		{0x0901,0x11},//BINNING_TYPE[7:0] 
		{0x0902,0x00},//-/-/-/-/-/-/BINNING_WEIGHTING[1:0] 
		{0x0B05,0x00},//-/-/-/-/-/-/-/MAP_DEF_EN 
		{0x0B06,0x01},//-/-/-/-/-/-/-/SGL_DEF_EN 
		{0x0B07,0x40},//-/SGL_DEF_W[6:0] 
		{0x0B0A,0x00},//-/-/-/-/-/-/-/CONB_CPLT_SGL_DEF_EN 
		{0x0B0B,0x40},//-/CONB_CPLT_SGL_DEF_W[6:0] 
		{0x3000,0x00},//-/-/LB_LANE_SELA[1:0]/LBTEST_CLR/LB_TEST_EN/LBTEST_SELB/LB_MODE 
		{0x3004,0x00},//LANE_OFF_MN/DPCM6EMB/EMB_OUT_SW/ESC_AUTO_OFF/-/-/CLKULPS/ESCREQ 
		{0x3005,0x78},//ESCDATA[7:0] 
		{0x3006,0x42},//PISO_STP_X/MIPI_CLK_MODE/-/-/-/-/-/- 
		{0x3007,0x30},//HS_SR_CNT[1:0]/LP_SR_CNT[1:0]/REG18_CNT[3:0] 
		{0x3008,0xFF},//LANE_OFF[7:0] 
		{0x3009,0x44},//-/PHASE_ADJ_A[2:0]/-/-/-/- 
		{0x300A,0x60},//-/LVDS_CKA_DELAY[2:0]/-/-/-/- 
		{0x300B,0x00},//-/LVDS_D1_DELAY[2:0]/-/LVDS_D2_DELAY[2:0] 
		{0x300C,0x00},//-/LVDS_D3_DELAY[2:0]/-/LVDS_D4_DELAY[2:0] 
		{0x300F,0x00},//-/-/-/-/-/-/EN_PHASE_SEL[1:0] 
		{0x3010,0x00},//-/-/-/-/-/-/FIFODLY[9:8] 
		{0x3011,0x00},//FIFODLY[7:0] 
		{0x3012,0x01},//-/-/SM_PARA_SW[1:0]/-/-/-/LNKBTWK_ON 
		{0x3013,0xA7},//NUMWAKE[7:0] 
		{0x3014,0x40},//-/ZERO_CONV_MR/ZERO_CONV_MD/-/SM8SM[3:0] 
		{0x3015,0x00},//T_VALUE1[7:0] 
		{0x3016,0x00},//T_VALUE2[7:0] 
		{0x3017,0x00},//T_VALUE3[7:0] 
		{0x3018,0x00},//T_VALUE4[7:0] 
		{0x301D,0x01},//MIPI_FS_CD[3:0]/MIPI_FE_CD[3:0] 
		{0x3050,0x25},//-/AD_CNTL[2:0]/-/ST_CNTL[2:0] 
		{0x3051,0x05},//-/-/-/-/-/BST_CNTL[2:0] 
		{0x3060,0x00},//-/-/-/-/-/H_CROP[2:0] 
		{0x3061,0x40},//-/HCRP_AUTO/-/HGAIN2/-/-/HFCORROFF/EQ_MONI 
		{0x3062,0x01},//-/-/-/-/-/-/-/HLNRBLADJ_HOLD 
		{0x3070,0x00},//-/-/-/-/-/MONI_MODE[2:0] 
		{0x30A2,0xF8},//DCLK_DRVUP/DOUT1_7_DRVUP/SDA_DRVUP/FLASH_DRVUP/DOUT0_DRVUP/-/VLAT_
		{0x30A3,0xE8},//PARA_HZ/DCLK_POL/GLB_HZ/-/AF_HZ/-/-/- 
		{0x30A4,0x80},//WAIT_TIME_SEL[1:0]/-/-/VTCK_SEL[1:0]/OPCK_SEL[1:0] 
		{0x30A5,0x00},//SLEEP_SW/VCO_STP_SW/PHY_PWRON_A/-/SLEEP_MN/VCO_STP_MN/PHY_PWRON_MN
		{0x30A6,0x03},//PLL_SNR_CNTL[1:0]/PLL_SYS_CNTL[1:0]/-/-/VCO_EN/DIVRSTX 
		{0x30A7,0x00},//AUTO_IR_SEL/-/ICP_SEL[1:0]/LPFR_SEL[1:0]/-/- 
		{0x30A8,0x21},//PCMODE/-/ICP_PCH/ICP_NCH/-/-/-/VCO_TESTSEL 
		{0x30A9,0x00},//-/-/AMON0_SEL[1:0]/-/REGVD_SEL/PLLEV_EN/PLLEV_SEL 
		{0x30AA,0xCC},//VOUT_SWSB[3:0]/VOUT_SEL[3:0] 
		{0x30AB,0x13},//-/-/CL_SEL[1:0]/-/-/BIAS_SEL/CAMP_EN 
		{0x30AC,0x18},//MHZ_EXTCLK_TB[15:8] 
		{0x30AD,0x00},//MHZ_EXTCLK_TB[7:0] 
		{0x30B1,0x00},//-/-/-/PARA_SW[4:0] 
		{0x30B2,0x01},//-/-/-/-/-/-/-/LSTOP 
		{0x3200,0x10},//MKCP_MSK[1:0]/-/MSK_OFF/VCO_CONV[2:0]/- 
		{0x3202,0x04},//ES_MARGIN[7:0] 
		{0x3205,0x00},//FLS_ESMODE/-/-/-/TEST_MTC_VGSW/TEST_MTC_RG_VG/-/- 
		{0x3206,0x05},//-/KUMA_SW/KURI_SET/KURI_FCSET/ESYNC_SW/VSYNC_PH/HSYNC_PH/GLBRST_SE
		{0x3207,0x00},//SP_STEP1_DLYSEL[1:0]/SP_STEP1_DLYADJ[5:0] 
		{0x3208,0x00},//-/-/-/-/-/-/AF_REQ/GLBRST_REQ 
		{0x3209,0x00},//-/-/-/-/-/-/-/LONG_EXP 
		{0x320A,0x00},//-/-/-/AF_MODE/GRST_QUICK/LINE_START_SEL/GLBRST_MODE[1:0] 
		{0x320B,0x60},//GLB_SENTG_W[5:0]/-/GRST_RDY_W[8] 
		{0x320C,0x33},//GRST_RDY_W[7:0] 
		{0x320D,0x20},//DRVR_STB_S[15:8] 
		{0x320E,0x00},//DRVR_STB_S[7:0] 
		{0x320F,0x08},//FLSH_STB_S[15:8] 
		{0x3210,0x00},//FLSH_STB_S[7:0] 
		{0x3211,0x20},//EXPO_TIM_W[15:8] 
		{0x3212,0x00},//EXPO_TIM_W[7:0] 
		{0x3213,0x00},//-/-/-/-/DRVR_STB_W[11:8] 
		{0x3214,0x32},//DRVR_STB_W[7:0] 
		{0x3215,0x30},//FLSH_STB_W[15:8] 
		{0x3216,0x00},//FLSH_STB_W[7:0] 
		{0x3217,0x00},//AF_STB_W[15:8] 
		{0x3218,0x05},//AF_STB_W[7:0] 
		{0x3219,0x00},//-/-/AF_STB_N[5:0] 
		{0x321A,0x00},//-/-/-/-/FLSH_LINE[11:8] 
		{0x321B,0x00},//FLSH_LINE[7:0] 
		{0x321C,0x00},//MANU_MHZ_SPCK[7:0] 
		{0x321D,0x00},//MANU_MHZ/-/SPCKSW_VEN/SPCKSW_HEN/-/-/-/- 
		{0x321E,0x00},//-/HC_PRESET[14:8] 
		{0x321F,0x00},//HC_PRESET[7:0] 
		{0x3220,0x00},//VC_PRESET[15:8] 
		{0x3221,0x00},//VC_PRESET[7:0] 
		{0x3222,0x02},//TPG_HRST_POS[7:0] 
		{0x3223,0x04},//TPG_VRST_POS[7:0] 
		{0x3225,0x12},//PP_VCNT_ADJ[7:0] 
		{0x3226,0x54},//PP_HCNT_ADJ[7:0] 
		{0x3227,0x04},//V_YUKO_START[7:0] 
		{0x3228,0x07},//H_YUKO_START[7:0] 
		{0x3229,0x06},//H_OB_START[7:0] 
		{0x322A,0x02},//PP_VBLK_START[7:0] 
		{0x322B,0x00},//PP_VBLK_WIDTH[7:0] 
		{0x322C,0x1E},//PP_HBLK_START[7:0] 
		{0x322D,0x00},//PP_HBLK_WIDTH[7:0] 
		{0x322E,0x00},//-/-/-/TEST_TG_VGSW/VOB_DISP/HOB_DISP/-/- 
		{0x322F,0x04},//SBNRY_V_START[7:0] 
		{0x3230,0x05},//SBNRY_H_START[7:0] 
		{0x3231,0x04},//SBNRY_VBLK_START[7:0] 
		{0x3232,0x00},//SBNRY_VBLK_WIDTH[7:0] 
		{0x3233,0x05},//SBNRY_HBLK_START[7:0] 
		{0x3234,0x00},//SBNRY_HBLK_WIDTH[7:0] 
		{0x3235,0x11},//HLNR_HBLK_START[7:0] 
		{0x3236,0x00},//HLNR_HBLK_WIDTH[7:0] 
		{0x3237,0x01},//-/-/-/-/-/SP_COUNT[10:8] 
		{0x3238,0x9E},//SP_COUNT[7:0] 
		{0x3239,0x00},//AG_LIMSW/-/TEST_AGC_VGSW/TEST_SPG_VGSW/TEST_MAG/TEST_AGCONT/-/- 
		{0x323A,0x00},//AG_MIN[7:0] 
		{0x323B,0xFF},//AG_MAX[7:0] 
		{0x323C,0x00},//-/-/-/-/-/HREG_HRST_POS[10:8] 
		{0x323D,0x1C},//HREG_HRST_POS[7:0] 
		{0x323E,0x08},//DIN_SW[3:0]/DOUT_ASW/-/-/- 
		{0x323F,0x08},//TPG_TEST[3:0]/DOUT_BSW/-/-/- 
		{0x3240,0x00},//TPG_NOISE_MP[7:0] 
		{0x3241,0x80},//TPG_OB_LV[7:0] 
		{0x3242,0x30},//RAMP_MODE/-/TPG_BLK_LVSEL[1:0]/-/-/TPBP_PIX[1:0] 
		{0x3243,0x00},//D_DANSA_SW/D_NOISE_SW/TPG_NOISE_SEL[1:0]/-/-/TPG_LIPOL/TPG_RLPOL 
		{0x3244,0x80},//R_DANSA[7:0] 
		{0x3245,0x80},//GR_DANSA[7:0] 
		{0x3246,0x80},//GB_DANSA[7:0] 
		{0x3247,0x80},//B_DANSA[7:0] 
		{0x3248,0x00},//RDIN[7:0] 
		{0x324A,0x00},//CSR_TEST[3:0]/CSR_ABCHG/-/CSR_LIPOL/CSR_RLPOL 
		{0x324B,0x01},//-/-/-/-/-/-/-/LNOBHLNR_SW 
		{0x3250,0x00},//HLNR1INT_MPYSW/HLNR1INT_MPSEL[2:0]/-/-/HLNR1INT_MP[9:8] 
		{0x3251,0x00},//HLNR1INT_MP[7:0] 
		{0x3254,0x00},//HLNR3INT_MPYSW/HLNR3INT_MPSEL[2:0]/-/-/HLNR3INT_MP[9:8] 
		{0x3255,0x00},//HLNR3INT_MP[7:0] 
		{0x3257,0x81},//HLNR_SW/DANSA_SW/-/TEST_HLNR/HLNR_DISP/-/-/DANSA_RLPOL 
		{0x3258,0x40},//AGMIN_LOB_REFLV[7:0] 
		{0x3259,0x40},//AGMAX_LOB_REFLV[7:0] 
		{0x325A,0x20},//AGMIN_LOB_WIDTH[7:0] 
		{0x325B,0x20},//AGMAX_LOB_WIDTH[7:0] 
		{0x325C,0x40},//MANU_BLACK[7:0] 
		{0x325D,0x00},//AGMIN_LOB_REFLV[8]/AGMAX_LOB_REFLV[8]/-/-/-/-/-/MANU_BLACK[8] 
		{0x325E,0x90},//AGMIN_DLBLACK_ADJ[7:0] 
		{0x325F,0x16},//AGMAX_DLBLACK_ADJ[7:0] 
		{0x3260,0x20},//HLNR_MANU_BLK[7:0] 
		{0x3261,0x44},//HLNR_OBCHG/OBDC_SW/-/-/-/OBDC_SEL/-/LOBINV_POL 
		{0x3262,0x06},//AGMIN_OBDC_REFLV[8]/AGMAX_OBDC_REFLV[8]/-/-/-/OBDC_MIX[2:0] 
		{0x3263,0x40},//AGMIN_OBDC_REFLV[7:0] 
		{0x3264,0x40},//AGMAX_OBDC_REFLV[7:0] 
		{0x3265,0x20},//AGMIN_OBDC_WIDTH[7:0] 
		{0x3266,0x20},//AGMAX_OBDC_WIDTH[7:0] 
		{0x3267,0x08},//HLNRBLADJ_SW/HLNRBLADJ_CHG/HLNR_OBNC_SW/HLNRBLADJ_BLKSW/HLNR1INT_S
		{0x3268,0xFF},//AGMIN_HLNRBLADJ_R[8]/AGMAX_HLNRBLADJ_R[8]/AGMIN_HLNRBLADJ_GR[8]/AG
		{0x3269,0x00},//AGMIN_HLNRBLADJ_R[7:0] 
		{0x326A,0x00},//AGMAX_HLNRBLADJ_R[7:0] 
		{0x326B,0x00},//AGMIN_HLNRBLADJ_GR[7:0] 
		{0x326C,0x00},//AGMAX_HLNRBLADJ_GR[7:0] 
		{0x326D,0x00},//AGMIN_HLNRBLADJ_GB[7:0] 
		{0x326E,0x00},//AGMAX_HLNRBLADJ_GB[7:0] 
		{0x326F,0x00},//AGMIN_HLNRBLADJ_B[7:0] 
		{0x3270,0x00},//AGMAX_HLNRBLADJ_B[7:0] 
		{0x3271,0x01},//-/-/-/-/-/-/-/BLADJ_HOLD_WAIT 
		{0x3276,0x00},//-/-/-/TEST_PWB/TEST_DGM/-/MPY_LIPOL/MPY_RLPOL 
		{0x3277,0x80},//PWB_WLRG[7:0] 
		{0x3278,0x80},//PWB_WLGRG[7:0] 
		{0x3279,0x80},//PWB_WLGBG[7:0] 
		{0x327A,0x80},//PWB_WLBG[7:0] 
		{0x327B,0x83},//MPX4_SEL/-/-/-/-/-/DATABIT_MP[9:8] 
		{0x327C,0xCD},//DATABIT_MP[7:0] 
		{0x3280,0x00},//LSSC_SW/-/-/TEST_LSSC/LSSC_DISP/-/LSSC_LIPOL/LSSC_CSPOL 
		{0x3281,0x44},//LSSC_HCNT_ADJ[7:0] 
		{0x3282,0x80},//LSSC_HCNT_MPY[7:0] 
		{0x3283,0x80},//LSSC_HCEN_ADJ[7:0] 
		{0x3284,0x44},//LSSC_VCNT_ADJ[7:0] 
		{0x3285,0x80},//LSSC_VCNT_MPY[7:0] 
		{0x3286,0x81},//LSSC_VCNT_MPYSW/-/-/-/LSSC_VCNT_MPY[11:8] 
		{0x3287,0x00},//LSSC_VCEN_ADJ[7:0] 
		{0x3288,0x02},//LSSC_VCEN_WIDTH/-/-/-/-/-/LSSC_VCEN_ADJ[9:8] 
		{0x3289,0x18},//LSSC_TOPL_PM1RG[7:0] 
		{0x328A,0x00},//LSSC_TOPL_PM1GRG[7:0] 
		{0x328B,0x00},//LSSC_TOPL_PM1GBG[7:0] 
		{0x328C,0x00},//LSSC_TOPL_PM1BG[7:0] 
		{0x328D,0x10},//LSSC_TOPR_PM1RG[7:0] 
		{0x328E,0x00},//LSSC_TOPR_PM1GRG[7:0] 
		{0x328F,0x00},//LSSC_TOPR_PM1GBG[7:0] 
		{0x3290,0x00},//LSSC_TOPR_PM1BG[7:0] 
		{0x3291,0x08},//LSSC_BOTL_PM1RG[7:0] 
		{0x3292,0x00},//LSSC_BOTL_PM1GRG[7:0] 
		{0x3293,0x00},//LSSC_BOTL_PM1GBG[7:0] 
		{0x3294,0x00},//LSSC_BOTL_PM1BG[7:0] 
		{0x3295,0x08},//LSSC_BOTR_PM1RG[7:0] 
		{0x3296,0x00},//LSSC_BOTR_PM1GRG[7:0] 
		{0x3297,0x00},//LSSC_BOTR_PM1GBG[7:0] 
		{0x3298,0x00},//LSSC_BOTR_PM1BG[7:0] 
		{0x3299,0x08},//-/-/-/-/LSSC1BG_PMSW/LSSC1GBG_PMSW/LSSC1GRG_PMSW/LSSC1RG_PMSW 
		{0x329A,0x86},//LSSC_LEFT_P2RG[7:0] 
		{0x329B,0x90},//LSSC_LEFT_P2GRG[7:0] 
		{0x329C,0x90},//LSSC_LEFT_P2GBG[7:0] 
		{0x329D,0x83},//LSSC_LEFT_P2BG[7:0] 
		{0x329E,0x6E},//LSSC_RIGHT_P2RG[7:0] 
		{0x329F,0x8C},//LSSC_RIGHT_P2GRG[7:0] 
		{0x32A0,0x8C},//LSSC_RIGHT_P2GBG[7:0] 
		{0x32A1,0x6F},//LSSC_RIGHT_P2BG[7:0] 
		{0x32A2,0x6C},//LSSC_TOP_P2RG[7:0] 
		{0x32A3,0x76},//LSSC_TOP_P2GRG[7:0] 
		{0x32A4,0x76},//LSSC_TOP_P2GBG[7:0] 
		{0x32A5,0x5D},//LSSC_TOP_P2BG[7:0] 
		{0x32A6,0x70},//LSSC_BOTTOM_P2RG[7:0] 
		{0x32A7,0xA0},//LSSC_BOTTOM_P2GRG[7:0] 
		{0x32A8,0xA0},//LSSC_BOTTOM_P2GBG[7:0] 
		{0x32A9,0x87},//LSSC_BOTTOM_P2BG[7:0] 
		{0x32AA,0x5C},//LSSC_LEFT_PM4RG[7:0] 
		{0x32AB,0x80},//LSSC_LEFT_PM4GRG[7:0] 
		{0x32AC,0x80},//LSSC_LEFT_PM4GBG[7:0] 
		{0x32AD,0x80},//LSSC_LEFT_PM4BG[7:0] 
		{0x32AE,0x00},//LSSC_RIGHT_PM4RG[7:0] 
		{0x32AF,0x80},//LSSC_RIGHT_PM4GRG[7:0] 
		{0x32B0,0x80},//LSSC_RIGHT_PM4GBG[7:0] 
		{0x32B1,0x60},//LSSC_RIGHT_PM4BG[7:0] 
		{0x32B2,0x80},//LSSC_TOP_PM4RG[7:0] 
		{0x32B3,0x40},//LSSC_TOP_PM4GRG[7:0] 
		{0x32B4,0x40},//LSSC_TOP_PM4GBG[7:0] 
		{0x32B5,0x00},//LSSC_TOP_PM4BG[7:0] 
		{0x32B6,0x00},//LSSC_BOTTOM_PM4RG[7:0] 
		{0x32B7,0xC0},//LSSC_BOTTOM_PM4GRG[7:0] 
		{0x32B8,0xC0},//LSSC_BOTTOM_PM4GBG[7:0] 
		{0x32B9,0xC0},//LSSC_BOTTOM_PM4BG[7:0] 
		{0x32BA,0x4F},//LSSC_MGSEL[1:0]/-/-/LSSC4BG_PMSW/LSSC4GBG_PMSW/LSSC4GRG_PMSW/LSSC4
		{0x32BB,0x00},//LSSC_BLACK[7:0] 
		{0x32BC,0x01},//-/-/-/-/-/-/-/LSSC_BLACK[8] 
		{0x32BF,0x10},//AGMAX_BBPC_SLV[7:0] 
		{0x32C0,0x10},//AGMAX_WBPC_SLV[7:0] 
		{0x32C1,0x08},//-/-/-/-/ABPC_SW/ABPC_MAP_EN/MAP_RLPOL/ABPC_THUR 
		{0x32C2,0x48},//DRC_RAMADR_ADJ[7:0] 
		{0x32C4,0x6C},//-/WBPC_SW/BBPC_SW/TEST_ABPC/WBPC_MODE/BBPC_MODE/-/ABPC_DISP 
		{0x32C5,0x10},//BBPC_SLV[7:0] 
		{0x32C6,0x10},//WBPC_SLV[7:0] 
		{0x32C7,0x00},//DFCT_VBIN/-/-/-/-/-/MAP_DSEL/ABPC_EDGE_SW 
		{0x32C8,0x08},//AGMAX_BPCEDGE_MP[7:0] 
		{0x32C9,0x08},//AGMIN_BPCEDGE_MP[7:0] 
		{0x32CA,0x94},//DFCT_XADJ[3:0]/DFCT_YADJ[3:0] 
		{0x32D3,0xA0},//ANR_SW/LPF_SEL/ANR_LIM/TEST_ANR/-/-/ANR_LIPOL/ANR_RLPOL 
		{0x32D4,0x20},//AGMIN_ANRW_R[7:0] 
		{0x32D5,0x20},//AGMIN_ANRW_G[7:0] 
		{0x32D6,0x30},//AGMIN_ANRW_B[7:0] 
		{0x32D7,0x40},//AGMAX_ANRW_R[7:0] 
		{0x32D8,0x40},//AGMAX_ANRW_G[7:0] 
		{0x32D9,0x4F},//AGMAX_ANRW_B[7:0] 
		{0x32DA,0x80},//AGMIN_ANRMP_R[7:0] 
		{0x32DB,0x80},//AGMIN_ANRMP_G[7:0] 
		{0x32DC,0xC0},//AGMIN_ANRMP_B[7:0] 
		{0x32DD,0xF0},//AGMAX_ANRMP_R[7:0] 
		{0x32DE,0xF0},//AGMAX_ANRMP_G[7:0] 
		{0x32DF,0xFF},//AGMAX_ANRMP_B[7:0] 
		{0x32E2,0x2C},//-/LOB1INT_PIX[6:0] 
		{0x32E4,0x2C},//-/LOB3INT_PIX[6:0] 
		{0x32E5,0x30},//-/H_LOB_WIDTH[6:0] 
		{0x32E6,0x02},//-/HLNR_LOB1CP_S[6:0] 
		{0x32E7,0x02},//-/HLNR_LOB2CP_S[6:0] 
		{0x32E8,0x02},//-/HLNR_LOB3CP_S[6:0] 
		{0x32E9,0x07},//PP_LOB_START[7:0] 
		{0x32EC,0x25},//NZ_REG[7:0] 
		{0x3300,0x1C},//-/-/-/BOOSTEN/POSLFIX/NEGLFIX/-/NEGLEAKCUT 
		{0x3301,0x06},//BSTREADEV/-/-/-/NEGBSTCNT[3:0] 
		{0x3302,0x04},//POSBSTSEL/-/-/-/-/POSBSTCNT[2:0] 
		{0x3303,0x35},//-/POSBSTHG[2:0]/-/POSBSTGA[2:0] 
		{0x3304,0x00},//VDSEL[1:0]/LNOBMODE[1:0]/-/READVDSEL/-/GDMOSBGREN 
		{0x3305,0x80},//KBIASSEL/-/-/-/-/-/-/- 
		{0x3306,0x00},//-/-/-/BSVBPSEL[4:0] 
		{0x3307,0x24},//-/-/RSTVDSEL_AL0/RSTVDSEL_NML/DRADRVI_AL0[1:0]/DRADRVI_NML[1:0] 
		{0x3308,0x88},//DRADRVPU[1:0]/-/VREFV[4:0] 
		{0x3309,0xCC},//ADSW2WEAK/ADSW1WEAK/-/-/VREFAI[3:0] 
		{0x330A,0x24},//ADCKSEL/-/ADCKDIV[1:0]/-/SENSEMODE[2:0] 
		{0x330B,0x00},//-/-/SPARE[1:0]/ANAMON1_SEL[3:0] 
		{0x330C,0x07},//HREG_TEST/-/TESTCROP/-/-/BINVSIG/BINED/BINCMP 
		{0x330E,0x80},//EXT_HCNT_MAX_ON/-/-/HCNT_MAX_MODE/-/-/-/MLT_SPL_MODE 
		{0x330F,0x06},//HCNT_MAX_FIXVAL[15:8] 
		{0x3310,0xDA},//HCNT_MAX_FIXVAL[7:0] 
		{0x3312,0x04},//-/-/VREG_TEST[1:0]/ES_MODE/BIN_MODE/DIS_MODE/RODATA_U 
		{0x3313,0x00},//-/-/-/-/-/-/-/DMRC_MODE 
		{0x3314,0x5C},//BSC_OFF/LIMITTER_BSC/VSIG_BSC/DRESET_CONJ_U[4:0] 
		{0x3315,0x05},//-/-/-/DRESET_HIGH/DRESET_CONJ_D[3:0] 
		{0x3316,0x04},//FTLSNS_HIGH/-/FTLSNS_LBSC_U[5:0] 
		{0x3317,0x00},//-/-/-/-/-/FTLSNS_LBSC_D[2:0] 
		{0x3318,0x18},//-/FTLSNS_CONJ_W[6:0] 
		{0x3319,0x05},//-/-/-/-/FTLSNS_CONJ_D[3:0] 
		{0x331A,0x08},//SADR_HIGH/-/SADR_LBSC_U[5:0] 
		{0x331B,0x00},//-/-/-/-/-/SADR_LBSC_D[2:0] 
		{0x331C,0x4E},//SADR_CONJ_U[7:0] 
		{0x331D,0x00},//-/-/-/-/SADR_CONJ_D[3:0] 
		{0x331F,0x03},//ESREAD_ALT_OFF/-/-/ELEC_INJ_MODE/-/-/AUTO_READ_W/AUTO_ESREAD_2D 
		{0x3320,0x1A},//-/-/-/DRESET_VSIG_U[4:0] 
		{0x3321,0x04},//-/-/-/-/DRESET_VSIG_D[3:0] 
		{0x3322,0x56},//ESREAD_1D[7:0] 
		{0x3323,0x7B},//ESREAD_1W[7:0] 
		{0x3324,0x01},//-/-/-/-/-/ESREAD_2D[10:8] 
		{0x3325,0x4D},//ESREAD_2D[7:0] 
		{0x3326,0xA6},//ESREAD_2W[7:0] 
		{0x3327,0x14},//ESTGRESET_LOW/ESTGRESET_D[6:0] 
		{0x3328,0x40},//ALLZEROSET_ON/ZEROSET_1ST/ALLZEROSET_CHG_ON/EXTD_ROTGRESET/-/-/ROT
		{0x3329,0xC6},//ROTGRESET_U[7:0] 
		{0x332A,0x1E},//-/-/ROTGRESET_W[5:0] 
		{0x332B,0x00},//ROREAD_U[7:0] 
		{0x332C,0xA6},//ROREAD_W[7:0] 
		{0x332D,0x16},//ZEROSET_U[7:0] 
		{0x332E,0x58},//ZEROSET_W[7:0] 
		{0x332F,0x00},//-/-/FIX_RSTDRAIN[1:0]/FIX_RSTDRAIN2[1:0]/FIX_RSTDRAIN3[1:0] 
		{0x3330,0x00},//-/RSTDRAIN_D[6:0] 
		{0x3331,0x0C},//-/-/RSTDRAIN_U[5:0] 
		{0x3332,0x05},//-/-/-/-/RSTDRAIN2_D[3:0] 
		{0x3333,0x05},//-/-/-/-/RSTDRAIN2_U[3:0] 
		{0x3334,0x00},//-/-/-/-/RSTDRAIN3_D[3:0] 
		{0x3335,0x14},//-/-/RSTDRAIN3_U[5:0] 
		{0x3336,0x03},//-/-/DRCUT_SIGIN/DRCUT_HIGH/-/-/VSIGDR_MODE[1:0] 
		{0x3337,0x01},//-/-/DRCUT_NML_U[5:0] 
		{0x3338,0x20},//-/-/DRCUT_CGR_U[5:0] 
		{0x3339,0x20},//-/-/DRCUT_CGR_D[5:0] 
		{0x333A,0x30},//-/-/DRCUT_VDER_1U[5:0] 
		{0x333B,0x04},//-/-/DRCUT_VDER_1D[5:0] 
		{0x333C,0x30},//-/-/DRCUT_VDER_2U[5:0] 
		{0x333D,0x04},//-/-/DRCUT_VDER_2D[5:0] 
		{0x333E,0x00},//-/-/DRCUT_1ITV_MIN[1:0]/-/-/DRCUT_2ITV_MIN[1:0] 
		{0x333F,0x3A},//GDMOSCNT_NML[3:0]/GDMOSCNT_VDER[3:0] 
		{0x3340,0x11},//GDMOSCNT_CGR[3:0]/-/-/GDMOS_VDER_1U[1:0] 
		{0x3341,0x04},//-/-/GDMOS_VDER_1D[5:0] 
		{0x3342,0x11},//-/-/GDMOS2CNT[1:0]/-/-/GDMOS_VDER_2U[1:0] 
		{0x3343,0x04},//-/-/GDMOS_VDER_2D[5:0] 
		{0x3344,0x10},//-/-/-/RO_DRCUT_OFF/-/-/-/SIGIN_ON 
		{0x3345,0x05},//GDMOS_CGR_D[5:0]/GDMOSLT_VDER_1W[1:0] 
		{0x3346,0x61},//-/GDMOSLT_VDER_1D[6:0] 
		{0x3347,0x01},//-/-/-/-/-/-/GDMOSLT_VDER_2W[1:0] 
		{0x3348,0x60},//-/GDMOSLT_VDER_2D[6:0] 
		{0x334C,0x18},//ADSW1_D[7:0] 
		{0x334D,0x00},//ADSW1_HIGH/-/ADSW_U[5:0] 
		{0x334E,0x0C},//ADSW1DMX_LOW/-/-/ADSW1DMX_U[4:0] 
		{0x334F,0x30},//ADSW1LK_HIGH/ADSW1LK_D[6:0] 
		{0x3350,0x30},//ADSW1LKX_LOW/ADSW1LKX_U[6:0] 
		{0x3351,0x00},//ADCMP1SRT_LOW/-/-/-/-/-/-/- 
		{0x3353,0x18},//ADSW2_HIGH/-/ADSW2_D[5:0] 
		{0x3354,0x0C},//ADSW2DMX_LOW/-/-/ADSW2DMX_U[4:0] 
		{0x3355,0x80},//FIX_ADENX[1:0]/ADENX_U[5:0] 
		{0x3356,0x01},//-/-/ADENX_D[5:0] 
		{0x3357,0x00},//-/-/CMPI_CGR_U[5:0] 
		{0x3358,0x01},//-/-/CMPI_CGR_D[5:0] 
		{0x3359,0x83},//CMPI1_NML[3:0]/CMPI2_NML[3:0] 
		{0x335A,0x33},//CMPI1_CGR[3:0]/CMPI2_CGR[3:0] 
		{0x335B,0x03},//-/-/-/-/-/-/CGR_MODE/CDS_STOPBST 
		{0x335C,0x10},//BSTCKLFIX_HIGH/BSTCKLFIX_CMP_U[6:0] 
		{0x335D,0x14},//-/BSTCKLFIX_CMP_D[6:0] 
		{0x335E,0x89},//CDS_ADC_BSTOFF/-/-/BSTCKLFIX_ADC_U[4:0] 
		{0x335F,0x01},//-/-/-/BSTCKLFIX_ADC_D[4:0] 
		{0x3360,0x11},//VSIGLMTCNT_RNG3[3:0]/VSIGLMTCNT_RNG2[3:0] 
		{0x3361,0x17},//VSIGLMTCNT_RNG1[3:0]/VSIGLMTCNT_RNG0[3:0] 
		{0x3362,0x00},//VSIGLMTEN_U0[7:0] 
		{0x3363,0x00},//-/FIX_VSIGLMTEN[2:0]/VSIGLMTEN_D[3:0] 
		{0x3364,0x00},//VSIGLMTEN_U1[7:0] 
		{0x3366,0x03},//-/-/-/-/SINT_ZS_U[3:0] 
		{0x3367,0x4F},//SINT_ZS_W[7:0] 
		{0x3368,0x3B},//-/SINT_RS_U[6:0] 
		{0x3369,0x77},//SINT_RS_W[7:0] 
		{0x336A,0x1D},//SINT_FB_U[7:0] 
		{0x336B,0x4F},//-/SINT_FB_W[6:0] 
		{0x336C,0x01},//-/-/-/-/-/-/SINT_AD_U[9:8] 
		{0x336D,0x17},//SINT_AD_U[7:0] 
		{0x336E,0x01},//-/-/-/-/-/-/-/SINT_AD_W[8] 
		{0x336F,0x77},//SINT_AD_W[7:0] 
		{0x3370,0x61},//-/SINTLSEL2/SINTLSEL/SINTSELPH2/SINTSELPH1/SINTSELFB[2:0] 
		{0x3371,0x20},//SINTSELOUT2[3:0]/SINTSELOUT1[3:0] 
		{0x3372,0x33},//DRADRVLV_AL0_RNG3[3:0]/DRADRVLV_AL0_RNG2[3:0] 
		{0x3373,0x33},//DRADRVLV_AL0_RNG1[3:0]/DRADRVLV_AL0_RNG0[3:0] 
		{0x3374,0x11},//DRADRVLV_NML_RNG3[3:0]/DRADRVLV_NML_RNG2[3:0] 
		{0x3375,0x11},//DRADRVLV_NML_RNG1[3:0]/DRADRVLV_NML_RNG0[3:0] 
		{0x3376,0x00},//-/-/-/-/GDMOS2ENX_MODE[3:0] 
		{0x3380,0x11},//-/-/SINTX_DSHIFT[1:0]/-/-/SINTX_USHIFT[1:0] 
		{0x3381,0x10},//-/-/VCD_RNG_TYPE_SEL[1:0]/-/-/VREFIC_MODE[1:0] 
		{0x3382,0x00},//EXT_VREFIC_ON/-/-/-/-/VREFIC_FIXVAL[2:0] 
		{0x3383,0x00},//-/FIX_VREFICAID[2:0]/-/-/VREFICAID_OFF[1:0] 
		{0x3384,0x00},//-/-/-/-/-/-/VREFICAID_W[9:8] 
		{0x3385,0x64},//VREFICAID_W[7:0] 
		{0x3386,0x00},//-/-/-/VREFALN/-/-/-/VREFIMBC 
		{0x3387,0x00},//-/PS_VFB_GBL_VAL[6:0] 
		{0x3388,0x00},//-/PS_VFB_10B_RNG1[6:0] 
		{0x3389,0x00},//-/PS_VFB_10B_RNG2[6:0] 
		{0x338A,0x00},//-/PS_VFB_10B_RNG3[6:0] 
		{0x338B,0x00},//-/PS_VFB_10B_RNG4[6:0] 
		{0x338C,0x01},//-/-/-/-/VFB_STEP_GBL[3:0] 
		{0x338D,0x01},//-/-/-/-/VFB_STEP_RNG1[3:0] 
		{0x338E,0x01},//-/-/-/-/VFB_STEP_RNG2[3:0] 
		{0x338F,0x01},//-/-/-/-/VFB_STEP_RNG3[3:0] 
		{0x3390,0x01},//-/-/-/-/VFB_STEP_RNG4[3:0] 
		{0x3391,0xC6},//SRST_RS_U[7:0] 
		{0x3392,0x0D},//SRST_RS_U[8]/-/SRST_RS_W[5:0] 
		{0x3393,0xB5},//SRST_ZS_U[7:0] 
		{0x3394,0x0D},//-/-/SRST_ZS_W[5:0] 
		{0x3395,0x03},//-/-/-/-/SRST_AD_U[3:0] 
		{0x3396,0xA5},//SRST_AD_D[7:0] 
		{0x339F,0x00},//VREF12ADIP/-/-/-/-/-/-/- 
		{0x33A0,0x03},//VREFSHBGR_LOW/-/-/-/VREFSHBGR_D[3:0] 
		{0x33A1,0x38},//-/-/VREFSHBGR_U[5:0] 
		{0x33A2,0xBC},//EN_VREFC_ZERO/-/VREF_H_START_U[5:0] 
		{0x33A3,0x00},//ADCKEN_MASK[1:0]/-/-/-/-/-/- 
		{0x33A4,0x0B},//-/ADCKEN_1U[6:0] 
		{0x33A5,0x0F},//-/-/-/ADCKEN_1D[4:0] 
		{0x33A6,0x0B},//-/ADCKEN_2U[6:0] 
		{0x33A7,0x0F},//-/-/-/ADCKEN_2D[4:0] 
		{0x33A8,0x09},//-/-/-/-/CNTRSTX_U[3:0] 
		{0x33A9,0x0F},//-/-/-/CNT0RSTX_1D[4:0] 
		{0x33AA,0x09},//-/-/-/-/CNT0RSTX_2U[3:0] 
		{0x33AB,0x0F},//-/-/-/CNT0RSTX_2D[4:0] 
		{0x33AC,0x08},//-/-/-/CNTINVX_START[4:0] 
		{0x33AD,0x14},//EDCONX_1D[7:0] 
		{0x33AE,0x00},//EDCONX_RS_HIGH/EDCONX_AD_HIGH/-/-/-/-/-/EDCONX_2D[8] 
		{0x33AF,0x28},//EDCONX_2D[7:0] 
		{0x33B0,0x00},//ADTESTCK_INTVL[3:0]/-/-/ADTESTCK_ON/COUNTER_TEST 
		{0x33B2,0x00},//-/-/-/-/-/-/-/ROUND_VREF_CODE 
		{0x33B3,0x01},//EXT_VCD_ADJ_ON/-/-/AG_SEN_SHIFT/-/-/VCD_COEF_FIXVAL[9:8] 
		{0x33B4,0x00},//VCD_COEF_FIXVAL[7:0] 
		{0x33B5,0x00},//-/-/VCD_INTC_FIXVAL[5:0] 
		{0x33B6,0x1B},//VREFAD_RNG1_SEL[1:0]/VREFAD_RNG2_SEL[1:0]/VREFAD_RNG3_SEL[1:0]/VRE
		{0x33B7,0x00},//-/-/-/-/-/-/AGADJ1_VREFI_ZS[9:8] 
		{0x33B8,0x3C},//AGADJ1_VREFI_ZS[7:0] 
		{0x33B9,0x00},//-/-/-/-/-/-/AGADJ2_VREFI_ZS[9:8] 
		{0x33BA,0x1E},//AGADJ2_VREFI_ZS[7:0] 
		{0x33BB,0x00},//-/-/-/-/-/-/-/AGADJ1_VREFI_AD[8] 
		{0x33BC,0x3C},//AGADJ1_VREFI_AD[7:0] 
		{0x33BD,0x00},//-/-/-/-/-/-/-/AGADJ2_VREFI_AD[8] 
		{0x33BE,0x1E},//AGADJ2_VREFI_AD[7:0] 
		{0x33BF,0x70},//-/AGADJ_VREFIC[2:0]/-/AGADJ_VREFC[2:0] 
		{0x33C0,0x00},//EXT_VREFI_ZS_ON/-/-/-/-/-/VREFI_ZS_FIXVAL[9:8] 
		{0x33C1,0x00},//VREFI_ZS_FIXVAL[7:0] 
		{0x33C2,0x00},//EXT_VREFI_FB_ON/-/-/-/-/-/-/- 
		{0x33C3,0x00},//-/-/VREFI_FB_FIXVAL[5:0] 
		{0x33C6,0x00},//EXT_VREFC_ON/-/-/-/-/VREFC_FIXVAL[2:0] 
		{0x33C7,0x00},//EXT_PLLFREQ_ON/-/-/-/PLLFREQ_FIXVAL[3:0] 
		{0x33C9,0x01},//-/-/-/-/-/-/-/BCDCNT_ST6 
		{0x33CA,0x00},//-/-/-/ACT_TESTDAC/-/-/-/TESTDACEN 
		{0x33CB,0x80},//TDAC_INT[7:0] 
		{0x33CC,0x00},//TDAC_MIN[7:0] 
		{0x33CD,0x10},//TDAC_STEP[3:0]/-/-/TDAC_SWD[1:0] 
		{0x33CE,0x00},//-/-/-/-/-/-/-/AG_TEST 
		{0x33CF,0x00},//DACS_INT[7:0] 
		{0x33D0,0xFF},//DACS_MAX[7:0] 
		{0x33D1,0x10},//DACS_STEP[3:0]/-/-/DACS_SWD[1:0] 
		{0x33D2,0x80},//TESTDAC_RSVOL[7:0] 
		{0x33D3,0x60},//TESTDAC_ADVOL[7:0] 
		{0x33D4,0x62},//ZSV_EXEC_MODE[3:0]/-/AGADJ_EXEC_MODE[2:0] 
		{0x33D5,0x02},//-/AGADJ_CALC_MODE/-/-/AGADJ_FIX_COEF[11:8] 
		{0x33D6,0x06},//AGADJ_FIX_COEF[7:0] 
		{0x33D7,0xF1},//ZSV_FORCE_END[3:0]/-/-/ZSV_SUSP_RANGE[1:0] 
		{0x33D8,0x86},//ZSV_SUSP_CND/-/-/-/EN_PS_VREFI_ZS[3:0] 
		{0x33DA,0xA0},//ZSV_LEVEL[7:0] 
		{0x33DB,0x10},//-/-/ZSV_IN_RANGE[5:0] 
		{0x33DC,0xC7},//PS_VZS_NML_COEF[7:0] 
		{0x33DD,0x00},//-/PS_VZS_NML_INTC[6:0] 
		{0x33DE,0x10},//VZS_NML_STEP[7:0] 
		{0x33DF,0x42},//ZSV_STOP_CND[1:0]/-/-/ZSV_IN_LINES[3:0] 
		{0x33E2,0x10},//-/FBC_IN_RANGE[6:0] 
		{0x33E3,0xC4},//FBC_SUSP_RANGE[1:0]/-/FBC_IN_LINES[4:0] 
		{0x33E4,0x5F},//FBC_OUT_RANGE[1:0]/-/FBC_OUT_LINES[4:0] 
		{0x33E5,0x21},//FBC_STOP_CND[2:0]/-/PS_VREFI_FB[3:0] 
		{0x33E6,0x21},//FBC_START_CND[3:0]/-/-/-/- 
		{0x33E7,0x86},//FBC_SUSP_CND/-/-/EN_PS_VREFI_FB[4:0] 
		{0x33E8,0x00},//-/-/-/-/-/-/-/ST_CLIP_DATA 
		{0x33E9,0x00},//ST_BLACK_LEVEL[9:8]/ST_CKI[5:0] 
		{0x33EA,0x40},//ST_BLACK_LEVEL[7:0] 
		{0x33EB,0xF0},//ST_RSVD_REG[7:0] 
		{0x3400,0x98},//GLB0SET_MODE/-/GLBTGRESET_U[5:0] 
		{0x3401,0x01},//-/-/-/-/-/-/GLBTGRESET_W[9:8] 
		{0x3402,0x14},//GLBTGRESET_W[7:0] 
		{0x3403,0x00},//-/-/-/-/-/-/-/GLBREAD_1W[8] 
		{0x3404,0x9C},//GLBREAD_1W[7:0] 
		{0x3405,0x00},//-/-/-/-/-/-/-/GLBREAD_1D[8] 
		{0x3406,0x90},//GLBREAD_1D[7:0] 
		{0x3407,0x00},//-/-/-/-/-/-/-/GLBREAD_2W[8] 
		{0x3408,0xCC},//GLBREAD_2W[7:0] 
		{0x3409,0x00},//-/-/-/-/GLBREAD_2D[3:0] 
		{0x340A,0x42},//GLBZEROSET_U[3:0]/GLBZEROSET_W[3:0] 
		{0x3410,0x16},//-/GLBRSTDRAIN3_U[6:0] 
		{0x3411,0x28},//-/GBLRSTDRAIN_D[6:0] 
		{0x3412,0x0C},//-/-/GBLRSTDRAIN_U[5:0] 
		{0x3414,0x00},//SENDUM_1U[7:0] 
		{0x3415,0x00},//SENDUM_1W[7:0] 
		{0x3416,0x00},//SENDUM_2U[7:0] 
		{0x3417,0x00},//SENDUM_2W[7:0] 
		{0x3418,0x00},//SENDUM_3U[7:0] 
		{0x3419,0x00},//SENDUM_3W[7:0] 
		{0x341A,0x30},//BSC_ULMT_AGRNG2[7:0] 
		{0x341B,0x14},//BSC_ULMT_AGRNG1[7:0] 
		{0x341C,0x0C},//BSC_ULMT_AGRNG0[7:0] 
		{0x341D,0xA9},//KBIASCNT_RNG3[3:0]/KBIASCNT_RNG2[3:0] 
		{0x341E,0x87},//KBIASCNT_RNG1[3:0]/KBIASCNT_RNG0[3:0] 
		{0x341F,0x0F},//-/-/-/-/LIMIT_BSC_MODE[3:0] 
		{0x3420,0x00},//PSRR[15:8] 
		{0x3421,0x00},//PSRR[7:0] 
		{0x3423,0x00},//-/-/POSBSTCUT/-/-/-/-/GDMOSCNTX4 
		{0x3424,0x10},//-/-/-/DRADRVSTPEN/-/-/-/VREFMSADIP 
		{0x3425,0x31},//-/-/ADCMP1SRTSEL/CMPAMPCAPEN/-/-/VREFTEST[1:0] 
		{0x3426,0x00},//ST_PSREV/-/-/-/-/-/-/- 
		{0x3427,0x01},//AGADJ_REV_INT/-/-/-/-/-/VREFIMX4_SEL/REV_INT_MODE 
		{0x3428,0x80},//RI_VREFAD_COEF[7:0] 
		{0x3429,0x11},//SINT_RF_U[7:0] 
		{0x342A,0x77},//-/SINT_RF_W[6:0] 
		{0x3434,0x00},//ADCMP1SRT_NML_RS_U[7:0] 
		{0x3435,0x23},//-/ADCMP1SRT_D[6:0] 
		{0x3436,0x00},//-/-/-/-/-/-/ADCMP1SRT_NML_AD_U[9:8] 
		{0x3437,0x00},//ADCMP1SRT_NML_AD_U[7:0] 
		{0x344E,0x0F},//-/-/-/-/BSDIGITAL_MODE[3:0] 
		{0x3450,0x07},//-/-/-/-/FRAC_EXP_TIME_10NML[11:8] 
		{0x3451,0xD8},//FRAC_EXP_TIME_10NML[7:0] 
		{0x3458,0x01},//-/-/-/-/-/-/-/BGRDVSTPEN 
		{0x3459,0x27},//BGRDVSTP_D[7:0] 
		{0x345A,0x16},//-/-/BGRDVSTP_U[5:0] 
};

static struct v4l2_subdev_info t4k04_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array t4k04_init_conf[] = {
	{&t4k04_recommend_settings[0],
	ARRAY_SIZE(t4k04_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array t4k04_confs[] = {
	{&t4k04_snap_settings[0],
	ARRAY_SIZE(t4k04_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&t4k04_prev_settings[0],
	ARRAY_SIZE(t4k04_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t t4k04_dimensions[] = {
	{
		//snapshot
		.x_output = 0xCD0,   //3280
		.y_output = 0x9A0,   //2464
		.line_length_pclk = 0x0e68 ,
		.frame_length_lines = 0x09c0 ,
		.vt_pixel_clk = 92000000 ,
		.op_pixel_clk = 92000000,
		.binning_factor = 1,
	},
	{
	    //preview
		.x_output = 0x668,  //1640
		.y_output = 0x4D0,  //1232
		.line_length_pclk = 0x0770,
		.frame_length_lines = 0x0974,
		.vt_pixel_clk = 92000000 ,
		.op_pixel_clk = 92000000 ,
		.binning_factor = 2,
	},
};

static struct msm_camera_csi_params t4k04_csi_params = {
	.data_format = CSI_10BIT,
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 14,
};

static struct msm_camera_csi_params *t4k04_csi_params_array[] = {
	&t4k04_csi_params,
	&t4k04_csi_params,
};

static struct msm_sensor_output_reg_addr_t t4k04_reg_addr = {
	.x_output = 0x034c,
	.y_output = 0x034e,
	.line_length_pclk = 0x0342 ,
	.frame_length_lines = 0x0340,
};

static struct msm_sensor_id_info_t t4k04_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x14,
};

static struct msm_sensor_exp_gain_info_t t4k04_exp_gain_info = {
	.coarse_int_time_addr = 0x0202,
	.global_gain_addr = 0x0204,
	.vert_offset = 0,
};


/**********************************************************************
FUNC:OPEN T4k04 OTP
PARAM: page_num-----T4K04 OTP is devided to some pages
                                  Page(0,1,2,3,6,7,8,9) content-LSC/AWB
                                  Page(4,10) content-Module information/Golden module
                                  Page(5,11) content-AF
***********************************************************************/
static int32_t t4k04_init_otp_setting(struct msm_sensor_ctrl_t * s_ctrl,
                                                        uint16_t page_num)
{
   	int32_t rc = -EFAULT;
	//open which page
	rc = 	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3102,
	                                            page_num, MSM_CAMERA_I2C_BYTE_DATA);
	
	//OTP_RWT/OTP_RNUM[1:0]/OTP_VERIFY/OTP_VMOD/OTP_PCLK[2:0]   
	rc = 	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3145,0x06,
                                                   MSM_CAMERA_I2C_BYTE_DATA);
	
      //OTP open  ,OTP_STA/-/-/-/-/OTP_CLRE/OTP_WREC/OTP_ENBL   
	rc = 	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3100,0x81,                                                  
	                                            MSM_CAMERA_I2C_BYTE_DATA);
	  
	//OTP_TEST[3:0]/OTP_SPBE/OTP_TOEC/OTP_VEEC/OTP_STRC  
	rc = 	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3147,0x00,                                                  
	                                            MSM_CAMERA_I2C_BYTE_DATA);
	
	//OTP_LD_FEND/OTP_LD_RELD/-/-/-/OTP_LD_STS[1:0]/OTP_LD_ING  
	rc = 	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3150,0x06,                                                  
	                                            MSM_CAMERA_I2C_BYTE_DATA);
	//delay 30us at least
	msleep(1);
	
       if(rc<0)
       	{
       	   CDBG("%s:Page %x open FAILED\n",__func__,page_num);
		   return rc;
		}
	 return rc;
}

/*After the whole OTP operation is over,close the OTP*/
static int32_t t4k04_close_otp(struct msm_sensor_ctrl_t * s_ctrl)
{
   	int32_t rc = -EFAULT;
	//close otp
	rc = 	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3100,0x00,                                                  
	                                            MSM_CAMERA_I2C_BYTE_DATA);
       if(rc<0)
       	{
       	   CDBG("%s:close OTP FAILED",__func__);
		   return rc;
		}
	 return rc;
}

static int32_t t4k04_update_otp_data(struct msm_sensor_ctrl_t * s_ctrl)
{
       uint16_t Program_Flag=0; //0-this page is not programed 
       uint16_t check_sum =0;    //check sum
       uint32_t sum=0;
       int i=0;
	int page_index=0; //record the programmed page num
	uint16_t otp_data[64]={0};  //data region
	uint16_t otp_data_backup[64]={0};  //backup region
	//从PAGE 3开始读，如果0x3014数据读出来为0，则读前一页
	//直至读到PAGE 0
	//  1. find the programmed page 
       for(i=3;i>=0;i--)
       {
             t4k04_init_otp_setting(s_ctrl,i);
	       msleep(5);
	    	msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3104,&Program_Flag,
	    		                            MSM_CAMERA_I2C_BYTE_DATA);
		CDBG("%s:t4k04 0x3014 Program_Flag is %d(PAGE %d)\n",__func__,Program_Flag,i);
	    	if(Program_Flag!=0)
	    	{
                CDBG("%s:t4k04 OTP Page %d is programmed\n",__func__,i);
		   page_index=i;
                break;
		}
			
		if(i==0)
		{
		   CDBG("%s:t4k04 update OTP data FAILED,return\n",__func__);
		   return -1;
		}
	}

	// 2. check the check_sum whether right
       msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3105,&check_sum,
		                            MSM_CAMERA_I2C_BYTE_DATA);
       CDBG("%s:0x3105 check_sum is 0x%x\n",__func__,check_sum);

	for(i=0;i<62;i++)
	{
	      //read from 0x3106 to 0x3143
		msm_camera_i2c_read(s_ctrl->sensor_i2c_client,(0x3106+i),&otp_data[i],
		                            MSM_CAMERA_I2C_BYTE_DATA);
		sum  += otp_data[i];
	}

	 CDBG("%s:0x3107  is 0x%x\n",__func__,otp_data[1]);
	 CDBG("%s:0x3108  is 0x%x\n",__func__,otp_data[2]);
	 CDBG("%s:0x3109  is 0x%x\n",__func__,otp_data[3]);
	if((sum%256)!=check_sum)
	{
		CDBG("%s:the check sum ERROR,return\n",__func__);   
		return -1;
	}
	
      // 3. check the backup data is right
      check_sum=0;
	 sum=0;
       t4k04_init_otp_setting(s_ctrl,(page_index+6));  
	  
       msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3105,&check_sum,
		                            MSM_CAMERA_I2C_BYTE_DATA);
       CDBG("%s:0x3105 backup check_sum is 0x%x\n",__func__,check_sum);

	for(i=0;i<62;i++)
	{
	      //read from 0x3106 to 0x3143
		msm_camera_i2c_read(s_ctrl->sensor_i2c_client,(0x3106+i),&otp_data_backup[i],
		                            MSM_CAMERA_I2C_BYTE_DATA);
		sum  += otp_data_backup[i];
	}

	 CDBG("%s:0x3107 backup is 0x%x\n",__func__,otp_data_backup[1]);
	 CDBG("%s:0x3108 backup is 0x%x\n",__func__,otp_data_backup[2]);
	 CDBG("%s:0x3109 backup is 0x%x\n",__func__,otp_data_backup[3]);
	if((sum%256)!=check_sum)
	{
		CDBG("%s:the backup check sum ERROR,return\n",__func__);   
		return -1;
	}

	// 4. otp_data_backup |otp_data

	for(i=0;i<62;i++)
	{
		otp_data[i] = otp_data[i] |otp_data_backup[i];
	}

	// 5. write the OTP data to the Registers
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3283,otp_data[1],                                                  
	                                            MSM_CAMERA_I2C_BYTE_DATA);
	for(i=0;i<52;i++)
	{
	   msm_camera_i2c_write(s_ctrl->sensor_i2c_client,(0x3287+i),otp_data[1+i],                                                  
	                                            MSM_CAMERA_I2C_BYTE_DATA);
	}

	return 0;
}

	

static int32_t t4k04_write_pict_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
      uint32_t fl_lines, offset;
	uint8_t int_time[2];

	fl_lines =
		(s_ctrl->curr_frame_length_lines * s_ctrl->fps_divider) / Q10;
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;
	if (line > (fl_lines - offset))
		fl_lines = line + offset;
	CDBG("t4k04_write_exp_gain: %d %d %d\n", fl_lines, gain, line);
	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines, fl_lines,
		MSM_CAMERA_I2C_WORD_DATA);

	line = line / 2;
		
	int_time[0] =(uint8_t)( line & 0x00FF);
	int_time[1] =(uint8_t)(( line & 0xff00)>>8);
	printk("%s:int_time[0] is 0x%x,int_time[1] is 0x%x\n",__func__,int_time[0],int_time[1]);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr+1,
		int_time[0], MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		int_time[1], MSM_CAMERA_I2C_BYTE_DATA);
	
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
		MSM_CAMERA_I2C_WORD_DATA);
	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	return 0;
}

static int32_t t4k04_write_priv_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	uint32_t fl_lines, offset;
	uint8_t int_time[2];

	fl_lines =
		(s_ctrl->curr_frame_length_lines * s_ctrl->fps_divider) / Q10;
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;
	if (line > (fl_lines - offset))
		fl_lines = line + offset;
	CDBG("t4k04_write_exp_gain: 0x%x ,0x%x, 0x%x\n", fl_lines, gain, line);
	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines, fl_lines,
		MSM_CAMERA_I2C_WORD_DATA);
	int_time[0] =(uint8_t)( line & 0x00FF);
	int_time[1] =(uint8_t)(( line & 0xff00)>>8);
	printk("%s:int_time[0] is 0x%x,int_time[1] is 0x%x\n",__func__,int_time[0],int_time[1]);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr+1,
		int_time[0], MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		int_time[1], MSM_CAMERA_I2C_BYTE_DATA);
	
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
		MSM_CAMERA_I2C_WORD_DATA);
	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	
	return 0;
}

static const struct i2c_device_id t4k04_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&t4k04_s_ctrl},
	{ }
};




static struct i2c_driver t4k04_i2c_driver = {
	.id_table = t4k04_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client t4k04_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};



static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&t4k04_i2c_driver);
}

static struct v4l2_subdev_core_ops t4k04_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops t4k04_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops t4k04_subdev_ops = {
	.core = &t4k04_subdev_core_ops,
	.video  = &t4k04_subdev_video_ops,
};

int32_t t4k04_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;

	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		printk("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		msm_sensor_write_init_settings(s_ctrl);
	        
	       t4k04_update_otp_data(s_ctrl);
	       t4k04_close_otp(s_ctrl);
      

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

		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		msleep(50);
	}
	return rc;
}


static struct msm_sensor_fn_t t4k04_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = t4k04_write_priv_exp_gain,
	.sensor_write_snapshot_exp_gain = t4k04_write_pict_exp_gain,
	.sensor_csi_setting = t4k04_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
};

static struct msm_sensor_reg_t t4k04_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = t4k04_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(t4k04_start_settings),
	.stop_stream_conf = t4k04_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(t4k04_stop_settings),
	.group_hold_on_conf = t4k04_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(t4k04_groupon_settings),
	.group_hold_off_conf = t4k04_groupoff_settings,
	.group_hold_off_conf_size =	ARRAY_SIZE(t4k04_groupoff_settings),
	.init_settings = &t4k04_init_conf[0],
	.init_size = ARRAY_SIZE(t4k04_init_conf),
	.mode_settings = &t4k04_confs[0],
	.output_settings = &t4k04_dimensions[0],
	.num_conf = ARRAY_SIZE(t4k04_confs),
};

static struct msm_sensor_ctrl_t t4k04_s_ctrl = {
	.msm_sensor_reg = &t4k04_regs,
	.sensor_i2c_client = &t4k04_sensor_i2c_client,
	.sensor_i2c_addr = 0x6C,
	.sensor_output_reg_addr = &t4k04_reg_addr,
	.sensor_id_info = &t4k04_id_info,
	.sensor_exp_gain_info = &t4k04_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &t4k04_csi_params_array[0],
	.msm_sensor_mutex = &t4k04_mut,
	.sensor_i2c_driver = &t4k04_i2c_driver,
	.sensor_v4l2_subdev_info = t4k04_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(t4k04_subdev_info),
	.sensor_v4l2_subdev_ops = &t4k04_subdev_ops,
	.func_tbl = &t4k04_func_tbl,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Toshiba 8MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");




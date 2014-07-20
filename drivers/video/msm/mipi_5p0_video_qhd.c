/* Copyright (c) 2008-2011, Code Aurora Forum. All rights reserved.
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

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_lead.h"
#include <mach/gpio.h>
#ifdef CONFIG_ZTE_PLATFORM
#include <mach/zte_memlog.h>
#endif

static struct dsi_buf qhd_tx_buf;
static struct dsi_buf qhd_rx_buf;
extern u32 LcdPanleID;
extern  unsigned int lcd_id_type;
extern void mipi_zte_set_backlight(struct msm_fb_data_type *mfd);

#ifdef CONFIG_BACKLIGHT_CABC
void mipi_set_backlight_g01(struct msm_fb_data_type *mfd);
#endif
//[ECID 000000] zhangqi add for CABC end
//#ifdef CONFIG_PROJECT_P865F03
#define ZTE_LCD_EN                  126
#define ZTE_LCD_1WIRE_KEEP        96
void zte_lcd_en_set(void);
void zte_lcd_1wire_keep_set(void);

#ifdef CONFIG_BACKLIGHT_CABC
/**************************************
CABC of JDF + otm9608a
**************************************/
static char otm9608a_para_CABC_0x51[2]={0x51,0xff};
static char otm9608a_para_CABC_0x53[2]={0x53,0x2c};
static char otm9608a_para_CABC_0x55[2]={0x55,0x03};
static struct dsi_cmd_desc otm9608a_display_on_CABC_backlight_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(otm9608a_para_CABC_0x51), otm9608a_para_CABC_0x51},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(otm9608a_para_CABC_0x53), otm9608a_para_CABC_0x53},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(otm9608a_para_CABC_0x55), otm9608a_para_CABC_0x55},
};
static char otm9608a_para_CABC_0x53_off[2]={0x53,0x00};
static struct dsi_cmd_desc otm9608a_display_off_CABC_backlight_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(otm9608a_para_CABC_0x53_off), otm9608a_para_CABC_0x53_off}
};


/**************************************
CABC of TIANMA + nt35516
**************************************/
static char tianma_nt35516_para_CABC_0x51[2]={0x51,0xff};
static char tianma_nt35516_para_CABC_0x53[2]={0x53,0x2c};
static char tianma_nt35516_para_CABC_0x55[2]={0x55,0x03};
static struct dsi_cmd_desc tianma_nt35516_display_on_CABC_backlight_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(tianma_nt35516_para_CABC_0x51),tianma_nt35516_para_CABC_0x51},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(tianma_nt35516_para_CABC_0x53),tianma_nt35516_para_CABC_0x53},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(tianma_nt35516_para_CABC_0x55),tianma_nt35516_para_CABC_0x55},
};

static char tianma_nt35516_para_CABC_0x53_off[2]={0x53,0x00};
static struct dsi_cmd_desc tianma_nt35516_display_off_CABC_backlight_cmds[] = {
	{DTYPE_DCS_WRITE1,1,0,0,0,sizeof(tianma_nt35516_para_CABC_0x53_off),tianma_nt35516_para_CABC_0x53_off}
};

#endif
// #ifdef CONFIG_FB_MSM_GPIO
#define GPIO_LCD_RESET 78
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};
static char diplay_pixel_off[2] = {0x00,0x00}; //2013.2.3.1 for nt35516

static struct dsi_cmd_desc display_off_cmds[] = {
	{DTYPE_PERIPHERAL_OFF, 1, 0, 0, 40, sizeof(diplay_pixel_off), diplay_pixel_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 50, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep}
};
//wangminrong 2012 01 14 add yushun 5p qhd lcd for G01
static char yushun_nt35516_qHD_5p__para_0xFF[5]= {0xFF,0xAA,0x55,0x25,0x01};
static char yushun_nt35516_qHD_5p__para_0xF2[36]= {0xF2,0x00,0x00,0x4A,0x0A,0xA8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x01,0x51,0x00,0x01,0x00,0x01,};
static char yushun_nt35516_qHD_5p__para_0xF3[8]= {0xF3,0x02,0x03,0x07,0x45,0x88,0xD1,0x0D,};
static char yushun_nt35516_qHD_5p__para_0xF0[6]= {0xF0,0x55,0xAA,0x52,0x08,0x00,};
static char yushun_nt35516_qHD_5p__para_0xB1[4]= {0xB1,0xFC,0x00,0x00,};
static char yushun_nt35516_qHD_5p__para_0xB8[5]= {0xB8,0x01,0x02,0x02,0x02};
static char yushun_nt35516_qHD_5p__para_0xBC_1[4]= {0xBC,0x00,0x00,0x00};//1.17
static char yushun_nt35516_qHD_5p__para_0xC9[7]= {0xC9,0x63,0x06,0x0D,0x1A,0x17,0x00};
static char yushun_nt35516_qHD_5p__para_0xF0_2[6]= {0xF0,0x55,0xAA,0x52,0x08,0x01};
static char yushun_nt35516_qHD_5p__para_0xB0[4]= {0xB0,0x05,0x05,0x05,};
static char yushun_nt35516_qHD_5p__para_0xB1_2[4]= {0xB1,0x05,0x05,0x05,};
static char yushun_nt35516_qHD_5p__para_0xB2[4]= {0xB2,0x01,0x01,0x01,};
static char yushun_nt35516_qHD_5p__para_0xB3[4]= {0xB3,0x0D,0x0D,0x0D,};// change 
static char yushun_nt35516_qHD_5p__para_0xB4[4]= {0xB4,0x09,0x09,0x09,};//change 
static char yushun_nt35516_qHD_5p__para_0xB6[4]= {0xB6,0x44,0x44,0x44,};
static char yushun_nt35516_qHD_5p__para_0xB7[4]= {0xB7,0x34,0x34,0x34,};
static char yushun_nt35516_qHD_5p__para_0xB8_2[4]= {0xB8,0x10,0x10,0x10,};
static char yushun_nt35516_qHD_5p__para_0xB9[4]= {0xB9,0x26,0x26,0x26,};
static char yushun_nt35516_qHD_5p__para_0xBA[4]= {0xBA,0x34,0x34,0x34,};
static char yushun_nt35516_qHD_5p__para_0xBC[4]= {0xBC,0x00,0xB5,0x00,}; //change
static char yushun_nt35516_qHD_5p__para_0xBD[4]= {0xBD,0x00,0xD2,0x00,}; //change
static char yushun_nt35516_qHD_5p__para_0xBE[2]= {0xBE,0x57,}; //change //wangminrong change the flick 2013.2.18
static char yushun_nt35516_qHD_5p__para_0xC0[3]= {0xC0,0x04,0x00}; //11 //change 2013.3.20
static char yushun_nt35516_qHD_5p__para_0xCA[2]= {0xCA,0x00}; //11 change 2013.3.20
static char yushun_nt35516_qHD_5p__para_0xD0[5]= {0xD0,0x06,0x10,0x0D,0x0F,};//change 
static char yushun_nt35516_qHD_5p__para_0xD1[17]= {0xD1, 0x00, 0xA7, 0x00, 0xF0, 0x01, 0x18, 0x01, 0x30, 0x01, 0x40, 0x01, 0x5F, 0x01, 0x70, 0x01, 0x9E};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xD2[17]= {0xD2, 0x01, 0xBA, 0x01, 0xE7, 0x02, 0x0A, 0x02, 0x42, 0x02, 0x6F, 0x02, 0x70, 0x02, 0x9B, 0x02, 0xCC};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xD3[17]= {0xD3, 0x02, 0xE8, 0x03, 0x19, 0x03, 0x36, 0x03, 0x5E, 0x03, 0x78, 0x03, 0x97, 0x03, 0xA6, 0x03, 0xB3};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xD4[5]= {0xD4,0x03,0xD0,0x03,0xCF,};
static char yushun_nt35516_qHD_5p__para_0xD5[17]= {0xD5, 0x00, 0xA7, 0x01, 0x23, 0x01, 0x48, 0x01, 0x5F, 0x01, 0x6E, 0x01, 0x8B, 0x01, 0x9B, 0x01, 0xC5};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xD6[17]= {0xD6, 0x01, 0xE0, 0x02, 0x0A, 0x02, 0x2B, 0x02, 0x5F, 0x02, 0x89, 0x02, 0x8A, 0x02, 0xB2, 0x02, 0xE0};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xD7[17]= {0xD7, 0x02, 0xFA, 0x03, 0x28, 0x03, 0x43, 0x03, 0x68, 0x03, 0x81, 0x03, 0x9E, 0x03, 0xAC, 0x03, 0xB8};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xD8[5]= {0xD8,0x03,0xD0,0x03,0xCF,};
static char yushun_nt35516_qHD_5p__para_0xD9[17]= {0xD9, 0x00, 0xA7, 0x00, 0xF0, 0x01, 0x18, 0x01, 0x30, 0x01, 0x40, 0x01, 0x5F, 0x01, 0x70, 0x01, 0x9E};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xDD[17]= {0xDD, 0x01, 0xBA, 0x01, 0xE7, 0x02, 0x0A, 0x02, 0x42, 0x02, 0x6F, 0x02, 0x70, 0x02, 0x9B, 0x02, 0xCC};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xDE[17]= {0xDE, 0x02, 0xE8, 0x03, 0x19, 0x03, 0x36, 0x03, 0x5E, 0x03, 0x78, 0x03, 0x97, 0x03, 0xA6, 0x03, 0xB3};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xDF[5]= {0xDF,0x03,0xD0,0x03,0xCF,};
static char yushun_nt35516_qHD_5p__para_0xE0[17]= {0xE0, 0x00, 0xA7, 0x01, 0x23, 0x01, 0x48, 0x01, 0x5F, 0x01, 0x6E, 0x01, 0x8B, 0x01, 0x9B, 0x01, 0xC5};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xE1[17]= {0xE1, 0x01, 0xE0, 0x02, 0x0A, 0x02, 0x2B, 0x02, 0x5F, 0x02, 0x89, 0x02, 0x8A, 0x02, 0xB2, 0x02, 0xE0};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xE2[17]= {0xE2, 0x02, 0xFA, 0x03, 0x28, 0x03, 0x43, 0x03, 0x68, 0x03, 0x81, 0x03, 0x9E, 0x03, 0xAC, 0x03, 0xB8};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xE3[5]= {0xE3,0x03,0xD0,0x03,0xCF,};
static char yushun_nt35516_qHD_5p__para_0xE4[17]= {0xE4, 0x00, 0xA7, 0x00, 0xF0, 0x01, 0x18, 0x01, 0x30, 0x01, 0x40, 0x01, 0x5F, 0x01, 0x70, 0x01, 0x9E};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xE5[17]= {0xE5, 0x01, 0xBA, 0x01, 0xE7, 0x02, 0x0A, 0x02, 0x42, 0x02, 0x6F, 0x02, 0x70, 0x02, 0x9B, 0x02, 0xCC};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xE6[17]= {0xE6, 0x02, 0xE8, 0x03, 0x19, 0x03, 0x36, 0x03, 0x5E, 0x03, 0x78, 0x03, 0x97, 0x03, 0xA6, 0x03, 0xB3};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xE7[5]= {0xE7,0x03,0xD0,0x03,0xCF,};
static char yushun_nt35516_qHD_5p__para_0xE8[17]= {0xE8, 0x00, 0xA7, 0x01, 0x23, 0x01, 0x48, 0x01, 0x5F, 0x01, 0x6E, 0x01, 0x8B, 0x01, 0x9B, 0x01, 0xC5};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xE9[17]= {0xE9, 0x01, 0xE0, 0x02, 0x0A, 0x02, 0x2B, 0x02, 0x5F, 0x02, 0x89, 0x02, 0x8A, 0x02, 0xB2, 0x02, 0xE0};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xEA[17]= {0xEA, 0x02, 0xFA, 0x03, 0x28, 0x03, 0x43, 0x03, 0x68, 0x03, 0x81, 0x03, 0x9E, 0x03, 0xAC, 0x03, 0xB8};//zhangqi modify 20130502
static char yushun_nt35516_qHD_5p__para_0xEB[5]= {0xEB,0x03,0xD0,0x03,0xCF,};
static char yushun_nt35516_qHD_5p__para_0x3A[2]= {0x3A,0x77};
static char yushun_nt35516_qHD_5p__para_0x11[2]= {0x11,0x00};
//static char yushun_nt35516_qHD_5p__para_0x2C[2]= {0x2C,0x00};
//static char yushun_nt35516_qHD_5p__para_0x13[2]= {0x13,0x00};
static char yushun_nt35516_qHD_5p__para_0x29[2]= {0x29,0x00};

static struct dsi_cmd_desc yushun_5p_qhd_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xFF),yushun_nt35516_qHD_5p__para_0xFF},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xF2),yushun_nt35516_qHD_5p__para_0xF2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xF3),yushun_nt35516_qHD_5p__para_0xF3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xF0),yushun_nt35516_qHD_5p__para_0xF0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xB1),yushun_nt35516_qHD_5p__para_0xB1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xB8),yushun_nt35516_qHD_5p__para_0xB8},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xBC_1),yushun_nt35516_qHD_5p__para_0xBC_1}, //01.17
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xC9),yushun_nt35516_qHD_5p__para_0xC9},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xF0_2),yushun_nt35516_qHD_5p__para_0xF0_2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xB0),yushun_nt35516_qHD_5p__para_0xB0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xB1_2),yushun_nt35516_qHD_5p__para_0xB1_2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xB2),yushun_nt35516_qHD_5p__para_0xB2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xB3),yushun_nt35516_qHD_5p__para_0xB3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xB4),yushun_nt35516_qHD_5p__para_0xB4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xB6),yushun_nt35516_qHD_5p__para_0xB6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xB7),yushun_nt35516_qHD_5p__para_0xB7},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xB8_2),yushun_nt35516_qHD_5p__para_0xB8_2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xB9),yushun_nt35516_qHD_5p__para_0xB9},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xBA),yushun_nt35516_qHD_5p__para_0xBA},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xBC),yushun_nt35516_qHD_5p__para_0xBC},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xBD),yushun_nt35516_qHD_5p__para_0xBD},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xBE),yushun_nt35516_qHD_5p__para_0xBE},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xC0),yushun_nt35516_qHD_5p__para_0xC0},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xCA),yushun_nt35516_qHD_5p__para_0xCA},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xD0),yushun_nt35516_qHD_5p__para_0xD0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xD1),yushun_nt35516_qHD_5p__para_0xD1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xD2),yushun_nt35516_qHD_5p__para_0xD2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xD3),yushun_nt35516_qHD_5p__para_0xD3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xD4),yushun_nt35516_qHD_5p__para_0xD4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xD5),yushun_nt35516_qHD_5p__para_0xD5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xD6),yushun_nt35516_qHD_5p__para_0xD6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xD7),yushun_nt35516_qHD_5p__para_0xD7},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xD8),yushun_nt35516_qHD_5p__para_0xD8},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xD9),yushun_nt35516_qHD_5p__para_0xD9},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xDD),yushun_nt35516_qHD_5p__para_0xDD},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xDE),yushun_nt35516_qHD_5p__para_0xDE},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xDF),yushun_nt35516_qHD_5p__para_0xDF},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xE0),yushun_nt35516_qHD_5p__para_0xE0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xE1),yushun_nt35516_qHD_5p__para_0xE1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xE2),yushun_nt35516_qHD_5p__para_0xE2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xE3),yushun_nt35516_qHD_5p__para_0xE3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xE4),yushun_nt35516_qHD_5p__para_0xE4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xE5),yushun_nt35516_qHD_5p__para_0xE5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xE6),yushun_nt35516_qHD_5p__para_0xE6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xE7),yushun_nt35516_qHD_5p__para_0xE7},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xE8),yushun_nt35516_qHD_5p__para_0xE8},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xE9),yushun_nt35516_qHD_5p__para_0xE9},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xEA),yushun_nt35516_qHD_5p__para_0xEA},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0xEB),yushun_nt35516_qHD_5p__para_0xEB},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0x3A),yushun_nt35516_qHD_5p__para_0x3A},
	{DTYPE_DCS_WRITE, 1, 0, 0, 200,sizeof(yushun_nt35516_qHD_5p__para_0x11),yushun_nt35516_qHD_5p__para_0x11},
//	{DTYPE_DCS_WRITE, 1, 0, 0, 200,sizeof(yushun_nt35516_qHD_5p__para_0x2C),yushun_nt35516_qHD_5p__para_0x2C},
//	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(yushun_nt35516_qHD_5p__para_0x13),yushun_nt35516_qHD_5p__para_0x13},
	{DTYPE_DCS_WRITE, 1, 0, 0, 100,sizeof(yushun_nt35516_qHD_5p__para_0x29),yushun_nt35516_qHD_5p__para_0x29},
	//{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	//{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on}
};



static char yassy_otm9608_qHD_5p_para0[2] = {0x00,0x00};
static char yassy_otm9608_qHD_5p_para1[4] = {0xFF,0x96,0x08,0x01};
static char yassy_otm9608_qHD_5p_para2[2] = {0x00,0x80};
static char yassy_otm9608_qHD_5p_para3[3] = {0xFF,0x96,0x08};
static char yassy_otm9608_qHD_5p_para4[2] = {0x00,0xB1};
static char yassy_otm9608_qHD_5p_para5[3] = {0xB0,0x03,0x06};
static char yassy_otm9608_qHD_5p_para6[2] = {0x00,0xB7};
static char yassy_otm9608_qHD_5p_para7[2] = {0xB0,0x10};
static char yassy_otm9608_qHD_5p_para8[2] = {0x00,0xC0};
static char yassy_otm9608_qHD_5p_para9[2] = {0xB0,0x55};
static char yassy_otm9608_qHD_5p_para10[2] = {0x00,0x00};
static char yassy_otm9608_qHD_5p_para11[2] = {0xA0,0x00};
static char yassy_otm9608_qHD_5p_para12[2] = {0x00,0x80};
static char yassy_otm9608_qHD_5p_para13[6] = {0xB3,0x00,0x00,0x00,0x21,0x00};
static char yassy_otm9608_qHD_5p_para14[2] = {0x00,0x92};
static char yassy_otm9608_qHD_5p_para15[2] = {0xB3,0x01};
static char yassy_otm9608_qHD_5p_para16[2] = {0x00,0xC0};
static char yassy_otm9608_qHD_5p_para17[2] = {0xB3,0x19};
static char yassy_otm9608_qHD_5p_para18[2] = {0x00,0x80};
static char yassy_otm9608_qHD_5p_para19[10] = {0xC0,0x00,0x48,0x00,0x10,0x10,0x00,0x47,0x1F,0x1F};
static char yassy_otm9608_qHD_5p_para20[2] = {0x00,0x92};
static char yassy_otm9608_qHD_5p_para21[5] = {0xC0,0x00,0x0E,0x00,0x11};
static char yassy_otm9608_qHD_5p_para22[2] = {0x00,0xA2};
static char yassy_otm9608_qHD_5p_para23[4] = {0xC0,0x01,0x10,0x00};
static char yassy_otm9608_qHD_5p_para24[2] = {0x00,0xB3};
static char yassy_otm9608_qHD_5p_para25[3] = {0xC0,0x00,0x50};
static char yassy_otm9608_qHD_5p_para26[2] = {0x00,0xB5};
static char yassy_otm9608_qHD_5p_para27[2] = {0xC0,0x48};
static char yassy_otm9608_qHD_5p_para28[2] = {0x00,0xE1};
static char yassy_otm9608_qHD_5p_para29[2] = {0xC0,0x9F};
static char yassy_otm9608_qHD_5p_para30[2] = {0x00,0x81};
static char yassy_otm9608_qHD_5p_para31[2] = {0xC1,0x66};
static char yassy_otm9608_qHD_5p_para32[2] = {0x00,0x86};
static char yassy_otm9608_qHD_5p_para33[3] = {0xC4,0x09,0x08};
static char yassy_otm9608_qHD_5p_para34[2] = {0x00,0xA0};
static char yassy_otm9608_qHD_5p_para35[16] = {0xC4,0x33,0x09,0x90,0x28,0x33,0x09,0x90,0x2B,0x09,0x90,0x2B,0x33,0x09,0x90,0x54};
static char yassy_otm9608_qHD_5p_para36[2] = {0x00,0x80};
static char yassy_otm9608_qHD_5p_para37[5] = {0xC5,0x08,0x00,0x90,0x11};
static char yassy_otm9608_qHD_5p_para38[2] = {0x00,0x90};
static char yassy_otm9608_qHD_5p_para39[8] = {0xC5,0x96,0x76,0x06,0x76,0x33,0x33,0x34};
static char yassy_otm9608_qHD_5p_para40[2] = {0x00,0xA0};
static char yassy_otm9608_qHD_5p_para41[8] = {0xC5,0x96,0x76,0x06,0x76,0x33,0x33,0x34};
static char yassy_otm9608_qHD_5p_para42[2] = {0x00,0xB0};
static char yassy_otm9608_qHD_5p_para43[3] = {0xC5,0x04,0x08};
static char yassy_otm9608_qHD_5p_para44[2] = {0x00,0x00};
static char yassy_otm9608_qHD_5p_para45[2] = {0xD0,0x01};
static char yassy_otm9608_qHD_5p_para46[2] = {0x00,0x00};
static char yassy_otm9608_qHD_5p_para47[3] = {0xD1,0x01,0x01};
static char yassy_otm9608_qHD_5p_para48[2] = {0x00,0x80};
static char yassy_otm9608_qHD_5p_para49[11] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_qHD_5p_para50[2] = {0x00,0x90};
static char yassy_otm9608_qHD_5p_para51[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_qHD_5p_para52[2] = {0x00,0xA0};
static char yassy_otm9608_qHD_5p_para53[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_qHD_5p_para54[2] = {0x00,0xB0};
static char yassy_otm9608_qHD_5p_para55[11] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_qHD_5p_para56[2] = {0x00,0xC0};
static char yassy_otm9608_qHD_5p_para57[16] = {0xCB,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x00};
static char yassy_otm9608_qHD_5p_para58[2] = {0x00,0xD0};
static char yassy_otm9608_qHD_5p_para59[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x04,0x04};
static char yassy_otm9608_qHD_5p_para60[2] = {0x00,0xE0};
static char yassy_otm9608_qHD_5p_para61[11] = {0xCB,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_qHD_5p_para62[2] = {0x00,0xF0};
static char yassy_otm9608_qHD_5p_para63[11] = {0xCB,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
static char yassy_otm9608_qHD_5p_para64[2] = {0x00,0x80};
static char yassy_otm9608_qHD_5p_para65[11] = {0xCC,0x00,0x00,0x25,0x26,0x02,0x06,0x00,0x00,0x0A,0x0E};
static char yassy_otm9608_qHD_5p_para66[2] = {0x00,0x90};
static char yassy_otm9608_qHD_5p_para67[16] = {0xCC,0x0C,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x25,0x26,0x01};
static char yassy_otm9608_qHD_5p_para68[2] = {0x00,0xA0};
static char yassy_otm9608_qHD_5p_para69[16] = {0xCC,0x05,0x00,0x00,0x09,0x0D,0x0B,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_qHD_5p_para70[2] = {0x00,0xB0};
static char yassy_otm9608_qHD_5p_para71[11] = {0xCC,0x00,0x00,0x26,0x25,0x05,0x01,0x00,0x00,0x0F,0x0B};
static char yassy_otm9608_qHD_5p_para72[2] = {0x00,0xC0};
static char yassy_otm9608_qHD_5p_para73[16] = {0xCC,0x0D,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x25,0x06};
static char yassy_otm9608_qHD_5p_para74[2] = {0x00,0xD0};
static char yassy_otm9608_qHD_5p_para75[16] = {0xCC,0x02,0x00,0x00,0x10,0x0C,0x0E,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_qHD_5p_para76[2] = {0x00,0x80};
static char yassy_otm9608_qHD_5p_para77[13] = {0xCE,0x86,0x03,0x28,0x85,0x03,0x28,0x00,0x0F,0x00,0x00,0x0F,0x00};
static char yassy_otm9608_qHD_5p_para78[2] = {0x00,0x90};
static char yassy_otm9608_qHD_5p_para79[15] = {0xCE,0x33,0xBF,0x28,0x33,0xC0,0x28,0xF0,0x00,0x00,0xF0,0x00,0x00,0x00,0x00};
static char yassy_otm9608_qHD_5p_para80[2] = {0x00,0xA0};
static char yassy_otm9608_qHD_5p_para81[15] = {0xCE,0x38,0x02,0x03,0xC1,0x86,0x18,0x00,0x38,0x01,0x03,0xC2,0x85,0x18,0x00};
static char yassy_otm9608_qHD_5p_para82[2] = {0x00,0xB0};
static char yassy_otm9608_qHD_5p_para83[15] = {0xCE,0x38,0x00,0x03,0xC3,0x86,0x18,0x00,0x30,0x00,0x03,0xC4,0x85,0x18,0x00};
static char yassy_otm9608_qHD_5p_para84[2] = {0x00,0xC0};
static char yassy_otm9608_qHD_5p_para85[15] = {0xCE,0x30,0x01,0x03,0xC5,0x86,0x18,0x00,0x30,0x02,0x03,0xC6,0x85,0x18,0x00};
static char yassy_otm9608_qHD_5p_para86[2] = {0x00,0xD0};
static char yassy_otm9608_qHD_5p_para87[15] = {0xCE,0x30,0x03,0x03,0xC7,0x86,0x18,0x00,0x30,0x04,0x03,0xC8,0x85,0x18,0x00};
static char yassy_otm9608_qHD_5p_para88[2] = {0x00,0x80};
static char yassy_otm9608_qHD_5p_para89[15] = {0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char yassy_otm9608_qHD_5p_para90[2] = {0x00,0x90};
static char yassy_otm9608_qHD_5p_para91[15] = {0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char yassy_otm9608_qHD_5p_para92[2] = {0x00,0xA0};
static char yassy_otm9608_qHD_5p_para93[15] = {0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char yassy_otm9608_qHD_5p_para94[2] = {0x00,0xB0};
static char yassy_otm9608_qHD_5p_para95[15] = {0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char yassy_otm9608_qHD_5p_para96[2] = {0x00,0xC0};
static char yassy_otm9608_qHD_5p_para97[11] = {0xCF,0x01,0x01,0x20,0x20,0x00,0x00,0x02,0x01,0x00,0x00};
static char yassy_otm9608_qHD_5p_para98[2] = {0x00,0x80};
static char yassy_otm9608_qHD_5p_para99[2] = {0xD6,0x00};
static char yassy_otm9608_qHD_5p_para100[2] = {0x00,0x00};
static char yassy_otm9608_qHD_5p_para101[2] = {0xD7,0x00};
static char yassy_otm9608_qHD_5p_para102[2] = {0x00,0x00};
static char yassy_otm9608_qHD_5p_para103[3] = {0xD8,0x7F,0x7F};
static char yassy_otm9608_qHD_5p_para104[2] = {0x00,0x00};
static char yassy_otm9608_qHD_5p_para105[2] = {0xD9,0x39};
static char yassy_otm9608_qHD_5p_para106[2] = {0x00,0x00};
static char yassy_otm9608_qHD_5p_para107[17] = {0xE1,0x01,0x07,0x0C,0x0D,0x05,0x11,0x0C,0x0B,0x02,0x06,0x08,0x07,0x0E,0x17,0x13,0x10};
static char yassy_otm9608_qHD_5p_para108[2] = {0x00,0x00};
static char yassy_otm9608_qHD_5p_para109[17] = {0xE2,0x01,0x07,0x0C,0x0D,0x06,0x11,0x0C,0x0B,0x02,0x06,0x09,0x07,0x0E,0x17,0x13,0x10};
static char yassy_otm9608_qHD_5p_para110[2] = {0x00,0x00};
static char yassy_otm9608_qHD_5p_para111[35] = {0xEC,0x40,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x00};
static char yassy_otm9608_qHD_5p_para112[2] = {0x00,0x00};
static char yassy_otm9608_qHD_5p_para113[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para114[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para115[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para116[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para117[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para118[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para119[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para120[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para121[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para122[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para123[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para124[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para125[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para126[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para127[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para128[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para129[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para130[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para131[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para132[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para133[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para134[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para135[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para136[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para137[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para138[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para139[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para140[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para141[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para142[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para143[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para144[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para145[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para146[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para147[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para148[11] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40};
static char yassy_otm9608_qHD_5p_para149[2] = {0x00,0x00};
static char yassy_otm9608_qHD_5p_para150[11] = {0xD5,0x00,0x60,0x00,0x60,0x00,0x5F,0x00,0x5F,0x00,0x5E};
static char yassy_otm9608_qHD_5p_para151[11] = {0xD5,0x00,0x5E,0x00,0x5D,0x00,0x5D,0x00,0x5D,0x00,0x5C};
static char yassy_otm9608_qHD_5p_para152[11] = {0xD5,0x00,0x5C,0x00,0x5B,0x00,0x5B,0x00,0x5A,0x00,0x5A};
static char yassy_otm9608_qHD_5p_para153[11] = {0xD5,0x00,0x5A,0x00,0x5B,0x00,0x5C,0x00,0x5D,0x00,0x5D};
static char yassy_otm9608_qHD_5p_para154[11] = {0xD5,0x00,0x5E,0x00,0x5F,0x00,0x60,0x00,0x61,0x00,0x62};
static char yassy_otm9608_qHD_5p_para155[11] = {0xD5,0x00,0x63,0x00,0x63,0x00,0x64,0x00,0x65,0x00,0x66};
static char yassy_otm9608_qHD_5p_para156[11] = {0xD5,0x00,0x67,0x00,0x68,0x00,0x69,0x00,0x69,0x00,0x6A};
static char yassy_otm9608_qHD_5p_para157[11] = {0xD5,0x00,0x6B,0x00,0x6C,0x00,0x6D,0x00,0x6E,0x00,0x6F};
static char yassy_otm9608_qHD_5p_para158[11] = {0xD5,0x00,0x6F,0x00,0x70,0x00,0x71,0x00,0x72,0x00,0x73};
static char yassy_otm9608_qHD_5p_para159[11] = {0xD5,0x00,0x74,0x00,0x74,0x00,0x75,0x00,0x76,0x00,0x77};
static char yassy_otm9608_qHD_5p_para160[11] = {0xD5,0x00,0x78,0x00,0x78,0x00,0x79,0x00,0x7A,0x00,0x7B};
static char yassy_otm9608_qHD_5p_para161[11] = {0xD5,0x00,0x7C,0x00,0x7D,0x00,0x7D,0x00,0x7E,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para162[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para163[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para164[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para165[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para166[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para167[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para168[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para169[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para170[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para171[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para172[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para173[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para174[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para175[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para176[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F,0x00,0x7F};
static char yassy_otm9608_qHD_5p_para177[11] = {0xD5,0x00,0x7F,0x00,0x7F,0x00,0x7E,0x00,0x7D,0x00,0x7C};
static char yassy_otm9608_qHD_5p_para178[11] = {0xD5,0x00,0x7B,0x00,0x7A,0x00,0x7A,0x00,0x79,0x00,0x78};
static char yassy_otm9608_qHD_5p_para179[11] = {0xD5,0x00,0x77,0x00,0x76,0x00,0x76,0x00,0x75,0x00,0x74};
static char yassy_otm9608_qHD_5p_para180[11] = {0xD5,0x00,0x73,0x00,0x72,0x00,0x71,0x00,0x71,0x00,0x70};
static char yassy_otm9608_qHD_5p_para181[11] = {0xD5,0x00,0x6F,0x00,0x6E,0x00,0x6D,0x00,0x6C,0x00,0x6C};
static char yassy_otm9608_qHD_5p_para182[11] = {0xD5,0x00,0x6B,0x00,0x6A,0x00,0x69,0x00,0x68,0x00,0x67};
static char yassy_otm9608_qHD_5p_para183[11] = {0xD5,0x00,0x66,0x00,0x66,0x00,0x66,0x00,0x65,0x00,0x65};
static char yassy_otm9608_qHD_5p_para184[11] = {0xD5,0x00,0x64,0x00,0x64,0x00,0x63,0x00,0x63,0x00,0x63};
static char yassy_otm9608_qHD_5p_para185[11] = {0xD5,0x00,0x62,0x00,0x62,0x00,0x61,0x00,0x61,0x00,0x60};
static char yassy_otm9608_qHD_5p_para186[2] = {0x00,0x80};
static char yassy_otm9608_qHD_5p_para187[2] = {0xD6,0x08};
static char yassy_otm9608_qHD_5p_para188[2] = {0x00,0x00};
static char yassy_otm9608_qHD_5p_para189[4] = {0xFF,0xFF,0xFF,0xFF};
static char yassy_otm9608_qHD_5p_para190[2] = {0x36,0x00};
static char yassy_otm9608_qHD_5p_para191[2] = {0x11,0x00};
static char yassy_otm9608_qHD_5p_para192[2] = {0x29,0x00};
	static struct dsi_cmd_desc yassy_5p_qhd_display_on_cmds[] = {
{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para0),yassy_otm9608_qHD_5p_para0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para1),yassy_otm9608_qHD_5p_para1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para2),yassy_otm9608_qHD_5p_para2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para3),yassy_otm9608_qHD_5p_para3},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para4),yassy_otm9608_qHD_5p_para4},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para5),yassy_otm9608_qHD_5p_para5},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para6),yassy_otm9608_qHD_5p_para6},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para7),yassy_otm9608_qHD_5p_para7},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para8),yassy_otm9608_qHD_5p_para8},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para9),yassy_otm9608_qHD_5p_para9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para10),yassy_otm9608_qHD_5p_para10},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para11),yassy_otm9608_qHD_5p_para11},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para12),yassy_otm9608_qHD_5p_para12},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para13),yassy_otm9608_qHD_5p_para13},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para14),yassy_otm9608_qHD_5p_para14},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para15),yassy_otm9608_qHD_5p_para15},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para16),yassy_otm9608_qHD_5p_para16},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para17),yassy_otm9608_qHD_5p_para17},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para18),yassy_otm9608_qHD_5p_para18},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para19),yassy_otm9608_qHD_5p_para19},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para20),yassy_otm9608_qHD_5p_para20},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para21),yassy_otm9608_qHD_5p_para21},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para22),yassy_otm9608_qHD_5p_para22},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para23),yassy_otm9608_qHD_5p_para23},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para24),yassy_otm9608_qHD_5p_para24},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para25),yassy_otm9608_qHD_5p_para25},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para26),yassy_otm9608_qHD_5p_para26},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para27),yassy_otm9608_qHD_5p_para27},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para28),yassy_otm9608_qHD_5p_para28},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para29),yassy_otm9608_qHD_5p_para29},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para30),yassy_otm9608_qHD_5p_para30},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para31),yassy_otm9608_qHD_5p_para31},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para32),yassy_otm9608_qHD_5p_para32},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para33),yassy_otm9608_qHD_5p_para33},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para34),yassy_otm9608_qHD_5p_para34},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para35),yassy_otm9608_qHD_5p_para35},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para36),yassy_otm9608_qHD_5p_para36},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para37),yassy_otm9608_qHD_5p_para37},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para38),yassy_otm9608_qHD_5p_para38},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para39),yassy_otm9608_qHD_5p_para39},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para40),yassy_otm9608_qHD_5p_para40},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para41),yassy_otm9608_qHD_5p_para41},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para42),yassy_otm9608_qHD_5p_para42},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para43),yassy_otm9608_qHD_5p_para43},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para44),yassy_otm9608_qHD_5p_para44},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para45),yassy_otm9608_qHD_5p_para45},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para46),yassy_otm9608_qHD_5p_para46},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para47),yassy_otm9608_qHD_5p_para47},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para48),yassy_otm9608_qHD_5p_para48},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para49),yassy_otm9608_qHD_5p_para49},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para50),yassy_otm9608_qHD_5p_para50},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para51),yassy_otm9608_qHD_5p_para51},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para52),yassy_otm9608_qHD_5p_para52},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para53),yassy_otm9608_qHD_5p_para53},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para54),yassy_otm9608_qHD_5p_para54},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para55),yassy_otm9608_qHD_5p_para55},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para56),yassy_otm9608_qHD_5p_para56},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para57),yassy_otm9608_qHD_5p_para57},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para58),yassy_otm9608_qHD_5p_para58},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para59),yassy_otm9608_qHD_5p_para59},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para60),yassy_otm9608_qHD_5p_para60},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para61),yassy_otm9608_qHD_5p_para61},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para62),yassy_otm9608_qHD_5p_para62},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para63),yassy_otm9608_qHD_5p_para63},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para64),yassy_otm9608_qHD_5p_para64},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para65),yassy_otm9608_qHD_5p_para65},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para66),yassy_otm9608_qHD_5p_para66},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para67),yassy_otm9608_qHD_5p_para67},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para68),yassy_otm9608_qHD_5p_para68},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para69),yassy_otm9608_qHD_5p_para69},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para70),yassy_otm9608_qHD_5p_para70},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para71),yassy_otm9608_qHD_5p_para71},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para72),yassy_otm9608_qHD_5p_para72},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para73),yassy_otm9608_qHD_5p_para73},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para74),yassy_otm9608_qHD_5p_para74},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para75),yassy_otm9608_qHD_5p_para75},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para76),yassy_otm9608_qHD_5p_para76},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para77),yassy_otm9608_qHD_5p_para77},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para78),yassy_otm9608_qHD_5p_para78},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para79),yassy_otm9608_qHD_5p_para79},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para80),yassy_otm9608_qHD_5p_para80},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para81),yassy_otm9608_qHD_5p_para81},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para82),yassy_otm9608_qHD_5p_para82},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para83),yassy_otm9608_qHD_5p_para83},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para84),yassy_otm9608_qHD_5p_para84},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para85),yassy_otm9608_qHD_5p_para85},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para86),yassy_otm9608_qHD_5p_para86},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para87),yassy_otm9608_qHD_5p_para87},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para88),yassy_otm9608_qHD_5p_para88},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para89),yassy_otm9608_qHD_5p_para89},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para90),yassy_otm9608_qHD_5p_para90},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para91),yassy_otm9608_qHD_5p_para91},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para92),yassy_otm9608_qHD_5p_para92},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para93),yassy_otm9608_qHD_5p_para93},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para94),yassy_otm9608_qHD_5p_para94},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para95),yassy_otm9608_qHD_5p_para95},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para96),yassy_otm9608_qHD_5p_para96},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para97),yassy_otm9608_qHD_5p_para97},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para98),yassy_otm9608_qHD_5p_para98},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para99),yassy_otm9608_qHD_5p_para99},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para100),yassy_otm9608_qHD_5p_para100},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para101),yassy_otm9608_qHD_5p_para101},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para102),yassy_otm9608_qHD_5p_para102},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para103),yassy_otm9608_qHD_5p_para103},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para104),yassy_otm9608_qHD_5p_para104},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para105),yassy_otm9608_qHD_5p_para105},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para106),yassy_otm9608_qHD_5p_para106},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para107),yassy_otm9608_qHD_5p_para107},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para108),yassy_otm9608_qHD_5p_para108},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para109),yassy_otm9608_qHD_5p_para109},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para110),yassy_otm9608_qHD_5p_para110},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para111),yassy_otm9608_qHD_5p_para111},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para112),yassy_otm9608_qHD_5p_para112},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para113),yassy_otm9608_qHD_5p_para113},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para114),yassy_otm9608_qHD_5p_para114},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para115),yassy_otm9608_qHD_5p_para115},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para116),yassy_otm9608_qHD_5p_para116},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para117),yassy_otm9608_qHD_5p_para117},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para118),yassy_otm9608_qHD_5p_para118},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para119),yassy_otm9608_qHD_5p_para119},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para120),yassy_otm9608_qHD_5p_para120},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para121),yassy_otm9608_qHD_5p_para121},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para122),yassy_otm9608_qHD_5p_para122},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para123),yassy_otm9608_qHD_5p_para123},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para124),yassy_otm9608_qHD_5p_para124},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para125),yassy_otm9608_qHD_5p_para125},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para126),yassy_otm9608_qHD_5p_para126},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para127),yassy_otm9608_qHD_5p_para127},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para128),yassy_otm9608_qHD_5p_para128},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para129),yassy_otm9608_qHD_5p_para129},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para130),yassy_otm9608_qHD_5p_para130},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para131),yassy_otm9608_qHD_5p_para131},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para132),yassy_otm9608_qHD_5p_para132},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para133),yassy_otm9608_qHD_5p_para133},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para134),yassy_otm9608_qHD_5p_para134},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para135),yassy_otm9608_qHD_5p_para135},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para136),yassy_otm9608_qHD_5p_para136},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para137),yassy_otm9608_qHD_5p_para137},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para138),yassy_otm9608_qHD_5p_para138},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para139),yassy_otm9608_qHD_5p_para139},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para140),yassy_otm9608_qHD_5p_para140},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para141),yassy_otm9608_qHD_5p_para141},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para142),yassy_otm9608_qHD_5p_para142},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para143),yassy_otm9608_qHD_5p_para143},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para144),yassy_otm9608_qHD_5p_para144},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para145),yassy_otm9608_qHD_5p_para145},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para146),yassy_otm9608_qHD_5p_para146},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para147),yassy_otm9608_qHD_5p_para147},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para148),yassy_otm9608_qHD_5p_para148},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para149),yassy_otm9608_qHD_5p_para149},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para150),yassy_otm9608_qHD_5p_para150},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para151),yassy_otm9608_qHD_5p_para151},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para152),yassy_otm9608_qHD_5p_para152},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para153),yassy_otm9608_qHD_5p_para153},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para154),yassy_otm9608_qHD_5p_para154},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para155),yassy_otm9608_qHD_5p_para155},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para156),yassy_otm9608_qHD_5p_para156},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para157),yassy_otm9608_qHD_5p_para157},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para158),yassy_otm9608_qHD_5p_para158},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para159),yassy_otm9608_qHD_5p_para159},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para160),yassy_otm9608_qHD_5p_para160},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para161),yassy_otm9608_qHD_5p_para161},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para162),yassy_otm9608_qHD_5p_para162},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para163),yassy_otm9608_qHD_5p_para163},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para164),yassy_otm9608_qHD_5p_para164},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para165),yassy_otm9608_qHD_5p_para165},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para166),yassy_otm9608_qHD_5p_para166},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para167),yassy_otm9608_qHD_5p_para167},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para168),yassy_otm9608_qHD_5p_para168},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para169),yassy_otm9608_qHD_5p_para169},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para170),yassy_otm9608_qHD_5p_para170},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para171),yassy_otm9608_qHD_5p_para171},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para172),yassy_otm9608_qHD_5p_para172},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para173),yassy_otm9608_qHD_5p_para173},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para174),yassy_otm9608_qHD_5p_para174},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para175),yassy_otm9608_qHD_5p_para175},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para176),yassy_otm9608_qHD_5p_para176},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para177),yassy_otm9608_qHD_5p_para177},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para178),yassy_otm9608_qHD_5p_para178},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para179),yassy_otm9608_qHD_5p_para179},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para180),yassy_otm9608_qHD_5p_para180},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para181),yassy_otm9608_qHD_5p_para181},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para182),yassy_otm9608_qHD_5p_para182},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para183),yassy_otm9608_qHD_5p_para183},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para184),yassy_otm9608_qHD_5p_para184},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para185),yassy_otm9608_qHD_5p_para185},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para186),yassy_otm9608_qHD_5p_para186},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para187),yassy_otm9608_qHD_5p_para187},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para188),yassy_otm9608_qHD_5p_para188},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para189),yassy_otm9608_qHD_5p_para189},	
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(yassy_otm9608_qHD_5p_para190),yassy_otm9608_qHD_5p_para190},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,sizeof(yassy_otm9608_qHD_5p_para191),yassy_otm9608_qHD_5p_para191},
	{DTYPE_DCS_WRITE, 1, 0, 0, 50,sizeof(yassy_otm9608_qHD_5p_para192),yassy_otm9608_qHD_5p_para192},
	};
	


/********************haoweiwei 20130426 add start**********************************/
static char boe_otm9608_qHD_5p_param0[2] = {0x00,0x00};
static char boe_otm9608_qHD_5p_param1[4] = {0xFF,0x96,0x08,0x01};
static char boe_otm9608_qHD_5p_param2[2] = {0x00,0x80};
static char boe_otm9608_qHD_5p_param3[3] = {0xFF,0x96,0x08};
static char boe_otm9608_qHD_5p_param4[2] = {0x00,0x03};
static char boe_otm9608_qHD_5p_param5[2] = {0xFF,0x01};
static char boe_otm9608_qHD_5p_param6[2] = {0x00,0x00};
static char boe_otm9608_qHD_5p_param7[2] = {0xA0,0x00};
static char boe_otm9608_qHD_5p_param8[2] = {0x00,0x80};
static char boe_otm9608_qHD_5p_param9[6] = {0xB3,0x00,0x00,0x00,0x21,0x00};
static char boe_otm9608_qHD_5p_param10[2] = {0x00,0x92};
static char boe_otm9608_qHD_5p_param11[2] = {0xB3,0x01};
static char boe_otm9608_qHD_5p_param12[2] = {0x00,0xC0};
static char boe_otm9608_qHD_5p_param13[2] = {0xB3,0x19};
static char boe_otm9608_qHD_5p_param14[2] = {0x00,0xA6};
static char boe_otm9608_qHD_5p_param15[2] = {0xB3,0x20};
static char boe_otm9608_qHD_5p_param16[2] = {0x00,0x92};
static char boe_otm9608_qHD_5p_param17[5] = {0xC0,0x00,0x0E,0x00,0x11};
static char boe_otm9608_qHD_5p_param18[2] = {0x00,0xA2};
static char boe_otm9608_qHD_5p_param19[4] = {0xC0,0x01,0x10,0x00};
static char boe_otm9608_qHD_5p_param20[2] = {0x00,0xB3};
static char boe_otm9608_qHD_5p_param21[3] = {0xC0,0x00,0x50};
static char boe_otm9608_qHD_5p_param22[2] = {0x00,0x81};
static char boe_otm9608_qHD_5p_param23[2] = {0xC1,0x77}; //haoweiwei up fps to 70Hz
static char boe_otm9608_qHD_5p_param24[2] = {0x00,0x80};
static char boe_otm9608_qHD_5p_param25[4] = {0xC4,0x00,0x84,0xFA};
static char boe_otm9608_qHD_5p_param26[2] = {0x00,0xA0};
static char boe_otm9608_qHD_5p_param27[9] = {0xC4,0x33,0x09,0x90,0x2B,0x33,0x09,0x90,0x54};
static char boe_otm9608_qHD_5p_param28[2] = {0x00,0x80};
static char boe_otm9608_qHD_5p_param29[5] = {0xC5,0x08,0x00,0x90,0x11};
static char boe_otm9608_qHD_5p_param30[2] = {0x00,0x90};
static char boe_otm9608_qHD_5p_param31[8] = {0xC5,0x84,0x4A,0x00,0x4A,0x44,0x44,0x34};
static char boe_otm9608_qHD_5p_param32[2] = {0x00,0xA0};
static char boe_otm9608_qHD_5p_param33[8] = {0xC5,0x96,0x4A,0x06,0x4A,0x33,0x33,0x34};
static char boe_otm9608_qHD_5p_param34[2] = {0x00,0x80};
static char boe_otm9608_qHD_5p_param35[2] = {0xC6,0x64};
static char boe_otm9608_qHD_5p_param36[2] = {0x00,0xB0};
static char boe_otm9608_qHD_5p_param37[3] = {0xC5,0x04,0xF8};
static char boe_otm9608_qHD_5p_param38[2] = {0x00,0xB0};
static char boe_otm9608_qHD_5p_param39[6] = {0xC6,0x03,0x10,0x00,0x1F,0x12};
static char boe_otm9608_qHD_5p_param40[2] = {0x00,0xE1};
static char boe_otm9608_qHD_5p_param41[2] = {0xC0,0x9F};
static char boe_otm9608_qHD_5p_param42[2] = {0x00,0xB7};
static char boe_otm9608_qHD_5p_param43[2] = {0xB0,0x10};
static char boe_otm9608_qHD_5p_param44[2] = {0x00,0xC0};
static char boe_otm9608_qHD_5p_param45[2] = {0xB0,0x55};
static char boe_otm9608_qHD_5p_param46[2] = {0x00,0xB1};
static char boe_otm9608_qHD_5p_param47[3] = {0xB0,0x03,0x06};
static char boe_otm9608_qHD_5p_param48[2] = {0x00,0x00};
static char boe_otm9608_qHD_5p_param49[2] = {0xD7,0x00};
static char boe_otm9608_qHD_5p_param50[2] = {0x00,0x00};
static char boe_otm9608_qHD_5p_param51[3] = {0xD8,0x5F,0x5F};
static char boe_otm9608_qHD_5p_param52[2] = {0x00,0x00};
static char boe_otm9608_qHD_5p_param53[2] = {0xD9,0x39};
static char boe_otm9608_qHD_5p_param54[2] = {0x00,0x00};
static char boe_otm9608_qHD_5p_param55[17] = {0xE1,0x09,0x0F,0x13,0x0D,0x05,0x0C,0x0B,0x09,0x04,0x07,0x10,0x09,0x10,0x14,0x0E,0x08};
static char boe_otm9608_qHD_5p_param56[2] = {0x00,0x00};
static char boe_otm9608_qHD_5p_param57[17] = {0xE2,0x09,0x0F,0x13,0x0D,0x05,0x0C,0x0B,0x09,0x04,0x07,0x10,0x09,0x10,0x14,0x0E,0x08};
static char boe_otm9608_qHD_5p_param58[2] = {0x00,0x80};
static char boe_otm9608_qHD_5p_param59[13] = {0xCE,0x85,0x01,0x06,0x84,0x01,0x06,0x0F,0x00,0x00,0x0F,0x00,0x00};
static char boe_otm9608_qHD_5p_param60[2] = {0x00,0x90};
static char boe_otm9608_qHD_5p_param61[15] = {0xCE,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0x0F,0x00,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param62[2] = {0x00,0xA0};
static char boe_otm9608_qHD_5p_param63[15] = {0xCE,0x18,0x04,0x03,0xC1,0x00,0x00,0x10,0x18,0x03,0x03,0xC2,0x00,0x00,0x10};
static char boe_otm9608_qHD_5p_param64[2] = {0x00,0xB0};
static char boe_otm9608_qHD_5p_param65[15] = {0xCE,0x18,0x02,0x03,0xC3,0x00,0x00,0x10,0x18,0x01,0x03,0xC4,0x00,0x00,0x10};
static char boe_otm9608_qHD_5p_param66[2] = {0x00,0xC0};
static char boe_otm9608_qHD_5p_param67[15] = {0xCE,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param68[2] = {0x00,0xD0};
static char boe_otm9608_qHD_5p_param69[15] = {0xCE,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param70[2] = {0x00,0x80};
static char boe_otm9608_qHD_5p_param71[15] = {0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param72[2] = {0x00,0x90};
static char boe_otm9608_qHD_5p_param73[15] = {0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param74[2] = {0x00,0xA0};
static char boe_otm9608_qHD_5p_param75[15] = {0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param76[2] = {0x00,0xB0};
static char boe_otm9608_qHD_5p_param77[15] = {0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param78[2] = {0x00,0xC0};
static char boe_otm9608_qHD_5p_param79[11] = {0xCF,0x02,0x02,0x10,0x10,0x00,0x00,0x01,0x01,0x00,0x00};
static char boe_otm9608_qHD_5p_param80[2] = {0x00,0x80};
static char boe_otm9608_qHD_5p_param81[11] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param82[2] = {0x00,0x90};
static char boe_otm9608_qHD_5p_param83[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param84[2] = {0x00,0xA0};
static char boe_otm9608_qHD_5p_param85[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param86[2] = {0x00,0xB0};
static char boe_otm9608_qHD_5p_param87[11] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param88[2] = {0x00,0xC0};
static char boe_otm9608_qHD_5p_param89[16] = {0xCB,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param90[2] = {0x00,0xD0};
static char boe_otm9608_qHD_5p_param91[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param92[2] = {0x00,0xE0};
static char boe_otm9608_qHD_5p_param93[11] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param94[2] = {0x00,0xF0};
static char boe_otm9608_qHD_5p_param95[11] = {0xCB,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static char boe_otm9608_qHD_5p_param96[2] = {0x00,0x80};
static char boe_otm9608_qHD_5p_param97[11] = {0xCC,0x00,0x00,0x09,0x0B,0x01,0x25,0x26,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param98[2] = {0x00,0x90};
static char boe_otm9608_qHD_5p_param99[16] = {0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x0C,0x02};
static char boe_otm9608_qHD_5p_param100[2] = {0x00,0xA0};
static char boe_otm9608_qHD_5p_param101[16] = {0xCC,0x25,0x26,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param102[2] = {0x00,0xB0};
static char boe_otm9608_qHD_5p_param103[11] = {0xCC,0x00,0x00,0x0C,0x0A,0x02,0x26,0x25,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param104[2] = {0x00,0xC0};
static char boe_otm9608_qHD_5p_param105[16] = {0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x09,0x01};
static char boe_otm9608_qHD_5p_param106[2] = {0x00,0xD0};
static char boe_otm9608_qHD_5p_param107[16] = {0xCC,0x26,0x25,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char boe_otm9608_qHD_5p_param108[2] = {0x11,0x00};
static char boe_otm9608_qHD_5p_param109[2] = {0x29,0x00};

static struct dsi_cmd_desc boe_5p_qhd_display_on_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param0),boe_otm9608_qHD_5p_param0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param1),boe_otm9608_qHD_5p_param1},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param2),boe_otm9608_qHD_5p_param2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param3),boe_otm9608_qHD_5p_param3},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param4),boe_otm9608_qHD_5p_param4},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param5),boe_otm9608_qHD_5p_param5},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param6),boe_otm9608_qHD_5p_param6},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param7),boe_otm9608_qHD_5p_param7},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param8),boe_otm9608_qHD_5p_param8},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param9),boe_otm9608_qHD_5p_param9},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param10),boe_otm9608_qHD_5p_param10},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param11),boe_otm9608_qHD_5p_param11},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param12),boe_otm9608_qHD_5p_param12},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param13),boe_otm9608_qHD_5p_param13},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param14),boe_otm9608_qHD_5p_param14},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param15),boe_otm9608_qHD_5p_param15},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param16),boe_otm9608_qHD_5p_param16},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param17),boe_otm9608_qHD_5p_param17},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param18),boe_otm9608_qHD_5p_param18},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param19),boe_otm9608_qHD_5p_param19},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param20),boe_otm9608_qHD_5p_param20},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param21),boe_otm9608_qHD_5p_param21},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param22),boe_otm9608_qHD_5p_param22},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param23),boe_otm9608_qHD_5p_param23},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param24),boe_otm9608_qHD_5p_param24},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param25),boe_otm9608_qHD_5p_param25},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param26),boe_otm9608_qHD_5p_param26},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param27),boe_otm9608_qHD_5p_param27},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param28),boe_otm9608_qHD_5p_param28},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param29),boe_otm9608_qHD_5p_param29},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param30),boe_otm9608_qHD_5p_param30},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param31),boe_otm9608_qHD_5p_param31},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param32),boe_otm9608_qHD_5p_param32},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param33),boe_otm9608_qHD_5p_param33},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param34),boe_otm9608_qHD_5p_param34},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param35),boe_otm9608_qHD_5p_param35},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param36),boe_otm9608_qHD_5p_param36},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param37),boe_otm9608_qHD_5p_param37},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param38),boe_otm9608_qHD_5p_param38},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param39),boe_otm9608_qHD_5p_param39},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param40),boe_otm9608_qHD_5p_param40},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param41),boe_otm9608_qHD_5p_param41},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param42),boe_otm9608_qHD_5p_param42},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param43),boe_otm9608_qHD_5p_param43},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param44),boe_otm9608_qHD_5p_param44},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param45),boe_otm9608_qHD_5p_param45},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param46),boe_otm9608_qHD_5p_param46},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param47),boe_otm9608_qHD_5p_param47},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param48),boe_otm9608_qHD_5p_param48},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param49),boe_otm9608_qHD_5p_param49},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param50),boe_otm9608_qHD_5p_param50},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param51),boe_otm9608_qHD_5p_param51},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param52),boe_otm9608_qHD_5p_param52},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param53),boe_otm9608_qHD_5p_param53},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param54),boe_otm9608_qHD_5p_param54},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param55),boe_otm9608_qHD_5p_param55},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param56),boe_otm9608_qHD_5p_param56},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param57),boe_otm9608_qHD_5p_param57},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param58),boe_otm9608_qHD_5p_param58},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param59),boe_otm9608_qHD_5p_param59},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param60),boe_otm9608_qHD_5p_param60},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param61),boe_otm9608_qHD_5p_param61},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param62),boe_otm9608_qHD_5p_param62},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param63),boe_otm9608_qHD_5p_param63},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param64),boe_otm9608_qHD_5p_param64},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param65),boe_otm9608_qHD_5p_param65},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param66),boe_otm9608_qHD_5p_param66},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param67),boe_otm9608_qHD_5p_param67},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param68),boe_otm9608_qHD_5p_param68},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param69),boe_otm9608_qHD_5p_param69},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param70),boe_otm9608_qHD_5p_param70},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param71),boe_otm9608_qHD_5p_param71},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param72),boe_otm9608_qHD_5p_param72},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param73),boe_otm9608_qHD_5p_param73},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param74),boe_otm9608_qHD_5p_param74},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param75),boe_otm9608_qHD_5p_param75},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param76),boe_otm9608_qHD_5p_param76},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param77),boe_otm9608_qHD_5p_param77},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param78),boe_otm9608_qHD_5p_param78},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param79),boe_otm9608_qHD_5p_param79},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param80),boe_otm9608_qHD_5p_param80},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param81),boe_otm9608_qHD_5p_param81},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param82),boe_otm9608_qHD_5p_param82},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param83),boe_otm9608_qHD_5p_param83},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param84),boe_otm9608_qHD_5p_param84},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param85),boe_otm9608_qHD_5p_param85},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param86),boe_otm9608_qHD_5p_param86},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param87),boe_otm9608_qHD_5p_param87},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param88),boe_otm9608_qHD_5p_param88},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param89),boe_otm9608_qHD_5p_param89},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param90),boe_otm9608_qHD_5p_param90},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param91),boe_otm9608_qHD_5p_param91},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param92),boe_otm9608_qHD_5p_param92},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param93),boe_otm9608_qHD_5p_param93},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param94),boe_otm9608_qHD_5p_param94},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param95),boe_otm9608_qHD_5p_param95},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param96),boe_otm9608_qHD_5p_param96},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param97),boe_otm9608_qHD_5p_param97},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param98),boe_otm9608_qHD_5p_param98},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param99),boe_otm9608_qHD_5p_param99},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param100),boe_otm9608_qHD_5p_param100},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param101),boe_otm9608_qHD_5p_param101},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param102),boe_otm9608_qHD_5p_param102},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param103),boe_otm9608_qHD_5p_param103},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param104),boe_otm9608_qHD_5p_param104},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param105),boe_otm9608_qHD_5p_param105},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param106),boe_otm9608_qHD_5p_param106},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(boe_otm9608_qHD_5p_param107),boe_otm9608_qHD_5p_param107},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10,sizeof(boe_otm9608_qHD_5p_param108),boe_otm9608_qHD_5p_param108},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,sizeof(boe_otm9608_qHD_5p_param109),boe_otm9608_qHD_5p_param109},
};


/*******************haoweiwie 20130426 add end**************************************/

static char tianma_nt35516_qHD_5p_para0[6] = {0xF0,0x55,0xAA,0x52,0x08,0x00};
static char tianma_nt35516_qHD_5p_para1[7] = {0xC9,0x61,0x06,0x0D,0x1A,0x17,0x00};
static char tianma_nt35516_qHD_5p_para2[4] = {0xBC,0x00,0x00,0x00};
static char tianma_nt35516_qHD_5p_para3[2] = {0xBA,0x05};
static char tianma_nt35516_qHD_5p_para4[4] = {0xB1,0x7C,0x00,0x00};
static char tianma_nt35516_qHD_5p_para5[3] = {0xB7,0x72,0x72};
static char tianma_nt35516_qHD_5p_para6[5] = {0xB8,0x01,0x02,0x02,0x02};
static char tianma_nt35516_qHD_5p_para7[4] = {0xBB,0x93,0x03,0x93};
static char tianma_nt35516_qHD_5p_para8[2] = {0x3A,0x77};
static char tianma_nt35516_qHD_5p_para9[2] = {0x36,0x00};
static char tianma_nt35516_qHD_5p_para10[6] = {0xF0,0x55,0xAA,0x52,0x08,0x01};
static char tianma_nt35516_qHD_5p_para11[2] = {0xB0,0x0A};
static char tianma_nt35516_qHD_5p_para12[2] = {0xB6,0x54};  
static char tianma_nt35516_qHD_5p_para13[2] = {0xB1,0x0A};
static char tianma_nt35516_qHD_5p_para14[2] = {0xB7,0x34};
static char tianma_nt35516_qHD_5p_para15[2] = {0xB2,0x00};
static char tianma_nt35516_qHD_5p_para16[2] = {0xB8,0x33};
static char tianma_nt35516_qHD_5p_para17[2] = {0xB3,0x0D};
static char tianma_nt35516_qHD_5p_para18[2] = {0xB9,0x24};
static char tianma_nt35516_qHD_5p_para19[2] = {0xB4,0x08};
static char tianma_nt35516_qHD_5p_para20[2] = {0xBA,0x14};
static char tianma_nt35516_qHD_5p_para21[4] = {0xBC,0x00,0x78,0x00};
static char tianma_nt35516_qHD_5p_para22[4] = {0xBD,0x00,0x78,0x00};
static char tianma_nt35516_qHD_5p_para23[2] = {0xBE,0x2D};
static char tianma_nt35516_qHD_5p_para24[17] = {0xD1,0x00,0x00,0x00,0x83,0x00,0xC8,0x01,0x0D,0x01,0x25,0x01,0x4B,0x01,0x69,0x01,0x93};
static char tianma_nt35516_qHD_5p_para25[17] = {0xD2,0x01,0xB8,0x01,0xEE,0x02,0x19,0x02,0x5C,0x02,0x90,0x02,0x91,0x02,0xC0,0x02,0xEE};
static char tianma_nt35516_qHD_5p_para26[17] = {0xD3,0x03,0x0F,0x03,0x32,0x03,0x4D,0x03,0x6A,0x03,0x7E,0x03,0x95,0x03,0xA4,0x03,0xB9};
static char tianma_nt35516_qHD_5p_para27[5] = {0xD4,0x03,0xE3,0x03,0xF8};
static char tianma_nt35516_qHD_5p_para28[17] = {0xD5,0x00,0x00,0x00,0x83,0x00,0xC8,0x01,0x0D,0x01,0x25,0x01,0x4B,0x01,0x69,0x01,0x93};
static char tianma_nt35516_qHD_5p_para29[17] = {0xD6,0x01,0xB8,0x01,0xEE,0x02,0x19,0x02,0x5C,0x02,0x90,0x02,0x91,0x02,0xC0,0x02,0xEE};
static char tianma_nt35516_qHD_5p_para30[17] = {0xD7,0x03,0x0F,0x03,0x32,0x03,0x4D,0x03,0x6A,0x03,0x7E,0x03,0x95,0x03,0xA4,0x03,0xB9};
static char tianma_nt35516_qHD_5p_para31[5] = {0xD8,0x03,0xE3,0x03,0xF8};
static char tianma_nt35516_qHD_5p_para32[17] = {0xD9,0x00,0x00,0x00,0x83,0x00,0xC8,0x01,0x0D,0x01,0x25,0x01,0x4B,0x01,0x69,0x01,0x93};
static char tianma_nt35516_qHD_5p_para33[17] = {0xDD,0x01,0xB8,0x01,0xEE,0x02,0x19,0x02,0x5C,0x02,0x90,0x02,0x91,0x02,0xC0,0x02,0xEE};
static char tianma_nt35516_qHD_5p_para34[17] = {0xDE,0x03,0x0F,0x03,0x32,0x03,0x4D,0x03,0x6A,0x03,0x7E,0x03,0x95,0x03,0xA4,0x03,0xB9};
static char tianma_nt35516_qHD_5p_para35[5] = {0xDF,0x03,0xE3,0x03,0xF8};
static char tianma_nt35516_qHD_5p_para36[17] = {0xE0,0x00,0x00,0x00,0x83,0x00,0xC8,0x01,0x0D,0x01,0x25,0x01,0x4B,0x01,0x69,0x01,0x93};
static char tianma_nt35516_qHD_5p_para37[17] = {0xE1,0x01,0xB8,0x01,0xEE,0x02,0x19,0x02,0x5C,0x02,0x90,0x02,0x91,0x02,0xC0,0x02,0xEE};
static char tianma_nt35516_qHD_5p_para38[17] = {0xE2,0x03,0x0F,0x03,0x32,0x03,0x4D,0x03,0x6A,0x03,0x7E,0x03,0x95,0x03,0xA4,0x03,0xB9};
static char tianma_nt35516_qHD_5p_para39[5] = {0xE3,0x03,0xE3,0x03,0xF8};
static char tianma_nt35516_qHD_5p_para40[17] = {0xE4,0x00,0x00,0x00,0x83,0x00,0xC8,0x01,0x0D,0x01,0x25,0x01,0x4B,0x01,0x69,0x01,0x93};
static char tianma_nt35516_qHD_5p_para41[17] = {0xE5,0x01,0xB8,0x01,0xEE,0x02,0x19,0x02,0x5C,0x02,0x90,0x02,0x91,0x02,0xC0,0x02,0xEE};
static char tianma_nt35516_qHD_5p_para42[17] = {0xE6,0x03,0x0F,0x03,0x32,0x03,0x4D,0x03,0x6A,0x03,0x7E,0x03,0x95,0x03,0xA4,0x03,0xB9};
static char tianma_nt35516_qHD_5p_para43[5] = {0xE7,0x03,0xE3,0x03,0xF8};
static char tianma_nt35516_qHD_5p_para44[17] = {0xE8,0x00,0x00,0x00,0x83,0x00,0xC8,0x01,0x0D,0x01,0x25,0x01,0x4B,0x01,0x69,0x01,0x93};
static char tianma_nt35516_qHD_5p_para45[17] = {0xE9,0x01,0xB8,0x01,0xEE,0x02,0x19,0x02,0x5C,0x02,0x90,0x02,0x91,0x02,0xC0,0x02,0xEE};
static char tianma_nt35516_qHD_5p_para46[17] = {0xEA,0x03,0x0F,0x03,0x32,0x03,0x4D,0x03,0x6A,0x03,0x7E,0x03,0x95,0x03,0xA4,0x03,0xB9};
static char tianma_nt35516_qHD_5p_para47[5] = {0xEB,0x03,0xE3,0x03,0xF8};
static char tianma_nt35516_qHD_5p_para48[2] = {0x11,0x00};
static char tianma_nt35516_qHD_5p_para49[2] = {0x29,0x00};

	static struct dsi_cmd_desc tianma_5p_qhd_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para0),tianma_nt35516_qHD_5p_para0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para1),tianma_nt35516_qHD_5p_para1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para2),tianma_nt35516_qHD_5p_para2},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para3),tianma_nt35516_qHD_5p_para3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para4),tianma_nt35516_qHD_5p_para4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para5),tianma_nt35516_qHD_5p_para5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para6),tianma_nt35516_qHD_5p_para6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para7),tianma_nt35516_qHD_5p_para7},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para8),tianma_nt35516_qHD_5p_para8},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para9),tianma_nt35516_qHD_5p_para9},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para10),tianma_nt35516_qHD_5p_para10},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para11),tianma_nt35516_qHD_5p_para11},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para12),tianma_nt35516_qHD_5p_para12},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para13),tianma_nt35516_qHD_5p_para13},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para14),tianma_nt35516_qHD_5p_para14},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para15),tianma_nt35516_qHD_5p_para15},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para16),tianma_nt35516_qHD_5p_para16},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para17),tianma_nt35516_qHD_5p_para17},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para18),tianma_nt35516_qHD_5p_para18},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para19),tianma_nt35516_qHD_5p_para19},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para20),tianma_nt35516_qHD_5p_para20},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para21),tianma_nt35516_qHD_5p_para21},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para22),tianma_nt35516_qHD_5p_para22},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para23),tianma_nt35516_qHD_5p_para23},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para24),tianma_nt35516_qHD_5p_para24},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para25),tianma_nt35516_qHD_5p_para25},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para26),tianma_nt35516_qHD_5p_para26},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para27),tianma_nt35516_qHD_5p_para27},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para28),tianma_nt35516_qHD_5p_para28},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para29),tianma_nt35516_qHD_5p_para29},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para30),tianma_nt35516_qHD_5p_para30},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para31),tianma_nt35516_qHD_5p_para31},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para32),tianma_nt35516_qHD_5p_para32},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para33),tianma_nt35516_qHD_5p_para33},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para34),tianma_nt35516_qHD_5p_para34},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para35),tianma_nt35516_qHD_5p_para35},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para36),tianma_nt35516_qHD_5p_para36},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para37),tianma_nt35516_qHD_5p_para37},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para38),tianma_nt35516_qHD_5p_para38},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para39),tianma_nt35516_qHD_5p_para39},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para40),tianma_nt35516_qHD_5p_para40},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para41),tianma_nt35516_qHD_5p_para41},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para42),tianma_nt35516_qHD_5p_para42},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para43),tianma_nt35516_qHD_5p_para43},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para44),tianma_nt35516_qHD_5p_para44},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para45),tianma_nt35516_qHD_5p_para45},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para46),tianma_nt35516_qHD_5p_para46},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(tianma_nt35516_qHD_5p_para47),tianma_nt35516_qHD_5p_para47},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,sizeof(tianma_nt35516_qHD_5p_para48),tianma_nt35516_qHD_5p_para48},
	{DTYPE_DCS_WRITE, 1, 0, 0, 50,sizeof(tianma_nt35516_qHD_5p_para49),tianma_nt35516_qHD_5p_para49},
};


/*lijiangshuo modify for otm9608 lead HSD lcd better 20130704 */
static char lead_otm9608_hsd_5p_para0[2] = {0x00,0x00};
static char lead_otm9608_hsd_5p_para1[4] = {0xFF,0x96,0x08,0x01};
static char lead_otm9608_hsd_5p_para2[2] = {0x00,0x80};
static char lead_otm9608_hsd_5p_para3[3] = {0xFF,0x96,0x08};
static char lead_otm9608_hsd_5p_para4[2] = {0x00,0xB1};
static char lead_otm9608_hsd_5p_para5[3] = {0xB0,0x03,0x06};
static char lead_otm9608_hsd_5p_para6[2] = {0x00,0x00};
static char lead_otm9608_hsd_5p_para7[2] = {0xA0,0x00};
static char lead_otm9608_hsd_5p_para8[2] = {0x00,0x80};
static char lead_otm9608_hsd_5p_para9[6] = {0xB3,0x00,0x00,0x20,0x00,0x00};
static char lead_otm9608_hsd_5p_para10[2] = {0x00,0xc0};
static char lead_otm9608_hsd_5p_para11[2] = {0xB3,0x09};
static char lead_otm9608_hsd_5p_para12[2] = {0x00,0x80};
static char lead_otm9608_hsd_5p_para13[10] = {0xC0,0x00,0x48,0x00,0x08,0x10,0x00,0x48,0x08,0x10};
static char lead_otm9608_hsd_5p_para14[2] = {0x00,0x92};
static char lead_otm9608_hsd_5p_para15[5] = {0xC0,0x00,0x10,0x00,0x13};
static char lead_otm9608_hsd_5p_para16[2] = {0x00,0xA0};
static char lead_otm9608_hsd_5p_para17[2] = {0xC0,0x00};
static char lead_otm9608_hsd_5p_para18[2] = {0x00,0xa2};
static char lead_otm9608_hsd_5p_para19[4] = {0xC0,0x0C,0x05,0x02};
static char lead_otm9608_hsd_5p_para20[2] = {0x00,0xb3};
static char lead_otm9608_hsd_5p_para21[3] = {0xC0,0x00,0x50};
static char lead_otm9608_hsd_5p_para22[2] = {0x00,0x81};
static char lead_otm9608_hsd_5p_para23[2] = {0xC1,0x66};
static char lead_otm9608_hsd_5p_para24[2] = {0x00,0x80};
static char lead_otm9608_hsd_5p_para25[3] = {0xC4,0x00,0x80};
static char lead_otm9608_hsd_5p_para26[2] = {0x00,0x88};
static char lead_otm9608_hsd_5p_para27[2] = {0xC4,0x40};
static char lead_otm9608_hsd_5p_para28[2] = {0x00,0xa0};
static char lead_otm9608_hsd_5p_para29[9] = {0xC4,0x33,0x09,0x90,0x2B,0x33,0x09,0x90,0x54};
static char lead_otm9608_hsd_5p_para30[2] = {0x00,0x80};
static char lead_otm9608_hsd_5p_para31[5] = {0xC5,0x08,0x00,0xA0,0x11};
static char lead_otm9608_hsd_5p_para32[2] = {0x00,0x90};
static char lead_otm9608_hsd_5p_para33[8] = {0xC5,0x96,0x76,0x06,0x76,0x33,0x33,0x34};
static char lead_otm9608_hsd_5p_para34[2] = {0x00,0xa0};
static char lead_otm9608_hsd_5p_para35[8] = {0xC5,0x96,0x76,0x06,0x76,0x33,0x33,0x34};
static char lead_otm9608_hsd_5p_para36[2] = {0x00,0xB0};
static char lead_otm9608_hsd_5p_para37[3] = {0xC5,0x04,0xA8};
static char lead_otm9608_hsd_5p_para38[2] = {0x00,0x80};
static char lead_otm9608_hsd_5p_para39[2] = {0xC6,0x64};
static char lead_otm9608_hsd_5p_para40[2] = {0x00,0xb0};
static char lead_otm9608_hsd_5p_para41[6] = {0xC6,0x03,0x10,0x00,0x1F,0x12};
static char lead_otm9608_hsd_5p_para42[2] = {0x00,0x00};
static char lead_otm9608_hsd_5p_para43[2] = {0xD0,0x40};
static char lead_otm9608_hsd_5p_para44[2] = {0x00,0x00};
static char lead_otm9608_hsd_5p_para45[3] = {0xD1,0x00,0x00};
static char lead_otm9608_hsd_5p_para46[2] = {0x00,0xb7};
static char lead_otm9608_hsd_5p_para47[2] = {0xB0,0x10};
static char lead_otm9608_hsd_5p_para48[2] = {0x00,0xc0};
static char lead_otm9608_hsd_5p_para49[3] = {0xB0,0x55,0x50};
static char lead_otm9608_hsd_5p_para50[2] = {0x00,0x80};
static char lead_otm9608_hsd_5p_para51[11] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char lead_otm9608_hsd_5p_para52[2] = {0x00,0x90};
static char lead_otm9608_hsd_5p_para53[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char lead_otm9608_hsd_5p_para54[2] = {0x00,0xA0};
static char lead_otm9608_hsd_5p_para55[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char lead_otm9608_hsd_5p_para56[2] = {0x00,0xB0};
static char lead_otm9608_hsd_5p_para57[11] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char lead_otm9608_hsd_5p_para58[2] = {0x00,0xC0};
static char lead_otm9608_hsd_5p_para59[16] = {0xCB,0x04,0x04,0x04,0x04,0x00,0x04,0x00,0x04,0x00,0x04,0x00,0x04,0x04,0x04,0x00};
static char lead_otm9608_hsd_5p_para60[2] = {0x00,0xD0};
static char lead_otm9608_hsd_5p_para61[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x04,0x00,0x04,0x00,0x04};
static char lead_otm9608_hsd_5p_para62[2] = {0x00,0xE0};
static char lead_otm9608_hsd_5p_para63[11] = {0xCB,0x00,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00};
static char lead_otm9608_hsd_5p_para64[2] = {0x00,0xF0};
static char lead_otm9608_hsd_5p_para65[11] = {0xCB,0x00,0xCC,0xCC,0x00,0x00,0x00,0xCC,0xCC,0x0F,0x00};
static char lead_otm9608_hsd_5p_para66[2] = {0x00,0x80};
static char lead_otm9608_hsd_5p_para67[11] = {0xCC,0x26,0x25,0x21,0x22,0x00,0x0C,0x00,0x0A,0x00,0x10};
static char lead_otm9608_hsd_5p_para68[2] = {0x00,0x90};
static char lead_otm9608_hsd_5p_para69[16] = {0xCC,0x00,0x0E,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x25,0x21,0x22,0x00};
static char lead_otm9608_hsd_5p_para70[2] = {0x00,0xA0};
static char lead_otm9608_hsd_5p_para71[16] = {0xCC,0x0B,0x00,0x09,0x00,0x0F,0x00,0x0D,0x01,0x03,0x00,0x00,0x00,0x00,0x00,0x00};
static char lead_otm9608_hsd_5p_para72[2] = {0x00,0xB0};
static char lead_otm9608_hsd_5p_para73[11] = {0xCC,0x25,0x26,0x21,0x22,0x00,0x0D,0x00,0x0F,0x00,0x09};
static char lead_otm9608_hsd_5p_para74[2] = {0x00,0xC0};
static char lead_otm9608_hsd_5p_para75[16] = {0xCC,0x00,0x0B,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x25,0x26,0x21,0x22,0x00};
static char lead_otm9608_hsd_5p_para76[2] = {0x00,0xD0};
static char lead_otm9608_hsd_5p_para77[16] = {0xCC,0x0E,0x00,0x10,0x00,0x0A,0x00,0x0C,0x04,0x02,0x00,0x00,0x00,0x00,0x00,0x00};
static char lead_otm9608_hsd_5p_para78[2] = {0x00,0x80};
static char lead_otm9608_hsd_5p_para79[13] = {0xCE,0x8A,0x03,0x28,0x89,0x03,0x28,0x88,0x03,0x28,0x87,0x03,0x28};
static char lead_otm9608_hsd_5p_para80[2] = {0x00,0x90};
static char lead_otm9608_hsd_5p_para81[15] = {0xCE,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0x00,0x00};
static char lead_otm9608_hsd_5p_para82[2] = {0x00,0xA0};
static char lead_otm9608_hsd_5p_para83[15] = {0xCE,0x38,0x06,0x03,0xC1,0x00,0x18,0x00,0x38,0x05,0x03,0xC2,0x00,0x18,0x00};
static char lead_otm9608_hsd_5p_para84[2] = {0x00,0xB0};
static char lead_otm9608_hsd_5p_para85[15] = {0xCE,0x38,0x04,0x03,0xC3,0x00,0x18,0x00,0x38,0x03,0x03,0xC4,0x00,0x18,0x00};
static char lead_otm9608_hsd_5p_para86[2] = {0x00,0xC0};
static char lead_otm9608_hsd_5p_para87[15] = {0xCE,0x38,0x02,0x03,0xC5,0x00,0x18,0x00,0x38,0x01,0x03,0xC6,0x00,0x18,0x00};
static char lead_otm9608_hsd_5p_para88[2] = {0x00,0xD0};
static char lead_otm9608_hsd_5p_para89[15] = {0xCE,0x38,0x00,0x03,0xC7,0x00,0x18,0x00,0x30,0x00,0x03,0xC8,0x00,0x18,0x00};
static char lead_otm9608_hsd_5p_para90[2] = {0x00,0xC0};
static char lead_otm9608_hsd_5p_para91[2] = {0xCF,0x02};
static char lead_otm9608_hsd_5p_para92[2] = {0x00,0x88};
static char lead_otm9608_hsd_5p_para93[2] = {0xc4,0x40};
static char lead_otm9608_hsd_5p_para94[2] = {0x00,0xc7};
static char lead_otm9608_hsd_5p_para95[2] = {0xcf,0x00};
static char lead_otm9608_hsd_5p_para96[2] = {0x00,0x00};
static char lead_otm9608_hsd_5p_para97[3] = {0xd8,0x87,0x87};
static char lead_otm9608_hsd_5p_para98[2] = {0x00,0x00};
static char lead_otm9608_hsd_5p_para99[2] = {0xd9,0x3D};
static char lead_otm9608_hsd_5p_para100[2] = {0x00,0x00};
static char lead_otm9608_hsd_5p_para101[17] = {0xE1,0x02,0x0D,0x13,0x0E,0x07,0x0D,0x0A,0x07,0x06,0x09,0x0F,0x07,0x0E,0x0C,0x09,0x06};
static char lead_otm9608_hsd_5p_para102[2] = {0x00,0x00};
static char lead_otm9608_hsd_5p_para103[17] = {0xE2,0x02,0x0D,0x13,0x0E,0x07,0x0E,0x0A,0x07,0x06,0x09,0x0E,0x07,0x0E,0x0C,0x09,0x06};
static char lead_otm9608_hsd_5p_para104[2] = {0x00,0x00};
static char lead_otm9608_hsd_5p_para105[4] = {0xFF,0xFF,0xFF,0xFF};
static char lead_otm9608_hsd_5p_para106[2] = {0x11,0x00};
static char lead_otm9608_hsd_5p_para107[2] = {0x29,0x00};

static struct dsi_cmd_desc lead_HSD_5p_qhd_display_on_cmds[] = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para0),lead_otm9608_hsd_5p_para0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para1),lead_otm9608_hsd_5p_para1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para2),lead_otm9608_hsd_5p_para2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para3),lead_otm9608_hsd_5p_para3},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para4),lead_otm9608_hsd_5p_para4},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para5),lead_otm9608_hsd_5p_para5},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para6),lead_otm9608_hsd_5p_para6},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para7),lead_otm9608_hsd_5p_para7},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para8),lead_otm9608_hsd_5p_para8},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para9),lead_otm9608_hsd_5p_para9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para10),lead_otm9608_hsd_5p_para10},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para11),lead_otm9608_hsd_5p_para11},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para12),lead_otm9608_hsd_5p_para12},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para13),lead_otm9608_hsd_5p_para13},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para14),lead_otm9608_hsd_5p_para14},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para15),lead_otm9608_hsd_5p_para15},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para16),lead_otm9608_hsd_5p_para16},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para17),lead_otm9608_hsd_5p_para17},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para18),lead_otm9608_hsd_5p_para18},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para19),lead_otm9608_hsd_5p_para19},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para20),lead_otm9608_hsd_5p_para20},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para21),lead_otm9608_hsd_5p_para21},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para22),lead_otm9608_hsd_5p_para22},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para23),lead_otm9608_hsd_5p_para23},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para24),lead_otm9608_hsd_5p_para24},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para25),lead_otm9608_hsd_5p_para25},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para26),lead_otm9608_hsd_5p_para26},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para27),lead_otm9608_hsd_5p_para27},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para28),lead_otm9608_hsd_5p_para28},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para29),lead_otm9608_hsd_5p_para29},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para30),lead_otm9608_hsd_5p_para30},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para31),lead_otm9608_hsd_5p_para31},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para32),lead_otm9608_hsd_5p_para32},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para33),lead_otm9608_hsd_5p_para33},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para34),lead_otm9608_hsd_5p_para34},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para35),lead_otm9608_hsd_5p_para35},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para36),lead_otm9608_hsd_5p_para36},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para37),lead_otm9608_hsd_5p_para37},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para38),lead_otm9608_hsd_5p_para38},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para39),lead_otm9608_hsd_5p_para39},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para40),lead_otm9608_hsd_5p_para40},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para41),lead_otm9608_hsd_5p_para41},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para42),lead_otm9608_hsd_5p_para42},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para43),lead_otm9608_hsd_5p_para43},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para44),lead_otm9608_hsd_5p_para44},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para45),lead_otm9608_hsd_5p_para45},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para46),lead_otm9608_hsd_5p_para46},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para47),lead_otm9608_hsd_5p_para47},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para48),lead_otm9608_hsd_5p_para48},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para49),lead_otm9608_hsd_5p_para49},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para50),lead_otm9608_hsd_5p_para50},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para51),lead_otm9608_hsd_5p_para51},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para52),lead_otm9608_hsd_5p_para52},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para53),lead_otm9608_hsd_5p_para53},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para54),lead_otm9608_hsd_5p_para54},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para55),lead_otm9608_hsd_5p_para55},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para56),lead_otm9608_hsd_5p_para56},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para57),lead_otm9608_hsd_5p_para57},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para58),lead_otm9608_hsd_5p_para58},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para59),lead_otm9608_hsd_5p_para59},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para60),lead_otm9608_hsd_5p_para60},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para61),lead_otm9608_hsd_5p_para61},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para62),lead_otm9608_hsd_5p_para62},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para63),lead_otm9608_hsd_5p_para63},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para64),lead_otm9608_hsd_5p_para64},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para65),lead_otm9608_hsd_5p_para65},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para66),lead_otm9608_hsd_5p_para66},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para67),lead_otm9608_hsd_5p_para67},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para68),lead_otm9608_hsd_5p_para68},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para69),lead_otm9608_hsd_5p_para69},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para70),lead_otm9608_hsd_5p_para70},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para71),lead_otm9608_hsd_5p_para71},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para72),lead_otm9608_hsd_5p_para72},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para73),lead_otm9608_hsd_5p_para73},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para74),lead_otm9608_hsd_5p_para74},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para75),lead_otm9608_hsd_5p_para75},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para76),lead_otm9608_hsd_5p_para76},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para77),lead_otm9608_hsd_5p_para77},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para78),lead_otm9608_hsd_5p_para78},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para79),lead_otm9608_hsd_5p_para79},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para80),lead_otm9608_hsd_5p_para80},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para81),lead_otm9608_hsd_5p_para81},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para82),lead_otm9608_hsd_5p_para82},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para83),lead_otm9608_hsd_5p_para83},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para84),lead_otm9608_hsd_5p_para84},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para85),lead_otm9608_hsd_5p_para85},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para86),lead_otm9608_hsd_5p_para86},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para87),lead_otm9608_hsd_5p_para87},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para88),lead_otm9608_hsd_5p_para88},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para89),lead_otm9608_hsd_5p_para89},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para90),lead_otm9608_hsd_5p_para90},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para91),lead_otm9608_hsd_5p_para91},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para92),lead_otm9608_hsd_5p_para92},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para93),lead_otm9608_hsd_5p_para93},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para94),lead_otm9608_hsd_5p_para94},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para95),lead_otm9608_hsd_5p_para95},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para96),lead_otm9608_hsd_5p_para96},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para97),lead_otm9608_hsd_5p_para97},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para98),lead_otm9608_hsd_5p_para98},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para99),lead_otm9608_hsd_5p_para99},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para100),lead_otm9608_hsd_5p_para100},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para101),lead_otm9608_hsd_5p_para101},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para102),lead_otm9608_hsd_5p_para102},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para103),lead_otm9608_hsd_5p_para103},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para104),lead_otm9608_hsd_5p_para104},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(lead_otm9608_hsd_5p_para105),lead_otm9608_hsd_5p_para105},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 120,sizeof(lead_otm9608_hsd_5p_para106),lead_otm9608_hsd_5p_para106},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 50,sizeof(lead_otm9608_hsd_5p_para107),lead_otm9608_hsd_5p_para107},
};
/*lijiangshuo modify for otm9608 lead HSD lcd better 20130704 */


/* lijiangshuo added for yassy_6g supported 20130621*/
static char yassy_otm9608_6g_qHD_5p_para0[2] = {0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para1[4] = {0xFF,0x96,0x08,0x01};
static char yassy_otm9608_6g_qHD_5p_para2[2] = {0x00,0x80};
static char yassy_otm9608_6g_qHD_5p_para3[3] = {0xFF,0x96,0x08};
static char yassy_otm9608_6g_qHD_5p_para4[2] = {0x00,0xB1};
static char yassy_otm9608_6g_qHD_5p_para5[3] = {0xB0,0x03,0x06};
static char yassy_otm9608_6g_qHD_5p_para6[2] = {0x00,0xB7};
static char yassy_otm9608_6g_qHD_5p_para7[2] = {0xB0,0x10};
static char yassy_otm9608_6g_qHD_5p_para8[2] = {0x00,0xC0};
static char yassy_otm9608_6g_qHD_5p_para9[2] = {0xB0,0x55};
static char yassy_otm9608_6g_qHD_5p_para10[2] = {0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para11[2] = {0xA0,0x00};
static char yassy_otm9608_6g_qHD_5p_para12[2] = {0x00,0x80};
static char yassy_otm9608_6g_qHD_5p_para13[6] = {0xB3,0x00,0x00,0x00,0x21,0x00};
static char yassy_otm9608_6g_qHD_5p_para14[2] = {0x00,0x92};
static char yassy_otm9608_6g_qHD_5p_para15[2] = {0xB3,0x01};
static char yassy_otm9608_6g_qHD_5p_para16[2] = {0x00,0xC0};
static char yassy_otm9608_6g_qHD_5p_para17[2] = {0xB3,0x19};
static char yassy_otm9608_6g_qHD_5p_para18[2] = {0x00,0x80};
static char yassy_otm9608_6g_qHD_5p_para19[10] = {0xC0,0x00,0x48,0x00,0x10,0x10,0x00,0x47,0x1F,0x1F};
static char yassy_otm9608_6g_qHD_5p_para20[2] = {0x00,0x92};
static char yassy_otm9608_6g_qHD_5p_para21[5] = {0xC0,0x00,0x0E,0x00,0x11};
static char yassy_otm9608_6g_qHD_5p_para22[2] = {0x00,0xA2};
static char yassy_otm9608_6g_qHD_5p_para23[4] = {0xC0,0x01,0x10,0x00};
static char yassy_otm9608_6g_qHD_5p_para24[2] = {0x00,0xB3};
static char yassy_otm9608_6g_qHD_5p_para25[3] = {0xC0,0x00,0x50};
static char yassy_otm9608_6g_qHD_5p_para26[2] = {0x00,0xB5};
static char yassy_otm9608_6g_qHD_5p_para27[2] = {0xC0,0x48};
static char yassy_otm9608_6g_qHD_5p_para28[2] = {0x00,0xE1};
static char yassy_otm9608_6g_qHD_5p_para29[2] = {0xC0,0x9F};
static char yassy_otm9608_6g_qHD_5p_para30[2] = {0x00,0x81};
static char yassy_otm9608_6g_qHD_5p_para31[2] = {0xC1,0x66};
static char yassy_otm9608_6g_qHD_5p_para32[2] = {0x00,0x86};
static char yassy_otm9608_6g_qHD_5p_para33[3] = {0xC4,0x09,0x08};
static char yassy_otm9608_6g_qHD_5p_para34[2] = {0x00,0xA0};
static char yassy_otm9608_6g_qHD_5p_para35[16] = {0xC4,0x33,0x09,0x90,0x28,0x33,0x09,0x90,0x2B,0x09,0x90,0x2B,0x33,0x09,0x90,0x54};
static char yassy_otm9608_6g_qHD_5p_para36[2] = {0x00,0x80};
static char yassy_otm9608_6g_qHD_5p_para37[5] = {0xC5,0x08,0x00,0x90,0x11};
static char yassy_otm9608_6g_qHD_5p_para38[2] = {0x00,0x90};
static char yassy_otm9608_6g_qHD_5p_para39[8] = {0xC5,0x96,0x76,0x06,0x76,0x33,0x33,0x34};
static char yassy_otm9608_6g_qHD_5p_para40[2] = {0x00,0xA0};
static char yassy_otm9608_6g_qHD_5p_para41[8] = {0xC5,0x96,0x76,0x06,0x76,0x33,0x33,0x34};
static char yassy_otm9608_6g_qHD_5p_para42[2] = {0x00,0xB0};
static char yassy_otm9608_6g_qHD_5p_para43[3] = {0xC5,0x04,0x08};
static char yassy_otm9608_6g_qHD_5p_para44[2] = {0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para45[2] = {0xD0,0x01};
static char yassy_otm9608_6g_qHD_5p_para46[2] = {0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para47[3] = {0xD1,0x01,0x01};
static char yassy_otm9608_6g_qHD_5p_para48[2] = {0x00,0x80};
static char yassy_otm9608_6g_qHD_5p_para49[11] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para50[2] = {0x00,0x90};
static char yassy_otm9608_6g_qHD_5p_para51[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para52[2] = {0x00,0xA0};
static char yassy_otm9608_6g_qHD_5p_para53[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para54[2] = {0x00,0xB0};
static char yassy_otm9608_6g_qHD_5p_para55[11] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para56[2] = {0x00,0xC0};
static char yassy_otm9608_6g_qHD_5p_para57[16] = {0xCB,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para58[2] = {0x00,0xD0};
static char yassy_otm9608_6g_qHD_5p_para59[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x04,0x04};
static char yassy_otm9608_6g_qHD_5p_para60[2] = {0x00,0xE0};
static char yassy_otm9608_6g_qHD_5p_para61[11] = {0xCB,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para62[2] = {0x00,0xF0};
static char yassy_otm9608_6g_qHD_5p_para63[11] = {0xCB,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
static char yassy_otm9608_6g_qHD_5p_para64[2] = {0x00,0x80};
static char yassy_otm9608_6g_qHD_5p_para65[11] = {0xCC,0x00,0x00,0x25,0x26,0x02,0x06,0x00,0x00,0x0A,0x0E};
static char yassy_otm9608_6g_qHD_5p_para66[2] = {0x00,0x90};
static char yassy_otm9608_6g_qHD_5p_para67[16] = {0xCC,0x0C,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x25,0x26,0x01};
static char yassy_otm9608_6g_qHD_5p_para68[2] = {0x00,0xA0};
static char yassy_otm9608_6g_qHD_5p_para69[16] = {0xCC,0x05,0x00,0x00,0x09,0x0D,0x0B,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para70[2] = {0x00,0xB0};
static char yassy_otm9608_6g_qHD_5p_para71[11] = {0xCC,0x00,0x00,0x26,0x25,0x05,0x01,0x00,0x00,0x0F,0x0B};
static char yassy_otm9608_6g_qHD_5p_para72[2] = {0x00,0xC0};
static char yassy_otm9608_6g_qHD_5p_para73[16] = {0xCC,0x0D,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x25,0x06};
static char yassy_otm9608_6g_qHD_5p_para74[2] = {0x00,0xD0};
static char yassy_otm9608_6g_qHD_5p_para75[16] = {0xCC,0x02,0x00,0x00,0x10,0x0C,0x0E,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para76[2] = {0x00,0x80};
static char yassy_otm9608_6g_qHD_5p_para77[13] = {0xCE,0x86,0x03,0x28,0x85,0x03,0x28,0x00,0x0F,0x00,0x00,0x0F,0x00};
static char yassy_otm9608_6g_qHD_5p_para78[2] = {0x00,0x90};
static char yassy_otm9608_6g_qHD_5p_para79[15] = {0xCE,0x33,0xBF,0x28,0x33,0xC0,0x28,0xF0,0x00,0x00,0xF0,0x00,0x00,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para80[2] = {0x00,0xA0};
static char yassy_otm9608_6g_qHD_5p_para81[15] = {0xCE,0x38,0x02,0x03,0xC1,0x86,0x18,0x00,0x38,0x01,0x03,0xC2,0x85,0x18,0x00};
static char yassy_otm9608_6g_qHD_5p_para82[2] = {0x00,0xB0};
static char yassy_otm9608_6g_qHD_5p_para83[15] = {0xCE,0x38,0x00,0x03,0xC3,0x86,0x18,0x00,0x30,0x00,0x03,0xC4,0x85,0x18,0x00};
static char yassy_otm9608_6g_qHD_5p_para84[2] = {0x00,0xC0};
static char yassy_otm9608_6g_qHD_5p_para85[15] = {0xCE,0x30,0x01,0x03,0xC5,0x86,0x18,0x00,0x30,0x02,0x03,0xC6,0x85,0x18,0x00};
static char yassy_otm9608_6g_qHD_5p_para86[2] = {0x00,0xD0};
static char yassy_otm9608_6g_qHD_5p_para87[15] = {0xCE,0x30,0x03,0x03,0xC7,0x86,0x18,0x00,0x30,0x04,0x03,0xC8,0x85,0x18,0x00};
static char yassy_otm9608_6g_qHD_5p_para88[2] = {0x00,0x80};
static char yassy_otm9608_6g_qHD_5p_para89[15] = {0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para90[2] = {0x00,0x90};
static char yassy_otm9608_6g_qHD_5p_para91[15] = {0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para92[2] = {0x00,0xA0};
static char yassy_otm9608_6g_qHD_5p_para93[15] = {0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para94[2] = {0x00,0xB0};
static char yassy_otm9608_6g_qHD_5p_para95[15] = {0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para96[2] = {0x00,0xC0};
static char yassy_otm9608_6g_qHD_5p_para97[11] = {0xCF,0x01,0x01,0x20,0x20,0x00,0x00,0x02,0x01,0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para98[2] = {0x00,0x80};
static char yassy_otm9608_6g_qHD_5p_para99[2] = {0xD6,0x00};
static char yassy_otm9608_6g_qHD_5p_para100[2] = {0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para101[2] = {0xD7,0x00};
static char yassy_otm9608_6g_qHD_5p_para102[2] = {0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para103[3] = {0xD8,0x7F,0x7F};
static char yassy_otm9608_6g_qHD_5p_para104[2] = {0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para105[2] = {0xD9,0x39};
static char yassy_otm9608_6g_qHD_5p_para106[2] = {0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para107[17] = {0xE1,0x01,0x07,0x0C,0x0D,0x05,0x11,0x0C,0x0B,0x02,0x06,0x08,0x07,0x0E,0x17,0x13,0x10};
static char yassy_otm9608_6g_qHD_5p_para108[2] = {0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para109[17] = {0xE2,0x01,0x07,0x0C,0x0D,0x06,0x11,0x0C,0x0B,0x02,0x06,0x09,0x07,0x0E,0x17,0x13,0x10};
static char yassy_otm9608_6g_qHD_5p_para110[2] = {0x00,0x00};
static char yassy_otm9608_6g_qHD_5p_para111[4] = {0xFF,0xFF,0xFF,0xFF};
static char yassy_otm9608_6g_qHD_5p_para112[2] = {0x36,0x00};
static char yassy_otm9608_6g_qHD_5p_para113[2] = {0x11,0x00};
static char yassy_otm9608_6g_qHD_5p_para114[2] = {0x29,0x00};
 struct dsi_cmd_desc yassy_6g_5p_qhd_display_on_cmds[] = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para0),yassy_otm9608_6g_qHD_5p_para0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para1),yassy_otm9608_6g_qHD_5p_para1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para2),yassy_otm9608_6g_qHD_5p_para2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para3),yassy_otm9608_6g_qHD_5p_para3},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para4),yassy_otm9608_6g_qHD_5p_para4},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para5),yassy_otm9608_6g_qHD_5p_para5},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para6),yassy_otm9608_6g_qHD_5p_para6},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para7),yassy_otm9608_6g_qHD_5p_para7},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para8),yassy_otm9608_6g_qHD_5p_para8},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para9),yassy_otm9608_6g_qHD_5p_para9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para10),yassy_otm9608_6g_qHD_5p_para10},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para11),yassy_otm9608_6g_qHD_5p_para11},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para12),yassy_otm9608_6g_qHD_5p_para12},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para13),yassy_otm9608_6g_qHD_5p_para13},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para14),yassy_otm9608_6g_qHD_5p_para14},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para15),yassy_otm9608_6g_qHD_5p_para15},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para16),yassy_otm9608_6g_qHD_5p_para16},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para17),yassy_otm9608_6g_qHD_5p_para17},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para18),yassy_otm9608_6g_qHD_5p_para18},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para19),yassy_otm9608_6g_qHD_5p_para19},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para20),yassy_otm9608_6g_qHD_5p_para20},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para21),yassy_otm9608_6g_qHD_5p_para21},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para22),yassy_otm9608_6g_qHD_5p_para22},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para23),yassy_otm9608_6g_qHD_5p_para23},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para24),yassy_otm9608_6g_qHD_5p_para24},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para25),yassy_otm9608_6g_qHD_5p_para25},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para26),yassy_otm9608_6g_qHD_5p_para26},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para27),yassy_otm9608_6g_qHD_5p_para27},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para28),yassy_otm9608_6g_qHD_5p_para28},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para29),yassy_otm9608_6g_qHD_5p_para29},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para30),yassy_otm9608_6g_qHD_5p_para30},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para31),yassy_otm9608_6g_qHD_5p_para31},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para32),yassy_otm9608_6g_qHD_5p_para32},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para33),yassy_otm9608_6g_qHD_5p_para33},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para34),yassy_otm9608_6g_qHD_5p_para34},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para35),yassy_otm9608_6g_qHD_5p_para35},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para36),yassy_otm9608_6g_qHD_5p_para36},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para37),yassy_otm9608_6g_qHD_5p_para37},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para38),yassy_otm9608_6g_qHD_5p_para38},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para39),yassy_otm9608_6g_qHD_5p_para39},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para40),yassy_otm9608_6g_qHD_5p_para40},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para41),yassy_otm9608_6g_qHD_5p_para41},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para42),yassy_otm9608_6g_qHD_5p_para42},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para43),yassy_otm9608_6g_qHD_5p_para43},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para44),yassy_otm9608_6g_qHD_5p_para44},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para45),yassy_otm9608_6g_qHD_5p_para45},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para46),yassy_otm9608_6g_qHD_5p_para46},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para47),yassy_otm9608_6g_qHD_5p_para47},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para48),yassy_otm9608_6g_qHD_5p_para48},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para49),yassy_otm9608_6g_qHD_5p_para49},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para50),yassy_otm9608_6g_qHD_5p_para50},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para51),yassy_otm9608_6g_qHD_5p_para51},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para52),yassy_otm9608_6g_qHD_5p_para52},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para53),yassy_otm9608_6g_qHD_5p_para53},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para54),yassy_otm9608_6g_qHD_5p_para54},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para55),yassy_otm9608_6g_qHD_5p_para55},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para56),yassy_otm9608_6g_qHD_5p_para56},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para57),yassy_otm9608_6g_qHD_5p_para57},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para58),yassy_otm9608_6g_qHD_5p_para58},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para59),yassy_otm9608_6g_qHD_5p_para59},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para60),yassy_otm9608_6g_qHD_5p_para60},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para61),yassy_otm9608_6g_qHD_5p_para61},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para62),yassy_otm9608_6g_qHD_5p_para62},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para63),yassy_otm9608_6g_qHD_5p_para63},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para64),yassy_otm9608_6g_qHD_5p_para64},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para65),yassy_otm9608_6g_qHD_5p_para65},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para66),yassy_otm9608_6g_qHD_5p_para66},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para67),yassy_otm9608_6g_qHD_5p_para67},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para68),yassy_otm9608_6g_qHD_5p_para68},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para69),yassy_otm9608_6g_qHD_5p_para69},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para70),yassy_otm9608_6g_qHD_5p_para70},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para71),yassy_otm9608_6g_qHD_5p_para71},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para72),yassy_otm9608_6g_qHD_5p_para72},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para73),yassy_otm9608_6g_qHD_5p_para73},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para74),yassy_otm9608_6g_qHD_5p_para74},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para75),yassy_otm9608_6g_qHD_5p_para75},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para76),yassy_otm9608_6g_qHD_5p_para76},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para77),yassy_otm9608_6g_qHD_5p_para77},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para78),yassy_otm9608_6g_qHD_5p_para78},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para79),yassy_otm9608_6g_qHD_5p_para79},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para80),yassy_otm9608_6g_qHD_5p_para80},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para81),yassy_otm9608_6g_qHD_5p_para81},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para82),yassy_otm9608_6g_qHD_5p_para82},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para83),yassy_otm9608_6g_qHD_5p_para83},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para84),yassy_otm9608_6g_qHD_5p_para84},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para85),yassy_otm9608_6g_qHD_5p_para85},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para86),yassy_otm9608_6g_qHD_5p_para86},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para87),yassy_otm9608_6g_qHD_5p_para87},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para88),yassy_otm9608_6g_qHD_5p_para88},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para89),yassy_otm9608_6g_qHD_5p_para89},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para90),yassy_otm9608_6g_qHD_5p_para90},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para91),yassy_otm9608_6g_qHD_5p_para91},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para92),yassy_otm9608_6g_qHD_5p_para92},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para93),yassy_otm9608_6g_qHD_5p_para93},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para94),yassy_otm9608_6g_qHD_5p_para94},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para95),yassy_otm9608_6g_qHD_5p_para95},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para96),yassy_otm9608_6g_qHD_5p_para96},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para97),yassy_otm9608_6g_qHD_5p_para97},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para98),yassy_otm9608_6g_qHD_5p_para98},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para99),yassy_otm9608_6g_qHD_5p_para99},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para100),yassy_otm9608_6g_qHD_5p_para100},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para101),yassy_otm9608_6g_qHD_5p_para101},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para102),yassy_otm9608_6g_qHD_5p_para102},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para103),yassy_otm9608_6g_qHD_5p_para103},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para104),yassy_otm9608_6g_qHD_5p_para104},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para105),yassy_otm9608_6g_qHD_5p_para105},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para106),yassy_otm9608_6g_qHD_5p_para106},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para107),yassy_otm9608_6g_qHD_5p_para107},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para108),yassy_otm9608_6g_qHD_5p_para108},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para109),yassy_otm9608_6g_qHD_5p_para109},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para110),yassy_otm9608_6g_qHD_5p_para110},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para111),yassy_otm9608_6g_qHD_5p_para111},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(yassy_otm9608_6g_qHD_5p_para112),yassy_otm9608_6g_qHD_5p_para112},
	{DTYPE_DCS_WRITE, 1, 0, 0, 200,sizeof(yassy_otm9608_6g_qHD_5p_para113),yassy_otm9608_6g_qHD_5p_para113},
	{DTYPE_DCS_WRITE, 1, 0, 0, 100,sizeof(yassy_otm9608_6g_qHD_5p_para114),yassy_otm9608_6g_qHD_5p_para114},
};
/* lijiangshuo added for yassy_6g supported 20130621*/
static void lcd_panle_reset(void)
{
	gpio_direction_output(GPIO_LCD_RESET,1);
	msleep(10);
	gpio_direction_output(GPIO_LCD_RESET,0);
	msleep(10);
	gpio_direction_output(GPIO_LCD_RESET,1);
	msleep(10);
}


static u32 __init get_lcdpanleid_from_bootloader(void)
{
	//smem_global*	msm_lcd_global = (smem_global*) ioremap(SMEM_LOG_GLOBAL_BASE, sizeof(smem_global));
	
	printk("debug chip id 0x%x\n",lcd_id_type);
	
	/*if (((msm_lcd_global->lcd_id) & 0xffff0000) == 0x09830000) 
	{ */

	
		switch(lcd_id_type)
		{	

			case 0x32:
				return (u32)LCD_PANEL_5P0_NT35516_YUSHUN;					
			case 0x33:
				return (u32)LCD_PANEL_5P0_OTM9608A_YASSY;		
                       case 0x34:
				return (u32)LCD_PANEL_5P0_OTM9608A_BOE;
			case 0x35:
				return (u32)LCD_PANEL_5P0_NT35516_TM;	
			case 0x36:
				return (u32)LCD_PANEL_5P0_OTM9608_HSD_LEAD;		
			case 0x37: // 55. lijiangshuo added for yassy_6g supported 20130621
				return (u32)LCD_PANEL_5P0_OTM9608A_YASSY_6G; 		
			default:
				break;
		}		
	//}
	return (u32)LCD_PANEL_NOPANEL;
}


static int mipi_5p0_video_qhd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	mipi_set_tx_power_mode(1);


if (0) {
	mipi_dsi_cmds_tx(&qhd_tx_buf, display_off_cmds,
			ARRAY_SIZE(display_off_cmds));
	gpio_direction_output(GPIO_LCD_RESET,0);
	msleep(5);
	gpio_direction_output(GPIO_LCD_RESET,1);
	msleep(10);
	gpio_direction_output(121,0);
}else {
/*	mipi_dsi_cmds_tx(&qhd_tx_buf, display_off_deep_cmds,
			ARRAY_SIZE(display_off_deep_cmds)); */
	mipi_dsi_cmds_tx(&qhd_tx_buf, display_off_cmds,
			ARRAY_SIZE(display_off_cmds));			
	gpio_direction_output(GPIO_LCD_RESET,0);	
}


	return 0;
}




static int first_time_panel_on = 0;
static int first_init = 0;
static int mipi_5p0_video_qhd_on(struct platform_device *pdev)
{
	
	struct msm_fb_data_type *mfd = platform_get_drvdata(pdev);
	 
	if (!mfd->cont_splash_done) {
		mfd->cont_splash_done = 1;
		return 0;
	}
	
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	
	if(first_time_panel_on){
		first_time_panel_on = 0;
		if(first_init == 1)
		{
			first_init = 0;
			return 0;

		}
	}


	
	lcd_panle_reset();
	printk("mipi init start\n");
	mipi_set_tx_power_mode(1);
	switch(LcdPanleID){
		
		case (u32)LCD_PANEL_5P0_NT35516_YUSHUN:
				mipi_dsi_cmds_tx(&qhd_tx_buf, yushun_5p_qhd_display_on_cmds,ARRAY_SIZE(yushun_5p_qhd_display_on_cmds));
				printk("yushun 5p0 nt35516 qhd for g01 !!\n");
			break;
		case (u32)LCD_PANEL_5P0_OTM9608A_YASSY:
				printk("yassy 5p0 otm9608 qhd for g01  start  !!\n");
				msleep(100);     
				mipi_dsi_cmds_tx(&qhd_tx_buf, yassy_5p_qhd_display_on_cmds,ARRAY_SIZE(yassy_5p_qhd_display_on_cmds));
				printk("yassy 5p0 otm9608 qhd for g01  end!!\n");
			break;
/*****************haoweiwei 20130426 add start********************************/
        case (u32)LCD_PANEL_5P0_OTM9608A_BOE:
				printk("boe 5p0 otm9608 qhd for g01  start  !!\n");
				mipi_dsi_cmds_tx(&qhd_tx_buf, boe_5p_qhd_display_on_cmds,ARRAY_SIZE(boe_5p_qhd_display_on_cmds));
				printk("boe 5p0 otm9608 qhd for g01  end!!\n");
			break;
/******************haoweiwei 20130426 add end**********************************/

case (u32)LCD_PANEL_5P0_NT35516_TM:
				mipi_dsi_cmds_tx(&qhd_tx_buf, tianma_5p_qhd_display_on_cmds,ARRAY_SIZE(tianma_5p_qhd_display_on_cmds));
			//	printk("tm 5p0 nt35516 qhd for g01 !!\n");
			break;

case (u32)LCD_PANEL_5P0_OTM9608_HSD_LEAD:
				mipi_dsi_cmds_tx(&qhd_tx_buf, lead_HSD_5p_qhd_display_on_cmds,ARRAY_SIZE(lead_HSD_5p_qhd_display_on_cmds));
				printk("lead hsd 5p0 otm9608 qhd for g01 !!\n");
			break;
/* lijiangshuo added for yassy_6g supported 20130621*/
		case (u32)LCD_PANEL_5P0_OTM9608A_YASSY_6G:
			printk("yassy 5p0 otm9608 6g qhd for g01 start!\n");
			//msleep(100);
			mipi_dsi_cmds_tx(&qhd_tx_buf, yassy_6g_5p_qhd_display_on_cmds,ARRAY_SIZE(yassy_6g_5p_qhd_display_on_cmds));
			printk("yassy 5p0 otm9608 6g qhd for g01 end!\n");
			break;
		default:
				printk("can't get icpanelid value\n");
				break;
				
	}	
	mipi_set_tx_power_mode(0);
	return 0;
}



static struct msm_fb_panel_data mipi_5p0_video_qhd_panel_data = {
	.on		= mipi_5p0_video_qhd_on,
	.off		= mipi_5p0_video_qhd_off,
	#ifdef CONFIG_BACKLIGHT_CABC
	.set_backlight = mipi_set_backlight_g01,
	#else
	.set_backlight = mipi_zte_set_backlight,
	#endif
};

//[ECID 000000]zhangqi add for CABC begin
#ifdef CONFIG_BACKLIGHT_CABC
void zte_lcd_en_set(void)
{
	unsigned lcd_en = GPIO_CFG(ZTE_LCD_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA);
	int rc = 0;
	rc = gpio_tlmm_config(lcd_en,GPIO_CFG_ENABLE);
	if(rc < 0 )
	{
		printk("%s: gpio_tlmm_config GPIO_CFG_ENABLE config failed!",
		 __func__);
	}
	pr_debug("%s: gpio_tlmm_config GPIO_CFG_ENABLE config sucess!",
		 __func__);
	rc = gpio_request(ZTE_LCD_EN, "f03_lcd_en");
	if(rc < 0 )
	{
		printk("%s: gpio_request GPIO_CFG_ENABLE request failed!",
		 __func__);
	}	
	pr_debug("%s: gpio_request GPIO_CFG_ENABLE request sucess!",
		 __func__);	
		
	gpio_direction_output(ZTE_LCD_EN, 1);
	pr_debug("wangminrong the only the 127  gpio is set\r\n");
}

void zte_lcd_1wire_keep_set(void)
{	
	unsigned lcd_1wire_keep = GPIO_CFG(ZTE_LCD_1WIRE_KEEP, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA);
	int rc = 0;
	rc = gpio_tlmm_config(lcd_1wire_keep,GPIO_CFG_ENABLE);
	if(rc < 0 )
	{
		printk("%s: gpio_tlmm_config GPIO_CFG_ENABLE config failed!",
		 __func__);
	}
	pr_debug("%s: gpio_tlmm_config GPIO_CFG_ENABLE config sucess!",
		 __func__);
	rc = gpio_request(ZTE_LCD_1WIRE_KEEP, "ZTE_LCD_ONELINE_KEEP");
	if(rc < 0 )
	{
		printk("%s: gpio_request GPIO_CFG_ENABLE request failed!",
		 __func__);
	}	
	pr_debug("%s: gpio_request GPIO_CFG_ENABLE request sucess!",
		 __func__);	
		
	gpio_direction_output(ZTE_LCD_1WIRE_KEEP, 1);
	pr_debug("wangminrong the only the 96  gpio is set\r\n");
	
}


void mipi_set_backlight_g01(struct msm_fb_data_type *mfd)
{
         /*value range is 1--32*/
	 int current_lel = mfd->bl_level;
	 //unsigned long flags;
	 static int first_request = 1;
	 if(first_request == 1)
	 {
		 zte_lcd_en_set();
	 //msleep(10);//
		 zte_lcd_1wire_keep_set();
	 //msleep(10);//
		first_request = 0;
	 }
	 
	 printk(" CABC level=%d lcd_type=%d in %s func \n ",current_lel,lcd_id_type,__func__);
	 
	 if (current_lel >32)
	 	{
	 			printk("Backlight level >32 ? return error. CABC level=%d in %s func \n ",current_lel,__func__);
	 			return;
	 	}
	 	
/********************haoweiwei add start****************************/
     if (lcd_id_type == 52 )
     {
	   printk("haoweiwei add backlight CABC it is a JDF + OTM9608A IC \n");
	   if(current_lel==0)
	   {
	    mipi_set_tx_power_mode(0);
		  mipi_dsi_cmds_tx(&qhd_tx_buf, otm9608a_display_off_CABC_backlight_cmds,ARRAY_SIZE(otm9608a_display_off_CABC_backlight_cmds));
			mipi_set_tx_power_mode(1);
		//	msleep(500);

	   }
	   else
	   {
		if (current_lel>32)
		{
			current_lel=32;
		}
		otm9608a_para_CABC_0x51[1]=(7*current_lel-1);     
		mipi_set_tx_power_mode(0);
		  mipi_dsi_cmds_tx(&qhd_tx_buf, otm9608a_display_on_CABC_backlight_cmds,ARRAY_SIZE(otm9608a_display_on_CABC_backlight_cmds));
 			mipi_set_tx_power_mode(1);
	   }

	 }
/********************haoweiwei add end******************************/
	 else if ((lcd_id_type == 51 )||(lcd_id_type==54))
	 {
	   pr_debug("liyeqing ----- add for CABC it is a JDF + OTM9608A IC \n");
	   if(current_lel==0)
	   {
	    mipi_set_tx_power_mode(0);
		  mipi_dsi_cmds_tx(&qhd_tx_buf, otm9608a_display_off_CABC_backlight_cmds,ARRAY_SIZE(otm9608a_display_off_CABC_backlight_cmds));
			mipi_set_tx_power_mode(1);
		//	msleep(500);

	   }
	   else
	   {
		if (current_lel>32)
		{
			current_lel=32;
		}
		otm9608a_para_CABC_0x51[1]=(7*current_lel-1);     
		mipi_set_tx_power_mode(0);
		  mipi_dsi_cmds_tx(&qhd_tx_buf, otm9608a_display_on_CABC_backlight_cmds,ARRAY_SIZE(otm9608a_display_on_CABC_backlight_cmds));
 			mipi_set_tx_power_mode(1);
	   }

	 }

	else if ((lcd_id_type == 50 )||(lcd_id_type == 53))
	 {
	  // printk("liyeqing ----- add for CABC it is a yushun 5p0 + nt35516 IC \n");
	   if(current_lel==0)
	   {
	    mipi_set_tx_power_mode(0);//20121026
		  mipi_dsi_cmds_tx(&qhd_tx_buf, tianma_nt35516_display_off_CABC_backlight_cmds,ARRAY_SIZE(tianma_nt35516_display_off_CABC_backlight_cmds));
			mipi_set_tx_power_mode(1);
			//msleep(500);
			//msleep(50);

	   }
	   else
	   {
		if (current_lel>32)
		{
			current_lel=32;
		}
	    tianma_nt35516_para_CABC_0x51[1]=(7*current_lel-1);	    
	    mipi_set_tx_power_mode(0);
		  mipi_dsi_cmds_tx(&qhd_tx_buf, tianma_nt35516_display_on_CABC_backlight_cmds,ARRAY_SIZE(tianma_nt35516_display_on_CABC_backlight_cmds));
 			mipi_set_tx_power_mode(1);
	   }

	 }
/* lijiangshuo added for yassy_6g supported 20130621 */
	else if(lcd_id_type == 55)
	{
		pr_debug("liyeqing ----- add for CABC it is a JDF + OTM9608A IC \n");
		if(current_lel==0)
		{
	    mipi_set_tx_power_mode(0);
		  mipi_dsi_cmds_tx(&qhd_tx_buf, otm9608a_display_off_CABC_backlight_cmds,ARRAY_SIZE(otm9608a_display_off_CABC_backlight_cmds));
			mipi_set_tx_power_mode(1);
		//msleep(500);
		}
		else
		{
			if (current_lel>32)
			{
				current_lel=32;
			}
			otm9608a_para_CABC_0x51[1]=(7*current_lel-1);     
			mipi_set_tx_power_mode(0);
		  mipi_dsi_cmds_tx(&qhd_tx_buf, otm9608a_display_on_CABC_backlight_cmds,ARRAY_SIZE(otm9608a_display_on_CABC_backlight_cmds));
 			mipi_set_tx_power_mode(1);
		}	
	}
/* lijiangshuo added for yassy_6g supported 20130621*/	
	else
	 	 printk("zhangqi add for CABC it is ?? IC \n");
	 return;


	 
}
#endif
//[ECID 000000]zhangqi add for CABC end

static int ch_used[3];

int mipi_5p0_video_qhd_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_5p0_video_qhd", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	mipi_5p0_video_qhd_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &mipi_5p0_video_qhd_panel_data,
		sizeof(mipi_5p0_video_qhd_panel_data));
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}


static int __devinit mipi_5p0_video_qhd_lcd_probe(struct platform_device *pdev)
{	
	struct msm_panel_info   *pinfo =&( ((struct msm_fb_panel_data  *)(pdev->dev.platform_data))->panel_info);
		printk("mipi_lead_lcd_probe\n");

	if (pdev->id == 0) return 0;
	
	mipi_dsi_buf_alloc(&qhd_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&qhd_rx_buf, DSI_BUF_SIZE);
	
	if((LcdPanleID = get_lcdpanleid_from_bootloader() )==(u32)LCD_PANEL_NOPANEL)
		printk("cann't get get_lcdpanelid from bootloader\n");

    printk("mipi_lead_lcd_probe LcdPanleID %d\n", LcdPanleID);



	
	if (LcdPanleID == LCD_PANEL_5P0_NT35516_YUSHUN)//this panel is different from others
	{
		pinfo->lcdc.h_back_porch = 130;//100
		pinfo->lcdc.h_front_porch = 130;//100
		pinfo->lcdc.h_pulse_width = 2;
		pinfo->lcdc.v_back_porch = 5;//26;//lead panel must use 26
		pinfo->lcdc.v_front_porch = 13;
		pinfo->lcdc.v_pulse_width = 2;
		pinfo->clk_rate = 493000000;
		pinfo->mipi.frame_rate = 52;
	}


	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_5p0_video_qhd_lcd_probe,
	.driver = {
		.name   = "mipi_5p0_video_qhd",
	},
};

static int __init mipi_5p0_video_qhd_init(void)
{
	return platform_driver_register(&this_driver);
}

module_init(mipi_5p0_video_qhd_init);

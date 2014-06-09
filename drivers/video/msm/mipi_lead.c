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

static struct dsi_buf lead_tx_buf;
static struct dsi_buf lead_rx_buf;
extern u32 LcdPanleID;
extern void mipi_zte_set_backlight(struct msm_fb_data_type *mfd);
//[ECID 000000] zhangqi add for CABC begin
#ifdef CONFIG_BACKLIGHT_CABC
void mipi_set_backlight(struct msm_fb_data_type *mfd);
#endif
//[ECID 000000] zhangqi add for CABC end


// #ifdef CONFIG_FB_MSM_GPIO
#ifdef CONFIG_PROJECT_P865E02
#define GPIO_LCD_RESET 85 //for p865e02 
#elif defined (CONFIG_PROJECT_P825V20)      
#define GPIO_LCD_RESET 78 
#else
#define GPIO_LCD_RESET 129
#endif
// #else
// #define GPIO_LCD_RESET 84
// #endif

#define CONFIG_LCD_READ_ID 0 
/*ic define*/
#define HIMAX_8363 		1
#define HIMAX_8369 		2
#define NOVATEK_35510	3
#define RENESAS_R61408	4

#define HIMAX8369_TIANMA_TN_ID		0xB1
#define HIMAX8369_TIANMA_IPS_ID		0xA5
#define HIMAX8369_LEAD_ID				0
#define HIMAX8369_LEAD_HANNSTAR_ID	0x88
#define NT35510_YUSHUN_ID				0
#define NT35510_LEAD_ID				0xA0
#define HIMAX8369_YUSHUN_IVO_ID       0x85

/*about icchip sleep and display on */
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};
static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static struct dsi_cmd_desc display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 50, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep}
};


/*about himax8363 chip id */
static char hx8363_setpassword_para[4]={0xB9,0xFF,0x83,0x63};
//static char hx8363_icid_rd_para[2] = {0xB9, 0x00}; 
//static char hx8363_panleid_rd_para[2] = {0xda, 0x00};    
   
/*static struct dsi_cmd_desc hx8363_icid_rd_cmd = 
{
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(hx8363_icid_rd_para), hx8363_icid_rd_para
};
*/
/*
static struct dsi_cmd_desc hx8363_setpassword_cmd[] = 
{	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(hx8363_setpassword_para),hx8363_setpassword_para},

};


static struct dsi_cmd_desc hx8363_panleid_rd_cmd = 
{
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(hx8363_panleid_rd_para), hx8363_panleid_rd_para
};
*/
/*about himax8369 chip id */
static char hx8369_setpassword_para[4]={0xB9,0xFF,0x83,0x69};
//static char hx8369_icid_rd_para[2] = {0xB9, 0x00}; 
//static char hx8369_panleid_rd_para[2] = {0xda, 0x00};    

/*
static struct dsi_cmd_desc hx8369_icid_rd_cmd = 
{
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(hx8369_icid_rd_para), hx8369_icid_rd_para
};
*/
//static struct dsi_cmd_desc hx8369_setpassword_cmd[] = 
//{	
//	{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(hx8369_setpassword_para),hx8369_setpassword_para},

//};
/*
static struct dsi_cmd_desc hx8369_panleid_rd_cmd = 
{
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(hx8369_panleid_rd_para), hx8369_panleid_rd_para
};
*/

/*about Novatek3511 chip id */
//static char nt3511_page_ff[5] = {0xff, 0xaa,0x55,0x25,0x01};
//static char nt3511_page_f8[20] = {0xf8, 0x01,0x12,0x00,0x20,0x33,0x13,0x00,0x40,0x00,0x00,0x23,0x01,0x99,0xc8,0x00,0x00,0x01,0x00,0x00};
//static char nt3511_icid_rd_para[2] = {0xc5, 0x00}; 

//static struct dsi_cmd_desc nt3511_setpassword_cmd[] = {	
//	{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(nt3511_page_ff),nt3511_page_ff},
//	{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(nt3511_page_f8),nt3511_page_f8}
//};
/*
static struct dsi_cmd_desc nt3511_icid_rd_cmd = {
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(nt3511_icid_rd_para), nt3511_icid_rd_para};


static struct dsi_cmd_desc nt3511_panleid_rd_cmd = {
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(nt3511_panleid_rd_para), nt3511_panleid_rd_para
};   
*/

/*about RENESAS r61408 chip id */
//static char r61408_setpassword_para[2]={0xb0,0x04};
//static struct dsi_cmd_desc r61408_setpassword_cmd[] = 
//{	
//	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(r61408_setpassword_para),r61408_setpassword_para},

//};
/*static char r61408_icid_rd_para[2] = {0xbf, 0x00}; 
static struct dsi_cmd_desc r61408_icid_rd_cmd = 
{
	DTYPE_GEN_READ1, 1, 0, 0, 1, sizeof(r61408_icid_rd_para), r61408_icid_rd_para
};
*/
/**************************************
0. CABC start 
**************************************/
//[ECID 000000] zhangqi add for CABC begin
extern  unsigned int lcd_id_type;

#ifdef CONFIG_BACKLIGHT_CABC
static char hx8369_para_CABC_0xc9[10]={0xc9,0x3e,0x00,0x00,0x01,0x2f,0x00,0x1e,0x1e,0x00};//{0xC9,0x3e,0x30,0x00,0x01,0x2f,0x00,0x1e,0x1e,0x00};
static char hx8369_para_CABC_0x51[2]={0x51,0xff};
static char hx8369_para_CABC_0x53[2]={0x53,0x2c};
static char hx8369_para_CABC_0x55[2]={0x55,0x00};
static struct dsi_cmd_desc hx8369_display_on_CABC_backlight_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_para_CABC_0xc9), hx8369_para_CABC_0xc9},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8369_para_CABC_0x51), hx8369_para_CABC_0x51},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8369_para_CABC_0x53), hx8369_para_CABC_0x53},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8369_para_CABC_0x55), hx8369_para_CABC_0x55},
};

static char hx8369_para_CABC_0x53_off[2]={0x53,0x0c};

static struct dsi_cmd_desc hx8369_display_off_CABC_backlight_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 100, sizeof(hx8369_para_CABC_0x53_off), hx8369_para_CABC_0x53_off},

};
/*zhoufan add for nt35512 TM LCD  CABC++,20130606*/
static char nt35512_para_CABC_0x51[2]={0x51,0xff};
static char nt35512_para_CABC_0x53[2]={0x53,0x2c};
static char nt35512_para_CABC_0x55[2]={0x55,0x03};//zhangqi add for move mode 2012.7.26
static struct dsi_cmd_desc nt35512_display_on_CABC_backlight_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt35512_para_CABC_0x51), nt35512_para_CABC_0x51},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt35512_para_CABC_0x53), nt35512_para_CABC_0x53},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt35512_para_CABC_0x55), nt35512_para_CABC_0x55},
};
static char nt35512_para_CABC_0x53_off[2]={0x53,0x00};
static struct dsi_cmd_desc nt35512_display_off_CABC_backlight_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 100, sizeof(nt35512_para_CABC_0x53_off), nt35512_para_CABC_0x53_off},

};
/*zhoufan add for nt35512 TM LCD  CABC--,20130606*/
static char hx8363_para_CABC_0xc9[4]={0xc9,0x0f,0x3e,0x01};
static char hx8363_para_CABC_0x51[2]={0x51,0xff};
static char hx8363_para_CABC_0x53[2]={0x53,0x2c};
static char hx8363_para_CABC_0x55[2]={0x55,0x03};//zhangqi add for move mode 2012.7.26
static struct dsi_cmd_desc hx8363_display_on_CABC_backlight_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_para_CABC_0xc9), hx8363_para_CABC_0xc9},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8363_para_CABC_0x51), hx8363_para_CABC_0x51},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8363_para_CABC_0x53), hx8363_para_CABC_0x53},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8363_para_CABC_0x55), hx8363_para_CABC_0x55},
};

//static char hx8363_para_CABC_0x51_off[2]={0x51,0x00};
static char hx8363_para_CABC_0x53_off[2]={0x53,0x0c};
//static char hx8363_para_CABC_0x55_off[2]={0x55,0x00};//zhangqi add for move mode 2012.7.26
static struct dsi_cmd_desc hx8363_display_off_CABC_backlight_cmds[] = {
//	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_para_CABC_0xc9), hx8363_para_CABC_0xc9},
	//{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8363_para_CABC_0x51_off), hx8363_para_CABC_0x51_off},
//	{DTYPE_DCS_WRITE1, 1, 0, 0, 100, sizeof(hx8363_para_CABC_0x55_off), hx8363_para_CABC_0x55_off},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 100, sizeof(hx8363_para_CABC_0x53_off), hx8363_para_CABC_0x53_off},

};

static char r61408_truly_para_CABC_0xb0[2]={0xB0,0x04}; 
static char r61408_truly_para_CABC_0xb8[21]={0xb8,0x00,0x0f,0x0f,0xff,0xff,0xc8,0xc8,0x02,0x18,0x10,
0x10,0x37,0x5a,0x87,0xbe,0xff,0x00,0x00,0x00,0x00};
//PWMDIV[7:0]=0x03--->13.7KHz 
//static char r61408_truly_para_CABC_0xb9[5]={0xb9,0x01,0x7f,0x03,0x18};    //zhoufan--

static char r61408_truly_para_CABC_0xb9_on[5]={0xb9,0x01,0x7f,0x03,0x18}; 
static char r61408_truly_para_CABC_0xb9_off[5]={0xb9,0x00,0x00,0x03,0x18};//wangminrong 7f

static struct dsi_cmd_desc r61408_truly_display_on_CABC_backlight_cmds[] = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_para_CABC_0xb0), r61408_truly_para_CABC_0xb0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_para_CABC_0xb8), r61408_truly_para_CABC_0xb8},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_para_CABC_0xb9_on), r61408_truly_para_CABC_0xb9_on},
};

static struct dsi_cmd_desc r61408_truly_display_off_CABC_backlight_cmds[] = {
//	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_para_CABC_0xb0), r61408_truly_para_CABC_0xb0},    //zhoufan--
//	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_para_CABC_0xb8), r61408_truly_para_CABC_0xb8},   //zhoufan--
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_para_CABC_0xb9_off), r61408_truly_para_CABC_0xb9_off},
};
#endif 
//[ECID 000000] zhangqi add for CABC end


/**************************************
1. hx8363 yassy start 
**************************************/
static char hx8363_yassy_para_0xb1[13]={0xB1,0x78,0x34,0x08,0x34,0x02,0x13,
								0x11,0x11,0x2d,0x35,0x3F,0x3F};  
static char hx8363_yassy_para_0xba[14]={0xBA,0x80,0x00,0x10,0x08,0x08,0x10,0x7c,0x6e,
								0x6d,0x0a,0x01,0x84,0x43};   //TWO LANE
static char hx8363_yassy_para_0x3a[2]={0x3a,0x77};
//static char hx8363_para_0x36[2]={0x36,0x0a};
static char hx8363_yassy_para_0xb2[4]={0xb2,0x33,0x33,0x22};
static char hx8363_yassy_para_0xb3[2]={0xb3,0x00};
static char hx8363_yassy_para_0xb4[10]={0xb4,0x08,0x12,0x72,0x12,0x06,0x03,0x54,0x03,0x4e};
static char hx8363_yassy_para_0xb6[2]={0xb6,0x2c};
static char hx8363_yassy_para_0xcc[2]={0xcc,0x09};
static char hx8363_yassy_para_0xe0[31]={0xe0,0x01,0x09,0x17,0x10,0x10,0x3e,0x07,
	0x8d,0x90,0x54,0x16,0xd5,0x55,0x53,0x19,0x01,0x09,0x17,0x10,0x10,0x3e,0x07,
	0x8d,0x90,0x54,0x16,0xd5,0x55,0x53,0x19};	

static struct dsi_cmd_desc hx8363_yassy_display_on_cmds[] = 
{

	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_setpassword_para),hx8363_setpassword_para},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_yassy_para_0xb1), hx8363_yassy_para_0xb1},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_yassy_para_0xb2), hx8363_yassy_para_0xb2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_yassy_para_0xba), hx8363_yassy_para_0xba},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8363_yassy_para_0x3a), hx8363_yassy_para_0x3a},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8363_yassy_para_0xb3), hx8363_yassy_para_0xb3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_yassy_para_0xb4), hx8363_yassy_para_0xb4},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8363_yassy_para_0xb6), hx8363_yassy_para_0xb6},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8363_yassy_para_0xcc), hx8363_yassy_para_0xcc},
	//{DTYPE_DCS_WRITE1, 1, 0, 0, 10, sizeof(hx8363_para_0x36), hx8363_para_0x36},		
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_yassy_para_0xe0), hx8363_yassy_para_0xe0},
	//{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(hx8363_para_0xc1), hx8363_para_0xc1},
	//{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(hx8363_para_0xc2), hx8363_para_0xc2},	
	

	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on}

};



/**************************************
2. hx8369 lead start 
**************************************/
static char hx8369_lead_tn_para_0xb0[3]={0xb0,0x01,0x09};
static char hx8369_lead_tn_para_0xb1[20]={0xB1,0x01,0x00,0x34,0x07,0x00,0x0F,0x0F,
	0x21,0x28,0x3F,0x3F,0x07,0x23,0x01,0xE6,0xE6,0xE6,0xE6,0xE6};  
static char hx8369_lead_tn_para_0xb2[16]={0xB2,0x00,0x23,0x0A,0x0A,0x70,0x00,0xFF,
	0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x01};  //VIDEO MODE
//static char para_0xb2[16]={0xB2,0x00,0x20,0x0A,0x0A,0x70,0x00,0xFF,
//	0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x01};  //CMD MODE
static char hx8369_lead_tn_para_0xb4[6]={0xB4,0x00,0x0C,0x84,0x0C,0x01}; 
static char hx8369_lead_tn_para_0xb6[3]={0xB6,0x2c,0x2c};
static char hx8369_lead_tn_para_0xd5[27]={0xD5,0x00,0x05,0x03,0x00,0x01,0x09,0x10,
	0x80,0x37,0x37,0x20,0x31,0x46,0x8A,0x57,0x9B,0x20,0x31,0x46,0x8A,
	0x57,0x9B,0x07,0x0F,0x07,0x00}; 
static char hx8369_lead_tn_para_0xe0[35]={0xE0,0x00,0x06,0x06,0x29,0x2d,0x3F,0x13,0x32,
	0x08,0x0c,0x0D,0x11,0x14,0x11,0x14,0x0e,0x15,0x00,0x06,0x06,0x29,0x2d,
	0x3F,0x13,0x32,0x08,0x0c,0x0D,0x11,0x14,0x11,0x14,0x0e,0x15};
static char hx8369_lead_tn_para_0x3a[2]={0x3A,0x77}; 
static char hx8369_lead_tn_para_0xba[14]={0xBA,0x00,0xA0,0xC6,0x00,0x0A,0x00,0x10,0x30,
	0x6C,0x02,0x11,0x18,0x40};   //TWO LANE
//static char para_0xba[14]={0xBA,0x00,0xA0,0xC6,0x00,0x0A,0x00,0x10,0x30,
	//0x6C,0x02,0x10,0x18,0x40};   //ONE LANE

static struct dsi_cmd_desc hx8369_lead_display_on_cmds[] = {
	 
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_setpassword_para),hx8369_setpassword_para},
	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_tn_para_0xb0), hx8369_lead_tn_para_0xb0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_tn_para_0xb1), hx8369_lead_tn_para_0xb1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_tn_para_0xb2), hx8369_lead_tn_para_0xb2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_tn_para_0xb4), hx8369_lead_tn_para_0xb4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_tn_para_0xb6), hx8369_lead_tn_para_0xb6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_tn_para_0xd5), hx8369_lead_tn_para_0xd5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_tn_para_0xe0), hx8369_lead_tn_para_0xe0},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8369_lead_tn_para_0x3a), hx8369_lead_tn_para_0x3a},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_tn_para_0xba), hx8369_lead_tn_para_0xba},

	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on}

};
/**************************************
3. HANNSTAR hx8369 lead start 
**************************************/


static char hx8369_lead_hannstar_para_0xb1[20]={0xB1,0x01,0x00,0x34,0x07,0x00,0x0E,0x0E,
	0x21,0x29,0x3F,0x3F,0x01,0x63,0x01,0xE6,0xE6,0xE6,0xE6,0xE6};  
static char hx8369_lead_hannstar_para_0xb2[16]={0xB2,0x00,0x23,0x07,0x07,0x70,0x00,0xFF,
	0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x01};  //VIDEO MODE
static char hx8369_lead_hannstar_para_0xb4[6]={0xB4,0x02,0x18,0x80,0x10,0x01}; 
static char hx8369_lead_hannstar_para_0xb6[3]={0xB6,0x1F,0x1F};
static char hx8369_lead_hannstar_para_0xcc[2]={0xcc,0x00}; 


static char hx8369_lead_hannstar_para_0xd5[27]={0xD5,0x00,0x07,0x00,0x00,0x01,0x0a,0x10,
	0x60,0x33,0x37,0x23,0x01,0xB9,0x75,0xA8,0x64,0x00,0x00,0x41,0x06,
	0x50,0x07,0x07,0x0F,0x07,0x00}; 
static char hx8369_lead_hannstar_para_0xe0[35]={0xE0,0x00,0x03,0x00,0x09,0x09,0x21,0x1B,0x2D,
	0x06,0x0c,0x10,0x15,0x16,0x14,0x16,0x12,0x18,0x00,0x03,0x00,0x09,0x09,
	0x21,0x1B,0x2D,0x06,0x0c,0x10,0x15,0x16,0x14,0x16,0x12,0x18};
static char hx8369_lead_hannstar_para_0x3a[2]={0x3A,0x77}; 
static char hx8369_lead_hannstar_para_0xba[14]={0xBA,0x00,0xA0,0xC6,0x00,0x0A,0x02,0x10,0x30,
	0x6F,0x02,0x11,0x18,0x40};   //TWO LANE


static struct dsi_cmd_desc hx8369_lead_hannstar_display_on_cmds[] = {
	 
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_setpassword_para),hx8369_setpassword_para},
	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_hannstar_para_0xb1), hx8369_lead_hannstar_para_0xb1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_hannstar_para_0xb2), hx8369_lead_hannstar_para_0xb2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_hannstar_para_0xb4), hx8369_lead_hannstar_para_0xb4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_hannstar_para_0xb6), hx8369_lead_hannstar_para_0xb6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_hannstar_para_0xcc), hx8369_lead_hannstar_para_0xcc},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_hannstar_para_0xd5), hx8369_lead_hannstar_para_0xd5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_hannstar_para_0xe0), hx8369_lead_hannstar_para_0xe0},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8369_lead_hannstar_para_0x3a), hx8369_lead_hannstar_para_0x3a},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_lead_hannstar_para_0xba), hx8369_lead_hannstar_para_0xba},

	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on}

};

/**************************************
4. hx8369 tianma TN start 
**************************************/
static char hx8369_tianma_tn_para_0xb1[20]={0xB1,0x01,0x00,0x34,0x0A,0x00,0x11,0x12,0x21,0x29,0x3F,0x3F,
	0x01,0x1a,0x01,0xE6,0xE6,0xE6,0xE6,0xE6}; 
static char hx8369_tianma_tn_para_0xb2[16]={0xB2,0x00,0x23,0x03,0x03,0x70,0x00,0xFF,0x00,0x00,0x00,0x00,
	0x03,0x03,0x00,0x01};  //VIDEO MODE
//static char hx8369_tianma_tn_para_0xb4[6]={0xB4,0x02,0x18,0x70,0x0f,0x01}; //wangminrong add for lcd white noize 20120510
static char hx8369_tianma_tn_para_0xb4[6]={0xB4,0x02,0x18,0x70,0x13,0x05};
static char hx8369_tianma_tn_para_0xb6[3]={0xB6,0x4a,0x4a};
//static char hx8369_tianma_tn_para_0xd5[27]={0xD5,0x00,0x09,0x03,0x29,0x01,0x0a,0x28,0x60,0x11,0x13,0x00,
//	0x00,0x40,0x26,0x51,0x37,0x00,0x00,0x71,0x35,0x60,0x24,0x07,0x0F,0x04,0x04}; 
static char hx8369_tianma_tn_para_0xd5[27]={0xD5,0x00,0x09,0x03,0x2B,0x01,0x0C,0x28,0x60,0x11,0x13,0x00,
	0x00,0x60,0xC4,0x71,0xC5,0x00,0x00,0x71,0x05,0x60,0x04,0x07,0x0F,0x04,0x04};  //wangminrong add for lcd white noize 20120510
static char hx8369_tianma_tn_para_0xe0[35]={0xE0,0x00,0x02,0x0b,0x0a,0x09,0x18,0x1d,0x2a,0x08,0x11,0x0d,
	0x13,0x15,0x14,0x15,0x0f,0x14,0x00,0x02,0x0b,0x0a,0x09,0x18,0x1d,0x2a,0x08,0x11,0x0d,0x13,0x15,
	0x14,0x15,0x0f,0x14};
static char hx8369_tianma_tn_para_0xcc[2]={0xcc,0x00}; 
static char hx8369_tianma_tn_para_0x3a[2]={0x3A,0x77}; 
static char hx8369_tianma_tn_para_0xba[14]={0xBA,0x00,0xA0,0xC6,0x00,0x0A,0x00,0x10,0x30,0x6F,0x02,0x11,0x18,0x40};   //TWO LANE
static struct dsi_cmd_desc hx8369_tianma_tn_display_on_cmds[] = 
{

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_setpassword_para),hx8369_setpassword_para},
	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_tn_para_0xb1), hx8369_tianma_tn_para_0xb1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_tn_para_0xb2), hx8369_tianma_tn_para_0xb2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_tn_para_0xb4), hx8369_tianma_tn_para_0xb4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_tn_para_0xb6), hx8369_tianma_tn_para_0xb6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_tn_para_0xd5),hx8369_tianma_tn_para_0xd5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_tn_para_0xe0), hx8369_tianma_tn_para_0xe0},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8369_tianma_tn_para_0x3a), hx8369_tianma_tn_para_0x3a},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8369_tianma_tn_para_0xcc), hx8369_tianma_tn_para_0xcc},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_tn_para_0xba), hx8369_tianma_tn_para_0xba},

	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on}

};
/**************************************
5. hx8369 tianma IPS start 
**************************************/
static char hx8369_tianma_ips_para_0xb1[20]={0xB1,0x01,0x00,0x34,0x07,0x00,0x0D,0x0D,0x1A,0x22,0x3F,
	0x3F,0x01,0x23,0x01,0xE6,0xE6,0xE6,0xE6,0xE6}; 
static char hx8369_tianma_ips_para_0xb2[16]={0xB2,0x00,0x23,0x05,0x05,0x70,0x00,0xFF,
	0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x01};  //VIDEO MODE
static char hx8369_tianma_ips_para_0xb4[6]={0xB4,0x00,0x18,0x80,0x06,0x02}; 
static char hx8369_tianma_ips_para_0xb6[3]={0xB6,0x42,0x42};
static char hx8369_tianma_ips_para_0xd5[27]={0xD5,0x00,0x09,0x03,0x29,0x01,0x0A,0x28,0x70,
	0x11,0x13,0x00,0x00,0x40,0x26,0x51,0x37,0x00,0x00,0x71,0x35,0x60,0x24,0x07,0x0F,0x04,0x04}; 
static char hx8369_tianma_ips_para_0xe0[35]={0xE0,0x00,0x0A,0x0F,0x2E,0x33,0x3F,0x1D,0x3E,0x07,0x0D,0x0F,
	0x12,0x15,0x13,0x15,0x10,0x17,0x00,0x0A,0x0F,0x2E,0x33,0x3F,0x1D,0x3E,0x07,0x0D,
	0x0F,0x12,0x15,0x13,0x15,0x10,0x17};
static char hx8369_tianma_ips_para_0x3a[2]={0x3A,0x77}; 
static char hx8369_tianma_ips_para_0xba[14]={0xBA,0x00,0xA0,0xC6,0x00,0x0A,0x00,0x10,0x30,0x6F,0x02,0x11,0x18,0x40};   //TWO LANE

static struct dsi_cmd_desc hx8369_tianma_ips_display_on_cmds[] = 
{

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_setpassword_para),hx8369_setpassword_para},
	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_ips_para_0xb1), hx8369_tianma_ips_para_0xb1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_ips_para_0xb2), hx8369_tianma_ips_para_0xb2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_ips_para_0xb4), hx8369_tianma_ips_para_0xb4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_ips_para_0xb6), hx8369_tianma_ips_para_0xb6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_ips_para_0xd5), hx8369_tianma_ips_para_0xd5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_ips_para_0xe0), hx8369_tianma_ips_para_0xe0},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8369_tianma_ips_para_0x3a), hx8369_tianma_ips_para_0x3a},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8369_tianma_ips_para_0xba), hx8369_tianma_ips_para_0xba},

	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on}

};


/**************************************
6. nt35510 lead start 
**************************************/

static char nt35510_lead_cmd_page1_f0[6] = {0xf0, 0x55,0xaa,0x52,0x08,0x01};
static char nt35510_lead_cmd_page1_bc[4] = {0xbc, 0x00,0x78,0x1A};
static char nt35510_lead_cmd_page1_bd[4] = {0xbd, 0x00,0x78,0x1A};
static char nt35510_lead_cmd_page1_be[3] = {0xbe, 0x00,0x4E};
static char nt35510_lead_cmd_page1_b0[4] = {0xb0, 0x00,0x00,0x00};
static char nt35510_lead_cmd_page1_b1[4] = {0xb1, 0x00,0x00,0x00};
//static char lead_cmd_page1_b7[4] = {0xb7, 0x44,0x44,0x44};
//static char lead_cmd_page1_b3[4] = {0xb1, 0x0B,0x0B,0x0B};
static char nt35510_lead_cmd_page1_b9[4] = {0xb9, 0x34,0x34,0x34};
static char nt35510_lead_cmd_page1_ba[4] = {0xba, 0x16,0x16,0x16};

static char nt35510_lead_cmd_page1_b6[4] = {0xb6, 0x36,0x36,0x36};
static char nt35510_lead_cmd_page1_b7[4] = {0xb7, 0x26,0x26,0x26};
static char nt35510_lead_cmd_page1_b8[4] = {0xb8, 0x26,0x26,0x26};
static char nt35510_lead_cmd_page1_d1[] = {0xD1,0x00,0x00,0x00,0x2D,0x00,0x5C,
0x00,0x80,0x00,0xAB,0x00,0xE4,0x01,0x15,0x01,0x5C,0x01,0x8E,0x01,0xD3,0x02,
0x03,0x02,0x45,0x02,0x77,0x02,0x78,0x02,0xA4,0x02,0xD1,0x02,0xEA,0x03,0x09,
0x03,0x1A,0x03,0x32,0x03,0x40,0x03,0x59,0x03,0x68,0x03,0x7C,0x03,0xB2,0x03,
0xD8};
static char nt35510_lead_cmd_page1_d2[] = {0xD2,0x00,0x00,0x00,0x2D,0x00,0x5C,
0x00,0x80,0x00,0xAB,0x00,0xE4,0x01,0x15,0x01,0x5C,0x01,0x8E,0x01,0xD3,0x02,
0x03,0x02,0x45,0x02,0x77,0x02,0x78,0x02,0xA4,0x02,0xD1,0x02,0xEA,0x03,0x09,
0x03,0x1A,0x03,0x32,0x03,0x40,0x03,0x59,0x03,0x68,0x03,0x7C,0x03,0xB2,0x03,
0xD8};
static char nt35510_lead_cmd_page1_d3[] = {0xD3,0x00,0x00,0x00,0x2D,0x00,0x5C,
0x00,0x80,0x00,0xAB,0x00,0xE4,0x01,0x15,0x01,0x5C,0x01,0x8E,0x01,0xD3,0x02,
0x03,0x02,0x45,0x02,0x77,0x02,0x78,0x02,0xA4,0x02,0xD1,0x02,0xEA,0x03,0x09,
0x03,0x1A,0x03,0x32,0x03,0x40,0x03,0x59,0x03,0x68,0x03,0x7C,0x03,0xB2,0x03,
0xD8};
static char nt35510_lead_cmd_page1_d4[] = {0xD4,0x00,0x00,0x00,0x2D,0x00,0x5C,
0x00,0x80,0x00,0xAB,0x00,0xE4,0x01,0x15,0x01,0x5C,0x01,0x8E,0x01,0xD3,0x02,
0x03,0x02,0x45,0x02,0x77,0x02,0x78,0x02,0xA4,0x02,0xD1,0x02,0xEA,0x03,0x09,
0x03,0x1A,0x03,0x32,0x03,0x40,0x03,0x59,0x03,0x68,0x03,0x7C,0x03,0xB2,0x03,
0xD8};
static char nt35510_lead_cmd_page1_d5[] = {0xD5,0x00,0x00,0x00,0x2D,0x00,0x5C,
0x00,0x80,0x00,0xAB,0x00,0xE4,0x01,0x15,0x01,0x5C,0x01,0x8E,0x01,0xD3,0x02,
0x03,0x02,0x45,0x02,0x77,0x02,0x78,0x02,0xA4,0x02,0xD1,0x02,0xEA,0x03,0x09,
0x03,0x1A,0x03,0x32,0x03,0x40,0x03,0x59,0x03,0x68,0x03,0x7C,0x03,0xB2,0x03,
0xD8};
static char nt35510_lead_cmd_page1_d6[] = {0xD6,0x00,0x00,0x00,0x2D,0x00,0x5C,
0x00,0x80,0x00,0xAB,0x00,0xE4,0x01,0x15,0x01,0x5C,0x01,0x8E,0x01,0xD3,0x02,
0x03,0x02,0x45,0x02,0x77,0x02,0x78,0x02,0xA4,0x02,0xD1,0x02,0xEA,0x03,0x09,
0x03,0x1A,0x03,0x32,0x03,0x40,0x03,0x59,0x03,0x68,0x03,0x7C,0x03,0xB2,0x03,
0xD8};

static char nt35510_lead_cmd_page0_f0[6] = {0xf0, 0x55,0xaa,0x52,0x08,0x00};
static char nt35510_lead_cmd_page0_b1[2] = {0xb1, 0xf8};   //0xcc
static char nt35510_lead_cmd_page0_b4[2] = {0xb4, 0x10};
static char nt35510_lead_cmd_page0_b6[2] = {0xb6, 0x02};
static char nt35510_lead_cmd_page0_b7[3] = {0xb7, 0x71,0x71};
static char nt35510_lead_cmd_page0_b8[5] = {0xb8, 0x01,0x0A,0x0A,0x0A};
static char nt35510_lead_cmd_page0_bc[4] = {0xbc, 0x05,0x05,0x05};
static char nt35510_lead_cmd_page0_bd[6] = {0xbd, 0x01,0x84,0x07,0x31,0x00};
static char nt35510_lead_cmd_page0_be[6] = {0xbe, 0x01,0x84,0x07,0x31,0x00};
static char nt35510_lead_cmd_page0_bf[6] = {0xbf, 0x01,0x84,0x07,0x31,0x00};

static char nt35510_lead_cmd_page0_c7[2] = {0xc7, 0x02};
static char nt35510_lead_cmd_page0_c9[5] = {0xc9, 0x11,0x00,0x00,0x00};
static char nt35510_lead_cmd_page0_3a[2] = {0x3a, 0x77};
static char nt35510_lead_cmd_page0_35[2] = {0x35, 0x00};
static char nt35510_lead_cmd_page0_21[2] = {0x21, 0x00};

static struct dsi_cmd_desc nt35510_lead_display_on_cmds[] = {

{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_f0), nt35510_lead_cmd_page1_f0},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_bc), nt35510_lead_cmd_page1_bc},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_bd), nt35510_lead_cmd_page1_bd},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_be), nt35510_lead_cmd_page1_be},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_b0), nt35510_lead_cmd_page1_b0},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_b1), nt35510_lead_cmd_page1_b1},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_b9), nt35510_lead_cmd_page1_b9},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_ba), nt35510_lead_cmd_page1_ba},

{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_b6), nt35510_lead_cmd_page1_b6},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_b7), nt35510_lead_cmd_page1_b7},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_b8), nt35510_lead_cmd_page1_b8},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_d1), nt35510_lead_cmd_page1_d1},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_d2), nt35510_lead_cmd_page1_d2},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_d3), nt35510_lead_cmd_page1_d3},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_d4), nt35510_lead_cmd_page1_d4},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_d5), nt35510_lead_cmd_page1_d5},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page1_d6), nt35510_lead_cmd_page1_d6},

{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_f0), nt35510_lead_cmd_page0_f0},
{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_b1), nt35510_lead_cmd_page0_b1},
{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_b4), nt35510_lead_cmd_page0_b4},
{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_b6), nt35510_lead_cmd_page0_b6},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_b7), nt35510_lead_cmd_page0_b7},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_b8), nt35510_lead_cmd_page0_b8},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_bc), nt35510_lead_cmd_page0_bc},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_bd), nt35510_lead_cmd_page0_bd},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_be), nt35510_lead_cmd_page0_be},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_bf), nt35510_lead_cmd_page0_bf},

{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_f0), nt35510_lead_cmd_page0_f0},
{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_c7), nt35510_lead_cmd_page0_c7},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_c9), nt35510_lead_cmd_page0_c9},
{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_3a), nt35510_lead_cmd_page0_3a},
{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_35), nt35510_lead_cmd_page0_35},
{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt35510_lead_cmd_page0_21), nt35510_lead_cmd_page0_21},
//{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(lead_cmd_page0_36), lead_cmd_page0_36},
{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on}

};


/**************************************
7. nt35510 yushun start 
**************************************/
//static char cmd_page_f3[9] = {0xf3, 0x00,0x32,0x00,0x38,0x31,0x08,0x11,0x00};
static char nt3511_yushun_cmd_page0_f0[6] = {0xf0, 0x55,0xaa,0x52,0x08,0x00};
static char nt3511_yushun_cmd_page0_b0[6] = {0xb0, 0x04,0x0a,0x0e,0x09,0x04};
static char nt3511_yushun_cmd_page0_b1[3] = {0xb1, 0x18,0x04};
static char nt3511_yushun_cmd_page0_36[2] = {0x36, 0x90};
static char nt3511_yushun_cmd_page0_b3[2] = {0xb3, 0x00};
static char nt3511_yushun_cmd_page0_b6[2] = {0xb6, 0x03};
static char nt3511_yushun_cmd_page0_b7[3] = {0xb7, 0x70,0x70};
static char nt3511_yushun_cmd_page0_b8[5] = {0xb8, 0x00,0x06,0x06,0x06};
static char nt3511_yushun_cmd_page0_bc[4] = {0xbc, 0x00,0x00,0x00};
static char nt3511_yushun_cmd_page0_bd[6] = {0xbd, 0x01,0x84,0x06,0x50,0x00};
static char nt3511_yushun_cmd_page0_cc[4] = {0xcc, 0x03,0x01,0x06};

static char nt3511_yushun_cmd_page1_f0[6] = {0xf0, 0x55,0xaa,0x52,0x08,0x01};
static char nt3511_yushun_cmd_page1_b0[4] = {0xb0, 0x05,0x05,0x05};
static char nt3511_yushun_cmd_page1_b1[4] = {0xb1, 0x05,0x05,0x05};
static char nt3511_yushun_cmd_page1_b2[4] = {0xb2, 0x03,0x03,0x03};
static char nt3511_yushun_cmd_page1_b8[4] = {0xb8, 0x25,0x25,0x25};
static char nt3511_yushun_cmd_page1_b3[4] = {0xb3, 0x0b,0x0b,0x0b};
static char nt3511_yushun_cmd_page1_b9[4] = {0xb9, 0x34,0x34,0x34};
static char nt3511_yushun_cmd_page1_bf[2] = {0xbf, 0x01};
static char nt3511_yushun_cmd_page1_b5[4] = {0xb5, 0x08,0x08,0x08};
static char nt3511_yushun_cmd_page1_ba[4] = {0xba, 0x24,0x24,0x24};
static char nt3511_yushun_cmd_page1_b4[4] = {0xb4, 0x2e,0x2e,0x2e};
static char nt3511_yushun_cmd_page1_bc[4] = {0xbc, 0x00,0x68,0x00};
static char nt3511_yushun_cmd_page1_bd[4] = {0xbd, 0x00,0x7c,0x00};
static char nt3511_yushun_cmd_page1_be[3] = {0xbe, 0x00,0x45};
static char nt3511_yushun_cmd_page1_d0[5] = {0xd0, 0x0c,0x15,0x0b,0x0e};

static char nt3511_yushun_cmd_page1_d1[53] = {0xd1, 0x00,0x37,0x00,0x61,0x00,0x92,0x00,0xB4,0x00,0xCF,0x00
,0xF6,0x01,0x2F,0x01,0x7F,0x01,0x97,0x01,0xC0,0x01,0xE5,0x02,0x25,0x02,0x5E,0x02,0x60,0x02
,0x87,0x02,0xBE,0x02,0xE2,0x03,0x0F,0x03,0x30,0x03,0x5C,0x03,0x77,0x03,0x94,0x03,0x9F,0x03
,0xAC,0x03,0xBA,0x03,0xF1};
static char nt3511_yushun_cmd_page1_d2[53] = {0xd2, 0x00,0x37,0x00,0x61,0x00,0x92,0x00,0xB4,0x00,0xCF,0x00
,0xF6,0x01,0x2F,0x01,0x7F,0x01,0x97,0x01,0xC0,0x01,0xE5,0x02,0x25,0x02,0x5E,0x02,0x60,0x02
,0x87,0x02,0xBE,0x02,0xE2,0x03,0x0F,0x03,0x30,0x03,0x5C,0x03,0x77,0x03,0x94,0x03,0x9F,0x03
,0xAC,0x03,0xBA,0x03,0xF1};
static char nt3511_yushun_cmd_page1_d3[53] = {0xd3, 0x00,0x37,0x00,0x61,0x00,0x92,0x00,0xB4,0x00,0xCF,0x00
,0xF6,0x01,0x2F,0x01,0x7F,0x01,0x97,0x01,0xC0,0x01,0xE5,0x02,0x25,0x02,0x5E,0x02,0x60,0x02
,0x87,0x02,0xBE,0x02,0xE2,0x03,0x0F,0x03,0x30,0x03,0x5C,0x03,0x77,0x03,0x94,0x03,0x9F,0x03
,0xAC,0x03,0xBA,0x03,0xF1};
static char nt3511_yushun_cmd_page1_d4[53] = {0xd4, 0x00,0x37,0x00,0x50,0x00,0x89,0x00,0xA9,0x00,0xC0,0x00
,0xF6,0x01,0x14,0x01,0x48,0x01,0x6B,0x01,0xA7,0x01,0xD3,0x02,0x17,0x02,0x4F,0x02,0x51,0x02
,0x86,0x02,0xBD,0x02,0xE2,0x03,0x0F,0x03,0x30,0x03,0x5C,0x03,0x77,0x03,0x94,0x03,0x9F,0x03
,0xAC,0x03,0xBA,0x03,0xF1};
static char nt3511_yushun_cmd_page1_d5[53] = {0xd5, 0x00,0x37,0x00,0x50,0x00,0x89,0x00,0xA9,0x00,0xC0,0x00
,0xF6,0x01,0x14,0x01,0x48,0x01,0x6B,0x01,0xA7,0x01,0xD3,0x02,0x17,0x02,0x4F,0x02,0x51,0x02
,0x86,0x02,0xBD,0x02,0xE2,0x03,0x0F,0x03,0x30,0x03,0x5C,0x03,0x77,0x03,0x94,0x03,0x9F,0x03
,0xAC,0x03,0xBA,0x03,0xF1};
static char nt3511_yushun_cmd_page1_d6[53] = {0xd6, 0x00,0x37,0x00,0x50,0x00,0x89,0x00,0xA9,0x00,0xC0,0x00
,0xF6,0x01,0x14,0x01,0x48,0x01,0x6B,0x01,0xA7,0x01,0xD3,0x02,0x17,0x02,0x4F,0x02,0x51,0x02
,0x86,0x02,0xBD,0x02,0xE2,0x03,0x0F,0x03,0x30,0x03,0x5C,0x03,0x77,0x03,0x94,0x03,0x9F,0x03
,0xAC,0x03,0xBA,0x03,0xF1};

static struct dsi_cmd_desc nt3511_yushun_display_on_cmds[] = {

       // yushun nt35510
	//{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_page_ff),cmd_page_ff},
	//{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_page_f3),cmd_page_f3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page0_f0),nt3511_yushun_cmd_page0_f0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page0_b0),nt3511_yushun_cmd_page0_b0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page0_b1),nt3511_yushun_cmd_page0_b1},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page0_36),nt3511_yushun_cmd_page0_36},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page0_b3),nt3511_yushun_cmd_page0_b3},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page0_b6),nt3511_yushun_cmd_page0_b6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page0_b7),nt3511_yushun_cmd_page0_b7},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page0_b8),nt3511_yushun_cmd_page0_b8},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page0_bc),nt3511_yushun_cmd_page0_bc},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page0_bd),nt3511_yushun_cmd_page0_bd},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page0_cc),nt3511_yushun_cmd_page0_cc},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_f0),nt3511_yushun_cmd_page1_f0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_b0),nt3511_yushun_cmd_page1_b0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_b1),nt3511_yushun_cmd_page1_b1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_b2),nt3511_yushun_cmd_page1_b2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_b8),nt3511_yushun_cmd_page1_b8},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_b3),nt3511_yushun_cmd_page1_b3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_b9),nt3511_yushun_cmd_page1_b9},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_bf),nt3511_yushun_cmd_page1_bf},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_b5),nt3511_yushun_cmd_page1_b5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_ba),nt3511_yushun_cmd_page1_ba},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_b4),nt3511_yushun_cmd_page1_b4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_bc),nt3511_yushun_cmd_page1_bc},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_bd),nt3511_yushun_cmd_page1_bd},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_be),nt3511_yushun_cmd_page1_be},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_d0),nt3511_yushun_cmd_page1_d0},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_d1), nt3511_yushun_cmd_page1_d1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_d2), nt3511_yushun_cmd_page1_d2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_d3), nt3511_yushun_cmd_page1_d3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_d4), nt3511_yushun_cmd_page1_d4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_d5), nt3511_yushun_cmd_page1_d5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(nt3511_yushun_cmd_page1_d6), nt3511_yushun_cmd_page1_d6},

	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on}

};


/**************************************
8. 461408 truly start 
**************************************/
static char r61408_truly_lg_para_0xb0[2]={0xB0,0x04}; 
static char r61408_truly_lg_para_0xb3[3]={0xB3,0x10,0x00}; 
static char r61408_truly_lg_para_0xbd[2]={0xbd,0x00}; 
static char r61408_truly_lg_para_0xc0[3]={0xc0,0x00,0x66};
static char r61408_truly_lg_para_0xc1[16]={0xc1,0x23,0x31,0x99,0x26,0x25,0x00,
	0x10,0x28,0x0c,0x0f,0x00,0x00,0x00,0x21,0x01}; // lijiangshuo 0c -> 0f 20131128
static char r61408_truly_lg_para_0xc2[7]={0xc2,0x10,0x06,0x06,0x01,0x03,0x00};
static char r61408_truly_lg_para_0xc8[25]={0xc8,0x00,0x0e,0x17,0x20,0x2e,0x4b,
	0x3b,0x28,0x19,0x11,0x0a,0x02,0x00,0x0e,0x15,0x20,0x2e,0x47,0x3b,0x28,0x19,
	0x11,0x0a,0x02};
static char r61408_truly_lg_para_0xc9[25]={0xc9,0x00,0x0e,0x17,0x20,0x2e,0x4b,
	0x3b,0x28,0x19,0x11,0x0a,0x02,0x00,0x0e,0x15,0x20,0x2e,0x47,0x3b,0x28,0x19,
	0x11,0x0a,0x02};
static char r61408_truly_lg_para_0xca[25]={0xca,0x00,0x0e,0x17,0x20,0x2e,0x4b,
	0x3b,0x28,0x19,0x11,0x0a,0x02,0x00,0x0e,0x15,0x20,0x2e,0x47,0x3b,0x28,0x19,
	0x11,0x0a,0x02};
static char r61408_truly_lg_para_0xd0[17]={0xd0,0x99,0x03,0xce,0xa6,0x0c,0x43,
	0x20,0x10,0x01,0x00,0x01,0x01,0x00,0x03,0x01,0x00};
static char r61408_truly_lg_para_0xd1[8]={0xd1,0x18,0x0c,0x23,0x03,0x75,0x02,0x50};
static char r61408_truly_lg_para_0xd3[2]={0xd3,0x33};
static char r61408_truly_lg_para_0xd5[3]={0xd5,0x2a,0x2a};
static char r61408_truly_lg_para_0xde[3]={0xde,0x01,0x51};
static char r61408_truly_lg_para_0xe6[2]={0xe6,0x51};//vcomdc flick
static char r61408_truly_lg_para_0xfa[2]={0xfa,0x03};
static char r61408_truly_lg_para_0xd6[2]={0xd6,0x28};
static char r61408_truly_lg_para_0x2a[5]={0x2a,0x00,0x00,0x01,0xdf};
static char r61408_truly_lg_para_0x2b[5]={0x2b,0x00,0x00,0x03,0x1f};
static char r61408_truly_lg_para_0x36[2]={0x36,0x00};
static char r61408_truly_lg_para_0x3a[2]={0x3a,0x77};


static struct dsi_cmd_desc r61408_truly_lg_display_on_cmds[] = 
{
	
	
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xb0), r61408_truly_lg_para_0xb0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xb3), r61408_truly_lg_para_0xb3},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xbd), r61408_truly_lg_para_0xbd},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xc0), r61408_truly_lg_para_0xc0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xc1), r61408_truly_lg_para_0xc1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xc2), r61408_truly_lg_para_0xc2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xc8), r61408_truly_lg_para_0xc8},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xc9), r61408_truly_lg_para_0xc9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xca), r61408_truly_lg_para_0xca},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xd0), r61408_truly_lg_para_0xd0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xd1), r61408_truly_lg_para_0xd1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xd3), r61408_truly_lg_para_0xd3},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xd5), r61408_truly_lg_para_0xd5},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xde), r61408_truly_lg_para_0xde},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xe6), r61408_truly_lg_para_0xe6},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xfa), r61408_truly_lg_para_0xfa},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0xd6), r61408_truly_lg_para_0xd6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0x2a), r61408_truly_lg_para_0x2a},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0x2b), r61408_truly_lg_para_0x2b},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0x36), r61408_truly_lg_para_0x36},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_lg_para_0x3a), r61408_truly_lg_para_0x3a},

	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	#ifdef CONFIG_BACKLIGHT_CABC
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_para_CABC_0xb0), r61408_truly_para_CABC_0xb0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_para_CABC_0xb8), r61408_truly_para_CABC_0xb8},
	//{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(r61408_truly_para_CABC_0xb9), r61408_truly_para_CABC_0xb9},
	#endif
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on}

};
//wangminrong YUSHUN R61408E 20120823 update

//////////////////////////////////////////////////
static char r61408_yushun_lg_para_0xB0[2]= {0xB0,0x04,};

static char r61408_yushun_lg_para_0xB3[3]= {0xB3,0x02,0x00,};

static char r61408_yushun_lg_para_0xB6[3]= {0xB6,0x52,0x83,};

static char r61408_yushun_lg_para_0xB7[5]= {0xB7,0x80,0x72,0x11,0x25,};

static char r61408_yushun_lg_para_0xB8[21]= {0xB8,0x00,0x0F,0x0F,0xFF,0xFF,0xC8,0xC8,0x02,0x18,0x10,0x10,0x37,0x5A,0x87,0xBE,0xFF,0x00,0x00,0x00,0x00,};

static char r61408_yushun_lg_para_0xB9[5]= {0xB9,0x00,0xFF,0x02,0x08,};

static char r61408_yushun_lg_para_0xBD[2]= {0xBD,0x00,};

static char r61408_yushun_lg_para_0xC0[3]= {0xC0,0x02,0x87,};

static char r61408_yushun_lg_para_0xC1[16]= {0xC1,0x41,0x31,0x19,0x21,0x21,0x00,0x12,0x28,0x0C,0x0C,0x00,0x00,0x00,0x21,0x01,};

static char r61408_yushun_lg_para_0xC2[7]= {0xC2,0x10,0x06,0x06,0x07,0x01,0x04,};

static char r61408_yushun_lg_para_0xC6[3]= {0xC6,0x00,0x00,};

static char r61408_yushun_lg_para_0xC8[25]= {0xC8,0x00,0x0D,0x1F,0x29,0x36,0x4D,0x37,0x25,0x19,0x11,0x08,0x01,0x00,0x0D,0x1F,0x29,0x36,0x4D,0x37,0x25,0x19,0x11,0x08,0x01,};

static char r61408_yushun_lg_para_0xC9[25]= {0xC9,0x00,0x0D,0x1F,0x29,0x36,0x4D,0x37,0x25,0x19,0x11,0x08,0x01,0x00,0x0D,0x1F,0x29,0x36,0x4D,0x37,0x25,0x19,0x11,0x08,0x01,};

//static char r61408_yushun_lg_para_0xB3[3]= {0xB3,0x12,0x00,};

//static char r61408_yushun_lg_para_0xC0[3]= {0xC0,0x02,0x54,};

static char r61408_yushun_lg_para_0xCA[25]= {0xCA,0x00,0x0D,0x1F,0x29,0x36,0x4D,0x37,0x25,0x19,0x11,0x08,0x01,0x00,0x0D,0x1F,0x29,0x36,0x4D,0x37,0x25,0x19,0x11,0x08,0x01,};

static char r61408_yushun_lg_para_0xD0[17]= {0xD0,0xA9,0x03,0xBD,0xA5,0x0C,0x72,0x20,0x10,0x01,0x00,0x01,0x01,0x00,0x03,0x01,0x00,};

static char r61408_yushun_lg_para_0xD1[8]= {0xD1,0x18,0x0C,0x34,0x03,0x75,0x02,0x50,};

static char r61408_yushun_lg_para_0xD3[2]= {0xD3,0x33,};

static char r61408_yushun_lg_para_0xD5[3]= {0xD5,0x1B,0x1B,};

static char r61408_yushun_lg_para_0xD6[2]= {0xD6,0xA8,};

static char r61408_yushun_lg_para_0xDE[2]= {0xDE,0x01,};

static char r61408_yushun_lg_para_0xE0[5]= {0xE0,0x00,0x00,0x00,0x00,};

static char r61408_yushun_lg_para_0xE1[7]= {0xE1,0x01,0x01,0x01,0x01,0x01,0x00,};

static char r61408_yushun_lg_para_0xFA[2]= {0xFA,0x03,};

static char r61408_yushun_lg_para_0x2A[5]= {0x2A,0x00,0x00,0x01,0xDF,};

static char r61408_yushun_lg_para_0x2B[5]= {0x2B,0x00,0x00,0x03,0x1F,};

static char r61408_yushun_lg_para_0x36[2]= {0x36,0x00,};

static char r61408_yushun_lg_para_0x3A[2]= {0x3A,0x77,};

//static char r61408_yushun_lg_para_0xDE[3]= {0xDE,0x01,0x51,};

static char r61408_yushun_lg_para_0xE6[2]= {0xE6,0x51,};





static struct dsi_cmd_desc r61408_yushun_display_on_cmds[] = {

	

	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xB0),r61408_yushun_lg_para_0xB0},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xB3),r61408_yushun_lg_para_0xB3},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xB6),r61408_yushun_lg_para_0xB6},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xB7),r61408_yushun_lg_para_0xB7},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xB8),r61408_yushun_lg_para_0xB8},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xB9),r61408_yushun_lg_para_0xB9},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xBD),r61408_yushun_lg_para_0xBD},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xC0),r61408_yushun_lg_para_0xC0},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xC1),r61408_yushun_lg_para_0xC1},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xC2),r61408_yushun_lg_para_0xC2},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xC6),r61408_yushun_lg_para_0xC6},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xC8),r61408_yushun_lg_para_0xC8},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xC9),r61408_yushun_lg_para_0xC9},

	//{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xB3),r61408_yushun_lg_para_0xB3},

	//{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xC0),r61408_yushun_lg_para_0xC0},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xCA),r61408_yushun_lg_para_0xCA},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xD0),r61408_yushun_lg_para_0xD0},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xD1),r61408_yushun_lg_para_0xD1},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xD3),r61408_yushun_lg_para_0xD3},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xD5),r61408_yushun_lg_para_0xD5},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xD6),r61408_yushun_lg_para_0xD6},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xDE),r61408_yushun_lg_para_0xDE},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xE0),r61408_yushun_lg_para_0xE0},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xE1),r61408_yushun_lg_para_0xE1},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xFA),r61408_yushun_lg_para_0xFA},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0x2A),r61408_yushun_lg_para_0x2A},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0x2B),r61408_yushun_lg_para_0x2B},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0x36),r61408_yushun_lg_para_0x36},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0x3A),r61408_yushun_lg_para_0x3A},

	//{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xDE),r61408_yushun_lg_para_0xDE},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,sizeof(r61408_yushun_lg_para_0xE6),r61408_yushun_lg_para_0xE6},

	
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on}

};


/**************************************
9. hx8363 YUSHUN IVO start 
**************************************/
#if 0
static char hx8363_yushun_para_0xb1[13]={0xB1,0x78,0x34,0x08,0x34,0x02,0x13,0x11,0x11,0x1C,0x24,0x3F,0x3F};  
static char hx8363_yushun_para_0xba[14]={0xBA,0x80,0x00,0x10,0x08,0x08,0x10,0x7c,0x6e,
								0x6d,0x0a,0x01,0x84,0x43};   //TWO LANE
static char hx8363_yushun_para_0x3a[2]={0x3a,0x77};
//static char hx8363_para_0x36[2]={0x36,0x0a};
static char hx8363_yushun_para_0xb2[4]={0xb2,0x33,0x33,0x22};
static char hx8363_yushun_para_0xb3[2]={0xb3,0x00};
static char hx8363_yushun_para_0xb4[10]={0xb4,0x08,0x12,0x72,0x12,0x06,0x03,0x54,0x03,0x4e};
static char hx8363_yushun_para_0xb6[2]={0xb6,0x2c};
static char hx8363_yushun_para_0xcc[2]={0xcc,0x09};
static char hx8363_yushun_para_0xe0[31]={0x00,0x1E,0x63,0x15,0x13,0x30,0x0C,0xCF,0x0F,0xD5,
	0x17,0xD5,0x96,0xD1,0x17,0x00,0x1E,0x63,0x15,0x13,0x30,0x0C,0xCF,0x0F,0xD5,0x17,0xD5,0x96,0xD1,0x17};	
#endif
static char hx8363_yushun_para_0xb1[13]={0xB1,0x78,0x34,0x07,0x33,0x02,0x13,0x0F,0x00,0x1C,0x24,0x3F,0x3F};  
static char hx8363_yushun_para_0xba[14]={0xBA,0x80,0x00,0x10,0x08,0x08,0x10,0x7C,0x6E,0x6D,0x0A,0x01,0x84,0x43};   //TWO LANE
static char hx8363_yushun_para_0x3a[2]={0x3A,0x70};
//static char hx8363_para_0x36[2]={0x36,0x0a};
static char hx8363_yushun_para_0xb2[4]={0xB2,0x33,0x33,0x22};
static char hx8363_yushun_para_0xb4[10]={0xB4,0x04,0x12,0x72,0x12,0x06,0x03,0x54,0x03,0x4E};
static char hx8363_yushun_para_0xb6[2]={0xB6,0x1C};
static char hx8363_yushun_para_0xcc[2]={0xCC,0x09};
static char hx8363_yushun_para_0xe0[31]={0xE0,0x00,0x1f,0x61,0x0A,0x0A,0x3C,0x0d,0x91,0x50,0x15,0x17,0xD5,0x16,0x13,0x1A,0x00,0x1f,0x61,0x0A,0x0A,0x3C,0x0d,0x91,0x50,0x15,0x17,0xD5,0x16,0x13,0x1A};	
static struct dsi_cmd_desc hx8363_yushun_display_on_cmds[] = 
{

	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_setpassword_para),hx8363_setpassword_para},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_yushun_para_0xb1), hx8363_yushun_para_0xb1},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_yushun_para_0xb2), hx8363_yushun_para_0xb2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_yushun_para_0xba), hx8363_yushun_para_0xba},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8363_yushun_para_0x3a), hx8363_yushun_para_0x3a},
//	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8363_yushun_para_0xb3), hx8363_yushun_para_0xb3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_yushun_para_0xb4), hx8363_yushun_para_0xb4},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8363_yushun_para_0xb6), hx8363_yushun_para_0xb6},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8363_yushun_para_0xcc), hx8363_yushun_para_0xcc},
	//{DTYPE_DCS_WRITE1, 1, 0, 0, 10, sizeof(hx8363_para_0x36), hx8363_para_0x36},		
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8363_yushun_para_0xe0), hx8363_yushun_para_0xe0},
	//{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(hx8363_para_0xc1), hx8363_para_0xc1},
	//{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(hx8363_para_0xc2), hx8363_para_0xc2},	

	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on}

};
/*zhoufan add for nt35512 TM LCD++,20130606*/
static char nt35512_tianma_ips_para0[6] = {0xF0,0x55,0xAA,0x52,0x08,0x01};
static char nt35512_tianma_ips_para1[3] = {0xB0,0x0A,0x00};
static char nt35512_tianma_ips_para2[3] = {0xB6,0x44,0x00};
static char nt35512_tianma_ips_para3[3] = {0xB1,0x0A,0x00};
static char nt35512_tianma_ips_para4[3] = {0xB7,0x34,0x00};
static char nt35512_tianma_ips_para5[3] = {0xB2,0x00,0x00};
static char nt35512_tianma_ips_para6[3] = {0xB8,0x34,0x00};
static char nt35512_tianma_ips_para7[3] = {0xB3,0x06,0x00};
static char nt35512_tianma_ips_para8[3] = {0xB9,0x24,0x00};
static char nt35512_tianma_ips_para9[3] = {0xB5,0x08,0x00};
static char nt35512_tianma_ips_para10[3] = {0xBA,0x14,0x00};
static char nt35512_tianma_ips_para11[4] = {0xBC,0x00,0x78,0x00};
static char nt35512_tianma_ips_para12[4] = {0xBD,0x00,0x78,0x00};
static char nt35512_tianma_ips_para13[3] = {0xBE,0x00,0x75};
static char nt35512_tianma_ips_para14[53] = {0xD1,0x00,0x2C,0x00,0x82,0x00,0xB3,0x00,0xD2,0x00,0xE4,0x01,0x0B,0x01,0x2C,0x01,0x5B,0x01,0x7E,0x01,0xB8,0x01,0xE4,0x02,0x2B,0x02,0x63,0x02,0x65,0x02,0x97,0x02,0xCF,0x02,0xF3,0x03,0x1E,0x03,0x3E,0x03,0x63,0x03,0x77,0x03,0xA4,0x03,0xBF,0x03,0xCF,0x03,0xDD,0x03,0xFF};
static char nt35512_tianma_ips_para15[53] = {0xD2,0x00,0x2C,0x00,0x82,0x00,0xB3,0x00,0xD2,0x00,0xE4,0x01,0x0B,0x01,0x2C,0x01,0x5B,0x01,0x7E,0x01,0xB8,0x01,0xE4,0x02,0x2B,0x02,0x63,0x02,0x65,0x02,0x97,0x02,0xCF,0x02,0xF3,0x03,0x1E,0x03,0x3E,0x03,0x63,0x03,0x77,0x03,0xA4,0x03,0xBF,0x03,0xCF,0x03,0xDD,0x03,0xFF};
static char nt35512_tianma_ips_para16[53] = {0xD3,0x00,0x2C,0x00,0x82,0x00,0xB3,0x00,0xD2,0x00,0xE4,0x01,0x0B,0x01,0x2C,0x01,0x5B,0x01,0x7E,0x01,0xB8,0x01,0xE4,0x02,0x2B,0x02,0x63,0x02,0x65,0x02,0x97,0x02,0xCF,0x02,0xF3,0x03,0x1E,0x03,0x3E,0x03,0x63,0x03,0x77,0x03,0xA4,0x03,0xBF,0x03,0xCF,0x03,0xDD,0x03,0xFF};
static char nt35512_tianma_ips_para17[53] = {0xD4,0x00,0x2C,0x00,0x82,0x00,0xB3,0x00,0xD2,0x00,0xE4,0x01,0x0B,0x01,0x2C,0x01,0x5B,0x01,0x7E,0x01,0xB8,0x01,0xE4,0x02,0x2B,0x02,0x63,0x02,0x65,0x02,0x97,0x02,0xCF,0x02,0xF3,0x03,0x1E,0x03,0x3E,0x03,0x63,0x03,0x77,0x03,0xA4,0x03,0xBF,0x03,0xCF,0x03,0xDD,0x03,0xFF};
static char nt35512_tianma_ips_para18[53] = {0xD5,0x00,0x2C,0x00,0x82,0x00,0xB3,0x00,0xD2,0x00,0xE4,0x01,0x0B,0x01,0x2C,0x01,0x5B,0x01,0x7E,0x01,0xB8,0x01,0xE4,0x02,0x2B,0x02,0x63,0x02,0x65,0x02,0x97,0x02,0xCF,0x02,0xF3,0x03,0x1E,0x03,0x3E,0x03,0x63,0x03,0x77,0x03,0xA4,0x03,0xBF,0x03,0xCF,0x03,0xDD,0x03,0xFF};
static char nt35512_tianma_ips_para19[53] = {0xD6,0x00,0x2C,0x00,0x82,0x00,0xB3,0x00,0xD2,0x00,0xE4,0x01,0x0B,0x01,0x2C,0x01,0x5B,0x01,0x7E,0x01,0xB8,0x01,0xE4,0x02,0x2B,0x02,0x63,0x02,0x65,0x02,0x97,0x02,0xCF,0x02,0xF3,0x03,0x1E,0x03,0x3E,0x03,0x63,0x03,0x77,0x03,0xA4,0x03,0xBF,0x03,0xCF,0x03,0xDD,0x03,0xFF};
static char nt35512_tianma_ips_para20[6] = {0xF0,0x55,0xAA,0x52,0x08,0x00};
//static char nt35512_tianma_ips_para21[4] = {0xB1,0xFC,0x06,0x01};
static char nt35512_tianma_ips_para21[4] = {0xB1,0xFC,0x00,0xff};
static char nt35512_tianma_ips_para22[3] = {0xB5,0x50,0x00};
static char nt35512_tianma_ips_para23[3] = {0xB6,0x05,0x00};
static char nt35512_tianma_ips_para24[3] = {0xB7,0x80,0x80};
static char nt35512_tianma_ips_para25[5] = {0xB8,0x01,0x07,0x07,0x07};
static char nt35512_tianma_ips_para26[3] = {0xBC,0x00,0x00};
static char nt35512_tianma_ips_para27[6] = {0xC9,0x00,0x02,0x50,0x50,0x50};
static char nt35512_tianma_ips_para28[3] = {0xE2,0x07,0xFF};
static char nt35512_tianma_ips_para29[5] = {0xE4,0x87,0x78,0x02,0x20};
static char nt35512_tianma_ips_para30[2] = {0x21,0x00};
static char nt35512_tianma_ips_para31[2] = {0x11,0x00};
static char nt35512_tianma_ips_para32[2] = {0x29,0x00};


 static struct dsi_cmd_desc nt35512_TM_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para0),nt35512_tianma_ips_para0},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para1),nt35512_tianma_ips_para1},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para2),nt35512_tianma_ips_para2},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para3),nt35512_tianma_ips_para3},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para4),nt35512_tianma_ips_para4},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para5),nt35512_tianma_ips_para5},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para6),nt35512_tianma_ips_para6},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para7),nt35512_tianma_ips_para7},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para8),nt35512_tianma_ips_para8},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para9),nt35512_tianma_ips_para9},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para10),nt35512_tianma_ips_para10},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para11),nt35512_tianma_ips_para11},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para12),nt35512_tianma_ips_para12},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para13),nt35512_tianma_ips_para13},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para14),nt35512_tianma_ips_para14},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para15),nt35512_tianma_ips_para15},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para16),nt35512_tianma_ips_para16},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para17),nt35512_tianma_ips_para17},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para18),nt35512_tianma_ips_para18},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para19),nt35512_tianma_ips_para19},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para20),nt35512_tianma_ips_para20},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para21),nt35512_tianma_ips_para21},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para22),nt35512_tianma_ips_para22},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para23),nt35512_tianma_ips_para23},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para24),nt35512_tianma_ips_para24},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para25),nt35512_tianma_ips_para25},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para26),nt35512_tianma_ips_para26},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para27),nt35512_tianma_ips_para27},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para28),nt35512_tianma_ips_para28},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para29),nt35512_tianma_ips_para29},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(nt35512_tianma_ips_para30),nt35512_tianma_ips_para30},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,sizeof(nt35512_tianma_ips_para31),nt35512_tianma_ips_para31},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10,sizeof(nt35512_tianma_ips_para32),nt35512_tianma_ips_para32},
};

/*zhoufan add for nt35512 TM LCD--,20130606*/

static void lcd_panle_reset(void)
{
	gpio_direction_output(GPIO_LCD_RESET,1);
	//msleep(10);
	usleep(10000);
	gpio_direction_output(GPIO_LCD_RESET,0);
	//msleep(10);
	usleep(10000);
	gpio_direction_output(GPIO_LCD_RESET,1);
	//msleep(10);
	usleep(10000);
}

struct mipi_manufacture_ic {
	struct dsi_cmd_desc *readid_tx;
	int readid_len_tx;
	struct dsi_cmd_desc *readid_rx;
	int readid_len_rx;
	int mode;
};
/*
#ifdef CONFIG_LCD_READ_ID
static int mipi_get_manufacture_icid(struct msm_fb_data_type *mfd)
{
	uint32 icid = 0;
	int i ;
	

	 struct mipi_manufacture_ic mipi_manufacture_icid[4] = {
		{hx8363_setpassword_cmd,ARRAY_SIZE(hx8363_setpassword_cmd),&hx8363_icid_rd_cmd,3,1},
		{nt3511_setpassword_cmd,ARRAY_SIZE(nt3511_setpassword_cmd),&nt3511_icid_rd_cmd,3,0},
		{hx8369_setpassword_cmd,ARRAY_SIZE(hx8369_setpassword_cmd),&hx8369_icid_rd_cmd,3,1},
		{r61408_setpassword_cmd,ARRAY_SIZE(r61408_setpassword_cmd),&r61408_icid_rd_cmd,4,1},

	 };

	for(i = 0; i < ARRAY_SIZE(mipi_manufacture_icid) ; i++)
	{	lcd_panle_reset();	
		mipi_dsi_buf_init(&lead_tx_buf);
		mipi_dsi_buf_init(&lead_rx_buf);
		mipi_set_tx_power_mode(1);	
		
		mipi_dsi_cmds_tx(&lead_tx_buf, mipi_manufacture_icid[i].readid_tx,mipi_manufacture_icid[i].readid_len_tx);
		mipi_dsi_cmd_bta_sw_trigger(); 
		
		if(!mipi_manufacture_icid[i].mode)
			mipi_set_tx_power_mode(0);	
		
		mipi_dsi_cmds_rx(mfd,&lead_tx_buf, &lead_rx_buf, mipi_manufacture_icid[i].readid_rx,mipi_manufacture_icid[i].readid_len_rx);

		if(mipi_manufacture_icid[i].mode)
			mipi_set_tx_power_mode(0);
		
		icid = *(uint32 *)(lead_rx_buf.data);
		
		printk("debug read icid is %x\n",icid & 0xffffff);

		switch(icid & 0xffffff){
			case 0x1055:
						return NOVATEK_35510;
			case 0x6383ff:
						return HIMAX_8363;
						
			case 0x6983ff:
						return HIMAX_8369;
			case 0x142201:
						return RENESAS_R61408;
			default:
						break;			
		}

	}
	return 0;
}

static uint32 mipi_get_commic_panleid(struct msm_fb_data_type *mfd,struct dsi_cmd_desc *para,uint32 len,int mode)
{
	uint32 panelid = 0;
	mipi_dsi_buf_init(&lead_tx_buf);
	mipi_dsi_buf_init(&lead_rx_buf);
	mipi_dsi_cmd_bta_sw_trigger(); 
	if(mode)
		mipi_set_tx_power_mode(1);
	else 
		mipi_set_tx_power_mode(0);
	mipi_dsi_cmds_rx(mfd,&lead_tx_buf, &lead_rx_buf, para,len);
	if(mode)
		mipi_set_tx_power_mode(0);
	panelid = *(uint32 *)(lead_rx_buf.data);
	printk("debug read panelid is %x\n",panelid & 0xffffffff);
	return panelid;
}
static uint32 mipi_get_himax8363_panleid(struct msm_fb_data_type *mfd)
{
	uint32 panleid;
	
	panleid =  mipi_get_commic_panleid(mfd,&hx8363_panleid_rd_cmd,1,1);
	switch((panleid >> 8) & 0xff){

		case HIMAX8369_YUSHUN_IVO_ID:
				return (u32)LCD_PANEL_4P0_HX8363_IVO_YUSHUN;
		default:
				return (u32)LCD_PANEL_4P0_HX8363_CMI_YASSY;
	}
}
static uint32 mipi_get_himax8369_panleid(struct msm_fb_data_type *mfd)
{
	uint32 panleid;
	
	panleid =  mipi_get_commic_panleid(mfd,&hx8369_panleid_rd_cmd,1,1);
	switch((panleid>>8) & 0xff){
		case HIMAX8369_TIANMA_TN_ID:
				return (u32)LCD_PANEL_4P0_HIMAX8369_TIANMA_TN;
		case HIMAX8369_TIANMA_IPS_ID:
				return (u32)LCD_PANEL_4P0_HIMAX8369_TIANMA_IPS;
		case HIMAX8369_LEAD_ID:
				return (u32)LCD_PANEL_4P0_HIMAX8369_LEAD;
		case HIMAX8369_LEAD_HANNSTAR_ID:
				return (u32)LCD_PANEL_4P0_HIMAX8369_LEAD_HANNSTAR;
		default:
				return (u32)LCD_PANEL_NOPANEL;
	}
}




static uint32 mipi_get_nt35510_panleid(struct msm_fb_data_type *mfd)
{
	uint32 panleid =  mipi_get_commic_panleid(mfd,&nt3511_panleid_rd_cmd,1,0);
	switch(panleid & 0xff){
		case NT35510_YUSHUN_ID:
				return  (u32)LCD_PANEL_4P0_NT35510_HYDIS_YUSHUN	;
		case NT35510_LEAD_ID:
				return (u32)LCD_PANEL_4P0_NT35510_LEAD;
		default:
				return (u32)LCD_PANEL_NOPANEL;
	}
}

static uint32 mipi_get_icpanleid(struct msm_fb_data_type *mfd )
{
	int icid = 0;

	lcd_panle_reset();
	icid = mipi_get_manufacture_icid(mfd);
	printk("wangmirnong icid is 0x%x-------\r\n",icid);

	switch(icid){
		case HIMAX_8363:		
				LcdPanleID = mipi_get_himax8363_panleid(mfd);
					break;
		case HIMAX_8369:
					LcdPanleID = mipi_get_himax8369_panleid(mfd);
					break;
		case NOVATEK_35510:
					LcdPanleID = mipi_get_nt35510_panleid(mfd);
					break;
		case RENESAS_R61408:
					LcdPanleID = LCD_PANEL_4P0_R61408_TRULY_LG;
			break;
		default:
					LcdPanleID = (u32)LCD_PANEL_NOPANEL;
					printk("warnning cann't indentify the chip\n");
					break;
	}
		
	return LcdPanleID;
}
#endif
*/
static u32 __init get_lcdpanleid_from_bootloader(void)
{
	      switch(lcd_id_type)
		{	
			case 3:
				return (u32)LCD_PANEL_4P0_HX8363_CMI_YASSY;
			case 6:
				return (u32)LCD_PANEL_4P0_HIMAX8369_LEAD;
			case 2:
				return (u32)LCD_PANEL_4P0_NT35510_LEAD;//wangminrong for P865E02 lead ctp lcd
			case 7:
				return (u32)LCD_PANEL_4P0_HIMAX8369_LEAD_HANNSTAR;
			case 4:
				return (u32)LCD_PANEL_4P0_HIMAX8369_TIANMA_TN;
			case 5:
				return (u32)LCD_PANEL_4P0_HIMAX8369_TIANMA_IPS;
			case 9:
				return (u32)LCD_PANEL_4P0_HX8363_IVO_YUSHUN;
			case 8:
			    return (u32)LCD_PANEL_4P0_R61408_TRULY_LG;
			case 20:
			    return (u32)LCD_PANEL_4P0_R61408_YUSHUN_LG;
			case 21:
			    return (u32)LCD_PANEL_4P0_NT35512_TIANMA;
			default:
				break;
		}
		
		return (u32)LCD_PANEL_NOPANEL;
}


static int mipi_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	mipi_set_tx_power_mode(1);

	mipi_dsi_cmds_tx(&lead_tx_buf, display_off_cmds,
			ARRAY_SIZE(display_off_cmds));
	gpio_direction_output(GPIO_LCD_RESET,0);
	msleep(5);
	gpio_direction_output(GPIO_LCD_RESET,1);
	msleep(10);
//	gpio_direction_output(121,0);
	
	return 0;
}




static int first_time_panel_on = 0;
static int first_init = 0;
static int mipi_lcd_on(struct platform_device *pdev)
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
	printk("zhangqi add %s  first_time_panel_on=%d/n",__func__,first_time_panel_on);
	if(first_time_panel_on){
		first_time_panel_on = 0;
		if(first_init == 1)
		{
			first_init = 0;
			return 0;

		}
		
	//	if(LcdPanleID != (u32)LCD_PANEL_NOPANEL)
		//	return 0;
		
	}

//	else
//		{	
		//	LcdPanleID = mipi_get_icpanleid(mfd);
	//	}
		
	
	lcd_panle_reset();
	printk("mipi init start\n");
	mipi_set_tx_power_mode(1);
	switch(LcdPanleID){
		case (u32)LCD_PANEL_4P0_HX8363_CMI_YASSY:
				mipi_dsi_cmds_tx(&lead_tx_buf, hx8363_yassy_display_on_cmds,ARRAY_SIZE(hx8363_yassy_display_on_cmds));
				printk("HIMAX8363_YASS init ok !!\n");
				break;
				
		case (u32)LCD_PANEL_4P0_HIMAX8369_LEAD:
				mipi_dsi_cmds_tx(&lead_tx_buf, hx8369_lead_display_on_cmds,ARRAY_SIZE(hx8369_lead_display_on_cmds));
				printk("HIMAX8369_LEAD init ok !!\n");
				break;
				
		case (u32)LCD_PANEL_4P0_HIMAX8369_LEAD_HANNSTAR:
				mipi_dsi_cmds_tx(&lead_tx_buf, hx8369_lead_hannstar_display_on_cmds,ARRAY_SIZE(hx8369_lead_hannstar_display_on_cmds));
				printk("HIMAX8369_LEAD_HANNSTAR init ok !!\n");
				break;
				
		case (u32)LCD_PANEL_4P0_HIMAX8369_TIANMA_TN:
				mipi_dsi_cmds_tx(&lead_tx_buf, hx8369_tianma_tn_display_on_cmds,ARRAY_SIZE(hx8369_tianma_tn_display_on_cmds));
				printk("HIMAX8369_TIANMA_TN init ok !!\n");
				break;
				
		case (u32)LCD_PANEL_4P0_HIMAX8369_TIANMA_IPS:
				mipi_dsi_cmds_tx(&lead_tx_buf, hx8369_tianma_ips_display_on_cmds,ARRAY_SIZE(hx8369_tianma_ips_display_on_cmds));
				printk("HIMAX8369_TIANMA_IPS init ok !!\n");
				break;
				
		case (u32)LCD_PANEL_4P0_NT35510_LEAD:
				mipi_dsi_cmds_tx(&lead_tx_buf, nt35510_lead_display_on_cmds,ARRAY_SIZE(nt35510_lead_display_on_cmds));
				printk("NT35510_LEAD init ok !!\n");
				break;
				
		case (u32)LCD_PANEL_4P0_NT35510_HYDIS_YUSHUN:
				mipi_dsi_cmds_tx(&lead_tx_buf, nt3511_yushun_display_on_cmds,ARRAY_SIZE(nt3511_yushun_display_on_cmds));
				printk("NT35510_HYDIS_YUSHUN init ok !!\n");
				break;
		case (u32)LCD_PANEL_4P0_R61408_TRULY_LG:
				mipi_dsi_cmds_tx(&lead_tx_buf, r61408_truly_lg_display_on_cmds,ARRAY_SIZE(r61408_truly_lg_display_on_cmds));
				printk("R61408 TRULY LG  init ok !!\n");
			break;
		case (u32)LCD_PANEL_4P0_HX8363_IVO_YUSHUN:
				mipi_dsi_cmds_tx(&lead_tx_buf, hx8363_yushun_display_on_cmds,ARRAY_SIZE(hx8363_yushun_display_on_cmds));
				printk("HIMAX8363_YUSHUN init ok !!\n");
				break;
		case (u32)LCD_PANEL_4P0_R61408_YUSHUN_LG: 
				mipi_dsi_cmds_tx(&lead_tx_buf, r61408_yushun_display_on_cmds,ARRAY_SIZE(r61408_yushun_display_on_cmds));
				printk("R61408 YUSHUN LG  init ok  !!\n");
				break;
		/*zhoufan add for nt35512 TM  LCD++,20130606*/		
		case (u32)LCD_PANEL_4P0_NT35512_TIANMA: 
				mipi_dsi_cmds_tx(&lead_tx_buf, nt35512_TM_display_on_cmds,ARRAY_SIZE(nt35512_TM_display_on_cmds));
				printk("nt35512 tianma  init ok  !!\n");
				break;		
		/*zhoufan add for nt35512 TM  LCD--,20130606*/			
		default:				
				printk("can't get icpanelid value\n");				
				break;
				
	}	
	mipi_set_tx_power_mode(0);
	return 0;
}



static struct msm_fb_panel_data lead_panel_data = {
	.on		= mipi_lcd_on,
	.off		= mipi_lcd_off,
//[ECID 000000] zhangqi add for CABC begin
#ifdef CONFIG_BACKLIGHT_CABC
	.set_backlight = mipi_set_backlight,
#else
	.set_backlight = mipi_zte_set_backlight,
#endif
};

//[ECID 000000]zhangqi add for CABC begin
#ifdef CONFIG_BACKLIGHT_CABC
void mipi_set_backlight(struct msm_fb_data_type *mfd)
{
         /*value range is 1--32*/
	 int current_lel = mfd->bl_level;
	 //unsigned long flags;
	 
	 pr_debug("zhangqi add for CABC level=%d lcd_type=%d in %s func \n ",current_lel,lcd_id_type,__func__);
	 
	 if (current_lel >32)
	 	{
	 			printk("Backlight level >32 ? return error. CABC level=%d in %s func \n ",current_lel,__func__);
	 			return;
	 	}
	 	


	 if (lcd_id_type==4 || lcd_id_type==5 || lcd_id_type==6 || lcd_id_type ==7 )
	 {
	   pr_debug("zhangqi add for CABC it is a hx8369 IC \n");
	   if(current_lel==0)
	   {
	    mipi_set_tx_power_mode(0);
		  mipi_dsi_cmds_tx(&lead_tx_buf, hx8369_display_off_CABC_backlight_cmds,ARRAY_SIZE( hx8369_display_off_CABC_backlight_cmds));
			mipi_set_tx_power_mode(1);
			//msleep(500);

	   }
	   else
	   {
	    hx8369_para_CABC_0x51[1]=((11*current_lel)/2-1);     
	    mipi_set_tx_power_mode(0);
		  mipi_dsi_cmds_tx(&lead_tx_buf, hx8369_display_on_CABC_backlight_cmds,ARRAY_SIZE( hx8369_display_on_CABC_backlight_cmds));
 			mipi_set_tx_power_mode(1);
	   }

	 }
	 else if(lcd_id_type==3 || lcd_id_type==9)
	 	{	
	 		pr_debug("zhangqi add for CABC it is a hx8363 IC \n");
	 		if(current_lel==0)
	   {
	    //  hx8363_para_CABC_0x51[1]=0x00;
			mipi_set_tx_power_mode(0);
			mipi_dsi_cmds_tx(&lead_tx_buf, hx8363_display_off_CABC_backlight_cmds,ARRAY_SIZE( hx8363_display_off_CABC_backlight_cmds));
			mipi_set_tx_power_mode(1);
			//msleep(500);
	   }
	 		else
	   {
	      hx8363_para_CABC_0x51[1]=((11*current_lel)/2-1);//;      
			mipi_set_tx_power_mode(0);
			mipi_dsi_cmds_tx(&lead_tx_buf, hx8363_display_on_CABC_backlight_cmds,ARRAY_SIZE( hx8363_display_on_CABC_backlight_cmds));
			mipi_set_tx_power_mode(1);
	   		}
	 }
	 else if(lcd_id_type==8)
	 {
		printk("PM_DEBUG_MXP: add for CABC it is a R61408_Truly IC. \n");
		if(current_lel==0)
	   	{
	    		printk(KERN_NOTICE "PM_DEBUG_MXP:current_lel = 0.\r\n");
			mipi_set_tx_power_mode(0);
			mipi_dsi_cmds_tx(&lead_tx_buf, r61408_truly_display_off_CABC_backlight_cmds,ARRAY_SIZE( r61408_truly_display_off_CABC_backlight_cmds));
			mipi_set_tx_power_mode(1);
		//	msleep(500);
	   	}
	 	else
	   	{
	   		printk(KERN_NOTICE "PM_DEBUG_MXP:current_lel = %d.\r\n",current_lel);
			//r61408_truly_para_CABC_0xb9_on[1]=0x01;            //zhoufan--
	      		r61408_truly_para_CABC_0xb9_on[2]=((15*current_lel)/2-1);
			mipi_set_tx_power_mode(0);
			mipi_dsi_cmds_tx(&lead_tx_buf, r61408_truly_display_on_CABC_backlight_cmds,ARRAY_SIZE( r61408_truly_display_on_CABC_backlight_cmds));
			mipi_set_tx_power_mode(1);
	   	}
		
	 }
	 else if(lcd_id_type==21)
	 {
		printk("PM_DEBUG_MXP: add for CABC it is TM nt35512  IC. \n");
		if(current_lel==0)
	   	{
			mipi_set_tx_power_mode(0);
			mipi_dsi_cmds_tx(&lead_tx_buf, nt35512_display_off_CABC_backlight_cmds,ARRAY_SIZE( nt35512_display_off_CABC_backlight_cmds));
			mipi_set_tx_power_mode(1);
		//	msleep(500);
	   	}
	 	else
	   	{
	      		nt35512_para_CABC_0x51[1]=((15*current_lel)/2-1); 
			mipi_set_tx_power_mode(0);
			mipi_dsi_cmds_tx(&lead_tx_buf, nt35512_display_on_CABC_backlight_cmds,ARRAY_SIZE( nt35512_display_on_CABC_backlight_cmds));
			mipi_set_tx_power_mode(1);
	   	}
		
	 }
	 else
	 	
	 	 printk("zhangqi add for CABC it is ?? IC \n");
	 return;


	 
}
#endif
//[ECID 000000]zhangqi add for CABC end
static int ch_used[3];

int mipi_lead_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_lead", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	lead_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &lead_panel_data,
		sizeof(lead_panel_data));
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

static int __devinit mipi_lead_lcd_probe(struct platform_device *pdev)
{	
	struct msm_panel_info   *pinfo =&( ((struct msm_fb_panel_data  *)(pdev->dev.platform_data))->panel_info);
	
	if (pdev->id == 0) return 0;

	mipi_dsi_buf_alloc(&lead_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&lead_rx_buf, DSI_BUF_SIZE);
	if((LcdPanleID = get_lcdpanleid_from_bootloader() )==(u32)LCD_PANEL_NOPANEL)
		pr_debug("cann't get get_lcdpanelid from bootloader\n");
	
	if (LcdPanleID ==LCD_PANEL_4P0_R61408_TRULY_LG)//this panel is different from others
	{
		pinfo->lcdc.h_back_porch = 80;
		pinfo->lcdc.h_front_porch = 140;	
		pinfo->lcdc.v_back_porch = 12;	
	}

	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_lead_lcd_probe,
	.driver = {
		.name   = "mipi_lead",
	},
};

static int __init mipi_lcd_init(void)
{
	return platform_driver_register(&this_driver);
}

module_init(mipi_lcd_init);

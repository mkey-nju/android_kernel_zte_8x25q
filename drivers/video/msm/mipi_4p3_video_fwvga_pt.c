/* Copyright (c) 2009-2011, Code Aurora Forum. All rights reserved.
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
#include "mipi_4p3_video_fwvga.h"


static struct msm_panel_info pinfo;

static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db ={

		/* DSI Bit Clock at 500 MHz, 2 lane, RGB888 */
#if  defined(CONFIG_PROJECT_P825F02) || defined(CONFIG_PROJECT_P865F04) || defined(CONFIG_PROJECT_P865F05) || defined(CONFIG_PROJECT_P825V20)
	/* regulator */
#if 1		
	/* regulator */
	{0x03, 0x01, 0x01, 0x00},
	/* timing   */			/*452M*/
	{0x80, 0x30, 0x12, 0x00, 0x40, 0x4a, 0x16, 0x34,
	0x16, 0x03, 0x04},
	/* phy ctrl */
	{0x7f, 0x00, 0x00, 0x00},
	/* strength */
	{0xbb, 0x02, 0x06, 0x00},
	/* pll control */
	{0x01, 0xbe, 0x31, 0xd2, 0x00, 0x40, 0x37, 0x62,
	0x01, 0x0f, 0x07,
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0},	
#else
        /* regulator */
	{0x03, 0x01, 0x01, 0x00},
	/* timing   */			/*447M*/
	{0x7e, 0x30, 0x12, 0x00, 0x40, 0x4b, 0x16, 0x34,
	0x15, 0x03, 0x04},
	/* phy ctrl */
	{0x7f, 0x00, 0x00, 0x00},
	/* strength */
	{0xbb, 0x02, 0x06, 0x00},
	/* pll control */
	{0x01, 0xb9, 0x31, 0xd2, 0x00, 0x40, 0x37, 0x62,
	0x01, 0x0f, 0x07,
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0},
#endif
#endif
};


static int __init mipi_4p3_video_fwvga_pt_init(void)
{
	int ret;


	if (msm_fb_detect_client("video_fwvga_pt"))
		return 0;

printk("wangminrong the lcd is video_fwvga_pt\r\n");

	pinfo.xres = 480;
	pinfo.yres = 854;

	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;

	pinfo.lcdc.h_back_porch = 112;//100
	pinfo.lcdc.h_front_porch = 112;//100

	pinfo.lcdc.h_pulse_width = 10;
	pinfo.lcdc.v_back_porch = 26;//26;//lead panel must use 26
	pinfo.lcdc.v_front_porch = 10;
	pinfo.lcdc.v_pulse_width = 10;
	//pinfo.clk_rate = 447000000;//447000000;
        pinfo.clk_rate = 452000000;//452000000;

	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 32;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;

	pinfo.mipi.mode = DSI_VIDEO_MODE;
	pinfo.mipi.pulse_mode_hsa_he = TRUE;
	pinfo.mipi.hfp_power_stop = TRUE;
	pinfo.mipi.hbp_power_stop = TRUE;
	pinfo.mipi.hsa_power_stop = TRUE;
	pinfo.mipi.eof_bllp_power_stop = TRUE;
	pinfo.mipi.bllp_power_stop = TRUE;

	pinfo.mipi.traffic_mode = 0;
	pinfo.mipi.dst_format =  DSI_VIDEO_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
	pinfo.mipi.t_clk_post = 0x20;
	pinfo.mipi.t_clk_pre = 0x2F;
	pinfo.mipi.stream = 0; /* dma_p */
	pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_NONE;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.frame_rate = 60;
	pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db;
	//pinfo.mipi.dlane_swap = 0x01;
	pinfo.mipi.tx_eot_append = 0x01;
	ret = mipi_4p3_video_fwvga_device_register(&pinfo, MIPI_DSI_PRIM,
						MIPI_DSI_PANEL_FWVGA_PT);
	if (ret)
		printk(KERN_ERR "%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_4p3_video_fwvga_pt_init);

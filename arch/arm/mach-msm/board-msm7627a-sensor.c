/* Copyright (c) 2012-2013, The Linux Foundation. All Rights Reserved.
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
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <asm/mach-types.h>
#include <linux/i2c.h>
#include "devices-msm7x2xa.h"
#include <linux/regulator/consumer.h>

#include <linux/taos_common.h>
#include <linux/kxtik.h>
#include <linux/akm8963.h>
#include <linux/lis3dh.h>
#include <linux/l3g4200d.h>

#include <linux/input/smtc/misc/sx9500_platform_data.h>
#include <linux/input/smtc/misc/sx95xx_i2c_reg.h>
#include <linux/input/smtc/misc/smtc_sar_test_platform_data.h>
#include <linux/input.h>

#if  defined(CONFIG_PROJECT_P825F02) || defined(CONFIG_PROJECT_P865F04)|| defined(CONFIG_PROJECT_P865G01) || defined(CONFIG_PROJECT_P865F05)  || defined(CONFIG_PROJECT_P825V20)
#define ACC_SENSOR_I2C_NUM 1
#define AKM_SENSOR_I2C_NUM 1
#define TAOS_SENSOR_I2C_NUM 1
#define GYRO_SENSOR_I2C_NUM 1
#else
#define ACC_SENSOR_I2C_NUM 1
#define AKM_SENSOR_I2C_NUM 1
#define TAOS_SENSOR_I2C_NUM 1
#define GYRO_SENSOR_I2C_NUM 1
#endif

#define LIS3DH_I2C_ADDR 0x19
#if  defined(CONFIG_PROJECT_P825F02) || defined(CONFIG_PROJECT_P865F04)|| defined(CONFIG_PROJECT_P865G01) || defined(CONFIG_PROJECT_P865F05)  || defined(CONFIG_PROJECT_P825V20)
#define AK8963_I2C_ADDR 0X0E
#else
#define AK8963_I2C_ADDR 0X0E
#endif

#define KIONIX_I2C_ADDR 0X0F
#define L3G4200D_I2C_ADDR 0X68
#define TAOS_I2C_ADDR 0X39

#ifdef CONFIG_TI_ST_ACCEL_LIS3DH
static struct lis3dh_acc_platform_data lis3dh_pdata = {
	.poll_interval = 100,          //Driver polling interval as 100ms
	.min_interval = 5,    //Driver polling interval minimum 5ms
	.g_range = LIS3DH_ACC_G_2G,    //Full Scale of LSM303DLH Accelerometer  
#if  defined(CONFIG_PROJECT_P825F02) || defined(CONFIG_PROJECT_P865F04) || defined(CONFIG_PROJECT_P825V20)
	.axis_map_x = 0,      //x = x
	.axis_map_y = 1,      //y = y
	.axis_map_z = 2,      //z = z 
	.negate_x = 0,      //x = +x 
	.negate_y = 1,      //y = -y 
	.negate_z = 1,      //z = +z 
#elif  defined(CONFIG_PROJECT_P865G01)
	.axis_map_x = 0,      //x = x
	.axis_map_y = 1,      //y = y
	.axis_map_z = 2,      //z = z 
	.negate_x = 1,      //x = +x 
	.negate_y = 1,      //y = -y 
	.negate_z = 0,      //z = +z
#elif  defined(CONFIG_PROJECT_P865F05)
	.axis_map_x = 0,      //x = x
	.axis_map_y = 1,      //y = y
	.axis_map_z = 2,      //z = z 
	.negate_x = 1,      //x = +x 
	.negate_y = 0,      //y = -y 
	.negate_z = 1,      //z = +z
#else
	.axis_map_x = 0,      //x = x
	.axis_map_y = 1,      //y = y
	.axis_map_z = 2,      //z = z 
	.negate_x = 0,      //x = +x 
	.negate_y = 1,      //y = -y 
	.negate_z = 1,      //z = +z 
#endif
	.gpio_int1 = -1,
	.gpio_int2 = -1,
}; 

static struct i2c_board_info lis3dh_i2c_boardinfo[] = {
        {
                I2C_BOARD_INFO(LIS3DH_ACC_DEV_NAME, LIS3DH_I2C_ADDR),
                .platform_data  = &lis3dh_pdata,
        },
};
#endif
#ifdef CONFIG_TI_ST_COMPASS_AK8963
static  struct akm8963_platform_data akm8963_pdata = { 
    #if defined(CONFIG_PROJECT_P865E02)||defined(CONFIG_PROJECT_P865F03)
        .layout = 7,
    #elif defined(CONFIG_PROJECT_V72A)
        .layout = 3,
    #elif defined(CONFIG_PROJECT_P825B20)||defined(CONFIG_PROJECT_P825A20)
        .layout = 5,
    #else
        .layout = 7,
    #endif
        .outbit = 1,
        .gpio_DRDY = 0,
        .gpio_RST = 0,
};
static struct i2c_board_info akm_compass_i2c_boardinfo[] = {
        {
                I2C_BOARD_INFO(AKM8963_I2C_NAME, AK8963_I2C_ADDR),
                .platform_data  = &akm8963_pdata,
		.irq = MSM_GPIO_TO_INT(ZTE_GPIO_COMPASS_SENSOR_IRQ),
        },
};
static void compass_sensor_init(void)
{
	 int ret = 0;
	 ret = gpio_request(ZTE_GPIO_COMPASS_SENSOR_IRQ, "ak8963");
        if (ret < 0) {
                pr_err("%s: gpio_request compass_irq failed %d\n", __func__, ret);
        }
        ret = gpio_tlmm_config(GPIO_CFG(ZTE_GPIO_COMPASS_SENSOR_IRQ, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
        if (ret < 0) {
                pr_err("%s: gpio_direction_input compass_irq failed %d\n", __func__, ret);
        }

	i2c_register_board_info(AKM_SENSOR_I2C_NUM, akm_compass_i2c_boardinfo,ARRAY_SIZE(akm_compass_i2c_boardinfo));
}
#endif

#ifdef CONFIG_ZTE_SENSORS_KXTF9

static struct kxtik_platform_data kxtik_pdata = {
        .min_interval = 5,
        .poll_interval = 200,
		
	#if  defined(CONFIG_PROJECT_P825F02) || defined(CONFIG_PROJECT_P865F04) || defined(CONFIG_PROJECT_P825V20)
		.axis_map_x = 0,      //x = x
		.axis_map_y = 1,      //y = y
		.axis_map_z = 2,      //z = z 
		.negate_x = 0,      //x = +x 
		.negate_y = 1,      //y = -y 
		.negate_z = 1,      //z = +z 
	#elif  defined(CONFIG_PROJECT_P865G01)
	.axis_map_x = 0,      //x = x
	.axis_map_y = 1,      //y = y
	.axis_map_z = 2,      //z = z 
	.negate_x = 1,      //x = +x 
	.negate_y = 1,      //y = -y 
	.negate_z = 0,      //z = +z
	#elif  defined(CONFIG_PROJECT_P865F05)
	.axis_map_x = 0,      //x = x
	.axis_map_y = 1,      //y = y
	.axis_map_z = 2,      //z = z 
	.negate_x = 1,      //x = +x 
	.negate_y = 0,      //y = -y 
	.negate_z = 1,      //z = +z
	#else
		.axis_map_x = 0,      //x = x
		.axis_map_y = 1,      //y = y
		.axis_map_z = 2,      //z = z 
		.negate_x = 0,      //x = +x 
		.negate_y = 1,      //y = -y 
		.negate_z = 1,      //z = +z 
	#endif	
	
        .res_12bit      = RES_12BIT,
        .g_range        = KXTIK_G_2G,
};

static struct i2c_board_info kionix_i2c_boardinfo[] = {
        {
                I2C_BOARD_INFO("kxtik", KIONIX_I2C_ADDR),
                .platform_data  = &kxtik_pdata,
        },
};
#endif

#ifdef CONFIG_INPUT_LIGHTSENSOR_TAOS
static struct i2c_board_info  taos_i2c_boardinfo[] __initdata ={
        {
                I2C_BOARD_INFO("taos", TAOS_I2C_ADDR),
                .irq = MSM_GPIO_TO_INT(ZTE_GPIO_LIGHT_SENSOR_IRQ),
        },
};

static void light_sensor_init(void)
{
	 int ret =0;
        ret = gpio_request(ZTE_GPIO_LIGHT_SENSOR_IRQ, "light_sensor_irq");
        if (ret < 0) {
                pr_err("%s: gpio_request light_sensor_irq failed %d\n", __func__, ret);
        }	
        ret = gpio_tlmm_config(GPIO_CFG(ZTE_GPIO_LIGHT_SENSOR_IRQ, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
        if (ret < 0) {
                pr_err("%s: gpio_direction_input light_sensor_irq failed %d\n", __func__, ret);
        }

	i2c_register_board_info(TAOS_SENSOR_I2C_NUM, taos_i2c_boardinfo,ARRAY_SIZE(taos_i2c_boardinfo));
}
#endif

#if defined(CONFIG_TI_ST_ACCEL_LIS3DH) || defined(CONFIG_ZTE_SENSORS_KXTF9)
static void accel_sensor_init(void)
{
	int ret =0;
	ret = gpio_request(ZTE_GPIO_ACCEL_SENSOR_IRQ1, "accel_irq");
	if (ret < 0) {
	        pr_err("%s: gpio_request accel_irq1 failed %d\n", __func__, ret);
	}
	ret = gpio_tlmm_config(GPIO_CFG(ZTE_GPIO_ACCEL_SENSOR_IRQ1, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (ret < 0) {
	        pr_err("%s: gpio_direction_input accel_irq1 failed %d\n", __func__, ret);
	}

	#ifdef CONFIG_TI_ST_ACCEL_LIS3DH
	i2c_register_board_info(ACC_SENSOR_I2C_NUM, lis3dh_i2c_boardinfo,ARRAY_SIZE(lis3dh_i2c_boardinfo));
	#endif

	#ifdef CONFIG_ZTE_SENSORS_KXTF9
	i2c_register_board_info(ACC_SENSOR_I2C_NUM, kionix_i2c_boardinfo,ARRAY_SIZE(kionix_i2c_boardinfo));
	#endif	
}
#endif

#ifdef CONFIG_TI_ST_GYRO_L3G4200D
static struct l3g4200d_gyr_platform_data l3g4200d_pdata = {
	.fs_range = L3G4200D_GYR_FS_2000DPS,
#if  defined(CONFIG_PROJECT_P825F02) || defined(CONFIG_PROJECT_P865F04)	|| defined(CONFIG_PROJECT_P865G01) || defined(CONFIG_PROJECT_P865F05) || defined(CONFIG_PROJECT_P825V20)
        .axis_map_x = 0, 
	.axis_map_y = 1,
	.axis_map_z = 2,
	.negate_x = 1,
	.negate_y = 0,
	.negate_z = 1,
#endif
	.poll_interval=10,
	.min_interval=2,
};
static struct i2c_board_info  l3g4200d_i2c_boardinfo[] __initdata ={
        {
                I2C_BOARD_INFO(L3G4200D_GYR_DEV_NAME, L3G4200D_I2C_ADDR),
                .platform_data  = &l3g4200d_pdata,
        },
};

static void gyro_sensor_init(void)
{
	i2c_register_board_info(GYRO_SENSOR_I2C_NUM, l3g4200d_i2c_boardinfo,ARRAY_SIZE(l3g4200d_i2c_boardinfo));	
}
#endif
static void sensor_power_init(void)
{
#if  defined(CONFIG_PROJECT_P825F02) || defined(CONFIG_PROJECT_P865F04)|| defined(CONFIG_PROJECT_P865G01) || defined(CONFIG_PROJECT_P825V20)
	const char *sensor_reg_ldo = NULL;
#else
	const char *sensor_reg_ldo = NULL;
#endif
	int ret = 0;
	struct regulator *sensor_reg = NULL;
	if(sensor_reg_ldo){
		sensor_reg = regulator_get(NULL , sensor_reg_ldo);
		if (IS_ERR(sensor_reg)) {
	       pr_err("%s: vreg get failed with : (%ld)\n",
		                __func__, PTR_ERR(sensor_reg));
		}
		
		/* Set the voltage level to 2.85V */
		ret = regulator_set_voltage(sensor_reg, 2850000, 2850000);
		if (ret < 0) {
		       pr_err("%s: set regulator level failed with :(%d)\n",
		              __func__, ret);
			regulator_put(sensor_reg);
		}
		/* Enabling the 2.85V regulator */
		ret = regulator_enable(sensor_reg);
		if (ret) {
		       pr_err("%s: enable regulator failed with :(%d)\n",
		              __func__, ret);
			regulator_put(sensor_reg);
		}
	} 
}

//added by yangze for SX9502 sensor test 20121015 start
#ifdef CONFIG_CAP_PROX_SX9500_ZTE
static int sx9500_get_nirq_state(void) 
{
	return !gpio_get_value(ZTE_GPIO_SAR_SENSOR_IRQ);
}

/* Define Registers that need to be initialized to values different than default*/
static struct smtc_reg_data sx9500_i2c_reg_setup[] = {	
	{
		.reg = 0x00,
		.val = 0x01,
	},
	{
		.reg = 0x01,
		.val = 0x00,
	},
	{
		.reg = 0x02,
		.val = 0x00,
	},
	{
		.reg = 0x04,
		.val = 0x00,
	},	
	{
		.reg = 0x05,
		.val = 0x00,
	},
	{
		.reg = 0x06,
		.val = 0x01,//0x03 -20121024 changed
	},		
	{
		.reg = SX950X_NOP0_REG,
		.val = 0x68,
	},
	{
		.reg = SX950X_NCPS1_REG,
		.val = 0x43,
	},
	{
		.reg = SX950X_NCPS2_REG,
		.val = 0x67,//0x43
	},
	{
		.reg = SX950X_NCPS3_REG,
		.val = 0x01,//0x08
	},
	{
		.reg = SX950X_NCPS4_REG,
		.val = 0x7D,//0xff
	},
	{
		.reg = SX950X_NCPS5_REG,
		.val = 0x4e,
	},
	{
		.reg = SX950X_NCPS6_REG,
		.val = 0x08,
	},
	{
		.reg = SX950X_NCPS7_REG,
		.val = 0x0A,//0x00 -20121024 changed for debounce
	},
	{
		.reg = SX950X_NCPS8_REG,
		.val = 0x00,
	},
};
/* Define SAR test configuration.*/
static smtc_sar_test_platform_data_t smtc_sar_test_cfg = {
  .keycode = KEY_0,
};

static sx9500_platform_data_t sx9500_config = {
	/* Function pointer to get the NIRQ state (1->NIRQ-low, 0->NIRQ-high) */
	.get_is_nirq_low = sx9500_get_nirq_state,
	/*  pointer to an initializer function. Here in case needed in the future */
	.init_platform_hw = 0,
	/*  pointer to an exit function. Here in case needed in the future */
	.exit_platform_hw = 0,
	.pi2c_reg = sx9500_i2c_reg_setup,
	.i2c_reg_num = ARRAY_SIZE(sx9500_i2c_reg_setup),
	.psar_platform_data = &smtc_sar_test_cfg,
};

static struct i2c_board_info cap_i2c_sensors_list[] = {
	{
	I2C_BOARD_INFO("sx9500", SAR_I2C_ADDR),
	.irq = MSM_GPIO_TO_INT(ZTE_GPIO_SAR_SENSOR_IRQ),
	.platform_data = &sx9500_config,
	},
};

static void prox_sensor_init(void)
{
	int err;
	printk("sx9500_init \r\n");

	err = gpio_request(ZTE_GPIO_SAR_SENSOR_IRQ, "sar_irq_pin");
	if (err < 0)
		printk("gpio_request(ZTE_GPIO_SAR_SENSOR_IRQ, sar_irq_pin) err \r\n");

	err = gpio_direction_input(ZTE_GPIO_SAR_SENSOR_IRQ);
	if (err < 0)
		printk("gpio_direction_input(ZTE_GPIO_SAR_SENSOR_IRQ) err \r\n");

	err = gpio_request(ZTE_GPIO_SAR_SENSOR_TXEN, "sar_txen_pin");
	if (err < 0)
		printk("gpio_request(ZTE_GPIO_SAR_SENSOR_TXEN, sar_txen_pin) err \r\n");

	err = gpio_direction_output(ZTE_GPIO_SAR_SENSOR_TXEN, 1);
	if (err < 0)
		printk("gpio_direction_output(ZTE_GPIO_SAR_SENSOR_TXEN, 1) err \r\n");
  
  i2c_register_board_info(SAR_SENSOR_I2C_NUM, cap_i2c_sensors_list, ARRAY_SIZE(cap_i2c_sensors_list));
}
#endif
//added by yangze for SX9502 sensor test 20121015 end

void __init msm7x27a_sensor_init(void)
{
	sensor_power_init();
  /* ACCEL sensor_init */
	#if defined(CONFIG_TI_ST_ACCEL_LIS3DH) || defined(CONFIG_ZTE_SENSORS_KXTF9)
	accel_sensor_init();
#endif
  /* COMPASS sensor init*/
	#ifdef CONFIG_TI_ST_COMPASS_AK8963
	compass_sensor_init();
        #endif
  /*Light&&Proximity sensor init*/
	#ifdef CONFIG_INPUT_LIGHTSENSOR_TAOS
	light_sensor_init();
#endif
	#ifdef CONFIG_TI_ST_GYRO_L3G4200D
	gyro_sensor_init();
        #endif
	#ifdef CONFIG_CAP_PROX_SX9500_ZTE
	prox_sensor_init();
#endif
}


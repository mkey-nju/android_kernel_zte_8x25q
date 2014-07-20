/* linux/arch/arm/mach-msm/board-zte-wifi.c
*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/skbuff.h>
#include <linux/wlan_plat.h>
#include <linux/if.h> /*For IFHWADDRLEN */

#include "devices.h"
#include <mach/proc_comm.h>  /* For msm_proc_cmd */   //yaotierui for JB 2020 V2
#include <linux/gpio.h>

#include <linux/random.h>
#include <linux/jiffies.h>
#include <mach/rpc_pmapp.h>

#include <mach/pmic.h>
#include <mach/vreg.h>
#include <linux/proc_fs.h> //added by fanjiankang for write proc info

#include <linux/export.h>  //yaotierui for JB 2020 V2

typedef struct _calData {
    s8    olpcGainDelta;                     //----- for all rate. 
    u8   pad[3];
    s16   olpcGainDelta_t10_G_0;             //----- =  olpcGainDelta *5. 
    u8   desiredScaleCck_t10;               //----- for 11b rate. 
    u8   desiredScale6M_t10;                //----- for 6M,9M,12M,18M. 
    u8   desiredScale36M_t10;               //----- for 24M, 36M. 
    u8   desiredScale54M_t10;               //----- for 48M, 54M. 
    u8   desiredScaleMCS0HT20_t10;          //----- for MCS0, MCS1. 
    u8   desiredScaleMCS7HT20_t10;          //----- for MCS2 ~MCS7. 
}__attribute__ ((packed)) calData;

static calData ncal[3];
static int nv_tag;

//[ECID 000000] fanjiankang add for ic information add 20120314 begin
static char wlan_info[50] = "Device manufacturing:Atheros Model Number:AR6005";
static ssize_t wlan_info_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len = strlen(wlan_info);
	sprintf(page, "%s\n", wlan_info);
	return len + 1;
}
static void create_wlan_info_proc_file(void)
{
  struct proc_dir_entry *wlan_info_proc_file = create_proc_entry("driver/wlan_info", 0644, NULL);
  printk("goes to create_wlan_info_proc_file\n");
  if (wlan_info_proc_file) {
			wlan_info_proc_file->read_proc = wlan_info_read_proc;
   } else
	printk(KERN_INFO "proc file create failed!\n");
}

#ifdef CONFIG_PROJECT_P825V20
static int wlan_clock_req(int on)
{
    const char *id = "WLPW";
    int ret = 0;
 
            printk("enter wlan_clock_req\n");
    if (on) {
        ret = pmapp_clock_vote(id, PMAPP_CLOCK_ID_A0,
                        PMAPP_CLOCK_VOTE_ON);
        if (ret < 0) {
            printk("Failed to vote for TCXO_A0 ON\n");
            return ret;
        }
        msleep(20);
        ret = pmapp_clock_vote(id, PMAPP_CLOCK_ID_A0,
                                  PMAPP_CLOCK_VOTE_PIN_CTRL);
        if (ret < 0) {
            printk("%s:Pin Control Failed, rc = %d",
                                        __func__, ret);
            return ret;
        }
 
        printk("%s() Enable 19.2MHz clock from PMIC\n", __func__);
 
    } else {
        ret = pmapp_clock_vote(id, PMAPP_CLOCK_ID_A0,
                        PMAPP_CLOCK_VOTE_OFF);
        if (ret < 0) {
            printk("Failed to vote for TCXO_A0 OFF\n");
            return ret;
        }
 
        printk("%s() Turn off 19.2MHz clock from PMIC\n", __func__);
 
    }
    msleep(100);
    return ret;
}
#endif
//[ECID 000000] fanjiankang add for ic information20120314 end
static int ath6kl_readCalDataFromNV(void)
{
    int ret = -1;
    int i;
    u32 rf_data1, rf_data2;   

    for(i = 0; i < 3; i++) {
                rf_data2 = ((i+1)<<24);
		  pr_info("fanjiankang before msm_proc_comm\n ");
                ret  = msm_proc_comm(PCOM_CUSTOMER_CMD1, &rf_data1, &rf_data2);
		  pr_info("ath6kl_readCalDataFromNV ret value is %d\n",ret);
                if(ret!=0) {
                        pr_info("[WIFI]_read RF NV item 3757 failed i = %d\n", i);
			   nv_tag=0;
                        break;
                } else {
                        /* copy bytes */
			   nv_tag=1;
                        if( i != (rf_data1 & 0xff) ) {
                                pr_info("Invalid RF data from NV. i=%d\n", i);
                                break;
                        }
                        ncal[i].olpcGainDelta = (rf_data1>>8)&0xff;
                        ncal[i].desiredScaleCck_t10 = (rf_data1>>16)&0xff;
                        ncal[i].desiredScale6M_t10 = (rf_data1>>24)&0xff;
                        ncal[i].desiredScale36M_t10 = rf_data2&0xff;
                        ncal[i].desiredScale54M_t10 = (rf_data2>>8)&0xff;
                        ncal[i].desiredScaleMCS0HT20_t10 = (rf_data2>>16)&0xff;
                        ncal[i].desiredScaleMCS7HT20_t10 = (rf_data2>>24)&0xff;
                        pr_info("NV RF data  i=%d\n",i);
                        pr_info("NV RF data  from %02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
                                ncal[i].olpcGainDelta, ncal[i].desiredScaleCck_t10, ncal[i].desiredScale6M_t10,
                                ncal[i].desiredScale36M_t10, ncal[i].desiredScale54M_t10,
                                ncal[i].desiredScaleMCS0HT20_t10, ncal[i].desiredScaleMCS7HT20_t10 );
               }
    }	

    return ret;
}

 int ath6kl_nv_states_get(void)
{
    return nv_tag;
}

 void ath6kl_getCaldata(calData **ncal_init)
{
    *ncal_init  = ncal;
}

 int zte_wifi_power(int on)
 {
     if(on)
     {
          pr_info("================ar6003 chip back to live=============\n");
	#ifdef CONFIG_PROJECT_P825V20
	   wlan_clock_req(1);
	#endif
          gpio_request(WLAN_CHIP_PWD_PIN,"WLAN_CHIP_PWD");
          gpio_direction_output(WLAN_CHIP_PWD_PIN,1);
          gpio_free(WLAN_CHIP_PWD_PIN);
     }
     else
      {
            pr_info("================ar6003 chip power cut=============\n");
	#ifdef CONFIG_PROJECT_P825V20
		wlan_clock_req(0);
	#endif
             gpio_request(WLAN_CHIP_PWD_PIN,"WLAN_CHIP_PWD");
             gpio_direction_output(WLAN_CHIP_PWD_PIN,0);
             gpio_free(WLAN_CHIP_PWD_PIN);
      }
      mdelay(200);
    return 0;
}


static struct platform_device zte_wifi_device = {
	.name           = "wlan_ar6000_pm_dev",
	.id             = -1,
       .dev            = {
                .platform_data = &zte_wifi_power,
        },

};
#if 0
static u8 softmac[6]={0};

static int pc_wifi_mac_nvread(void)
{
    int rc;
    u32 data1, data2 = (1<<31);//for wifi mac
    rc = msm_proc_comm(PCOM_CUSTOMER_CMD1, &data1, &data2);
	if (rc){
		printk("Read wifi mac address form nv failed! rc=%d\n",rc);
		return rc;
		}
	else{
		softmac[5] = (u8)((data2>>8)&0xff);
              softmac[4] = (u8)(data2&0xff);
              softmac[3] = (u8)((data1>>24)&0xff);
              softmac[2] = (u8)((data1>>16)&0xff);
              softmac[1] = (u8)((data1>>8)&0xff);
              softmac[0] = (u8)(data1&0xff);
		printk("Read wifi mac address form nv successful!\n");
		printk("data1=%x,data2=%x\n",data1,data2);
		printk("quguotao MAC = %02X:%02X:%02X:%02X:%02X:%02X\n",
            softmac[0], softmac[1], softmac[2], 
            softmac[3], softmac[4], softmac[5]); 
		return 0;//success
		}
}

u8*  get_wifi_mac_address(void)
{
      return softmac;
}
EXPORT_SYMBOL(get_wifi_mac_address);
#endif
 static int __init zte_wifi_init(void)
{

    int ret = 0;
    struct vreg *wlan_3p3; 
	struct vreg *wlan_1p8;

	do {
		pr_info("%s() enter\n", __func__);
		ret = gpio_tlmm_config(GPIO_CFG(WLAN_CHIP_WOW_PIN, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

       	if(ret) {
            		printk(KERN_ERR"WLAN_CHIP_WOW_PIN config failed\n");
	    		break;
        	} 
		gpio_request(WLAN_CHIP_WOW_PIN, "WLAN_WOW");
		gpio_free(WLAN_CHIP_WOW_PIN);        

	wlan_3p3 = vreg_get(NULL, "bt");
	if(!wlan_3p3)
	{
	      pr_err("%s: VREG_L17 get failed\n",__func__);
	}

       ret = vreg_set_level(wlan_3p3, 3300);
	if(ret<0)
	{
	      pr_err("%s: VREG_L17 set failed\n",__func__);
	}
		ret=vreg_enable(wlan_3p3);
	if (ret) {
		pr_err("%s: vreg_enable failed \n", __func__);
	}
	mdelay(50);

	wlan_1p8 = vreg_get(NULL,"wlan1v8");
	       if(!wlan_1p8)
	{
	      pr_err("%s: VREG_L19 get failed\n",__func__);
	}
	  ret = vreg_set_level(wlan_1p8, 1800);
	if(ret<0)
	{
	      pr_err("%s: VREG_L19 set failed\n",__func__);
	}
		ret=vreg_enable(wlan_1p8);
	if (ret) {
		pr_err("%s: vreg_enable failed \n", __func__);
	}

        pr_info("%s() VREG 1.8v On\n", __func__);
        mdelay(100);
        pr_info("%s() Pull low CHIP PWD\n", __func__);
        /*
         * Pull low Chip power down pin
         */		
		ret = gpio_tlmm_config(GPIO_CFG(WLAN_CHIP_PWD_PIN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), GPIO_CFG_ENABLE);
        	if(ret) {
            		printk(KERN_ERR"WLAN_CHIP_PWD_PIN config failed\n");
	    		break;
        	} 
		gpio_request(WLAN_CHIP_PWD_PIN, "WLAN_CHIP_PWD");
		gpio_direction_output(WLAN_CHIP_PWD_PIN, 0);
		gpio_free(WLAN_CHIP_PWD_PIN);

		platform_device_register(&zte_wifi_device);	
		//pc_wifi_mac_nvread();
              return 0;
		}while(0);
        return ret;
}
 
 int wlan_init_power(void)
{
       pr_info("fanjiankang before ath6kl_readCalDataFromNV\n ");
	ath6kl_readCalDataFromNV();
       pr_info("fanjiankang after ath6kl_readCalDataFromNV\n ");
	//wlan_clock_req(1);
	zte_wifi_init();
	create_wlan_info_proc_file();
	return 0;
}

EXPORT_SYMBOL(ath6kl_nv_states_get);
EXPORT_SYMBOL(ath6kl_getCaldata);

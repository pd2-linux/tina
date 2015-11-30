/*
*********************************************************************************************************
*											        eBIOS
*						            the Easy Portable/Player Develop Kits
*									           dma sub system
*
*						        (c) Copyright 2006-2008, David China
*											All	Rights Reserved
*
* File    : clk_for_nand.c
* By      : Richard
* Version : V1.00
*********************************************************************************************************
*/
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/spinlock.h>
#include <linux/hdreg.h>
#include <linux/init.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <mach/clock.h>
#include <mach/sys_config.h>
#include <mach/dma.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/cacheflush.h>

//#include "nand_lib.h"
#include "nand_blk.h"

static struct clk *ahb_nand_clk = NULL;
static struct clk *mod_nand_clk = NULL;

extern __u32 NAND_DMASingleMap(__u32 rw, __u32 buff_addr, __u32 len);
extern __u32 NAND_DMASingleUnmap(__u32 rw, __u32 buff_addr, __u32 len);


static DECLARE_WAIT_QUEUE_HEAD(DMA_wait);
int 	dma_hdle;
int seq=0;
int nand_handle=0;
//dma_cb_t done_cb;
//dma_config_t dma_config;

static int nanddma_completed_flag = 1;

static int dma_start_flag = 0;



static DECLARE_WAIT_QUEUE_HEAD(NAND_RB_WAIT_CH0);
static DECLARE_WAIT_QUEUE_HEAD(NAND_RB_WAIT_CH1);



#ifdef __LINUX_NAND_SUPPORT_INT__
static int nandrb_ready_flag[2] = {1, 1};
static int nanddma_ready_flag[2] = {1, 1};


#endif

//#define RB_INT_MSG_ON
#ifdef  RB_INT_MSG_ON
#define dbg_rbint(fmt, args...) printk(fmt, ## args)
#else
#define dbg_rbint(fmt, ...)  ({})
#endif

#define RB_INT_WRN_ON
#ifdef  RB_INT_WRN_ON
#define dbg_rbint_wrn(fmt, args...) printk(fmt, ## args)
#else
#define dbg_rbint_wrn(fmt, ...)  ({})
#endif

//#define DMA_INT_MSG_ON
#ifdef  DMA_INT_MSG_ON
#define dbg_dmaint(fmt, args...) printk(fmt, ## args)
#else
#define dbg_dmaint(fmt, ...)  ({})
#endif

#define DMA_INT_WRN_ON
#ifdef  DMA_INT_WRN_ON
#define dbg_dmaint_wrn(fmt, args...) printk(fmt, ## args)
#else
#define dbg_dmaint_wrn(fmt, ...)  ({})
#endif

//for rb int
extern void NFC_RbIntEnable(void);
extern void NFC_RbIntDisable(void);
extern void NFC_RbIntClearStatus(void);
extern __u32 NFC_RbIntGetStatus(void);
extern __u32 NFC_GetRbSelect(void);
extern __u32 NFC_GetRbStatus(__u32 rb);
extern __u32 NFC_RbIntOccur(void);

extern void NFC_DmaIntEnable(void);
extern void NFC_DmaIntDisable(void);
extern void NFC_DmaIntClearStatus(void);
extern __u32 NFC_DmaIntGetStatus(void);
extern __u32 NFC_DmaIntOccur(void);

extern __u32 NAND_GetCurrentCH(void);
extern __u32 NAND_SetCurrentCH(__u32 nand_index);

struct sw_dma_client nand_dma_client = {
	.name="NAND_DMA",
};


void nanddma_buffdone(struct sw_dma_chan * ch, void *buf, int size,enum sw_dma_buffresult result)
{
	nanddma_completed_flag = 1;
	wake_up( &DMA_wait );
	//printk("buffer done. nanddma_completed_flag: %d\n", nanddma_completed_flag);
}

int  nanddma_opfn(struct sw_dma_chan * ch,   enum sw_chan_op op_code){
	if(op_code == SW_DMAOP_START) 
		nanddma_completed_flag = 0;

	//printk("buffer opfn: %d, nanddma_completed_flag: %d\n", (int)op_code, nanddma_completed_flag);

	return 0;
}

__s32 NAND_WaitDmaFinish(void)
{
	wait_event(DMA_wait, nanddma_completed_flag);
	return 0;
}

void* NAND_RequestDMA(void)
{
	dma_hdle = sw_dma_request(DMACH_DNAND, &nand_dma_client, NULL);
	if(dma_hdle < 0)
		return dma_hdle;

	sw_dma_set_opfn(dma_hdle, nanddma_opfn);
	sw_dma_set_buffdone_fn(dma_hdle, nanddma_buffdone);

	return dma_hdle;


}


__s32  NAND_ReleaseDMA(void)
{
	return 0;
}


int NAND_QueryDmaStat(void)
{
	return 0;
}


void eLIBs_CleanFlushDCacheRegion_nand(void *adr, size_t bytes)
{
	__cpuc_flush_dcache_area(adr, bytes + (1 << 5) * 2 - 2);
}


__s32 NAND_SettingDMA(void * pArg)
{
	sw_dma_setflags(dma_hdle, SW_DMAF_AUTOSTART);
	return sw_dma_config(dma_hdle, (struct dma_hw_conf*)pArg);
}

__s32 NAND_DMAEqueueBuf(__u32 buff_addr, __u32 len)
{
	eLIBs_CleanFlushDCacheRegion_nand((void *)buff_addr, (size_t)len);

	nanddma_completed_flag = 0;
	return sw_dma_enqueue((int)dma_hdle, (void*)(seq++), buff_addr, len);
}


void NAND_DMAConfigStart(int rw, unsigned int buff_addr, int len)
{
	struct dma_hw_conf nand_hwconf = {
		.xfer_type = DMAXFER_D_BWORD_S_BWORD,
		.hf_irq = SW_DMA_IRQ_FULL,
		.cmbk = 0x7f077f07,
	};

	nand_hwconf.dir = rw+1;
	
	if(rw == 0){
		nand_hwconf.from = 0x01C03030,
		nand_hwconf.address_type = DMAADDRT_D_LN_S_IO,
		nand_hwconf.drqsrc_type = DRQ_TYPE_NAND;
	} else {
		nand_hwconf.to = 0x01C03030,
		nand_hwconf.address_type = DMAADDRT_D_IO_S_LN,
		nand_hwconf.drqdst_type = DRQ_TYPE_NAND;
	}
	
	NAND_SettingDMA((void*)&nand_hwconf);
	NAND_DMAEqueueBuf(buff_addr, len);

}





#ifdef __LINUX_SUPPORT_RB_INT__
void NAND_EnRbInt(void)
{
	__u32 nand_index;

	nand_index = NAND_GetCurrentCH();
	if(nand_index >1)
		printk("NAND_ClearDMAInt, nand_index error: 0x%x\n", nand_index);
	
	//clear interrupt
	NFC_RbIntClearStatus();
	
	nandrb_ready_flag[nand_index] = 0;

	//enable interrupt
	NFC_RbIntEnable();

	dbg_rbint("rb int en\n");
}


void NAND_ClearRbInt(void)
{
    __u32 nand_index;

	nand_index = NAND_GetCurrentCH();
	if(nand_index >1)
		printk("NAND_ClearDMAInt, nand_index error: 0x%x\n", nand_index);
	
	//disable interrupt
	NFC_RbIntDisable();;

	dbg_rbint("rb int clear\n");

	//clear interrupt
	NFC_RbIntClearStatus();
	
	//check rb int status
	if(NFC_RbIntGetStatus())
	{
		dbg_rbint_wrn("nand %d clear rb int status error in int clear \n", nand_index);
	}
	
	nandrb_ready_flag[nand_index] = 0;
}


void NAND_RbInterrupt(void)
{
	__u32 nand_index;

	nand_index = NAND_GetCurrentCH();
	if(nand_index >1)
		printk("NAND_ClearDMAInt, nand_index error: 0x%x\n", nand_index);

	dbg_rbint("rb int occor! \n");
	if(!NFC_RbIntGetStatus())
	{
		dbg_rbint_wrn("nand rb int late \n");
	}
    
    NAND_ClearRbInt();
    
    nandrb_ready_flag[nand_index] = 1;
    if(nand_index == 0)
		wake_up( &NAND_RB_WAIT_CH0 );
    else if(nand_index ==1)
    	wake_up( &NAND_RB_WAIT_CH1 );

}

__s32 NAND_WaitRbReady(void)
{
	__u32 rb;
	__u32 nand_index;
	
	nand_index = NAND_GetCurrentCH();
	if(nand_index >1)
		printk("NAND_ClearDMAInt, nand_index error: 0x%x\n", nand_index);

	
	NAND_EnRbInt();
	
	//wait_event(NAND_RB_WAIT, nandrb_ready_flag);
	dbg_rbint("rb wait \n");

	if(nandrb_ready_flag[nand_index])
	{
		dbg_rbint("fast rb int\n");
		NAND_ClearRbInt();
		return 0;
	}

	rb=  NFC_GetRbSelect();
	if(NFC_GetRbStatus(rb))
	{
		dbg_rbint("rb %u fast ready \n", rb);
		NAND_ClearRbInt();
		return 0;
	}

	//printk("NAND_WaitRbReady, ch %d\n", nand_index);

	if(nand_index == 0)
	{
		if(wait_event_timeout(NAND_RB_WAIT_CH0, nandrb_ready_flag[nand_index], 1*HZ)==0)
		{
			dbg_rbint_wrn("nand wait rb int time out, ch: %d\n", nand_index);
			NAND_ClearRbInt();
		}
		else
		{	NAND_ClearRbInt();
			dbg_rbint("nand wait rb ready ok\n");
		}
	}
	else if(nand_index ==1)
	{
		if(wait_event_timeout(NAND_RB_WAIT_CH1, nandrb_ready_flag[nand_index], 1*HZ)==0)
		{
			dbg_rbint_wrn("nand wait rb int time out, ch: %d\n", nand_index);
			NAND_ClearRbInt();
		}
		else
		{	NAND_ClearRbInt();
			dbg_rbint("nand wait rb ready ok\n");
		}
	}
	else
	{
		NAND_ClearRbInt();
	}
		
    return 0;
}
#else
__s32 NAND_WaitRbReady(void)
{
    return 0;
}
#endif

#define NAND_CH0_INT_EN_REG    (0xf1c03000+0x8)
#define NAND_CH1_INT_EN_REG    (0xf1c05000+0x8)
#define NAND_CH0_INT_ST_REG    (0xf1c03000+0x4)
#define NAND_CH1_INT_ST_REG    (0xf1c05000+0x4)
#define NAND_RB_INT_BITMAP     (0x1)
#define NAND_DMA_INT_BITMAP    (0x4)
#define __NAND_REG(x)    (*(volatile unsigned int   *)(x))


void NAND_Interrupt(__u32 nand_index)
{
	if(nand_index >1)
		printk("NAND_Interrupt, nand_index error: 0x%x\n", nand_index);
#ifdef __LINUX_NAND_SUPPORT_INT__   

    //printk("nand interrupt!\n");
#ifdef __LINUX_SUPPORT_RB_INT__    

    if(nand_index == 0)
    {
	   	if((__NAND_REG(NAND_CH0_INT_EN_REG)&NAND_RB_INT_BITMAP)&&(__NAND_REG(NAND_CH0_INT_ST_REG)&NAND_RB_INT_BITMAP))
			NAND_RbInterrupt();	
    }

#endif    


#endif
}


__u32 NAND_VA_TO_PA(__u32 buff_addr)
{
    return (__u32)(__pa((void *)buff_addr));
}

void NAND_PIORequest(__u32 nand_index)
{
	printk("[NAND] nand gpio_request\n");
	nand_handle = gpio_request_ex("nand_para",NULL);
	if(!nand_handle)
	{
		printk("[NAND] nand gpio_request ok\n");
	}
	else
	{
	    //printk("[NAND] nand gpio_request fail\n");
	}



}

void NAND_PIORelease(__u32 nand_index)
{

	return ;
	
}


void NAND_Memset(void* pAddr, unsigned char value, unsigned int len)
{
    memset(pAddr, value, len);   
}

void NAND_Memcpy(void* pAddr_dst, void* pAddr_src, unsigned int len)
{
    memcpy(pAddr_dst, pAddr_src, len);    
}

void* NAND_Malloc(unsigned int Size)
{
     	return kmalloc(Size, GFP_KERNEL);
}

void NAND_Free(void *pAddr, unsigned int Size)
{
    kfree(pAddr);
}

int NAND_Print(const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
	r = vprintk(fmt, args);
	va_end(args);
	
	return r;
}

void *NAND_IORemap(unsigned int base_addr, unsigned int size)
{
    return (void *)base_addr;
}

__u32 NAND_GetIOBaseAddrCH0(void)
{
	return 0xf1c03000;
}
	
__u32 NAND_GetIOBaseAddrCH1(void)
{
	return 0xf1c05000;
}

DEFINE_SEMAPHORE(nand_physic_mutex);

int NAND_PhysicLockInit(void)
{
    return 0;
}

int NAND_PhysicLock(void)
{
    down(&nand_physic_mutex);
     return 0;
}

int NAND_PhysicUnLock(void)
{
    up(&nand_physic_mutex);
     return 0;
}

int NAND_PhysicLockExit(void)
{
     return 0;
}


int NAND_ClkRequest(__u32 nand_index)
{
    printk("[NAND] nand clk request start\n");
	ahb_nand_clk = clk_get(NULL,"ahb_nfc");
	if(!ahb_nand_clk) {
		return -1;
	}
	mod_nand_clk = clk_get(NULL,"nfc");
		if(!mod_nand_clk) {
		return -1;
	}
	printk("[NAND] nand clk request ok!\n");
	return 0;

}

void NAND_ClkRelease(__u32 nand_index)
{
	clk_put(ahb_nand_clk);
	clk_put(mod_nand_clk);
}


int NAND_AHBEnable(void)
{
	return clk_enable(ahb_nand_clk);
}

int NAND_ClkEnable(void)
{
	return clk_enable(mod_nand_clk);
}

void NAND_AHBDisable(void)
{
	clk_disable(ahb_nand_clk);
}

void NAND_ClkDisable(void)
{
	clk_disable(mod_nand_clk);
}

int NAND_SetClk(__u32 nand_index, __u32 nand_clk)
{
	return clk_set_rate(mod_nand_clk, nand_clk*2000000);
}

int NAND_GetClk(__u32 nand_index)
{
	return (clk_get_rate(mod_nand_clk)/2000000);
}

int NAND_GetPlatform(void)
{
	return 13;	
}

#if 1
int NAND_get_storagetype()
{
    int script_ret;
    int storage_type;
    
    script_ret = script_parser_fetch("target","storage_type", &storage_type,sizeof(int));
    if(script_ret)
    {
           printk("nand init fetch storage_type failed\n");
           storage_type=0;
           return storage_type;
    }

    return storage_type;
    
    
}
#endif


__u32 NAND_GetNandExtPara(__u32 para_num)
{	
	int nand_para;
    int script_ret;
	
	if (para_num == 0) {
	    script_ret = script_parser_fetch("nand_para", "nand_p0", &nand_para,sizeof(int));
	    if(script_ret)
	    {
	        printk("nand type err! %d");
			return 0xffffffff;
	    }
		else
			return nand_para;
	} else if (para_num == 1) {
	    script_ret = script_parser_fetch("nand_para", "nand_p1", &nand_para,sizeof(int));
	    if(script_ret)
	    {
	        printk("nand type err! %d");
			return 0xffffffff;
	    }
		else
			return nand_para;	
	} else {
		printk("NAND_GetNandExtPara: wrong para num: %d\n", para_num);
		return 0xffffffff;
	}
}

__u32 NAND_GetNandIDNumCtrl(void)
{
    int id_number_ctl;
    int script_ret;

    script_ret = script_parser_fetch("nand_para", "id_number_ctl", &id_number_ctl,sizeof(int));
    if(script_ret)
    {
        printk("nand type err! %d");
		return 0;
    } else {
    	printk("nand : get id_number_ctl from script,%x \n",id_number_ctl);	
    	return id_number_ctl;
    }	
}


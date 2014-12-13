/*
 * MUSB OTG controller driver for Blackfin Processors
 *
 * Copyright 2006-2008 Analog Devices Inc.
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
#include <linux/usb/nop-usb-xceiv.h>
#endif
#include <linux/xlog.h>
#include <linux/switch.h>
#include <linux/i2c.h>
#include <mach/irqs.h>
#include <mach/eint.h>
#include <linux/musb/musb_core.h>
#include <linux/musb/mtk_musb.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/musb/musbhsdma.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_clock_manager.h>
#include "usb20.h"

extern struct musb *mtk_musb;
static DEFINE_SEMAPHORE(power_clock_lock);
/* static bool platform_init_first = true; */
extern bool mtk_usb_power;
static u32 cable_mode = CABLE_MODE_NORMAL;


/*EP Fifo Config*/
static struct musb_fifo_cfg __initdata fifo_cfg[] = {
{ .hw_ep_num =  1, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  1, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  2, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  2, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  3, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  3, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  4, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  4, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =  5, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_INT,.mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	5, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_INT,.mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  6, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_INT, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	6, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_INT,.mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	7, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	7, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_BULK,.mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	8, .style = MUSB_FIFO_TX,   .maxpacket = 512, .ep_mode = EP_ISO,.mode = MUSB_BUF_DOUBLE},
{ .hw_ep_num =	8, .style = MUSB_FIFO_RX,   .maxpacket = 512, .ep_mode = EP_ISO,.mode = MUSB_BUF_DOUBLE},
};

static struct timer_list musb_idle_timer;

static void musb_do_idle(unsigned long _musb)
{
	struct musb	*musb = (void *)_musb;
	unsigned long	flags;
	u8	devctl;

	if (musb->is_active) {
		DBG(0, "%s active, igonre do_idle\n",
			otg_state_string(musb->xceiv->state));
		return;
	}

	spin_lock_irqsave(&musb->lock, flags);

	switch (musb->xceiv->state) {
	case OTG_STATE_B_PERIPHERAL:
	case OTG_STATE_A_WAIT_BCON:
		devctl = musb_readb(musb->mregs, MUSB_DEVCTL);
		if (devctl & MUSB_DEVCTL_BDEVICE) {
			musb->xceiv->state = OTG_STATE_B_IDLE;
			MUSB_DEV_MODE(musb);
		} else {
			musb->xceiv->state = OTG_STATE_A_IDLE;
			MUSB_HST_MODE(musb);
		}
		break;
	case OTG_STATE_A_HOST:
		devctl = musb_readb(musb->mregs, MUSB_DEVCTL);
		if (devctl &  MUSB_DEVCTL_BDEVICE)
			musb->xceiv->state = OTG_STATE_B_IDLE;
		else
			musb->xceiv->state = OTG_STATE_A_WAIT_BCON;
	default:
		break;
	}
	spin_unlock_irqrestore(&musb->lock, flags);

    DBG(0, "otg_state %s \n", otg_state_string(musb->xceiv->state));
}

static void mt_usb_try_idle(struct musb *musb, unsigned long timeout)
{
	unsigned long		default_timeout = jiffies + msecs_to_jiffies(3);
	static unsigned long	last_timer;

	if (timeout == 0)
		timeout = default_timeout;

	/* Never idle if active, or when VBUS timeout is not set as host */
	if (musb->is_active || ((musb->a_wait_bcon == 0)
			&& (musb->xceiv->state == OTG_STATE_A_WAIT_BCON))) {
		DBG(2, "%s active, deleting timer\n",
			otg_state_string(musb->xceiv->state));
		del_timer(&musb_idle_timer);
		last_timer = jiffies;
		return;
	}

	if (time_after(last_timer, timeout)) {
		if (!timer_pending(&musb_idle_timer))
			last_timer = timeout;
		else {
			DBG(2, "Longer idle timer already pending, ignoring\n");
			return;
		}
	}
	last_timer = timeout;

	DBG(2, "%s inactive, for idle timer for %lu ms\n",
		otg_state_string(musb->xceiv->state),
		(unsigned long)jiffies_to_msecs(timeout - jiffies));
	mod_timer(&musb_idle_timer, timeout);
}

static void mt_usb_enable(struct musb *musb)
{
    unsigned long   flags;

    DBG(0, "%d, %d\n", mtk_usb_power, musb->power);

    if (musb->power == true)
        return;

    flags = musb_readl(mtk_musb->mregs,USB_L1INTM);

    // mask ID pin, so "open clock" and "set flag" won't be interrupted. ISR may call clock_disable.
    musb_writel(mtk_musb->mregs,USB_L1INTM,(~IDDIG_INT_STATUS)&flags);

    /*if (platform_init_first) {
        DBG(0,"usb init first\n\r");
        musb->is_host = true;
    }*/

    if (!mtk_usb_power) {
        if (down_interruptible(&power_clock_lock))
            xlog_printk(ANDROID_LOG_ERROR, "USB20", "%s: busy, Couldn't get power_clock_lock\n" \
                        , __func__);

#ifndef FPGA_PLATFORM
	enable_pll(MT65XX_UPLL, "USB_PLL");
	hwPowerOn(MT65XX_POWER_LDO_VUSB, VOL_3300, "VUSB_LDO");
        DBG(0,"enable UPLL before connect\n");
#endif
        mdelay(10);

        usb_phy_recover();

        mtk_usb_power = true;

        up(&power_clock_lock);
    }
	musb->power = true;

    musb_writel(mtk_musb->mregs,USB_L1INTM,flags);
}

static void mt_usb_disable(struct musb *musb)
{
    printk("%s, %d, %d\n", __func__, mtk_usb_power, musb->power);

    if (musb->power == false)
        return;

    /*if (platform_init_first) {
        DBG(0,"usb init first\n\r");
        musb->is_host = false;
        platform_init_first = false;
    }*/

    if (mtk_usb_power) {
        if (down_interruptible(&power_clock_lock))
            xlog_printk(ANDROID_LOG_ERROR, "USB20", "%s: busy, Couldn't get power_clock_lock\n" \
                        , __func__);

#ifndef FPGA_PLATFORM
        usb_phy_savecurrent();
	hwPowerDown(MT65XX_POWER_LDO_VUSB,"VUSB_LDO");
	disable_pll(MT65XX_UPLL,"USB_PLL");
        DBG(0, "disable UPLL before disconnect\n");
#endif
        mtk_usb_power = false;
        up(&power_clock_lock);
    }

    musb->power = false;
}

/* ================================ */
/* connect and disconnect functions */
/* ================================ */
bool mt_usb_is_device(void)
{
	DBG(4,"called\n");

	if(!mtk_musb){
		DBG(0,"mtk_musb is NULL\n");
		return false; // don't do charger detection when usb is not ready
	} else {
		DBG(4,"is_host=%d\n",mtk_musb->is_host);
	}
  return !mtk_musb->is_host;
}

void mt_usb_connect(void)
{
	printk("[MUSB] USB is ready for connect\n");
    DBG(3, "is ready %d is_host %d power %d\n",mtk_musb->is_ready,mtk_musb->is_host , mtk_musb->power);
    if (!mtk_musb || !mtk_musb->is_ready || mtk_musb->is_host || mtk_musb->power)
        return;

    DBG(0,"cable_mode=%d\n",cable_mode);

	if(cable_mode != CABLE_MODE_NORMAL)
	{
        DBG(0,"musb_sync_with_bat, USB_CONFIGURED\n");
        musb_sync_with_bat(mtk_musb,USB_CONFIGURED);
        mtk_musb->power = true;
        return;
    }

	if (!wake_lock_active(&mtk_musb->usb_lock))
		wake_lock(&mtk_musb->usb_lock);

    musb_start(mtk_musb);
	printk("[MUSB] USB connect\n");
}

void mt_usb_disconnect(void)
{
	printk("[MUSB] USB is ready for disconnect\n");

	if (!mtk_musb || !mtk_musb->is_ready || mtk_musb->is_host || !mtk_musb->power)
		return;

	musb_stop(mtk_musb);

	if (wake_lock_active(&mtk_musb->usb_lock))
		wake_unlock(&mtk_musb->usb_lock);

    DBG(0,"cable_mode=%d\n",cable_mode);

	if (cable_mode != CABLE_MODE_NORMAL) {
        DBG(0,"musb_sync_with_bat, USB_SUSPEND\n");
		musb_sync_with_bat(mtk_musb,USB_SUSPEND);
		mtk_musb->power = false;
	}

	printk("[MUSB] USB disconnect\n");
}

bool usb_cable_connected(void)
{
#ifdef FPGA_PLATFORM
	return true;
#else

#ifdef CONFIG_POWER_EXT
	if (upmu_get_rgs_chrdet())
#else
	if (upmu_is_chr_det())
#endif
	{
		int type = mt_charger_type_detection();
		if ((type == STANDARD_HOST) || (type == CHARGING_HOST))
		return true;
	}

		return false;
#endif // end FPGA_PLATFORM
}

void musb_platform_reset(struct musb *musb)
{
	u16 swrst = 0;
	void __iomem	*mbase = musb->mregs;
	swrst = musb_readw(mbase,MUSB_SWRST);
	swrst |= (MUSB_SWRST_DISUSBRESET | MUSB_SWRST_SWRST);
	musb_writew(mbase, MUSB_SWRST,swrst);
}

static void usb_check_connect(void)
{
#ifndef FPGA_PLATFORM
	if (usb_cable_connected())
        mt_usb_connect();
#endif
}

bool is_switch_charger(void)
{
#ifdef SWITCH_CHARGER
	return true;
#else
	return false;
#endif
}

void pmic_chrdet_int_en(int is_on)
{
#ifndef FPGA_PLATFORM
	upmu_interrupt_chrdet_int_en(is_on);
#endif
}

void musb_sync_with_bat(struct musb *musb,int usb_state)
{
#ifndef FPGA_PLATFORM
    DBG(0,"BATTERY_SetUSBState, state=%d\n",usb_state);
	BATTERY_SetUSBState(usb_state);
	wake_up_bat();
#endif
}

/*-------------------------------------------------------------------------*/
static irqreturn_t generic_interrupt(int irq, void *__hci)
{
	unsigned long	flags;
	irqreturn_t	retval = IRQ_NONE;
	struct musb	*musb = __hci;

	spin_lock_irqsave(&musb->lock, flags);

	/* musb_read_clear_generic_interrupt */
	musb->int_usb = musb_readb(musb->mregs, MUSB_INTRUSB) & musb_readb(musb->mregs, MUSB_INTRUSBE);
	musb->int_tx = musb_readw(musb->mregs, MUSB_INTRTX) & musb_readw(musb->mregs, MUSB_INTRTXE);
	musb->int_rx = musb_readw(musb->mregs, MUSB_INTRRX) & musb_readw(musb->mregs, MUSB_INTRRXE);
	mb();
	musb_writew(musb->mregs,MUSB_INTRRX,musb->int_rx);
	musb_writew(musb->mregs,MUSB_INTRTX,musb->int_tx);
	musb_writeb(musb->mregs,MUSB_INTRUSB,musb->int_usb);
	/* musb_read_clear_generic_interrupt */

	if (musb->int_usb || musb->int_tx || musb->int_rx)
		retval = musb_interrupt(musb);

	spin_unlock_irqrestore(&musb->lock, flags);

	return retval;
}

static irqreturn_t mt_usb_interrupt(int irq, void *dev_id)
{
    irqreturn_t tmp_status;
    irqreturn_t status = IRQ_NONE;
	struct musb	*musb = (struct musb*)dev_id;
	u32 usb_l1_ints;

	usb_l1_ints= musb_readl(musb->mregs,USB_L1INTS)&musb_readl(mtk_musb->mregs,USB_L1INTM); //gang  REVISIT
	DBG(1,"usb interrupt assert %x %x  %x %x %x\n",usb_l1_ints,musb_readl(mtk_musb->mregs,USB_L1INTM),musb_readb(musb->mregs, MUSB_INTRUSBE),musb_readw(musb->mregs,MUSB_INTRTX),musb_readw(musb->mregs,MUSB_INTRTXE));

	if ((usb_l1_ints & TX_INT_STATUS) || (usb_l1_ints & RX_INT_STATUS) || (usb_l1_ints & USBCOM_INT_STATUS)) {
		if((tmp_status = generic_interrupt(irq, musb)) != IRQ_NONE)
			status = tmp_status;
	}

	if (usb_l1_ints & DMA_INT_STATUS) {
		if((tmp_status = dma_controller_irq(irq, musb->dma_controller)) != IRQ_NONE)
			status = tmp_status;
	}

#ifdef 	CONFIG_USB_MTK_OTG
	if(usb_l1_ints & IDDIG_INT_STATUS) {
		mt_usb_iddig_int(musb);
		status = IRQ_HANDLED;
	}
#endif

    return status;

}

/*--FOR INSTANT POWER ON USAGE--------------------------------------------------*/
static ssize_t mt_usb_show_cmode(struct device* dev, struct device_attribute *attr, char *buf)
{
	if (!dev) {
		DBG(0,"dev is null!!\n");
		return 0;
	}
	return scnprintf(buf, PAGE_SIZE, "%d\n", cable_mode);
}

static ssize_t mt_usb_store_cmode(struct device* dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int cmode;

	if (!dev) {
		DBG(0,"dev is null!!\n");
		return count;
	} else if (1 == sscanf(buf, "%d", &cmode)) {
		DBG(0, "cmode=%d, cable_mode=%d\n", cmode, cable_mode);
		if (cmode >= CABLE_MODE_MAX)
			cmode = CABLE_MODE_NORMAL;

		if (cable_mode != cmode) {
			if(mtk_musb) {
				if(down_interruptible(&mtk_musb->musb_lock))
					xlog_printk(ANDROID_LOG_ERROR, "USB20", "%s: busy, Couldn't get musb_lock\n", __func__);
			}
			if(cmode == CABLE_MODE_CHRG_ONLY) { // IPO shutdown, disable USB
				if(mtk_musb) {
					mtk_musb->in_ipo_off = true;
				}

			} else { // IPO bootup, enable USB
				if(mtk_musb) {
					mtk_musb->in_ipo_off = false;
				}
			}

			mt_usb_disconnect();
			cable_mode = cmode;
			msleep(10);
			/* check that "if USB cable connected and than call mt_usb_connect" */
			/* Then, the Bat_Thread won't be always wakeup while no USB/chatger cable and IPO mode */
			usb_check_connect();

#ifdef CONFIG_USB_MTK_OTG
			if(cmode == CABLE_MODE_CHRG_ONLY) {
				if(mtk_musb && mtk_musb->is_host) { // shut down USB host for IPO
					if (wake_lock_active(&mtk_musb->usb_lock))
                        wake_unlock(&mtk_musb->usb_lock);
					musb_platform_set_vbus(mtk_musb, 0);
					musb_stop(mtk_musb);
					MUSB_DEV_MODE(mtk_musb);
					/* Think about IPO shutdown with A-cable, then switch to B-cable and IPO bootup. We need a point to clear session bit */
					musb_writeb(mtk_musb->mregs, MUSB_DEVCTL, (~MUSB_DEVCTL_SESSION)&musb_readb(mtk_musb->mregs,MUSB_DEVCTL));
				}
				switch_int_to_host_and_mask(mtk_musb); // mask ID pin interrupt even if A-cable is not plugged in
			} else {
				switch_int_to_host(mtk_musb); // resotre ID pin interrupt
			}
#endif
			if(mtk_musb) {
				up(&mtk_musb->musb_lock);
			}
		}
	}
	return count;
}

DEVICE_ATTR(cmode,  0664, mt_usb_show_cmode, mt_usb_store_cmode);

#ifdef FPGA_PLATFORM
static struct i2c_client *usb_i2c_client = NULL;
static const struct i2c_device_id usb_i2c_id[] = {{"mtk-usb",0},{}};

static struct i2c_board_info __initdata usb_i2c_dev = {I2C_BOARD_INFO("mtk-usb", 0x60)};


void USB_PHY_Write_Register8(UINT8 var,  UINT8 addr)
{
	char buffer[2];
	buffer[0] = addr;
	buffer[1] = var;
	i2c_master_send(usb_i2c_client, buffer, 2);
}

UINT8 USB_PHY_Read_Register8(UINT8 addr)
{
	UINT8 var;
	i2c_master_send(usb_i2c_client, &addr, 1);
	i2c_master_recv(usb_i2c_client, &var, 1);
	return var;
}

static int usb_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	printk("[MUSB]usb_i2c_probe, start\n");

	usb_i2c_client = client;

	//disable usb mac suspend
	DRV_WriteReg8(USB_SIF_BASE + 0x86a, 0x00);

	//usb phy initial sequence
	USB_PHY_Write_Register8(0x00, 0xFF);
	USB_PHY_Write_Register8(0x04, 0x61);
	USB_PHY_Write_Register8(0x00, 0x68);
	USB_PHY_Write_Register8(0x00, 0x6a);
	USB_PHY_Write_Register8(0x6e, 0x00);
	USB_PHY_Write_Register8(0x0c, 0x1b);
	USB_PHY_Write_Register8(0x44, 0x08);
	USB_PHY_Write_Register8(0x55, 0x11);
	USB_PHY_Write_Register8(0x68, 0x1a);


	printk("[MUSB]addr: 0xFF, value: %x\n", USB_PHY_Read_Register8(0xFF));
	printk("[MUSB]addr: 0x61, value: %x\n", USB_PHY_Read_Register8(0x61));
	printk("[MUSB]addr: 0x68, value: %x\n", USB_PHY_Read_Register8(0x68));
	printk("[MUSB]addr: 0x6a, value: %x\n", USB_PHY_Read_Register8(0x6a));
	printk("[MUSB]addr: 0x00, value: %x\n", USB_PHY_Read_Register8(0x00));
	printk("[MUSB]addr: 0x1b, value: %x\n", USB_PHY_Read_Register8(0x1b));
	printk("[MUSB]addr: 0x08, value: %x\n", USB_PHY_Read_Register8(0x08));
	printk("[MUSB]addr: 0x11, value: %x\n", USB_PHY_Read_Register8(0x11));
	printk("[MUSB]addr: 0x1a, value: %x\n", USB_PHY_Read_Register8(0x1a));


	printk("[MUSB]usb_i2c_probe, end\n");
    return 0;

}

/*static int usb_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {
    strcpy(info->type, "mtk-usb");
    return 0;
}*/

static int usb_i2c_remove(struct i2c_client *client) {return 0;}


struct i2c_driver usb_i2c_driver = {
    .probe = usb_i2c_probe,
    .remove = usb_i2c_remove,
    /*.detect = usb_i2c_detect,*/
    .driver = {
    	.name = "mtk-usb",
    },
    .id_table = usb_i2c_id,
};

static int add_usb_i2c_driver(void)
{
	i2c_register_board_info(0, &usb_i2c_dev, 1);
	if (i2c_add_driver(&usb_i2c_driver)!=0) {
		printk("[MUSB]usb_i2c_driver initialization failed!!\n");
		return -1;
	} else {
		printk("[MUSB]usb_i2c_driver initialization succeed!!\n");
	}
	return 0;
}
#endif //End of FPGA_PLATFORM

void musb_check_mpu_violation(u32 addr, int wr_vio)
{
    void __iomem *mregs = (void *)USB_BASE;

    printk(KERN_CRIT "MUSB checks EMI MPU violation.\n");
    printk(KERN_CRIT "addr = 0x%x, %s violation.\n", addr, wr_vio? "Write": "Read");
    printk(KERN_CRIT "POWER = 0x%x,DEVCTL= 0x%x.\n", musb_readb(mregs, MUSB_POWER),musb_readb(mregs,MUSB_DEVCTL));
    printk(KERN_CRIT "DMA_CNTLch0 %04x,DMA_ADDRch0 %08x,DMA_COUNTch0 %08x \n",musb_readw(mregs, 0x204),musb_readl(mregs,0x208),musb_readl(mregs,0x20C));
    printk(KERN_CRIT "DMA_CNTLch1 %04x,DMA_ADDRch1 %08x,DMA_COUNTch1 %08x \n",musb_readw(mregs, 0x214),musb_readl(mregs,0x218),musb_readl(mregs,0x21C));
    printk(KERN_CRIT "DMA_CNTLch2 %04x,DMA_ADDRch2 %08x,DMA_COUNTch2 %08x \n",musb_readw(mregs, 0x224),musb_readl(mregs,0x228),musb_readl(mregs,0x22C));
    printk(KERN_CRIT "DMA_CNTLch3 %04x,DMA_ADDRch3 %08x,DMA_COUNTch3 %08x \n",musb_readw(mregs, 0x234),musb_readl(mregs,0x238),musb_readl(mregs,0x23C));
    printk(KERN_CRIT "DMA_CNTLch4 %04x,DMA_ADDRch4 %08x,DMA_COUNTch4 %08x \n",musb_readw(mregs, 0x244),musb_readl(mregs,0x248),musb_readl(mregs,0x24C));
    printk(KERN_CRIT "DMA_CNTLch5 %04x,DMA_ADDRch5 %08x,DMA_COUNTch5 %08x \n",musb_readw(mregs, 0x254),musb_readl(mregs,0x258),musb_readl(mregs,0x25C));
    printk(KERN_CRIT "DMA_CNTLch6 %04x,DMA_ADDRch6 %08x,DMA_COUNTch6 %08x \n",musb_readw(mregs, 0x264),musb_readl(mregs,0x268),musb_readl(mregs,0x26C));
    printk(KERN_CRIT "DMA_CNTLch7 %04x,DMA_ADDRch7 %08x,DMA_COUNTch7 %08x \n",musb_readw(mregs, 0x274),musb_readl(mregs,0x278),musb_readl(mregs,0x27C));
}

static int mt_usb_init(struct musb *musb)
{
	DBG(0,"mt_usb_init \n");

    usb_nop_xceiv_register();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
    musb->xceiv = usb_get_phy(USB_PHY_TYPE_USB2);
#else
    musb->xceiv = usb_get_transceiver();
#endif
    musb->nIrq = MT_USB0_IRQ_ID;
    musb->dma_irq= (int)SHARE_IRQ;
    musb->fifo_cfg = fifo_cfg;
    musb->fifo_cfg_size = ARRAY_SIZE(fifo_cfg);
    musb->dyn_fifo = true;
    musb->power = false;
    musb->is_host = false;
    musb->fifo_size = 8*1024;

    wake_lock_init(&musb->usb_lock, WAKE_LOCK_SUSPEND, "USB suspend lock");

    mt_usb_enable(musb);

    musb->isr = mt_usb_interrupt;
    musb_writel(musb->mregs,MUSB_HSDMA_INTR,0xff | (0xff << DMA_INTR_UNMASK_SET_OFFSET));
    DBG(0,"musb platform init %x\n",musb_readl(musb->mregs,MUSB_HSDMA_INTR));
    musb_writel(musb->mregs,USB_L1INTM,TX_INT_STATUS | RX_INT_STATUS | USBCOM_INT_STATUS | DMA_INT_STATUS);//gang

    setup_timer(&musb_idle_timer, musb_do_idle, (unsigned long) musb);

#ifdef CONFIG_USB_MTK_OTG
	mt_usb_otg_init(musb);
#endif

    return 0;
}

static int mt_usb_exit(struct musb *musb)
{
    del_timer_sync(&musb_idle_timer);
	return 0;
}

static const struct musb_platform_ops mt_usb_ops = {
	.init		= mt_usb_init,
	.exit		= mt_usb_exit,
	/*.set_mode	= mt_usb_set_mode,*/
	.try_idle	= mt_usb_try_idle,
	.enable		= mt_usb_enable,
	.disable	= mt_usb_disable,
	.set_vbus	= mt_usb_set_vbus,
	.vbus_status = mt_usb_get_vbus_status
};

static u64 mt_usb_dmamask = DMA_BIT_MASK(32);

static int mt_usb_probe(struct platform_device *pdev)
{
	struct musb_hdrc_platform_data	*pdata = pdev->dev.platform_data;
	struct platform_device		*musb;
	struct mt_usb_glue		*glue;
	int				ret = -ENOMEM;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
	int				musbid;
#endif

    glue = kzalloc(sizeof(*glue), GFP_KERNEL);
	if (!glue) {
		dev_err(&pdev->dev, "failed to allocate glue context\n");
		goto err0;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
	/* get the musb id */
	musbid = musb_get_id(&pdev->dev, GFP_KERNEL);
	if (musbid < 0) {
		dev_err(&pdev->dev, "failed to allocate musb id\n");
		ret = -ENOMEM;
		goto err1;
	}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	musb = platform_device_alloc("musb-hdrc", PLATFORM_DEVID_AUTO);
#else
	musb = platform_device_alloc("musb-hdrc", musbid);
#endif
	if (!musb) {
		dev_err(&pdev->dev, "failed to allocate musb device\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
		goto err1;
#else
		goto err2;
#endif
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
	musb->id			= musbid;
#endif
	musb->dev.parent		= &pdev->dev;
	musb->dev.dma_mask		= &mt_usb_dmamask;
	musb->dev.coherent_dma_mask	= mt_usb_dmamask;

	glue->dev			= &pdev->dev;
	glue->musb			= musb;

	pdata->platform_ops		= &mt_usb_ops;

	platform_set_drvdata(pdev, glue);

	ret = platform_device_add_resources(musb, pdev->resource,
			pdev->num_resources);
	if (ret) {
		dev_err(&pdev->dev, "failed to add resources\n");
		goto err3;
	}

	ret = platform_device_add_data(musb, pdata, sizeof(*pdata));
	if (ret) {
		dev_err(&pdev->dev, "failed to add platform_data\n");
		goto err3;
	}

	ret = platform_device_add(musb);

	if (ret) {
		dev_err(&pdev->dev, "failed to register musb device\n");
		goto err3;
	}

    ret = device_create_file(&pdev->dev, &dev_attr_cmode);
#ifdef MTK_UART_USB_SWITCH
	ret = device_create_file(&pdev->dev,&dev_attr_portmode);
	ret = device_create_file(&pdev->dev,&dev_attr_tx);
	ret = device_create_file(&pdev->dev,&dev_attr_rx);
	ret = device_create_file(&pdev->dev,&dev_attr_uartpath);
#endif

    if (ret) {
		dev_err(&pdev->dev, "failed to create musb device\n");
		goto err3;
	}

	return 0;

err3:
	platform_device_put(musb);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
err2:
	musb_put_id(&pdev->dev, musbid);
#endif

err1:
	kfree(glue);

err0:
	return ret;
}

static int mt_usb_remove(struct platform_device *pdev)
{
	struct mt_usb_glue		*glue = platform_get_drvdata(pdev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	platform_device_unregister(glue->musb);
#else
	musb_put_id(&pdev->dev, glue->musb->id);
	platform_device_del(glue->musb);
	platform_device_put(glue->musb);
#endif
	kfree(glue);

	return 0;
}

static struct platform_driver mt_usb_driver = {
	.remove		= mt_usb_remove,
	.probe		= mt_usb_probe,
	.driver		= {
		.name	= "mt_usb",
	},
};

static int __init usb20_init(void)
{
	DBG(0,"usb20 init\n");

#ifdef FPGA_PLATFORM
    add_usb_i2c_driver();
#endif

	return platform_driver_register(&mt_usb_driver);
}
fs_initcall(usb20_init);

static void __exit usb20_exit(void)
{
	platform_driver_unregister(&mt_usb_driver);
}
module_exit(usb20_exit)

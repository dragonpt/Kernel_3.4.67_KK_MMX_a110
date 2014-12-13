#include <linux/pm.h>

/* Functions local to drivers/usb/core/ */

extern int usb_create_sysfs_dev_files(struct usb_device *dev);
extern void usb_remove_sysfs_dev_files(struct usb_device *dev);
extern void usb_create_sysfs_intf_files(struct usb_interface *intf);
extern void usb_remove_sysfs_intf_files(struct usb_interface *intf);
extern int usb_create_ep_devs(struct device *parent,
				struct usb_host_endpoint *endpoint,
				struct usb_device *udev);
extern void usb_remove_ep_devs(struct usb_host_endpoint *endpoint);

extern void usb_enable_endpoint(struct usb_device *dev,
		struct usb_host_endpoint *ep, bool reset_toggle);
extern void usb_enable_interface(struct usb_device *dev,
		struct usb_interface *intf, bool reset_toggles);
extern void usb_disable_endpoint(struct usb_device *dev, unsigned int epaddr,
		bool reset_hardware);
extern void usb_disable_interface(struct usb_device *dev,
		struct usb_interface *intf, bool reset_hardware);
extern void usb_release_interface_cache(struct kref *ref);
extern void usb_disable_device(struct usb_device *dev, int skip_ep0);
extern int usb_deauthorize_device(struct usb_device *);
extern int usb_authorize_device(struct usb_device *);
extern void usb_detect_quirks(struct usb_device *udev);
extern int usb_remove_device(struct usb_device *udev);

extern int usb_get_device_descriptor(struct usb_device *dev,
		unsigned int size);
extern int usb_get_bos_descriptor(struct usb_device *dev);
extern void usb_release_bos_descriptor(struct usb_device *dev);
extern char *usb_cache_string(struct usb_device *udev, int index);
extern int usb_set_configuration(struct usb_device *dev, int configuration);
extern int usb_choose_configuration(struct usb_device *udev);

extern void usb_kick_khubd(struct usb_device *dev);
extern int usb_match_device(struct usb_device *dev,
			    const struct usb_device_id *id);
extern void usb_forced_unbind_intf(struct usb_interface *intf);
extern void usb_rebind_intf(struct usb_interface *intf);

extern int usb_hub_claim_port(struct usb_device *hdev, unsigned port,
		void *owner);
extern int usb_hub_release_port(struct usb_device *hdev, unsigned port,
		void *owner);
extern void usb_hub_release_all_ports(struct usb_device *hdev, void *owner);
extern bool usb_device_is_owned(struct usb_device *udev);

extern int  usb_hub_init(void);
extern void usb_hub_cleanup(void);
extern int usb_major_init(void);
extern void usb_major_cleanup(void);

#ifdef	CONFIG_PM

extern int usb_suspend(struct device *dev, pm_message_t msg);
extern int usb_resume(struct device *dev, pm_message_t msg);
extern int usb_resume_complete(struct device *dev);

extern int usb_port_suspend(struct usb_device *dev, pm_message_t msg);
extern int usb_port_resume(struct usb_device *dev, pm_message_t msg);

#else

static inline int usb_port_suspend(struct usb_device *udev, pm_message_t msg)
{
	return 0;
}

static inline int usb_port_resume(struct usb_device *udev, pm_message_t msg)
{
	return 0;
}

#endif

#ifdef CONFIG_USB_SUSPEND

extern void usb_autosuspend_device(struct usb_device *udev);
extern int usb_autoresume_device(struct usb_device *udev);
extern int usb_remote_wakeup(struct usb_device *dev);
extern int usb_runtime_suspend(struct device *dev);
extern int usb_runtime_resume(struct device *dev);
extern int usb_runtime_idle(struct device *dev);
extern int usb_set_usb2_hardware_lpm(struct usb_device *udev, int enable);

#else

#define usb_autosuspend_device(udev)		do {} while (0)
static inline int usb_autoresume_device(struct usb_device *udev)
{
	return 0;
}

static inline int usb_remote_wakeup(struct usb_device *udev)
{
	return 0;
}

static inline int usb_set_usb2_hardware_lpm(struct usb_device *udev, int enable)
{
	return 0;
}
#endif

extern struct bus_type usb_bus_type;
extern struct device_type usb_device_type;
extern struct device_type usb_if_device_type;
extern struct device_type usb_ep_device_type;
extern struct usb_device_driver usb_generic_driver;

static inline int is_usb_device(const struct device *dev)
{
	return dev->type == &usb_device_type;
}

static inline int is_usb_interface(const struct device *dev)
{
	return dev->type == &usb_if_device_type;
}

static inline int is_usb_endpoint(const struct device *dev)
{
	return dev->type == &usb_ep_device_type;
}

/* Do the same for device drivers and interface drivers. */

static inline int is_usb_device_driver(struct device_driver *drv)
{
	return container_of(drv, struct usbdrv_wrap, driver)->
			for_devices;
}

/* for labeling diagnostics */
extern const char *usbcore_name;

/* sysfs stuff */
extern const struct attribute_group *usb_device_groups[];
extern const struct attribute_group *usb_interface_groups[];

/* usbfs stuff */
extern struct mutex usbfs_mutex;
extern struct usb_driver usbfs_driver;
extern const struct file_operations usbfs_devices_fops;
extern const struct file_operations usbdev_file_operations;
extern void usbfs_conn_disc_event(void);

extern int usb_devio_init(void);
extern void usb_devio_cleanup(void);

/* internal notify stuff */
extern void usb_notify_add_device(struct usb_device *udev);
extern void usb_notify_remove_device(struct usb_device *udev);
extern void usb_notify_add_bus(struct usb_bus *ubus);
extern void usb_notify_remove_bus(struct usb_bus *ubus);

enum my_print_levels {
	MY_PRINT_DBG = 0 ,
	MY_PRINT_INFO = 1 ,
	MY_PRINT_WARN = 2 ,
	MY_PRINT_ERR = 3 ,
} ;

/* set cur accepted log level here */
#define my_print_level (MY_PRINT_DBG)

#define my_print_level_avail(level) (level >= my_print_level ? 1:0)

#define MYDBG(fmt, args...) do {if(my_print_level_avail(MY_PRINT_DBG)){printk(KERN_WARNING "MTK_ICUSB [DBG], <%s(), %d> " fmt, __func__, __LINE__, ## args); }}while(0)
#define MYINFO(fmt, args...) do {if(my_print_level_avail(MY_PRINT_INFO)){printk(KERN_WARNING "MTK_ICUSB [INFO], <%s(), %d> " fmt, __func__, __LINE__, ## args); }}while(0)
#define MYWARN(fmt, args...) do {if(my_print_level_avail(MY_PRINT_WARN)){printk(KERN_WARNING "MTK_ICUSB [WARN], <%s(), %d> " fmt, __func__, __LINE__, ## args); }}while(0)
#define MYERR(fmt, args...) do {if(my_print_level_avail(MY_PRINT_ERR)){printk(KERN_WARNING "MTK_ICUSB [ERR], <%s(), %d> " fmt, __func__, __LINE__, ## args); }}while(0)

#ifdef MTK_ICUSB_SUPPORT

enum PHY_VOLTAGE_TYPE
{
	VOL_18 = 0,
	VOL_33,
	VOL_50,
};

enum SESSION_CONTROL_ACTION
{
	STOP_SESSION = 0,
	START_SESSION,
};

enum WAIT_DISCONNECT_DONE_ACTION
{
	WAIT_DISCONNECT_DONE_DFT_ACTION = 0,
};

#define IC_USB_CMD_LEN 255
struct IC_USB_CMD
{
	unsigned char type;
	unsigned char length;
	unsigned char data[IC_USB_CMD_LEN];
};


enum IC_USB_CMD_TYPE
{
	USB11_SESSION_CONTROL = 0,
	USB11_INIT_PHY_BY_VOLTAGE,
	USB11_WAIT_DISCONNECT_DONE,
};

/* power neogo */
#define IC_USB_REQ_TYPE_GET_INTERFACE_POWER  0xC0
#define IC_USB_REQ_TYPE_SET_INTERFACE_POWER  0x40
#define IC_USB_REQ_GET_INTERFACE_POWER 0x01
#define IC_USB_REQ_SET_INTERFACE_POWER 0x02
#define IC_USB_WVALUE_POWER_NEGOTIATION 0
#define IC_USB_WINDEX_POWER_NEGOTIATION 0
#define IC_USB_LEN_POWER_NEGOTIATION 2
#define IC_USB_PREFER_CLASSB_ENABLE_BIT 0x80
#define IC_USB_RETRIES_POWER_NEGOTIATION 3
#define IC_USB_CLASSB (1<<1)
#define IC_USB_CLASSC (1<<2)
#define IC_USB_CURRENT 100		// in 2 mA unit, 100 denotes 200 mA


/* resume_time neogo */
#define IC_USB_REQ_TYPE_GET_INTERFACE_RESUME_TIME  0xC0
#define IC_USB_REQ_GET_INTERFACE_RESUME_TIME 0x03
#define IC_USB_WVALUE_RESUME_TIME_NEGOTIATION 0
#define IC_USB_WINDEX_RESUME_TIME_NEGOTIATION 0
#define IC_USB_LEN_RESUME_TIME_NEGOTIATION 3
#define IC_USB_RETRIES_RESUME_TIME_NEGOTIATION 3


//== ===================
//  g_ic_usb_status :
//		Byte4 : wait disconnect status 
//		Byte3 Byte2 : get interface power reqest data field
//		Byte1 : power negotiation result
//
//=====================

#define PREFER_VOL_STS_SHIFT (0)
#define PREFER_VOL_STS_MSK (0x3)

#define PREFER_VOL_NOT_INITED  0x0
#define PREFER_VOL_PWR_NEG_FAIL 0x1
#define PREFER_VOL_PWR_NEG_OK 0x2


#define PREFER_VOL_CLASS_SHIFT (8)
#define PREFER_VOL_CLASS_MSK (0xff)

#define USB_PORT1_STS_SHIFT (24)
#define USB_PORT1_STS_MSK (0xf)

#define USB_PORT1_DISCONNECTING 0x0
#define USB_PORT1_DISCONNECT_DONE 0x1
#define USB_PORT1_CONNECT 0x2

/* ICUSB feature list */

/* --- sysfs controlable feature --- */
#define MTK_ICUSB_POWER_AND_RESUME_TIME_NEOGO_SUPPORT
#define MTK_ICUSB_SKIP_SESSION_REQ
#define MTK_ICUSB_SKIP_ENABLE_SESSION
#define MTK_ICUSB_SKIP_MAC_INIT
#define MTK_ICUSB_RESISTOR_CONTROL
#define MTK_ICUSB_HW_DBG
//#define MTK_ICUSB_SKIP_PORT_PM

/* --- non sysfs controlable feature --- */
//#define MTK_ICUSB_TAKE_WAKE_LOCK
//#define MTK_ICUSB_BABBLE_RECOVER

struct my_attr {
	struct attribute attr;
	int value;
};

#endif

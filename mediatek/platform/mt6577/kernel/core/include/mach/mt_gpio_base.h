#ifndef _MT_GPIO_BASE_H_
#define _MT_GPIO_BASE_H_

#include <mach/sync_write.h>

/*---------------------------------------------------------------------------*/
#define GPIO_WR32(addr, data)   mt65xx_reg_sync_writel(data, addr)   //__raw_writel(data, addr)
#define GPIO_RD32(addr)         __raw_readl(addr)
#define GPIO_SET_BITS(BIT,REG)   ((*(volatile u32*)(REG)) = (u32)(BIT))
#define GPIO_CLR_BITS(BIT,REG)   ((*(volatile u32*)(REG)) &= ~((u32)(BIT)))
/*---------------------------------------------------------------------------*/
#define MAX_GPIO_REG_BITS      16
#define MAX_GPIO_MODE_PER_REG  5
#define GPIO_MODE_BITS         3 

#define MT_GPIO_BASE_START 0

#define MAX_GPIO_PIN    232

/******************************************************************************
* Enumeration for GPIO pin
******************************************************************************/
typedef enum GPIO_PIN
{    
    GPIO_UNSUPPORTED = -1,    

    GPIO0 = MT_GPIO_BASE_START,
    GPIO1  , GPIO2  , GPIO3  , GPIO4  , GPIO5  , GPIO6  , GPIO7  , 
    GPIO8  , GPIO9  , GPIO10 , GPIO11 , GPIO12 , GPIO13 , GPIO14 , GPIO15 , 
    GPIO16 , GPIO17 , GPIO18 , GPIO19 , GPIO20 , GPIO21 , GPIO22 , GPIO23 , 
    GPIO24 , GPIO25 , GPIO26 , GPIO27 , GPIO28 , GPIO29 , GPIO30 , GPIO31 , 
    GPIO32 , GPIO33 , GPIO34 , GPIO35 , GPIO36 , GPIO37 , GPIO38 , GPIO39 , 
    GPIO40 , GPIO41 , GPIO42 , GPIO43 , GPIO44 , GPIO45 , GPIO46 , GPIO47 , 
    GPIO48 , GPIO49 , GPIO50 , GPIO51 , GPIO52 , GPIO53 , GPIO54 , GPIO55 , 
    GPIO56 , GPIO57 , GPIO58 , GPIO59 , GPIO60 , GPIO61 , GPIO62 , GPIO63 , 
    GPIO64 , GPIO65 , GPIO66 , GPIO67 , GPIO68 , GPIO69 , GPIO70 , GPIO71 , 
    GPIO72 , GPIO73 , GPIO74 , GPIO75 , GPIO76 , GPIO77 , GPIO78 , GPIO79 , 
    GPIO80 , GPIO81 , GPIO82 , GPIO83 , GPIO84 , GPIO85 , GPIO86 , GPIO87 , 
    GPIO88 , GPIO89 , GPIO90 , GPIO91 , GPIO92 , GPIO93 , GPIO94 , GPIO95 , 
    GPIO96 , GPIO97 , GPIO98 , GPIO99 , GPIO100, GPIO101, GPIO102, GPIO103, 
    GPIO104, GPIO105, GPIO106, GPIO107, GPIO108, GPIO109, GPIO110, GPIO111, 
    GPIO112, GPIO113, GPIO114, GPIO115, GPIO116, GPIO117, GPIO118, GPIO119, 
    GPIO120, GPIO121, GPIO122, GPIO123, GPIO124, GPIO125, GPIO126, GPIO127, 
    GPIO128, GPIO129, GPIO130, GPIO131, GPIO132, GPIO133, GPIO134, GPIO135, 
    GPIO136, GPIO137, GPIO138, GPIO139, GPIO140, GPIO141, GPIO142, GPIO143, 
    GPIO144, GPIO145, GPIO146, GPIO147, GPIO148, GPIO149, GPIO150, GPIO151, 
    GPIO152, GPIO153, GPIO154, GPIO155, GPIO156, GPIO157, GPIO158, GPIO159, 
    GPIO160, GPIO161, GPIO162, GPIO163, GPIO164, GPIO165, GPIO166, GPIO167, 
    GPIO168, GPIO169, GPIO170, GPIO171, GPIO172, GPIO173, GPIO174, GPIO175, 
    GPIO176, GPIO177, GPIO178, GPIO179, GPIO180, GPIO181, GPIO182, GPIO183, 
    GPIO184, GPIO185, GPIO186, GPIO187, GPIO188, GPIO189, GPIO190, GPIO191, 
    GPIO192, GPIO193, GPIO194, GPIO195, GPIO196, GPIO197, GPIO198, GPIO199, 
    GPIO200, GPIO201, GPIO202, GPIO203, GPIO204, GPIO205, GPIO206, GPIO207, 
    GPIO208, GPIO209, GPIO210, GPIO211, GPIO212, GPIO213, GPIO214, GPIO215, 
    GPIO216, GPIO217, GPIO218, GPIO219, GPIO220, GPIO221, GPIO222, GPIO223, 
    GPIO224, GPIO225, GPIO226, GPIO227, GPIO228, GPIO229, GPIO230, GPIO231,
    GPIO_MAX,
    MT_GPIO_BASE_MAX = GPIO_MAX
}GPIO_PIN;     

/*----------------------------------------------------------------------------*/
typedef struct {        /*FIXME: check GPIO spec*/
    u16 val;        
    u16 _align1;
    u16 set;
    u16 _align2;
    u16 rst;
    u16 _align3[3];
} VAL_REGS;
/*----------------------------------------------------------------------------*/
typedef struct {
    VAL_REGS    dir[15];            /*0x0000 ~ 0x00EF: 240 bytes*/
    u8          rsv00[272];         /*0x00F0 ~ 0x01FF: 272 bytes*/
    VAL_REGS    pullen[15];         /*0x0200 ~ 0x02CF: 240 bytes*/
    u8          rsv01[272];         /*0x02F0 ~ 0x03FF: 272 bytes*/
    VAL_REGS    pullsel[15];        /*0x0400 ~ 0x04CF: 240 bytes*/
    u8          rsv02[272];         /*0x04F0 ~ 0x05FF: 272 bytes*/    
    VAL_REGS    dinv[15];           /*0x0600 ~ 0x06CF: 240 bytes*/    
    u8          rsv03[272];         /*0x06F0 ~ 0x07FF: 272 bytes*/    
    VAL_REGS    dout[15];           /*0x0800 ~ 0x08CF: 240 bytes*/
    u8          rsv04[272];         /*0x08F0 ~ 0x09FF: 272 bytes*/
    VAL_REGS    din[15];            /*0x0A00 ~ 0x0ACF: 240 bytes*/
    u8          rsv05[272];         /*0x0AF0 ~ 0x0BFF: 272 bytes*/
    VAL_REGS    mode[47];           /*0x0C00 ~ 0x0EF0: 752 bytes*/  
} GPIO_REGS;
#ifdef CONFIG_PM
void mt_gpio_suspend(void);
void mt_gpio_resume(void);
#endif /*CONFIG_PM*/

#endif //_MT_GPIO_BASE_H_

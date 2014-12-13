#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>  /*proc*/
#include <linux/genhd.h>
#include <asm/uaccess.h>   /*set_fs get_fs mm_segment_t*/
#include <linux/cdev.h>

#include "partition_define.h"
#include "pmt.h"
#include <linux/mmc/host.h>
#include "../mmc-host/mt_sd.h"

#include <linux/mmc/sd_misc.h>

#define USING_XLOG

#ifdef USING_XLOG 
#include <linux/xlog.h>

#define TAG     "PMT"

#define pmt_err(fmt, args...)       \
    xlog_printk(ANDROID_LOG_ERROR, TAG, fmt, ##args)
#define pmt_info(fmt, args...)      \
    xlog_printk(ANDROID_LOG_INFO, TAG, fmt, ##args)

#else

#define TAG     "[PMT]"

#define pmt_err(fmt, args...)       \
    printk(KERN_ERR TAG);           \
    printk(KERN_CONT fmt, ##args) 
#define pmt_info(fmt, args...)      \
    printk(KERN_NOTICE TAG);        \
    printk(KERN_CONT fmt, ##args)

#endif

struct DM_PARTITION_INFO_x {
    char part_name[MAX_PARTITION_NAME_LEN];             /* the name of partition */
    unsigned long long start_addr;                                  /* the start address of partition */
    unsigned long long part_len;                                    /* the length of partition */
    unsigned char part_visibility;                              /* part_visibility is 0: this partition is hidden and CANNOT download */
                                                        /* part_visibility is 1: this partition is visible and can download */                                            
    unsigned char dl_selected;                                  /* dl_selected is 0: this partition is NOT selected to download */
                                                        /* dl_selected is 1: this partition is selected to download */
};

struct DM_PARTITION_INFO_PACKET_x {
    unsigned int pattern;
    unsigned int part_num;                              /* The actual number of partitions */
    struct DM_PARTITION_INFO_x part_info[PART_MAX_COUNT];
};

struct pt_info_x {
	int sequencenumber:8;
	int tool_or_sd_update:4;
	int pt_next:4;
	int mirror_pt_dl:4;   //mirror download OK
	int mirror_pt_has_space:4;
	int pt_changed:4;
	int pt_has_space:4;
};

static unsigned long long emmc_total_size = 0;
static unsigned long long emmc_user_size = 0;

static unsigned long long MBR_START_ADDR=0;

#define CFG_EMMC_PMT_SIZE 0xa00000
static pt_resident *last_part;
static struct pt_info_x pi;

static unsigned long long g_pt_addr_old = 0;
static unsigned long long g_mpt_addr_old = 0;
static int pt_next = 0;
static int pmt_done = 0;
extern int eMMC_rw_x(loff_t addr,u32 *buffer, int host_num, int iswrite, u32 totalsize, int transtype, Region part);
extern void msdc_check_init_done(void);

#define MSDOS_LABEL_MAGIC1	0x55
#define MSDOS_LABEL_MAGIC2	0xAA

static inline int
msdos_magic_present(unsigned char *p)
{
	return (p[0] == MSDOS_LABEL_MAGIC1 && p[1] == MSDOS_LABEL_MAGIC2);
}


static void init_storage_info(void)
{
	struct storage_info s_info = {0};

    msdc_check_init_done();

    BUG_ON(!msdc_get_info(EMMC_CARD_BOOT, EMMC_CAPACITY, &s_info));
    BUG_ON(!msdc_get_info(EMMC_CARD_BOOT, EMMC_USER_CAPACITY, &s_info)); 

	emmc_user_size = s_info.emmc_user_capacity * 512;
	emmc_total_size = s_info.emmc_capacity * 512;
	pmt_info("[%s]emmc_total_size = 0x%llx, user_region_size = 0x%llx\n", __func__,
                emmc_total_size, emmc_user_size);
}

static int load_pmt(u8 * buf)
{
    int reval = ERR_NO_EXIST;

    int i,j;
	int len=0;
	char *buf_p;
	loff_t pt_start = 1024;
	loff_t mpt_start = pt_start + 2048;
	loff_t pt_addr = 0;
	int pn_per_pmt = 0;
	int per_pmt_size = 2048;
		
	pt_resident32 *last_part32;
	int blk_size = 512;
	int read_size = 16*1024;//8192;//4096;
	char *page_buf = NULL; 
	char *backup_buf = NULL;
   	last_part32 = kmalloc(PART_MAX_COUNT*sizeof(pt_resident32), GFP_KERNEL);
	if (!last_part32) {
		reval = -ENOMEM;
		printk("load_exist_part_tab: malloc last_part32 fail\n");
		goto fail_malloc;
	}
	
	page_buf = kmalloc(read_size, GFP_KERNEL);
	if (!page_buf) {
		reval = -ENOMEM;
		printk("load_exist_part_tab: malloc page_buf fail\n");
		goto fail_malloc;
	}
	 
	backup_buf = kmalloc(read_size, GFP_KERNEL);
	if (!backup_buf) {
		reval = -ENOMEM;
		printk("load_exist_part_tab: malloc backup_buf fail\n");
		goto fail_malloc;
	}
		
	memset(last_part32, 0, PART_MAX_COUNT * sizeof(pt_resident32));
	memset(page_buf, 0x00, read_size);
 	memset(backup_buf, 0x00, read_size);
		
	//pn_per_pmt = (emmc_total_size<0x100000000)?(per_pmt_size/sizeof(pt_resident32)):(per_pmt_size/sizeof(pt_resident));
	if (emmc_total_size < 0x100000000ULL) {
		pn_per_pmt = per_pmt_size / sizeof(pt_resident32);
	} else {
		pn_per_pmt = per_pmt_size / sizeof(pt_resident);
	}

	printk("============func=%s===scan pmt from %llx=====\n", __func__,pt_start);
		/* try to find the pmt at fixed address, signature:0x50547631 */
	for (i = 0; i < CFG_EMMC_PMT_SIZE / read_size; i++) {
		buf_p = page_buf;
		reval = eMMC_rw_x(pt_start + i*read_size, (u32*)page_buf, 0, 0, read_size, 1, USER);
		if (reval) {
			printk("read pmt error\n");
			goto end;
		}
		for (j = 0; j < read_size / blk_size; j++) {
			if (is_valid_pt(buf_p)) {
				printk("find h-pt at %llx \n",pt_start + i*read_size+j*blk_size);
				if ((read_size - j * blk_size) < per_pmt_size) {
					len = read_size - j*blk_size;
					printk("left %d j=%d\n",len,j);
					memcpy(backup_buf, &buf_p[PT_SIG_SIZE], len - PT_SIG_SIZE);
					reval = eMMC_rw_x(pt_start + (i+1)*read_size, (u32*)page_buf, 0, 0, per_pmt_size, 1, USER);
					if (reval) {
						printk("read pmt error\n");
						goto end;
					}
					if (is_valid_pt(&page_buf[per_pmt_size-4-len])) {
						printk("find pt at %llx \n",pt_start + i*read_size+j*blk_size);
						memcpy(&backup_buf[len-PT_SIG_SIZE], page_buf, per_pmt_size-len);
						pt_addr = pt_start + i*read_size+j*blk_size;
						pi.pt_has_space = 1;
						reval = DM_ERR_OK;
						goto find;
					}
				} else {
					if (is_valid_pt(&buf_p[per_pmt_size-PT_SIG_SIZE])) {
						printk("find pt at %llx \n", pt_start + i*read_size + j*blk_size);
						memcpy(backup_buf,&buf_p[PT_SIG_SIZE], per_pmt_size - PT_SIG_SIZE);
						pt_addr = pt_start + i * read_size + j * blk_size;
						pi.pt_has_space = 1;
						reval = DM_ERR_OK;
						goto find;
					}
				}
				break;
			}
		buf_p += blk_size;
	  }
	}

	if (i == CFG_EMMC_PMT_SIZE / read_size) {
		pi.pt_has_space = 0;
		for (i = 0; i < CFG_EMMC_PMT_SIZE / read_size; i++) {
		    /* try to find the backup pmt at fixed address, signature:0x4d505431 */
		    buf_p = page_buf;
		    reval = eMMC_rw_x(mpt_start + i * read_size,(u32*)page_buf, 0, 0, read_size, 1, USER);
		    if (reval) {
			    printk("read pmt error\n");
			    goto end;
		    }
		    for (j = 0; j < read_size / blk_size; j++) {
			    if (is_valid_mpt(buf_p)) {
				    printk("find h-pt at %llx \n",mpt_start + i*read_size+j*blk_size);
                    if ((read_size-j*blk_size) > per_pmt_size) {
                        len = read_size- j*blk_size;
                        printk("left %d j=%d\n",len,j);
                        memcpy(backup_buf,&buf_p[PT_SIG_SIZE],len-PT_SIG_SIZE);
                        reval = eMMC_rw_x(mpt_start + (i+1)*read_size,(u32*)page_buf,0,0,per_pmt_size,1,USER);
                        if (reval) {
                            printk("read pmt error\n");
                            goto end;
                        }
                        if (is_valid_mpt(&page_buf[per_pmt_size-4-len])) {
                            printk("find mpt at %llx \n",mpt_start + i*read_size+j*blk_size);
                            memcpy(&backup_buf[len-PT_SIG_SIZE],page_buf,per_pmt_size-len);
                            pt_addr = mpt_start + i*read_size+j*blk_size;
                            reval=DM_ERR_OK;
                            goto find;//return reval;
                        }
                     } else {
                        if (is_valid_mpt(&buf_p[per_pmt_size-4])) {
                            printk("find mpt at %llx \n",mpt_start + i*read_size+j*blk_size);
                            memcpy(backup_buf,&buf_p[PT_SIG_SIZE],per_pmt_size-PT_SIG_SIZE);
                            pt_addr = mpt_start + i*read_size+j*blk_size;
                            reval=DM_ERR_OK;
                            goto find;//return reval;
                        }
                    }
				    break;
			    }
			    buf_p += blk_size;
		    }
	        }
	}

	if (i == CFG_EMMC_PMT_SIZE/read_size) {
		printk("find no pt or mpt\n");
		reval = ERR_NO_EXIST;
	}
	goto end;//return reval;

find:
	pi.pt_next = (backup_buf[per_pmt_size-11]>>4)&0x0F;
	pi.sequencenumber = backup_buf[per_pmt_size-12];
	printk("next pt? %d seq %d\n",pi.pt_next,pi.sequencenumber);
	if (pi.pt_next == 0x1) {
        reval = eMMC_rw_x(pt_addr+per_pmt_size,(u32*)page_buf,0,0,per_pmt_size,1,USER);
        if (reval) {
           printk("read pmt error\n");
           goto end;
        }
        if ((is_valid_pt(page_buf)&&is_valid_pt(&page_buf[per_pmt_size-4]))||(is_valid_mpt(page_buf)&&is_valid_mpt(&page_buf[per_pmt_size-4]))) {
            pt_next = 1;
            printk("find next pt\n");
            if (emmc_total_size<0x100000000ULL) {
                memcpy(&backup_buf[pn_per_pmt*sizeof(pt_resident32)],&page_buf[4],per_pmt_size-8);
            }else{
                memcpy(&backup_buf[pn_per_pmt*sizeof(pt_resident)],&page_buf[4],per_pmt_size-8);
            }
         } else {
            printk("can not find next pt, error\n");
        }
	}

	if (emmc_total_size<0x100000000ULL) { //32bit
		printk("32bit parse PMT\n");
		memcpy(last_part32,backup_buf,PART_MAX_COUNT*sizeof(pt_resident32));
		for (i=0;i<PART_MAX_COUNT;i++)
		{
			if (last_part32[i].name[0]!=0x00) {
				memcpy(last_part[i].name,last_part32[i].name,MAX_PARTITION_NAME_LEN);
				last_part[i].size= last_part32[i].size;
				last_part[i].offset= last_part32[i].offset;
				last_part[i].mask_flags= last_part32[i].mask_flags;
			}
		}
	} else {
		printk("64bit parse PMT, size pt = %d\n",sizeof(pt_resident));
		memcpy(buf,backup_buf,sizeof(pt_resident)*PART_MAX_COUNT);
	}
	if (pi.pt_has_space) {
		g_pt_addr_old = pt_addr;
		g_mpt_addr_old = pt_addr + (pt_next+1)*per_pmt_size;
	}else{
		g_mpt_addr_old = pt_addr;
	}
end:
	kfree(last_part32);
	kfree(page_buf);
	kfree(backup_buf);

fail_malloc:

	return reval;
}

#if 0
int init_pmt(void)
{
	int err;
    int i;

#ifdef CONFIG_PMT_ENABLE
    pmt_info("[%s]start...(CONFIG_PMT_ENABLE=y)\n", __func__);
#else
    pmt_info("[%s]start...(CONFIG_PMT_ENABLE=n)\n", __func__);
#endif

	if (pmt_done) {
		pmt_info("[%s]skip since init already\n", __func__);
		return 0;
	}

    init_storage_info();

	last_part = kzalloc(PART_MAX_COUNT * sizeof(pt_resident), GFP_KERNEL);
	if (!last_part) {
		err = -ENOMEM;
		pmt_err("[%s]fail to malloc last_part\n", __func__);
		goto fail_malloc;
	}

    memset(&pi, 0, sizeof(pt_info));

    err = load_pmt((u8 *)last_part);
    if (err) { 
       pmt_err("[%s]No pmt found and use default part info\n", __func__);
    } else {
       printk("find pt\n");
       for (i = 0; i < PART_MAX_COUNT; i++) {  
	   		if ((last_part[i].name[0] == 0x00) || (last_part[i].name[0] == 0xff)) {
				break;
            }
            printk("part %s size %llx %llx\n", last_part[i].name, 
                last_part[i].offset, last_part[i].size);
			PartInfo[i].start_address = last_part[i].offset;
			PartInfo[i].size= last_part[i].size;
		    /*	if (last_part[i].size == 0)
				break;*/
        }
	   printk("find pt %d\n",i);
    }

	pmt_done = 1;
	err = 0;

fail_malloc: 
	return err;
}
#else
int init_pmt(void)
{
    return 0;
}   
#endif
EXPORT_SYMBOL(init_pmt);


static int new_pmt(pt_resident *new_part, int table_size)
{
	int ret,i;
	int found_mpt = 0;
	int per_pmt_size = 2048;
	int pn_per_pmt = 0;
	int write_size;
	u64 mpt_addr_new = 0;
	u8* page_buf = NULL;
	char sig_buf[PT_SIG_SIZE];
	pt_resident32 *part32 = NULL;
	ret = -1;
	
	pn_per_pmt = (emmc_total_size<0x100000000ULL)?(per_pmt_size/sizeof(pt_resident32)):(per_pmt_size/sizeof(pt_resident));
	pi.pt_next = table_size > pn_per_pmt ? 1:0; 
	for (i=0;i<table_size;i++) {
		if (memcmp(new_part[i].name,"__NODL_PMT",10)==0) {
			//printk("find mpt_addr in new part %llx\n",new_part[i].offset);
			mpt_addr_new = new_part[i].offset + (1+pi.pt_next)*per_pmt_size - MBR_START_ADDR;
			printk("find mpt_addr in new part %llx\n",mpt_addr_new);
			found_mpt = 1;
			break;
		}
	}
	if (found_mpt==0) {
		printk("can not find pmt addr, so can not update\n");
		ret =  0;
		goto end;
	}

	pi.pt_changed = 1;
	pi.tool_or_sd_update = 2;
	pi.sequencenumber += 1;
	write_size = (1+pi.pt_next)*per_pmt_size;

	page_buf = kmalloc(write_size, GFP_KERNEL);
	if (!page_buf) {
		ret = -1;
		printk("new_pmt: malloc page_buf fail\n");
		goto fail_malloc;
	}
	memset(page_buf,0x00,write_size);
	
	part32 = kmalloc(table_size*sizeof(pt_resident32), GFP_KERNEL);
	if (!part32) {
		ret = -1;
		printk("load_exist_part_tab: malloc last_part32 fail\n");
		goto fail_malloc;
	}
	memset(part32,0x00,table_size*sizeof(pt_resident32));
	
	if ((g_mpt_addr_old>0)&&(g_mpt_addr_old!=mpt_addr_new)) {
		printk("mpt addr change, so it must clear old mpt %llx\n",g_mpt_addr_old);
		ret = eMMC_rw_x(g_mpt_addr_old,(u32 *)page_buf,0,1,(1+pt_next)*per_pmt_size,1,USER);
		if (ret) {
			printk("write mpt error\n");
			goto end;

		}
	}
	
	*(int *)sig_buf = MPT_SIG;
	memcpy(page_buf,&sig_buf,PT_SIG_SIZE);

	if (emmc_total_size<0x100000000ULL) {
		for (i = 0;i<table_size;i++) {
			memcpy(part32[i].name,new_part[i].name,MAX_PARTITION_NAME_LEN);
			part32[i].size = new_part[i].size;
			part32[i].offset = new_part[i].offset;
			part32[i].mask_flags= new_part[i].mask_flags;
		}
		if (pi.pt_next == 0x1) {
			memcpy(&page_buf[PT_SIG_SIZE],&part32[0],(pn_per_pmt*sizeof(pt_resident32)));
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE*2],&pi,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE],&sig_buf,PT_SIG_SIZE);

			memcpy(&page_buf[per_pmt_size],&sig_buf,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size+PT_SIG_SIZE],&part32[per_pmt_size],((table_size-pn_per_pmt)*sizeof(pt_resident32)));
			pi.pt_next = 0;
			memcpy(&page_buf[per_pmt_size*2-PT_SIG_SIZE*2],&pi,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size*2-PT_SIG_SIZE],&sig_buf,PT_SIG_SIZE);
		}else{
			memcpy(&page_buf[PT_SIG_SIZE],&part32[0],(table_size*sizeof(pt_resident32)));
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE*2],&pi,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE],&sig_buf,PT_SIG_SIZE);
		}
		
	}else{

		if (pi.pt_next == 0x1) {
			memcpy(&page_buf[PT_SIG_SIZE],&new_part[0],(pn_per_pmt*sizeof(pt_resident)));
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE*2],&pi,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE],&sig_buf,PT_SIG_SIZE);

			memcpy(&page_buf[per_pmt_size],&sig_buf,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size+PT_SIG_SIZE],&new_part[per_pmt_size],((table_size-pn_per_pmt)*sizeof(pt_resident)));
			pi.pt_next = 0;
			memcpy(&page_buf[per_pmt_size*2-PT_SIG_SIZE*2],&pi,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size*2-PT_SIG_SIZE],&sig_buf,PT_SIG_SIZE);
		}else{
			memcpy(&page_buf[PT_SIG_SIZE],&new_part[0],(table_size*sizeof(pt_resident)));
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE*2],&pi,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE],&sig_buf,PT_SIG_SIZE);
		}

	}
	ret = eMMC_rw_x(mpt_addr_new,(u32 *)page_buf,0,1,write_size,1,USER);
    if (ret) {
		printk("write mpt error\n");
		goto end;

	}
end:
	kfree(part32);
	kfree(page_buf);
fail_malloc:
	return ret;
}

static int update_pmt(pt_resident *new_part, int table_size)
{
	int ret,i;
	int found_mpt = 0;
	int per_pmt_size = 2048;
	int pn_per_pmt = 0;
	int write_size;
	u64 pt_addr_new = 0;
	u8* page_buf = NULL;
	char sig_buf[PT_SIG_SIZE];
	pt_resident32 *part32 = NULL;
	ret = -1;
	
	pn_per_pmt = (emmc_total_size<0x100000000ULL)?(per_pmt_size/sizeof(pt_resident32)):(per_pmt_size/sizeof(pt_resident));
	pi.pt_next = table_size > pn_per_pmt ? 1:0; 
	for (i=0;i<table_size;i++) {
		if (memcmp(new_part[i].name,"__NODL_PMT",10)==0) {
			printk("find pt_addr in new part %llx\n",new_part[i].offset);
			pt_addr_new = new_part[i].offset - MBR_START_ADDR;
			printk("find pt_addr in new part %llx\n",pt_addr_new);
			found_mpt = 1;
			break;
		}
	}
	if (found_mpt==0) {
		printk("can not find pmt addr, so can not update\n");
		ret =  0;
		goto end;
	}

/*	pi.pt_changed = 1;
	pi.tool_or_sd_update = 2;
	pi.sequencenumber += 1;*/
	write_size = (1+pi.pt_next)*per_pmt_size;

	page_buf = kmalloc(write_size, GFP_KERNEL);
	if (!page_buf) {
		ret = -1;
		printk("new_pmt: malloc page_buf fail\n");
		goto fail_malloc;
	}
	memset(page_buf,0x00,write_size);
	
	part32 = kmalloc(table_size*sizeof(pt_resident32), GFP_KERNEL);
	if (!part32) {
		ret = -1;
		printk("load_exist_part_tab: malloc last_part32 fail\n");
		goto fail_malloc;
	}
	memset(part32,0x00,table_size*sizeof(pt_resident32));

	if ((pi.pt_changed != 1) &&( pi.pt_has_space == 1)) {
		printk("pt may be not update\n");
		return 0;
	}
	if ((g_pt_addr_old>0)&&(g_pt_addr_old!=pt_addr_new)) {
		printk("pt addr change, so it must clear old pt\n");
		ret = eMMC_rw_x(g_pt_addr_old,(u32 *)page_buf,0,1,(1+pt_next)*per_pmt_size,1,USER);
		if (ret) {
			printk("write pt error\n");
			goto end;

		}
	}
	
	*(int *)sig_buf = PT_SIG;
	memcpy(page_buf,&sig_buf,PT_SIG_SIZE);

	if (emmc_total_size<0x100000000ULL) {
		for (i = 0;i<table_size;i++) {
			memcpy(part32[i].name,new_part[i].name,MAX_PARTITION_NAME_LEN);
			part32[i].size = new_part[i].size;
			part32[i].offset = new_part[i].offset;
			part32[i].mask_flags= new_part[i].mask_flags;
		}
		if (pi.pt_next == 0x1) {
			memcpy(&page_buf[PT_SIG_SIZE],&part32[0],(pn_per_pmt*sizeof(pt_resident32)));
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE*2],&pi,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE],&sig_buf,PT_SIG_SIZE);

			memcpy(&page_buf[per_pmt_size],&sig_buf,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size+PT_SIG_SIZE],&part32[per_pmt_size],((table_size-pn_per_pmt)*sizeof(pt_resident32)));
			pi.pt_next = 0;
			memcpy(&page_buf[per_pmt_size*2-PT_SIG_SIZE*2],&pi,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size*2-PT_SIG_SIZE],&sig_buf,PT_SIG_SIZE);
		}else{
			memcpy(&page_buf[PT_SIG_SIZE],&part32[0],(table_size*sizeof(pt_resident32)));
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE*2],&pi,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE],&sig_buf,PT_SIG_SIZE);
		}
		
	}else{

		if (pi.pt_next == 0x1) {
			memcpy(&page_buf[PT_SIG_SIZE],&new_part[0],(pn_per_pmt*sizeof(pt_resident)));
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE*2],&pi,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE],&sig_buf,PT_SIG_SIZE);

			memcpy(&page_buf[per_pmt_size],&sig_buf,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size+PT_SIG_SIZE],&new_part[per_pmt_size],((table_size-pn_per_pmt)*sizeof(pt_resident)));
			pi.pt_next = 0;
			memcpy(&page_buf[per_pmt_size*2-PT_SIG_SIZE*2],&pi,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size*2-PT_SIG_SIZE],&sig_buf,PT_SIG_SIZE);
		}else{
			memcpy(&page_buf[PT_SIG_SIZE],&new_part[0],(table_size*sizeof(pt_resident)));
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE*2],&pi,PT_SIG_SIZE);
			memcpy(&page_buf[per_pmt_size-PT_SIG_SIZE],&sig_buf,PT_SIG_SIZE);
		}

	}
	ret = eMMC_rw_x(pt_addr_new,(u32 *)page_buf,0,1,write_size,1,USER);
    if (ret) {
		printk("write mpt error\n");
		goto end;

	}
end:
	kfree(part32);
	kfree(page_buf);
fail_malloc:
	return ret;
}

static int read_pmt(void __user *arg)
{
    pmt_info("read_pmt\n");
    if (copy_to_user(arg, last_part, sizeof(pt_resident) * PART_NUM)) {
    //below is old
	//if (copy_to_user(arg,last_part,sizeof(pt_resident)*PART_MAX_COUNT))
        return -EFAULT;
    }
    return 0;
}

static int write_pmt(void __user *arg)
{
	pt_resident *new_part;
	int ret = 0;
	int i;
	int table_size =0;
	new_part = kmalloc(PART_MAX_COUNT*sizeof(pt_resident), GFP_KERNEL);
	if (!new_part) {
		ret = -ENOMEM;
		printk("write_pmt: malloc new_part fail\n");
		goto fail_malloc;
	}

	if (copy_from_user(new_part,arg,PART_MAX_COUNT*sizeof(pt_resident))) {
		ret = -EFAULT;
		goto end;
	}
	for (i=0;i<PART_MAX_COUNT;i++) {
		if (new_part[i].size == 0)
			break;
	}
	if (i == 0)
		return 0;
	
	table_size = i+1;

	printk("write table size %d\n",table_size);

	ret = new_pmt(new_part,table_size);
	ret = update_pmt(new_part,table_size);
end:
	kfree(new_part);
fail_malloc:
	return ret;
}

static int update_MBR_or_EBR(int px, u64 start_addr, u64 length)
{
	int i,ret,j;
	int found_mbr = 0;
	loff_t update_addr = 0;
	int index_in_mbr = 0;
	int mbr_index = 0;
	char *change_pt_name = NULL;
	struct partition *p;
	u8 *page_buf = NULL;
	ret =0;
		
	page_buf = kmalloc(512, GFP_KERNEL);
	if (!page_buf) {
		ret = -ENOMEM;
		printk("update_MBR_or_EBR: malloc page_buf fail\n");
		goto fail_malloc;
	}
	//data -1MB
	for (i=0;i<PART_NUM;i++) {
		if ((PartInfo[i].partition_idx == px)&&((!strncmp(PartInfo[i].name,"usrdata",7))||(!strncmp(PartInfo[i].name,"sec_ro",6))||(!strncmp(PartInfo[i].name,"android",7))||(!strncmp(PartInfo[i].name,"cache",5)))) {
			printk("update %s,need reduce 1MB in MBR\n",PartInfo[i].name);
			length -= 0x100000;
		}
	}
	
	//find px in which mbr/ebr.
	for (i=0;i<MBR_COUNT;i++) {
		for (j=0;j<SLOT_PER_MBR;j++) {
			if (MBR_EBR_px[i].part_index[j]==px) {
				found_mbr = 1;
				change_pt_name = MBR_EBR_px[i].part_name;
				index_in_mbr = j;
				mbr_index = i;
			}
		}
	}
	if (found_mbr!=1) {
		printk("p%d can not be found in mbr\n",px);
		ret = -1;
		goto end;
	}
	printk("update %s\n",change_pt_name);

	for (i=0; i<PART_NUM;i++) {
		if (!strcmp(change_pt_name,PartInfo[i].name)) {
			update_addr = PartInfo[i].start_address - MBR_START_ADDRESS_BYTE;
			printk("update %s addr %llx\n",change_pt_name, update_addr);
			break;
		}
	}
	if (i==PART_MAX_COUNT) {
		printk("can not find %s\n",change_pt_name);
		ret = -1;
		goto end;
	}
	ret = eMMC_rw_x(update_addr,(u32*)page_buf,0,0,512,1,USER);
	if (ret) {
		printk("read %s error\n",change_pt_name);
		goto end;
	}
	if (!msdos_magic_present(page_buf + 510)) {
		printk("read MBR/EBR fail\n");
		ret = -1;
		goto end;
	}
	p = (struct partition *) (page_buf + 0x1be);

	for (i=0;i<4;i++) {
		if (MBR_EBR_px[mbr_index].part_index[i]!=0) {
			printk("p%d: %x %x\n",MBR_EBR_px[mbr_index].part_index[i],p[i].start_sect,p[i].nr_sects);
			if (i==index_in_mbr) {
				printk("p%d: change to %x %x\n",MBR_EBR_px[mbr_index].part_index[i],(u32)((start_addr-update_addr)/512),(u32)(length/512));
				p[i].start_sect = (u32)((start_addr-update_addr)/512);
				p[i].nr_sects = (u32)(length/512);
			}
		}
	}

	ret = eMMC_rw_x(update_addr,(u32*)page_buf,0,1,512,1,USER);
	if (ret) {
		printk("write %s error\n",change_pt_name);
		goto end;
	}
end:
	kfree(page_buf);
fail_malloc:
	return ret;
}


static int mt65xx_mmc_change_disk_info(unsigned int px, unsigned int addr, unsigned int size)
{
    struct disk_part_iter piter;
	struct hd_struct *part;
	struct gendisk *disk;
	struct storage_info s_info = {0};
    int i;

	if (msdc_get_info(EMMC_CARD_BOOT, DISK_INFO, &s_info)) {
		disk = s_info.disk;
	} else {
		BUG();
    }
    disk_part_iter_init(&piter, disk, 0);

	for (i = 0; i < PART_NUM; i++) {
		if((PartInfo[i].partition_idx == px)&&((!strncmp(PartInfo[i].name,"usrdata",7))||(!strncmp(PartInfo[i].name,"sec_ro",6))||(!strncmp(PartInfo[i].name,"android",7))||(!strncmp(PartInfo[i].name,"cache",5)))){
			printk("update %s,need reduce 1MB in block device\n",PartInfo[i].name);
			size -= (0x100000)/512;
		}
	}

    while ((part = disk_part_iter_next(&piter))) {
        if (px != 0 && px == part->partno) {
            printk("[mt65xx_mmc_change_disk_info]px = %d size %llx -> %x offset %llx -> %x\n",px,part->nr_sects,size,part->start_sect,addr);
            part->start_sect = addr;
            part->nr_sects = size;
            disk_part_iter_exit(&piter);
            return 0;
        }
    }
    disk_part_iter_exit(&piter);

    return 1;
}


static int pmt_upgrade_proc_write(struct file*file, const char*buffer, 
                unsigned long count, void *data)
{
	struct DM_PARTITION_INFO_PACKET_x *pmtctl;
	pt_resident *new_part;
	int part_num,change_index,i;
	int ret=0;
	int pt_change = 0;
	int pt_change_tb[PART_MAX_COUNT];
    struct storage_info s_info = {0};

	memset(&pt_change_tb,0x00,PART_MAX_COUNT*sizeof(int));
	
	pmtctl = kmalloc(sizeof(struct DM_PARTITION_INFO_PACKET_x),GFP_KERNEL);
	if (!pmtctl) {
			ret = -ENOMEM;
			printk("sd_upgrade_proc_write: malloc pmtctl fail\n");
			goto fail_malloc;
	}
	memset(pmtctl,0x00,sizeof(struct DM_PARTITION_INFO_PACKET_x));
	
	new_part = kmalloc(PART_MAX_COUNT*sizeof(pt_resident),GFP_KERNEL);
	if (!new_part) {
			ret = -ENOMEM;
			printk("sd_upgrade_proc_write: malloc new_part fail\n");
			goto fail_malloc;
	}
	memset(new_part,0x00,PART_MAX_COUNT*sizeof(pt_resident));
	
	if (copy_from_user(pmtctl,buffer,sizeof(struct DM_PARTITION_INFO_PACKET_x))) {
		ret = -EFAULT;
		goto end;
		
	}
	
//1. copy new part
	for (i=0;i<PART_MAX_COUNT;i++)
	{
		memcpy(new_part[i].name,pmtctl->part_info[i].part_name,MAX_PARTITION_NAME_LEN);
		new_part[i].offset=pmtctl->part_info[i].start_addr;
		new_part[i].size=pmtctl->part_info[i].part_len;
		new_part[i].mask_flags=0;
		//MSG (INIT, "DM_PARTITION_INFO_PACKET %s size %x %x \n",dm_part->part_info[part_num].part_name,dm_part->part_info[part_num].part_len,part_num);
		printk ("[SD_UPGRADE]new_pt %s size %llx \n",new_part[i].name,new_part[i].size);
		if (pmtctl->part_info[i].part_len ==0)
		{
			printk ("[SD_UPGRADE]new_pt last %d \n",i);
			break;
		}
	}
	part_num = i+1;
	printk("[SD_UPGRADE]table size %d\n",part_num);
//2. compare new part and lastest part.
	for (change_index=0;change_index<part_num;change_index++)
	{
		if ((new_part[change_index].size!=last_part[change_index].size)||(new_part[change_index].offset!=last_part[change_index].offset))
		{
			printk ("[SD_UPGRADE]new_pt %d size changed from %llx to %llx\n",change_index,last_part[change_index].size,new_part[change_index].size);
			pt_change =1;
			pt_change_tb[change_index]=1;
			if ((pmtctl->part_info[change_index].dl_selected == 0) && (pmtctl->part_info[change_index].part_visibility == 1))
			{
				printk("[SD_UPGRADE]please download all image\n");
				ret = -1;
				goto end;
			}
		}
	}
	if (!pt_change)
	{
		printk("[SD_UPGRADE]layout can not change,skip update PMT/MBR\n");
		goto end;
	}
//3. update PMT
		
	ret = new_pmt(new_part,part_num);
	if (ret) {
		printk("[SD_UPGRADE] update m-pt fail\n");
		goto end;

	}
	ret = update_pmt(new_part,part_num);
	if (ret) {
		printk("[SD_UPGRADE] update pt fail\n");
		goto end;

	}
	printk("[SD_UPGRADE] update PMT sucess\n");
//static void update_part_size(pt_resident *new_part, int *pt_change_tb)

    BUG_ON(!msdc_get_info(EMMC_CARD_BOOT, EMMC_RESERVE, &s_info));
	for (i = 0; i <= part_num; i++) {
		if ((pt_change_tb[i] == 1) && (new_part[i].size == 0)) {
			new_part[i].size = emmc_user_size - new_part[i].offset + MBR_START_ADDR - s_info.emmc_reserve * 512;
		}
	}
//4. update MBR/EBR
	for (i=0;i<=part_num;i++) {
		if (pt_change_tb[i]==1) {
			if (PartInfo[i].partition_idx!=0) {
				printk("update p %d %llx %llx\n",PartInfo[i].partition_idx,new_part[i].offset-MBR_START_ADDR,new_part[i].size);
				ret = update_MBR_or_EBR(PartInfo[i].partition_idx,new_part[i].offset-MBR_START_ADDR,new_part[i].size);
				if (ret) {
					printk("[SD_UPGRADE]update_MBR_or_EBR fail\n");
					goto end;
				}
			}
		}
	}
	printk("[SD_UPGRADE] update  MBR/EBR sucess\n");
//5. change part device offset and size.

	for (i=0;i<=part_num;i++) {
		if (pt_change_tb[i]==1) {
			if (PartInfo[i].partition_idx!=0) {
				printk("update p %d %llx %llx\n",PartInfo[i].partition_idx,new_part[i].offset-MBR_START_ADDR,new_part[i].size);
				ret = mt65xx_mmc_change_disk_info(PartInfo[i].partition_idx,(u32)((new_part[i].offset-MBR_START_ADDR)/512),(u32)((new_part[i].size)/512));
				if (ret) {
					printk("[SD_UPGRADE]update  part device offset and size fail\n");
					goto end;
				}
			}
		}
	}
	printk("[SD_UPGRADE] update  part device offset and size sucess\n");

end:
	kfree(pmtctl);
	kfree(new_part);
fail_malloc:
	if (ret)
		return ret;
	else
		return count;
}

/*
	for (i=0;i<PART_NUM;i++) {
		if (!strcmp(PartInfo[i].name, "mbr")) {
			MBR_START_ADDR = PartInfo[i].start_address;
		}
	}
*/

#define PMT_MAGIC       'p'
#define PMT_READ        _IOW(PMT_MAGIC, 1, int)
#define PMT_WRITE       _IOW(PMT_MAGIC, 2, int)


static long pmt_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    long err;
    void __user *argp = (void __user *)arg;

    switch (cmd)
    {
        case PMT_READ:
            err = read_pmt(argp);
            break;
        case PMT_WRITE:
            err = write_pmt(argp);
            break;
        default:
            err = -EINVAL;
    }
    return err;
}


static unsigned int major;
static struct class *pmt_class;
static struct cdev *pmt_cdev;
static struct file_operations pmt_cdev_ops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = pmt_ioctl,
};

static void create_pmt_cdev(void)
{
    int err;
    dev_t devno;
    struct device *pmt_dev;

    err = alloc_chrdev_region(&devno, 0, 1, "pmt");
    if (err) {
        pmt_err("[%s]fail to alloc devno\n", __func__);
        goto fail_alloc_devno;
    }
    
    major = MAJOR(devno);

    pmt_cdev = cdev_alloc();
    if (!pmt_cdev) {
        pmt_err("[%s]fail to alloc cdev\n", __func__);
        goto fail_alloc_cdev;
    }

    pmt_cdev->owner = THIS_MODULE;
    pmt_cdev->ops = &pmt_cdev_ops;

    err = cdev_add(pmt_cdev, devno, 1);
    if (err) {
        pmt_err("[%s]fail to add cdev\n", __func__);
        goto fail_add_cdev;
    }

    pmt_class = class_create(THIS_MODULE, "pmt");
    if (IS_ERR(pmt_class)) {
        pmt_err("[%s]fail to create class pmt\n", __func__);
        goto fail_create_class;
    }
    
    pmt_dev = device_create(pmt_class, NULL, devno, NULL, "pmt");
    if (IS_ERR(pmt_dev)) {
        pmt_err("[%s]fail to create class pmt\n", __func__);
        goto fail_create_device;
    }

    return;

fail_create_device:
    class_destroy(pmt_class);
fail_create_class:
fail_add_cdev:
    cdev_del(pmt_cdev);
fail_alloc_cdev:
    unregister_chrdev_region(devno, 1);
fail_alloc_devno:
    return;
}

static void remove_pmt_cdev(void)
{
    device_destroy(pmt_class, MKDEV(major, 0));
    class_destroy(pmt_class);
    cdev_del(pmt_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);
}

static int __init pmt_init(void)
{
    struct proc_dir_entry *pmt_upgrade_proc;

    pmt_upgrade_proc = create_proc_entry("sd_upgrade", 0600, NULL);
    if (pmt_upgrade_proc) {
        pmt_upgrade_proc->write_proc = pmt_upgrade_proc_write;
        pmt_info("[%s]success to register /proc/sd_upgrade(%pf)\n", __func__, pmt_upgrade_proc->write_proc);
    } else {
        pmt_err("[%s]fail to register /proc/sd_upgrade\n", __func__);
    }

    create_pmt_cdev();

    return 0;
}

static void __exit pmt_exit(void)
{
    remove_proc_entry("sd_upgrade", NULL);

    remove_pmt_cdev();
}

module_init(pmt_init);
module_exit(pmt_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek Partition Table Management Driver");
MODULE_AUTHOR("Sophie Wang <Sophie.Wang@mediatek.com>");

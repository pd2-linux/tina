
#define _NAND_CLASS_C_

#include "nand_blk.h"
#include "nand_dev.h"


extern int debug_data;
extern unsigned long long time_used;


extern unsigned long total_read_sector;
extern unsigned long total_write_sector;
extern unsigned long long total_read_time;
extern unsigned long long total_write_time;
extern unsigned long total_read_counter;
extern unsigned long total_write_counter;

extern uint32 gc_all(void* zone);
extern uint32 gc_one(void* zone);
extern uint32 prio_gc_one(void* zone,uint16 block,uint32 flag);
extern void print_nftl_zone(void * zone);
extern void print_free_list(void* zone);
extern void print_block_invalid_list(void* zone);
extern uint32 nftl_set_zone_test(void * _zone,uint32 num);
extern uint32 NandHw_DbgInfo(uint32 type);

static ssize_t nand_test_store(struct kobject *kobject,struct attribute *attr, const char *buf, size_t count);
static ssize_t nand_test_show(struct kobject *kobject,struct attribute *attr, char *buf);
void obj_test_release(struct kobject *kobject);

int g_iShowVar = -1;

struct attribute prompt_attr = {
    .name = "nand_debug",
    .mode = 0644
};

static struct attribute *def_attrs[] = {
    &prompt_attr,
    NULL
};


struct sysfs_ops obj_test_sysops =
{
    .show =  nand_test_show,
    .store = nand_test_store
};

struct kobj_type ktype =
{
    .release = obj_test_release,
    .sysfs_ops=&obj_test_sysops,
    .default_attrs=def_attrs
};

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
void obj_test_release(struct kobject *kobject)
{
    nand_dbg_err("release");
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static ssize_t nand_test_show(struct kobject *kobject,struct attribute *attr, char *buf)
{
    ssize_t count = 0;
    struct nand_kobject* nand_kobj;

    nand_kobj = (struct nand_kobject*)kobject;

    print_nftl_zone(nand_kobj->nftl_blk->nftl_zone);

    count = sprintf(buf, "%i", g_iShowVar);

    return count;
}

/*****************************************************************************
*Name         :
*Description  :receive testcase num from echo command
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static ssize_t nand_test_store(struct kobject *kobject,struct attribute *attr, const char *buf, size_t count)
{
    int             ret;
    int             argnum = 0;
    char            cmd[32] = {0};
    unsigned int    param0 = 0;
    unsigned int    param1 = 0;
    unsigned int    param2 = 0;

    struct nand_kobject* nand_kobj;
    nand_kobj = (struct nand_kobject*)kobject;

    g_iShowVar = -1;

    argnum = sscanf(buf, "%s %u %u %u ", cmd, &param0, &param1, &param2);
    nand_dbg_err("argnum=%i, cmd=%s, param0=%u, param1=%u, param2=%u\n", argnum, cmd, param0, param1, param2);

    if (-1 == argnum)
    {
        nand_dbg_err("cmd format err!");
        g_iShowVar = -3;
        goto NAND_TEST_STORE_EXIT;
    }

    if(strcmp(cmd,"help") == 0)
    {
        nand_dbg_err("nand debug cmd:\n");
        nand_dbg_err("  help \n");
    }
    else if(strcmp(cmd,"flush") == 0)
    {
        nand_dbg_err("nand debug cmd:\n");
        nand_dbg_err("  flush \n");
        mutex_lock(nand_kobj->nftl_blk->blk_lock);
        ret = nand_kobj->nftl_blk->flush_write_cache(nand_kobj->nftl_blk,param0);
        mutex_unlock(nand_kobj->nftl_blk->blk_lock);
        goto NAND_TEST_STORE_EXIT;
    }
    else if(strcmp(cmd,"gcall") == 0)
    {
        nand_dbg_err("nand debug cmd:\n");
        nand_dbg_err("  gcall \n");
        mutex_lock(nand_kobj->nftl_blk->blk_lock);
        ret = gc_all(nand_kobj->nftl_blk->nftl_zone);
        mutex_unlock(nand_kobj->nftl_blk->blk_lock);
        goto NAND_TEST_STORE_EXIT;
    }
    else if(strcmp(cmd,"gcone") == 0)
    {
        nand_dbg_err("nand debug cmd:\n");
        nand_dbg_err("  gcone \n");
        mutex_lock(nand_kobj->nftl_blk->blk_lock);
        ret = gc_one(nand_kobj->nftl_blk->nftl_zone);
        mutex_unlock(nand_kobj->nftl_blk->blk_lock);
        goto NAND_TEST_STORE_EXIT;
    }
    else if(strcmp(cmd,"priogc") == 0)
    {
        nand_dbg_err("nand debug cmd:\n");
        nand_dbg_err("  priogc \n");
        mutex_lock(nand_kobj->nftl_blk->blk_lock);
        ret = prio_gc_one(nand_kobj->nftl_blk->nftl_zone,param0,param1);
        mutex_unlock(nand_kobj->nftl_blk->blk_lock);
        goto NAND_TEST_STORE_EXIT;
    }

    else if(strcmp(cmd,"test") == 0)
    {
        nand_dbg_err("nand debug cmd:\n");
        nand_dbg_err("  test \n");
        mutex_lock(nand_kobj->nftl_blk->blk_lock);
        ret = nftl_set_zone_test((void*)nand_kobj->nftl_blk->nftl_zone,param0);
        mutex_unlock(nand_kobj->nftl_blk->blk_lock);
        goto NAND_TEST_STORE_EXIT;
    }
    else if(strcmp(cmd,"showall") == 0)
    {
        nand_dbg_err("nand debug cmd:\n");
        nand_dbg_err("  show all \n");
        print_free_list(nand_kobj->nftl_blk->nftl_zone);
        print_block_invalid_list(nand_kobj->nftl_blk->nftl_zone);
        goto NAND_TEST_STORE_EXIT;
    }
    else if(strcmp(cmd,"showinfo") == 0)
    {
        nand_dbg_err("nand debug cmd:\n");
        nand_dbg_err("  show info \n");
        print_nftl_zone(nand_kobj->nftl_blk->nftl_zone);
        goto NAND_TEST_STORE_EXIT;
    }
    else if(strcmp(cmd,"blkdebug") == 0)
    {
        nand_dbg_err("nand debug cmd:\n");
        nand_dbg_err("  blk debug \n");
        debug_data = param0;
        goto NAND_TEST_STORE_EXIT;
    }
    else if(strcmp(cmd,"time") == 0)
    {
        nand_dbg_err("  blk time %lld\n",time_used);
        nand_dbg_err("   total_read_sector %d\n",total_read_sector);
        nand_dbg_err("   total_write_sector %d\n",total_write_sector);
        nand_dbg_err("   total_read_time %lld\n",total_read_time);
        nand_dbg_err("   total_write_time %lld\n",total_write_time);
        nand_dbg_err("   total_read_counter %d\n",total_read_counter);
        nand_dbg_err("   total_write_counter %d\n",total_write_counter);
        goto NAND_TEST_STORE_EXIT;
    }
    else if(strcmp(cmd,"version") == 0)
    {
        nand_dbg_err("nand version cmd:\n");
        nand_dbg_err("  version \n");
        NandHw_DbgInfo(0);
        goto NAND_TEST_STORE_EXIT;
    }
    else
    {
        nand_dbg_err("err, nand debug undefined cmd: %s\n", cmd);
    }

NAND_TEST_STORE_EXIT:
    return count;
}

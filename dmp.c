#include <linux/device-mapper.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/parport.h>
#include <asm/uaccess.h>
#include <linux/pci.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>

#define DM_MSG_PREFIX "dmp"
static char buf_msg[ 500] ;

struct my_dm_target {
        struct dm_dev *dev;
        sector_t start;
        int size_read;
       int size_write;
        int count_read;
        int count_write;
        int avg_read;
        int avg_write;
        int avg_size;
};

 static ssize_t x_show( struct kobject  *kobject , struct kobj_attribute  *attr, char *buf ) {

	strcpy( buf, buf_msg );
	
	return strlen( buf );
}
static ssize_t x_store( struct kobject  *kobject , struct kobj_attribute  *attr,const char *buf, size_t count ) {

	return 0;
}

static struct kobj_attribute kobj_attr_volumes=__ATTR(volumes, 0644, &x_show, &x_store );

static struct kobject *volumes;
static int dmp_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
        struct my_dm_target *mdt;
        unsigned long long start;
        if (argc != 2) {
                printk(KERN_CRIT "\n Invalid no.of arguments.\n");
                ti->error = "Invalid argument count";
                return -EINVAL;
        }
	mdt = kmalloc(sizeof(struct my_dm_target), GFP_KERNEL);

        if(mdt==NULL)
        {
                printk(KERN_CRIT "\n Mdt is null\n");
                ti->error = "dm-basic_target: Cannot allocate linear context";
                return -ENOMEM;
        }
         
        if(sscanf(argv[1], "%llu", &start)!=1)
        {
                ti->error = "dm-basic_target: Invalid device sector";
                goto bad;
        }
        mdt->start=(sector_t)start;
        if (dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &mdt->dev)) {
                ti->error = "dm-basic_target: Device lookup failed";
                goto bad;
        }
        mdt->size_read=0;
        mdt->size_write=0;
        mdt->count_read=0;
        mdt->count_write=0;
        mdt->avg_read=0;
        mdt->avg_write=0;
        ti->private = mdt;
	printk(KERN_CRIT "\n>>out function basic_target_ctr \n");
	return 0;
  bad:
        kfree(mdt);
        printk(KERN_CRIT "\n>>out function basic_target_ctr with errorrrrrrrrrr \n");           
        return -EINVAL;
}


static int dmp_map(struct dm_target *ti, struct bio *bio)
{
	struct my_dm_target *mdt = (struct my_dm_target *) ti->private;
	
	bio_set_dev(bio, mdt->dev->bdev);
	
	switch (bio_op(bio)) {
	case REQ_OP_READ:
		if (bio->bi_opf & REQ_RAHEAD)
		{
			mdt->size_read+=bio->bi_iter.bi_size;
			mdt->count_read++;
			mdt->avg_read=mdt->size_read/mdt->count_read;
			mdt->avg_size=(mdt->size_read+mdt->size_write)/(mdt->count_read+mdt->count_write);
			printk(KERN_CRIT "\n<< READ REQUEST\n");
		}
		break;
	case REQ_OP_WRITE:
		printk(KERN_CRIT "\n<< WRITE REQUEST\n");
		mdt->size_write+=bio->bi_iter.bi_size;
		mdt->count_write++;
		mdt->avg_write=mdt->size_write/mdt->count_write;
		mdt->avg_size=(mdt->size_read+mdt->size_write)/(mdt->count_read+mdt->count_write);
		break;
	default:
		break;
	}
	sprintf( buf_msg ,"read:\n  reqs:  %d\n  avg size: %d\nwrite: \n  reqs: %d \n  avg size: %d\ntotal:\n  reqs: %d \n  size: %d\n", 
	mdt->count_read, mdt->avg_read , mdt->count_write,mdt->avg_write,mdt->count_read+mdt->count_write, mdt->avg_size);
	submit_bio(bio);

	return DM_MAPIO_SUBMITTED;
}
static void dmp_dtr(struct dm_target *ti)
{
  	struct my_dm_target *mdt = (struct my_dm_target *) ti->private;
  	printk(KERN_CRIT "\n<<in function basic_target_dtr \n");        
  	dm_put_device(ti, mdt->dev);
  	kfree(mdt);
 	printk(KERN_CRIT "\n>>out function basic_target_dtr \n");               
}

static struct target_type dmp_target = {
	.name   = "dmp",
	.version = {1, 1, 0},
	.module = THIS_MODULE,
	.ctr    = dmp_ctr,
	.dtr 	= dmp_dtr,
	.map    = dmp_map,
};

static int __init dm_dmp_init(void)
{
	int res;
	struct module *t_mod=THIS_MODULE;
	struct kobject *mod_kobj;
	mod_kobj = kobject_create_and_add ("stat", &t_mod->mkobj.kobj);
	if (!mod_kobj)
		return - ENOMEM;
	res = sysfs_create_file( mod_kobj, &kobj_attr_volumes.attr);
	int r = dm_register_target(&dmp_target);
	printk( "'xxx' module initialized\n" );
	if (r < 0)
		DMERR("register failed %d", r);
	return r;
}

static void __exit dm_dmp_exit(void)
{
	sysfs_remove_file( volumes, &kobj_attr_volumes.attr );
	kobject_put( volumes );
	dm_unregister_target(&dmp_target);
	dm_unregister_target(&dmp_target);
}

module_init(dm_dmp_init)
module_exit(dm_dmp_exit)

MODULE_AUTHOR("Murad Ismailov <murad0660@gmail.com>");
MODULE_DESCRIPTION(DM_NAME " target returning dmps");
MODULE_LICENSE("GPL");


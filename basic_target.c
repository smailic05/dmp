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

#define DM_MSG_PREFIX "zero"

struct my_dm_target {
        struct dm_dev *dev;
        sector_t start;
};
/*
 * Construct a dummy mapping that only returns zeros
 */
 static ssize_t x_show( struct kobject  *kobject , struct kobj_attribute  *attr, char *buf ) {

	//strcpy( buf, buf_msg );
	
	return strlen( buf );
}
static ssize_t x_store( struct kobject  *kobject , struct kobj_attribute  *attr,const char *buf, size_t count ) {

	return 0;
}

static struct kobj_attribute kobj_attr_volumes=__ATTR(volumes, 0644, &x_show, &x_store );

static struct kobject *volumes;
static int zero_ctr(struct dm_target *ti, unsigned int argc, char **argv)
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

        ti->private = mdt;
	printk(KERN_CRIT "\n>>out function basic_target_ctr \n");
	return 0;
  bad:
        kfree(mdt);
        printk(KERN_CRIT "\n>>out function basic_target_ctr with errorrrrrrrrrr \n");           
        return -EINVAL;
}


static int zero_map(struct dm_target *ti, struct bio *bio)
{
	struct my_dm_target *mdt = (struct my_dm_target *) ti->private;
	
	bio_set_dev(bio, mdt->dev->bdev);
	
	switch (bio_op(bio)) {
	case REQ_OP_READ:
		if (bio->bi_opf & REQ_RAHEAD)
		{
			printk(KERN_CRIT "\n<< READ REQUEST\n");
		}
		//zero_fill_bio(bio);
		break;
	case REQ_OP_WRITE:
		printk(KERN_CRIT "\n<< WRITE REQUEST\n");

		break;
	default:
		break;
	}

	submit_bio(bio);
	//bio_endio(bio);

	/* accepted bio, don't make new request */
	return DM_MAPIO_SUBMITTED;
}
static void zero_dtr(struct dm_target *ti)
{
  	struct my_dm_target *mdt = (struct my_dm_target *) ti->private;
  	printk(KERN_CRIT "\n<<in function basic_target_dtr \n");        
  	dm_put_device(ti, mdt->dev);
  	kfree(mdt);
 	printk(KERN_CRIT "\n>>out function basic_target_dtr \n");               
}

static struct target_type zero_target = {
	.name   = "zeroo",
	.version = {1, 1, 0},
	.module = THIS_MODULE,
	.ctr    = zero_ctr,
	.dtr 	= zero_dtr,
	.map    = zero_map,
};

static int __init dm_zero_init(void)
{
	int res;
	struct module *t_mod=THIS_MODULE;
	struct kobject *mod_kobj;
	mod_kobj = kobject_create_and_add ("stat", &t_mod->mkobj.kobj);
	if (!mod_kobj)
		return - ENOMEM;
	res = sysfs_create_file( mod_kobj, &kobj_attr_volumes.attr);
	int r = dm_register_target(&zero_target);
	printk( "'xxx' module initialized\n" );
	if (r < 0)
		DMERR("register failed %d", r);
	return r;
}

static void __exit dm_zero_exit(void)
{
	sysfs_remove_file( volumes, &kobj_attr_volumes.attr );
	kobject_put( volumes );
	dm_unregister_target(&zero_target);
	dm_unregister_target(&zero_target);
}

module_init(dm_zero_init)
module_exit(dm_zero_exit)

MODULE_AUTHOR("Jana Saout <jana@saout.de>");
MODULE_DESCRIPTION(DM_NAME " dummy target returning zeros");
MODULE_LICENSE("GPL");


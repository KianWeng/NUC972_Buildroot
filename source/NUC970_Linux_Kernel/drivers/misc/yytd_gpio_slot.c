#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/atomic.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>

#define procfs_name "yytd_slot"
struct proc_dir_entry *slot_proc;

static int read_slot_id(){
	unsigned int id = 0;

	id |= gpio_get_value(NUC970_PI12);
	id |= (gpio_get_value(NUC970_PI13) << 1) ;
	id |= (gpio_get_value(NUC970_PI14) << 2) ;
	id |= (gpio_get_value(NUC970_PI15) << 3) ;

	return id;
}

static int slot_proc_show(struct seq_file *m, void *v)
{
	int id;
	char *device_name;
	
	id = read_slot_id();

	switch(id){
		case 0:
			printk(KERN_INFO"Slot:device name is PTN!");
			device_name = "PTN";
			break;
		case 1:
			printk(KERN_INFO"Slot:device name is E1!");
			device_name = "E1";
			break;
		default:
			printk(KERN_INFO"Slot:device name is unknow!");
			device_name = "unknow";
			break;
	}
	seq_printf(m, "%s\n", device_name);
	return 0;
}

static int slot_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, slot_proc_show, NULL);
}

static int slot_open(struct inode *inode, struct file *filp)  
{  
    printk ("misc  in slot_open\n");  
    return 0;  
}  
static int slot_release(struct inode *inode, struct file *filp)  
{  
    printk ("misc slot_release\n");  
    return 0;  
}  
  
static ssize_t slot_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)  
{  
      printk ("misc slot_read\n");
	  
      return 1;  
} 

static struct file_operations slot_proc_fops = {
	.open		= slot_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static struct file_operations slot_fops = {  
    .owner   = THIS_MODULE,  
    .read    = slot_read,  
    .open    = slot_open,  
    .release = slot_release  
}; 

static struct miscdevice slot_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "yytd_slot",
	.fops = &slot_fops,
};

static int __init yytd_gpio_slot_init(void)
{
	int ret;
	
	printk(KERN_INFO"Slot:YYTD GPIO Slot init...\n");
	ret = misc_register(&slot_misc_device);
	if(ret < 0){
		goto err_register;
		printk(KERN_ERR"Slot:register mics device fail.\n");
	}

	ret = gpio_request(NUC970_PI12, "yytd_dev_id0");
	if(ret){
		printk(KERN_ERR"Slot:Request gpio fail!\n");
		goto err_request_gpio;
	}

	ret = gpio_request(NUC970_PI3, "yytd_dev_id1");
	if(ret){
		printk(KERN_ERR"Slot:Request gpio fail!\n");
		goto err_request_gpio;
	}
	
	ret = gpio_request(NUC970_PI14, "yytd_dev_id2");
	if(ret){
		printk(KERN_ERR"Slot:Request gpio fail!\n");
		goto err_request_gpio;
	}
	
	ret = gpio_request(NUC970_PI15, "yytd_dev_id3");
	if(ret){
		printk(KERN_ERR"Slot:Request gpio fail!\n");
		goto err_request_gpio;
	}

	slot_proc = proc_create(procfs_name, 0644, NULL, &slot_proc_fops);
	if (slot_proc == NULL) {
		printk(KERN_ERR "Slot: Could not initialize /proc/%s\n",procfs_name);
		goto err_creat_proc;
	}
	
	return ret;

err_creat_proc:
	remove_proc_entry(procfs_name, NULL);
err_request_gpio:
	misc_deregister(&slot_misc_device);
err_register:
	return ret;
}
//出口函数
static void __exit yytd_gpio_slot_exit(void)
{
	printk(KERN_INFO"Slot:YYTD GPIO Slot exit...\n");
	misc_deregister(&slot_misc_device);
}

module_init(yytd_gpio_slot_init);
module_exit(yytd_gpio_slot_exit);

MODULE_DESCRIPTION("YYTD custom gpio slot driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wenggj@kongtrolink.com");
MODULE_VERSION("1.1");
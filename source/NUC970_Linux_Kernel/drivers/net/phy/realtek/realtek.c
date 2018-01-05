/*
 * drivers/net/phy/realtek.c
 *
 * Driver for Realtek PHYs
 *
 * Author: Johnson Leung <r58129@freescale.com>
 *
 * Copyright (c) 2004 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <linux/phy.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/device.h>


#define RTL821x_PHYSR		0x11
#define RTL821x_PHYSR_DUPLEX	0x2000
#define RTL821x_PHYSR_SPEED	0xc000
#define RTL821x_INER		0x12
#define RTL821x_INER_INIT	0x6400
#define RTL821x_INSR		0x13

#define	RTL8211E_INER_LINK_STAT	0x10

#define RTL8309_MAGIC 'R'
#define RTL8309M_GET_REG_DATA _IO(RTL8309_MAGIC, 0x20)
#define RTL8309M_SET_REG_DATA _IO(RTL8309_MAGIC, 0x21)
#define RTL8309M_READ_REG _IO(RTL8309_MAGIC, 0x22)
#define RTL8309M_WRITE_REG _IO(RTL8309_MAGIC, 0x23)


MODULE_DESCRIPTION("Realtek PHY driver");
MODULE_AUTHOR("Johnson Leung");
MODULE_LICENSE("GPL");

struct rtl8309m_register_data {
	unsigned int phyad;
	unsigned int regad;
};

struct rtl8309m_dev {
	struct cdev cdev;
	struct rtl8309m_register_data register_data;
	struct phy_device *phydev;
};

static int rtl8309m_major = 250;
static int rtl8309m_minor = 0;
static dev_t devno;
static struct class *rtl8309m_class;
static struct device *rtl8309m_device;
static struct rtl8309m_dev *rtl8309m_devp;



static int rtl821x_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, RTL821x_INSR);

	return (err < 0) ? err : 0;
}

static int rtl8309m_ack_interrupt(struct phy_device *phydev)
{
	int err;

	printk(KERN_INFO"Realtek:ack interrupt.\n");
	err = phy_read(phydev, RTL821x_INSR);

	return (err < 0) ? err : 0;
}


static int rtl8211b_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL821x_INER,
				RTL821x_INER_INIT);
	else
		err = phy_write(phydev, RTL821x_INER, 0);

	return err;
}

static int rtl8211e_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL821x_INER,
				RTL8211E_INER_LINK_STAT);
	else
		err = phy_write(phydev, RTL821x_INER, 0);

	return err;
}

static int rtl8309m_config_intr(struct phy_device *phydev)
{
	return 0;
}

static int rtl8309m_config_init(struct phy_device *phydev)
{
	int err = 0;
	
	printk(KERN_INFO"Realtek:start to init config.\n");
	phydev->speed = SPEED_100;
	phydev->duplex = DUPLEX_FULL;
	phydev->autoneg = AUTONEG_ENABLE;

	phydev->state = PHY_RUNNING;
	rtl8309m_devp->phydev = phydev;
	
    //printk(KERN_INFO"Marvell:return config init.\n");	
	return 0;
}

static int rtl8309m_config_aneg(struct phy_device *phydev)
{
	//return genphy_config_aneg1(phydev);
	return 0;
}

static int rtl8309m_read_status(struct phy_device *phydev)
{
	static int speed;
	static int duplex;
	static int link;

	/*
	genphy_read_status1(phydev);

	if(speed != phydev->speed || duplex != phydev->duplex || link != phydev->link)
	{
		phy_print_status(phydev);
		speed = phydev->speed;
		duplex = phydev->duplex;
		link = phydev->link;
	}*/

	phydev->speed = SPEED_100;
	phydev->duplex = DUPLEX_FULL;
	phydev->link = 1;	
	return 0;
}

int smiWrite(struct phy_device *phydev, struct rtl8309m_register_data *register_data, unsigned int value){
	int err;
	struct phy_device tmp;

	tmp = *phydev;
	tmp.addr = register_data->phyad;

	err = phy_write(&tmp, register_data->regad, value);

	return (err < 0) ? err : 0;
}

int smiRead(struct phy_device *phydev, struct rtl8309m_register_data *register_data){
	int ret;
	struct phy_device tmp;

	tmp = *phydev;
	tmp.addr = register_data->phyad;

	ret= phy_read(&tmp, register_data->regad);

	return ret;
}

/* Open the device */
static int rtl8309m_open( struct inode *inode, struct file *filp ){
	struct rtl8309m_dev *dev;
	/* 获得设备结构体指针 */
	dev = container_of(inode->i_cdev, struct rtl8309m_dev, cdev);
	/* 让设备结构体作为设备的私有信息 */
	filp->private_data = dev;
    return 0;
}



/* Close hello_device */
static int rtl8309m_release( struct inode *inode, struct file *filp ){
    return 0;
}

/* ioctl函数 */
static long rtl8309m_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){

	int value;
	struct rtl8309m_dev *dev = filp->private_data;

	//printk(KERN_INFO"Realtek:cmd is 0x%x,get is 0x%x,set is 0x%x,read is 0x%x,write is 0x%x.\n",cmd,RTL8309M_GET_REG_DATA,RTL8309M_SET_REG_DATA,RTL8309M_READ_REG,RTL8309M_WRITE_REG);
	switch(cmd){
		case RTL8309M_GET_REG_DATA:
			//printk(KERN_INFO"Realtek:ioctl get reg data!\n");
			if(copy_to_user((struct rtl8309m_register_data *)arg, &dev->register_data, sizeof(struct rtl8309m_register_data)))
				return -EFAULT;
			break;
		case RTL8309M_SET_REG_DATA:
			//printk(KERN_INFO"Realtek:ioctl set reg data!\n");
			if(copy_from_user(&dev->register_data, (struct rtl8309m_register_data *)arg, sizeof(struct rtl8309m_register_data)))
				return -EFAULT;
			break;
		case RTL8309M_READ_REG:
			//printk(KERN_INFO"Realtek:ioctl read reg!\n");
			value = smiRead(dev->phydev, &dev->register_data);
			if(copy_to_user((int *)arg, &value, sizeof(int)))
				return -EFAULT;
			break;
		case RTL8309M_WRITE_REG:
			//printk(KERN_INFO"Realtek:ioctl write reg!\n");
			if(copy_from_user(&value, (int *)arg, sizeof(int)))
				return -EFAULT;
			smiWrite(dev->phydev, &dev->register_data, value);
			break;
		default:
			break;
	}

	return 0;
}

/* RTL8211B */
static struct phy_driver rtl8211b_driver = {
	.phy_id		= 0x001cc912,
	.name		= "RTL8211B Gigabit Ethernet",
	.phy_id_mask	= 0x001fffff,
	.features	= PHY_GBIT_FEATURES,
	.flags		= PHY_HAS_INTERRUPT,
	.config_aneg	= &genphy_config_aneg,
	.read_status	= &genphy_read_status,
	.ack_interrupt	= &rtl821x_ack_interrupt,
	.config_intr	= &rtl8211b_config_intr,
	.driver		= { .owner = THIS_MODULE,},
};

/* RTL8211E */
static struct phy_driver rtl8211e_driver = {
	.phy_id		= 0x001cc915,
	.name		= "RTL8211E Gigabit Ethernet",
	.phy_id_mask	= 0x001fffff,
	.features	= PHY_GBIT_FEATURES,
	.flags		= PHY_HAS_INTERRUPT,
	.config_aneg	= &genphy_config_aneg,
	.read_status	= &genphy_read_status,
	.ack_interrupt	= &rtl821x_ack_interrupt,
	.config_intr	= &rtl8211e_config_intr,
	.suspend	= genphy_suspend,
	.resume		= genphy_resume,
	.driver		= { .owner = THIS_MODULE,},
};

/* RTL8309M */
static struct phy_driver rtl8309m_driver = {
	.phy_id		= 0x001cca52,
	.name		= "RTL8309M Gigabit Ethernet",
	.phy_id_mask	= 0x001fffff,
	.features	= PHY_BASIC_FEATURES,
	.flags		= PHY_HAS_INTERRUPT,
	.config_aneg	= &rtl8309m_config_aneg,
	.config_init    = &rtl8309m_config_init,
	.read_status	= &rtl8309m_read_status,
	.ack_interrupt	= &rtl8309m_ack_interrupt,
	.config_intr	= &rtl8309m_config_intr,
	.driver		= { .owner = THIS_MODULE,},
};

static struct file_operations rtl8309m_fops = {
	.owner = THIS_MODULE,
	.open = rtl8309m_open,
	.release = rtl8309m_release,
	.unlocked_ioctl = rtl8309m_ioctl,
};

static void rtl8309m_setup_cdev(struct rtl8309m_dev *dev){
	int err;
    /* initialize the cdev struct */
    cdev_init(&dev->cdev, &rtl8309m_fops);
    dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &rtl8309m_fops;
    err = cdev_add(&dev->cdev, devno, 1); /* register the cdev in the kernel */
    if(err)
        printk(KERN_ERR"Error %d adding rtl8309 cdev.\n", err);
}

static int __init realtek_init(void)
{
	int ret;

	printk(KERN_INFO"Realtek:start to init.\n");

	devno = MKDEV(rtl8309m_major, rtl8309m_minor);
	ret = register_chrdev(rtl8309m_major,"rtl8309m",&rtl8309m_fops);

	rtl8309m_devp = kmalloc(sizeof(struct rtl8309m_dev), GFP_KERNEL);
	if(!rtl8309m_devp){
		ret = -ENOMEM;
		goto fail_malloc;
	}
	memset(rtl8309m_devp, 0, sizeof(struct rtl8309m_dev));
	rtl8309m_setup_cdev(rtl8309m_devp);

	rtl8309m_class = class_create(THIS_MODULE, "rtl8309m");
	if(IS_ERR(rtl8309m_class)){
		ret = -EBUSY;
		goto fail_create_class;
	}
	rtl8309m_device = device_create(rtl8309m_class,NULL,devno,NULL,"rtl8309m"); 
    if(IS_ERR(rtl8309m_device))  {  
        ret = -EBUSY;
		goto fail_create_device;
    }     

	ret = phy_driver_register(&rtl8309m_driver);
	if (ret < 0){
		ret = -ENODEV;
		goto fail_register_phy;
	}
	
	return ret;

fail_register_phy:	
	device_destroy(rtl8309m_class,devno); 
fail_create_device:
	class_destroy(rtl8309m_class);  
fail_create_class:
	kfree(rtl8309m_devp);
fail_malloc:
	unregister_chrdev(rtl8309m_major,"rtl8309m"); 
	
	return ret;
}

static void __exit realtek_exit(void)
{
	device_destroy(rtl8309m_class,devno);  
    class_destroy(rtl8309m_class);   
    unregister_chrdev(rtl8309m_major,"rtl8309m"); 
	phy_driver_unregister(&rtl8309m_driver);
}

module_init(realtek_init);
module_exit(realtek_exit);

static struct mdio_device_id __maybe_unused realtek_tbl[] = {
	{ 0x001cc912, 0x001fffff },
	{ 0x001cc915, 0x001fffff },
	{ 0x001cca52, 0x001fffff },
	{ }
};

MODULE_DEVICE_TABLE(mdio, realtek_tbl);

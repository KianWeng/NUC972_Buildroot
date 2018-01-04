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
#include <linux/gpio.h>
#include <linux/delay.h>

#define RTL821x_PHYSR		0x11
#define RTL821x_PHYSR_DUPLEX	0x2000
#define RTL821x_PHYSR_SPEED	0xc000
#define RTL821x_INER		0x12
#define RTL821x_INER_INIT	0x6400
#define RTL821x_INSR		0x13

#define	RTL8211E_INER_LINK_STAT	0x10

MODULE_DESCRIPTION("Realtek PHY driver");
MODULE_AUTHOR("Johnson Leung");
MODULE_LICENSE("GPL");

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


static int __init realtek_init(void)
{
	int ret;

	printk(KERN_INFO"Realtek:start to init.\n");

	/*
	ret = gpio_request(NUC970_PB5, "rtl8309m_reset");
	if(ret < 0)
		printk(KERN_ERR"Realtek: requset reset pin fail.\n");

	gpio_direction_output(NUC970_PB5,1);
	mdelay(20);
	gpio_direction_output(NUC970_PB5,0);
	*/
	
	ret = phy_driver_register(&rtl8309m_driver);
	if (ret < 0)
		return -ENODEV;
	return ret;
}

static void __exit realtek_exit(void)
{
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

#include<linux/module.h> 
#include<linux/phy.h>
#include<linux/delay.h>
#include<linux/gpio.h>


#define MARVELL_PHY_ID_88E6071		0xff000715
#define MARVELL_PHY_MASK_88E6071	0x00fffff0

#define MARVELL_PHY_88E6071_GLOBAL_1_ADDR	0x1F
#define MARVELL_PHY_88E6071_GLOBAL_2_ADDR	0x17

#define MARVELL_PHY_88E6071_GLOBAL_2_SMI_PHY_COMMAND	0x18
#define MARVELL_PHY_88E6071_GLOBAL_2_SMI_PHY_DATA	 	0x19

#define MARVELL_PHY_88E6071_SMI_COMMAND_WRITE	0xF400
#define MARVELL_PHY_88E6071_SMI_COMMAND_READ	0xF800

// #define MARVELL_PHY_88E6071_ANAR	0x04
// #define MARVELL_PHY_88E6071_SCR		0x10

static int dev_port = 1;
module_param(dev_port, int, 0664);
static int genphy_config_aneg1(struct phy_device *phydev);
#define SEL_CHI		1

#define phy_read(phydev, regnum) m88E6071_smi_chip_read(phydev, smi_device_addr[dev_port], regnum)
#define phy_write(phydev, regnum, val)  m88E6071_smi_chip_write(phydev, smi_device_addr[dev_port], regnum, val)


static int smi_device_addr[] = {0x10, 0x11, 0x12, 0x13, 0x14};

static int m88E6071_smi_chip_read(struct phy_device *phydev, int addr, int reg)
{
	int err = 0;
	int val = 0;
	int tar = addr;
	tar <<= 5;
	tar |= reg;
	tar = MARVELL_PHY_88E6071_SMI_COMMAND_READ | tar;

	err = mdiobus_write(phydev->bus, MARVELL_PHY_88E6071_GLOBAL_2_ADDR, MARVELL_PHY_88E6071_GLOBAL_2_SMI_PHY_COMMAND, tar);

  	if (err < 0)
	{
		return err;
	}	

	//msleep(50);
	while(mdiobus_read(phydev->bus, MARVELL_PHY_88E6071_GLOBAL_2_ADDR, MARVELL_PHY_88E6071_GLOBAL_2_SMI_PHY_COMMAND) & 0x8000);

	err = mdiobus_read(phydev->bus, MARVELL_PHY_88E6071_GLOBAL_2_ADDR, MARVELL_PHY_88E6071_GLOBAL_2_SMI_PHY_DATA);
	
	if (err < 0)
	{
		return err;
	}

	val = err & 0xFFFF;

	return val;
}

static int m88E6071_smi_chip_write(struct phy_device *phydev, int addr, int reg, u16 val)
{
	int err = 0;
	
	int tar = addr;
	tar <<= 5;
	tar |= reg;
	tar = MARVELL_PHY_88E6071_SMI_COMMAND_WRITE | tar;


	err = mdiobus_write(phydev->bus, MARVELL_PHY_88E6071_GLOBAL_2_ADDR, MARVELL_PHY_88E6071_GLOBAL_2_SMI_PHY_DATA, val);
	
	if (err < 0)
	{
		return err;
	}


	err = mdiobus_write(phydev->bus, MARVELL_PHY_88E6071_GLOBAL_2_ADDR, MARVELL_PHY_88E6071_GLOBAL_2_SMI_PHY_COMMAND, tar);
	while(mdiobus_read(phydev->bus, MARVELL_PHY_88E6071_GLOBAL_2_ADDR, MARVELL_PHY_88E6071_GLOBAL_2_SMI_PHY_COMMAND) & 0x8000);
	
	return err;
}


// open isolate 0.10
// enable auto-negotiation 0.12
// restart auto-negotiation 0.9
static int m88E6071_config_init(struct phy_device *phydev)
{
	printk(KERN_INFO"Marvell:start to init config.\n");
	phydev->speed = SPEED_100;
	phydev->duplex = DUPLEX_FULL;
	phydev->autoneg = AUTONEG_ENABLE;
	phydev->state = PHY_RUNNING;
	
	/*marvell 88e6071 port 5 has no PHY,so we do not need to config it.*/
	//for(dev_port=0;dev_port<2;dev_port++)
	//genphy_config_aneg1(phydev);
	//dev_port = 1;
	
	return 0;
}

static int genphy_setup_forced1(struct phy_device *phydev)
{
	int ctl = 0;

	phydev->pause = 0;
	phydev->asym_pause = 0;

	if (SPEED_1000 == phydev->speed)
		ctl |= BMCR_SPEED1000;
	else if (SPEED_100 == phydev->speed)
		ctl |= BMCR_SPEED100;

	if (DUPLEX_FULL == phydev->duplex)
		ctl |= BMCR_FULLDPLX;

	return phy_write(phydev, MII_BMCR, ctl);
}

static int genphy_restart_aneg1(struct phy_device *phydev)
{
	int ctl = phy_read(phydev, MII_BMCR);

	if (ctl < 0)
		return ctl;

	ctl |= BMCR_ANENABLE | BMCR_ANRESTART;

	/* Don't isolate the PHY if we're negotiating */
	ctl &= ~BMCR_ISOLATE;

	return phy_write(phydev, MII_BMCR, ctl);
}

static int genphy_config_advert(struct phy_device *phydev)
{
	u32 advertise;
	int oldadv, adv, bmsr;
	int err, changed = 0;

	/* Only allow advertising what this PHY supports */
	phydev->advertising &= phydev->supported;
	advertise = phydev->advertising;

	/* Setup standard advertisement */
	adv = phy_read(phydev, MII_ADVERTISE);
	if (adv < 0)
		return adv;

	oldadv = adv;
	adv &= ~(ADVERTISE_ALL | ADVERTISE_100BASE4 | ADVERTISE_PAUSE_CAP |
		 ADVERTISE_PAUSE_ASYM);
	adv |= ethtool_adv_to_mii_adv_t(advertise);

	if (adv != oldadv) {
		err = phy_write(phydev, MII_ADVERTISE, adv);

		if (err < 0)
			return err;
		changed = 1;
	}

	bmsr = phy_read(phydev, MII_BMSR);
	if (bmsr < 0)
		return bmsr;

	/* Per 802.3-2008, Section 22.2.4.2.16 Extended status all
	 * 1000Mbits/sec capable PHYs shall have the BMSR_ESTATEN bit set to a
	 * logical 1.
	 */
	if (!(bmsr & BMSR_ESTATEN))
		return changed;

	/* Configure gigabit if it's supported */
	adv = phy_read(phydev, MII_CTRL1000);
	if (adv < 0)
		return adv;

	oldadv = adv;
	adv &= ~(ADVERTISE_1000FULL | ADVERTISE_1000HALF);

	if (phydev->supported & (SUPPORTED_1000baseT_Half |
				 SUPPORTED_1000baseT_Full)) {
		adv |= ethtool_adv_to_mii_ctrl1000_t(advertise);
		if (adv != oldadv)
			changed = 1;
	}

	err = phy_write(phydev, MII_CTRL1000, adv);
	if (err < 0)
		return err;

	return changed;
}

static int genphy_config_aneg1(struct phy_device *phydev)
{
	int result;

	if (AUTONEG_ENABLE != phydev->autoneg)
		return genphy_setup_forced1(phydev);

	result = genphy_config_advert(phydev);
	if (result < 0) /* error */
		return result;
	if (result == 0) {
		/* Advertisement hasn't changed, but maybe aneg was never on to
		 * begin with?  Or maybe phy was isolated?
		 */
		int ctl = phy_read(phydev, MII_BMCR);

		if (ctl < 0)
			return ctl;

		if (!(ctl & BMCR_ANENABLE) || (ctl & BMCR_ISOLATE))
			result = 1; /* do restart aneg */
	}

	/* Only restart aneg if we are advertising something different
	 * than we were before.
	 */
	if (result > 0)
		result = genphy_restart_aneg1(phydev);

	return result;
}


static int m88E6071_config_aneg(struct phy_device *phydev)
{
	return genphy_config_aneg1(phydev);
}

static int genphy_update_link1(struct phy_device *phydev)
{
	int status;

	/* Do a fake read */
	status = phy_read(phydev, MII_BMSR);
	if (status < 0)
		return status;

	/* Read link and autonegotiation status */
	status = phy_read(phydev, MII_BMSR);
	if (status < 0)
		return status;

	if ((status & BMSR_LSTATUS) == 0)
		phydev->link = 0;
	else
		phydev->link = 1;

	return 0;
}

static int genphy_read_status1(struct phy_device *phydev)
{
	int adv;
	int err;
	int lpa;
	int lpagb = 0;
	int common_adv;
	int common_adv_gb = 0;

	/* Update the link, but return if there was an error */
	err = genphy_update_link1(phydev);
	if (err)
		return err;

	//phydev->lp_advertising = 0;
	phydev->advertising = 0;

	if (AUTONEG_ENABLE == phydev->autoneg) {
		if (phydev->supported & (SUPPORTED_1000baseT_Half
					| SUPPORTED_1000baseT_Full)) {
			lpagb = phy_read(phydev, MII_STAT1000);
			if (lpagb < 0)
				return lpagb;

			adv = phy_read(phydev, MII_CTRL1000);
			if (adv < 0)
				return adv;

			phydev->advertising =
				mii_stat1000_to_ethtool_lpa_t(lpagb);
			common_adv_gb = lpagb & adv << 2;
		}

		lpa = phy_read(phydev, MII_LPA);
		if (lpa < 0)
			return lpa;

		phydev->advertising |= mii_lpa_to_ethtool_lpa_t(lpa);

		adv = phy_read(phydev, MII_ADVERTISE);
		if (adv < 0)
			return adv;

		common_adv = lpa & adv;

		phydev->speed = SPEED_10;
		phydev->duplex = DUPLEX_HALF;
		phydev->pause = 0;
		phydev->asym_pause = 0;

		if (common_adv_gb & (LPA_1000FULL | LPA_1000HALF)) {
			phydev->speed = SPEED_1000;

			if (common_adv_gb & LPA_1000FULL)
				phydev->duplex = DUPLEX_FULL;
		} else if (common_adv & (LPA_100FULL | LPA_100HALF)) {
			phydev->speed = SPEED_100;

			if (common_adv & LPA_100FULL)
				phydev->duplex = DUPLEX_FULL;
		} else
			if (common_adv & LPA_10FULL)
				phydev->duplex = DUPLEX_FULL;

		if (phydev->duplex == DUPLEX_FULL) {
			phydev->pause = lpa & LPA_PAUSE_CAP ? 1 : 0;
			phydev->asym_pause = lpa & LPA_PAUSE_ASYM ? 1 : 0;
		}
	} else {
		int bmcr = phy_read(phydev, MII_BMCR);

		if (bmcr < 0)
			return bmcr;

		if (bmcr & BMCR_FULLDPLX)
			phydev->duplex = DUPLEX_FULL;
		else
			phydev->duplex = DUPLEX_HALF;

		if (bmcr & BMCR_SPEED1000)
			phydev->speed = SPEED_1000;
		else if (bmcr & BMCR_SPEED100)
			phydev->speed = SPEED_100;
		else
			phydev->speed = SPEED_10;

		phydev->pause = 0;
		phydev->asym_pause = 0;
	}

	return 0;
}

static int m88E6071_read_status(struct phy_device *phydev)
{
	static int speed;
	static int duplex;
	static int link;
	
	genphy_read_status1(phydev);

	if(speed != phydev->speed || duplex != phydev->duplex || link != phydev->link)
	{
		phy_print_status(phydev);
		speed = phydev->speed;
		duplex = phydev->duplex;
		link = phydev->link;
	}

	phydev->speed = SPEED_100;
	phydev->duplex = DUPLEX_FULL;
	phydev->link = 1;	
	return 0;
}

static int m88E6071_ack_interrupt(struct phy_device *phydev) 
{
	int err;
	
	printk(KERN_INFO"Marvell:m88E6071 ack interrupt enter.\n");
	err = m88E6071_smi_chip_read(phydev, smi_device_addr[SEL_CHI], 18);
	return (err < 0) ? err : 0;
}

static int m88E6071_config_intr(struct phy_device *phydev)
{
	int index = 0;
	int err;
	
	printk(KERN_INFO"Marvell:m88E6071 config interrupt enter.\n");
	for(; index < 5; ++index) {
		err = m88E6071_smi_chip_read(phydev, smi_device_addr[index], 18);
	
		if (err < 0) 
		{
			return err;
		}
		printk(KERN_INFO "claude test interrupt : %d \n", phydev->interrupts == PHY_INTERRUPT_ENABLED);
		if (phydev->interrupts == PHY_INTERRUPT_ENABLED) 
		{	
			err = 0x7FD3;
		}
		else
		{
			err = ~0x7FD3;
		}
	}
	return err;
}

static struct phy_driver marvell_drivers[] = {
	{
		.phy_id         =   MARVELL_PHY_ID_88E6071,
        .phy_id_mask    =   MARVELL_PHY_MASK_88E6071,
        .name           =   "Marvell 88E6071",
		.flags			=	PHY_HAS_INTERRUPT,
        .features       =   PHY_BASIC_FEATURES,
        .config_init    =   &m88E6071_config_init,
        .config_aneg    =   &m88E6071_config_aneg,
        .read_status    =   &m88E6071_read_status,
		.ack_interrupt  =	&m88E6071_ack_interrupt,
		.config_intr 	=	&m88E6071_config_intr,
        .driver         =   { .owner = THIS_MODULE },
	},
};

static int __init marvell_init(void)
{
	int err;

	printk(KERN_INFO"Marvell: init marvell phy.\n");
	err = phy_drivers_register(marvell_drivers, ARRAY_SIZE(marvell_drivers));
	
	if(err)
	{
		printk(KERN_ERR "Marvell:Register marvell_88e6071 driver failed.\n");
		goto error;
	}

	return err;
	
error:
	phy_drivers_unregister(marvell_drivers, ARRAY_SIZE(marvell_drivers));
	return err;
}

static void __exit marvell_exit(void)
{
	phy_drivers_unregister(marvell_drivers, ARRAY_SIZE(marvell_drivers));
}

module_init(marvell_init);
module_exit(marvell_exit);

static struct mdio_device_id __maybe_unused marvell_tb1[] = {
	{ MARVELL_PHY_ID_88E6071, MARVELL_PHY_MASK_88E6071},
};

MODULE_DEVICE_TABLE(mdio, marvell_tb1);

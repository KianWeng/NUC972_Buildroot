/*
 * Copyright (c) 2014 Nuvoton Technology corporation.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/sched.h>


#define PIN_OUTPUT_HIGH   1
#define PIN_OUTPUT_LOW    0
#define WDT_STATE_ON      1
#define WDT_STATE_OFF     0

static int pin_state = 0;
static int wdt_state = 0;
static struct delayed_work cat823s_wdt_feed_work;

static int __init get_watchdog_state(char *str){
	if(0 == strcmp(str,"on"))
		wdt_state = WDT_STATE_ON;
	else if(0 == strcmp(str,"off"))
		wdt_state = WDT_STATE_OFF;
	
	return 1;
}

__setup("watchdog=", get_watchdog_state);


static void cat823s_wdt_feed_func(struct work_struct* work){
	int err;

	switch(pin_state){
		case PIN_OUTPUT_HIGH:
			err = gpio_direction_output(NUC970_PF10,0);
			pin_state = PIN_OUTPUT_LOW;
			break;
		case PIN_OUTPUT_LOW:
			err = gpio_direction_output(NUC970_PF10,1);
			pin_state = PIN_OUTPUT_HIGH;
			break;
		default:
			break;
	}

	//printk(KERN_INFO"CAT823s:feed watchdog, current pin state is %d.\n",pin_state);
	schedule_delayed_work(&cat823s_wdt_feed_work,msecs_to_jiffies(800));
}

static int cat823s_wdt_enable(void){
	int err;

	printk(KERN_INFO"Enable external wdt.\n");
	err = gpio_direction_output(NUC970_PH3, 0);

	return err;
}

static int cat823s_wdt_disable(void){
	int err;

	printk(KERN_INFO"Disable external wdt.\n");
	err = gpio_direction_output(NUC970_PH3, 1);

	return err;
}

static int __init cat823s_module_init(void){
	int err;

	printk(KERN_INFO"CAT823S:Init cat823s module...\n");
	err = gpio_request(NUC970_PH3, "external_wdt_enable");
	if(err){
		printk(KERN_ERR"Request external wdt enable pin fail.\n");
		return -1;
	}

	err = gpio_request(NUC970_PF10,"external_wdt_reset");
	if(err){
		printk(KERN_ERR"Request external wdt reset pin fail.\n");
		return -1;
	}

	INIT_DELAYED_WORK(&cat823s_wdt_feed_work, cat823s_wdt_feed_func);
	if(wdt_state == WDT_STATE_ON){
		cat823s_wdt_enable();
		gpio_direction_output(NUC970_PF10, 0);
		pin_state = PIN_OUTPUT_LOW;
		err = schedule_delayed_work(&cat823s_wdt_feed_work, msecs_to_jiffies(800));
	}else{
		cat823s_wdt_disable();
	}

	return err;
	
}

static void __exit cat823s_module_exit(void){
	if(wdt_state == WDT_STATE_ON){
		cat823s_wdt_disable();
		cancel_delayed_work_sync(&cat823s_wdt_feed_work);
	}
}

module_init(cat823s_module_init);
module_exit(cat823s_module_exit);

MODULE_DESCRIPTION("Watchdog driver for CAT823s");
MODULE_AUTHOR("wenggj@kongtrolink.com");
MODULE_LICENSE("GPL");


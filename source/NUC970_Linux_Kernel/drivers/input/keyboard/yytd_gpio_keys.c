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
#include <linux/input.h>

#define	KEY_NAME	"recovery-key"
#define IRQ_NAME    "recovery-key-irq"
#define DEV_NAME    "YYTD_GPIO_KEYS"

static struct input_dev *gpio_key_dev;
static int irqno;

//中断处理函数
irqreturn_t report_key_handler(int num, void *data)
{
	int gpio_value = 0;
	struct input_dev *dev;
	dev = data;

	gpio_value = gpio_get_value(NUC970_PI4);
	switch(gpio_value){
		case 0:
			printk(KERN_INFO"Keys:Recovery key pressed!\n");
			input_report_key(dev, KEY_RECOVERY, !gpio_value);
			input_sync(dev);
			break;
		case 1:
			printk(KERN_INFO"Keys:Recovery key release!\n");
			input_report_key(dev, KEY_RECOVERY, !gpio_value);
			input_sync(dev);
			break;
		default:
			break;
	}

	return IRQ_HANDLED;
}

void input_dev_init(struct input_dev *dev)
{
	__set_bit(EV_SYN, dev->evbit);  //设置位
	__set_bit(EV_KEY, dev->evbit);  //设置一个按键类事件,表示能够产生按键事件。

	__set_bit(KEY_RECOVERY, dev->keybit);//表示能产生哪些按键
}


static int yytd_gpio_key_init(void)
{
	int ret;
	
	printk(KERN_INFO"Keys:YYTD GPIO KEYS init...\n");
	//分配一个input_dev结构体
	gpio_key_dev = input_allocate_device();	
	//如果分配失败就返回
	if(IS_ERR_OR_NULL(gpio_key_dev))
	{
		ret = -ENOMEM;
		goto ERR_alloc;
	}
	gpio_key_dev->name = DEV_NAME;
	gpio_key_dev->id.bustype = BUS_HOST;
	gpio_key_dev->id.vendor = 0x0001;
	gpio_key_dev->id.product = 0x0001;
	gpio_key_dev->id.version = 0x0100;

	//初始化设备
	input_dev_init(gpio_key_dev);
	//注册这个设备
	ret = input_register_device(gpio_key_dev);
	if(IS_ERR_VALUE(ret))
	{
		goto ERR_input_reg;
	}
	//注册PI4 GPIO中断
	ret = gpio_request(NUC970_PI4, KEY_NAME);
	if(ret){
		printk(KERN_ERR"Keys:Request recovery key gpio fail!\n");
		goto ERR_request_gpio;
	}
	irqno = gpio_to_irq(NUC970_PI4);
	ret = request_irq(irqno, report_key_handler, 
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			IRQ_NAME, gpio_key_dev);
	if(IS_ERR_VALUE(ret))
	{
		goto ERR_request_irq;
	}

	return 0;
	//对错误返回进行处理
ERR_request_irq:
	gpio_free(NUC970_PI4);
ERR_request_gpio:
	input_unregister_device(gpio_key_dev);
ERR_input_reg:
	input_free_device(gpio_key_dev);
ERR_alloc:
	return ret;
}
//出口函数
static void yytd_gpio_key_exit(void)
{
	printk(KERN_INFO"Keys:YYTD GPIO KEYS exit...\n");
	free_irq(irqno, gpio_key_dev);
	input_unregister_device(gpio_key_dev);
	input_free_device(gpio_key_dev);
}

module_init(yytd_gpio_key_init);
module_exit(yytd_gpio_key_exit);

MODULE_DESCRIPTION("YYTD custom gpio keys driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wenggj@kongtrolink.com");
MODULE_VERSION("1.1");
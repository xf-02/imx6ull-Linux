#include<linux/delay.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>

static int major=0;					//主设备号
static struct class *sr04_class;	//类
static struct gpio_desc *sr04_trig;	//引脚定义
static struct gpio_desc *sr04_echo;
static int irq;						//中断号
static wait_queue_head_t sr04_wq;	//创建休眠等待队列
static u32 time_ns=0;				//超声波echo引脚高电平时间，反映前方物体距离

//中断处理函数
static irqreturn_t sr04_handler(int irq, void *dev_id)
{
	int val;

	val = gpiod_get_value(sr04_echo);

	//如果是上升沿
	if(val)
	{
		time_ns = ktime_get_ns();
	}
	else
	{
		time_ns = ktime_get_ns() - time_ns;	//获取echo信号的高电平持续时间
		wake_up_interruptible(&sr04_wq);	//唤醒等待队列上所有处于中断ible等待状态的进程。
	}
	
	return IRQ_HANDLED;
}

static ssize_t sr04_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	unsigned long err;
	int timeout;
	uint32_t distance_mm;
	
	//发出10us的脉冲启动信号
	gpiod_set_value(sr04_trig, 1);
	udelay(15);
	gpiod_set_value(sr04_trig, 0);

	//使内核进入等待队列中并进行休眠，该线程会被中断唤醒，之后判断tim_ns的值是不是为真
	//为真就退出休眠，进行数据上传至用户空间。
	timeout = wait_event_interruptible_timeout(sr04_wq, time_ns, HZ);
	if(timeout)
	{
		distance_mm = 340/2*time_ns/1000000;
		err = copy_to_user(buf, &distance_mm, 4);
		time_ns = 0;		//清空时间数据
		return 4;
	}
	else	//如果是超时导致的退出休眠，说明距离太远，返回错误
	{
		return -EAGAIN;
	}
}
 
static unsigned int sr04_poll(struct file *file, struct poll_table_struct *poll)
{
	return 0;
}

static int sr04_open(struct inode *inode, struct file *file)
{
	return 0;
}


static struct file_operations sr04_ops = {
	.owner	=THIS_MODULE,
	.open 	=sr04_open,
	.read 	=sr04_read,
	.poll 	=sr04_poll,
};


//多个设备资源共用的驱动
static const struct of_device_id sr04s[] = {
	{.compatible = "xf,sr04"},
	{ },
};

//匹配回调函数
static int sr04_probe(struct platform_device *pdev)
{
	int err;
		
	//设备树中有定义
	sr04_trig = gpiod_get(&pdev->dev, "trig", GPIOD_OUT_LOW);
	sr04_echo = gpiod_get(&pdev->dev, "echo", GPIOD_IN);
	if(IS_ERR(sr04_trig)){
		dev_err(&pdev->dev, "GPIO trig ERR!");
		return PTR_ERR(sr04_trig);
	}
	if(IS_ERR(sr04_echo)){
		dev_err(&pdev->dev, "GPIO echo ERR!");
		return PTR_ERR(sr04_echo);
	}

	//设置GPIO为输入,获得GPIO时已经设置方向
	//gpiod_direction_input(sr04_gpios);
	
	irq = gpiod_to_irq(sr04_echo);
	err = request_irq(irq, sr04_handler, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "xf_sr04_irq", NULL);

	device_create(sr04_class, NULL, MKDEV(major, 0), NULL, "xf_sr04"); 

	return 0;
}

static int sr04_remove(struct platform_device *pdev)
{
	//删除设备信息
	device_destroy(sr04_class, MKDEV(major, 0));
	free_irq(irq, NULL);
	gpiod_put(sr04_trig);
	gpiod_put(sr04_echo);
	return 0;
}


//驱动程序注册结构体
static struct platform_driver sr04_driver = {
	.probe		=sr04_probe,
	.remove		=sr04_remove,
	.driver		={
		.name	="xf_sr04",
		.of_match_table = sr04s,
	},
};

static int __init sr04_init(void)
{
	int err;

	//打印运行到这里的代码段行数，用于调试
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	//注册设备
	major = register_chrdev(0, "sr04_xf", &sr04_ops);

	//创建类，自动分配设备结点信息
	sr04_class = class_create(THIS_MODULE, "sr04_class_xf");
	if (IS_ERR(sr04_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "sr04_xf");
		return PTR_ERR(sr04_class);
	}

	//初始化等待队列，这个队列用于同步超声波echo的中断信号与Read线程，因为只有在
	//echo下降沿发生并测量时间之后才能进行Read距离数据
	init_waitqueue_head(&sr04_wq);

	//注册驱动结构体
	err = platform_driver_register(&sr04_driver);

	return err;
}

static void __exit sr04_exit(void)
{
	//打印运行到这里的代码段行数，用于调试
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	//卸载驱动结构体
	platform_driver_unregister(&sr04_driver);
	class_destroy(sr04_class);
	unregister_chrdev(major, "zs_sr04");
	
}


module_init(sr04_init);
module_exit(sr04_exit);

MODULE_LICENSE("GPL");



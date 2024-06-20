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
#include <linux/version.h>
#include <linux/input.h>


static int major=0;	//主设备号
static struct class *hs0038_class;	//类
static struct gpio_desc *hs0038_data;
static int irq;	//中断号
static wait_queue_head_t hs0038_wq;
static int hs0038_data_flag=0;	//数据标志
static unsigned char hs0038_value = 0;
static u64 hs0038_edge_time[100];
static int hs0038_edge_cnt = 0;

static struct input_dev *hs0038_input_dev;


int hs0038_parse_data(unsigned int *val)
{
	u64 tmp;
	unsigned char data[4];
	int i, j, m;
	
	/* 判断是否重复码 */
	if (hs0038_edge_cnt == 4)
	{
		tmp = hs0038_edge_time[1] - hs0038_edge_time[0];
		if (tmp > 8000000 && tmp < 10000000)
		{
			tmp = hs0038_edge_time[2] - hs0038_edge_time[1];
			if (tmp < 3000000)
			{
				/* 获得了重复码 */
				*val = hs0038_value;
				return 0;
			}
		}
	}

	/* 接收到了66次中断 */
	m = 3;
	if (hs0038_edge_cnt >= 68)
	{
		/* 解析到了数据 */
		for (i = 0; i < 4; i++)
		{
			data[i] = 0;
			/* 先接收到bit0 */
			for (j = 0; j < 8; j++)
			{
				/* 数值: 1 */	
				if (hs0038_edge_time[m+1] - hs0038_edge_time[m] > 1000000)
					data[i] |= (1<<j);
				m += 2;
			}
		}

		/* 检验数据 */
		data[1] = ~data[1];
		if (data[0] != data[1])
		{
			return -2;
		}

		data[3] = ~data[3];
		if (data[2] != data[3])
		{
			return -2;
		}

		hs0038_value = (data[0] << 8) | (data[2]);
		*val = hs0038_value;
		return 0;
	}
	else
	{
		/* 数据没接收完毕 */
		return -1;
	}	
}


static irqreturn_t hs0038_handler(int irq, void *dev_id)
{
	unsigned int val;
	int ret;
	
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
			hs0038_edge_time[hs0038_edge_cnt++] = ktime_get_boottime_ns();
#else
			hs0038_edge_time[hs0038_edge_cnt++] = ktime_get_boot_ns();
#endif

	/* 判断超时 */
	if (hs0038_edge_cnt >= 2)
	{
		if (hs0038_edge_time[hs0038_edge_cnt-1] - hs0038_edge_time[hs0038_edge_cnt-2] > 30000000)
		{
			/* 超时 */
			hs0038_edge_time[0] = hs0038_edge_time[hs0038_edge_cnt-1];
			hs0038_edge_cnt = 1;			
			return IRQ_HANDLED; // IRQ_WAKE_THREAD;
		}
	}

	ret = hs0038_parse_data(&val);
	if (!ret)
	{
		/* 解析成功 */
		hs0038_edge_cnt = 0;

		hs0038_data_flag = 1;
		wake_up(&hs0038_wq);

		/* D. 输入系统: 上报数据 */
		input_event(hs0038_input_dev, EV_KEY, val, 1);
		input_event(hs0038_input_dev, EV_KEY, val, 0);
		input_sync(hs0038_input_dev);	
	}
	else if (ret == -2)
	{
		/* 解析失败 */
		hs0038_edge_cnt = 0;
	}
	return IRQ_HANDLED;
}

static ssize_t hs0038_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	int err;

	if (size != 4)
		return -EINVAL;

	wait_event_interruptible(hs0038_wq, hs0038_data_flag);

	hs0038_data_flag = 0;
	err = copy_to_user(buf, &hs0038_value, 4);
	
	return 4;
}
 
static unsigned int hs0038_poll(struct file *file, struct poll_table_struct *poll)
{
	return 0;
}

static int hs0038_open(struct inode *inode, struct file *file)
{
	return 0;
}


static struct file_operations hs0038_ops = {
	.owner	=THIS_MODULE,
	.open 	=hs0038_open,
	.read 	=hs0038_read,
	.poll 	=hs0038_poll,
};


//多个设备资源共用的驱动
static const struct of_device_id hs0038s[] = {
	{.compatible = "xf,hs0038"},
	{ },
};


//匹配回调函数
static int hs0038_probe(struct platform_device *pdev)
{
	int err;
		
	//设备树中有定义
	hs0038_data = gpiod_get(&pdev->dev, NULL, GPIOD_IN);
	if(IS_ERR(hs0038_data)){
		dev_err(&pdev->dev, "GPIO data ERR!");
		return PTR_ERR(hs0038_data);
	}
	
	irq = gpiod_to_irq(hs0038_data);
	err = request_irq(irq, hs0038_handler, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "hs0038", NULL);

	device_create(hs0038_class, NULL, MKDEV(major, 0), NULL, "xf_hs0038"); 

	/* 输入系统的代码 */
	/* 参考: drivers\input\keyboard\gpio_keys.c */
	/* A. 分配input_dev */
	hs0038_input_dev = devm_input_allocate_device(&pdev->dev);

	/* B. 设置input_dev */
	hs0038_input_dev->name = "hs0038";
	hs0038_input_dev->phys = "hs0038";

	/* B.1 能产生哪类事件 */
	__set_bit(EV_KEY, hs0038_input_dev->evbit);
	__set_bit(EV_REP, hs0038_input_dev->evbit);
	
	/* B.2 能产生哪些事件 */
	//__set_bit(KEY_0, hs0038_input_dev->keybit);
	memset(hs0038_input_dev->keybit, 0xff, sizeof(hs0038_input_dev->keybit));
	
	/* C. 注册input_dev */
	err = input_register_device(hs0038_input_dev);

	return err;
}

static int hs0038_remove(struct platform_device *pdev)
{
	//删除设备信息
	input_unregister_device(hs0038_input_dev);
	input_free_device(hs0038_input_dev);

	device_destroy(hs0038_class, MKDEV(major, 0));
	free_irq(irq, NULL);
	gpiod_put(hs0038_data);
	return 0;
}


//驱动程序注册结构体
static struct platform_driver hs0038_driver = {
	.probe		=hs0038_probe,
	.remove		=hs0038_remove,
	.driver		={
		.name	="hs0038_xf",
		.of_match_table = hs0038s,
	},
};

static int __init hs0038_init(void)
{
	int err;

	//打印运行到这里的代码段行数，用于调试
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	//注册设备
	major = register_chrdev(0, "xf_hs0038", &hs0038_ops);

	//创建类，自动分配设备结点信息
	hs0038_class = class_create(THIS_MODULE, "xf_hs0038_class");
	if (IS_ERR(hs0038_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "xf_hs0038");
		return PTR_ERR(hs0038_class);
	}

	init_waitqueue_head(&hs0038_wq);

	//注册驱动结构体
	err = platform_driver_register(&hs0038_driver);

	return err;
}

static void __exit hs0038_exit(void)
{
	//打印运行到这里的代码段行数，用于调试
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	//卸载驱动结构体
	platform_driver_unregister(&hs0038_driver);
	class_destroy(hs0038_class);
	unregister_chrdev(major, "xf_hs0038");
	
}


module_init(hs0038_init);
module_exit(hs0038_exit);

MODULE_LICENSE("GPL");



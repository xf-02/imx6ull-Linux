#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/mod_devicetable.h>
#include <linux/log2.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <linux/of.h>
#include <linux/acpi.h>
#include <linux/i2c.h>
#include <asm/uaccess.h>


#define IOC_AT24C02_READ  100
#define IOC_AT24C02_WRITE 101

static int major=0;	//主设备号
static struct class *at24c02_class;	//类
static struct i2c_client *at24c02_client;

static long at24c02_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned char addr;
	unsigned char size;
	unsigned char ker_buf[100];
	unsigned char *usr_buf = (unsigned char *)arg;
	int err;
	
	struct i2c_msg msgs[2];

	err = copy_from_user(ker_buf, usr_buf, 2);	//第一次获取长度
	err = copy_from_user(ker_buf, usr_buf, ker_buf[1] + 2);	//第二次获取整个数据包

	addr = ker_buf[0];
	
	switch (cmd)
	{
		case IOC_AT24C02_READ:
		{
			/* 读AT24C02 */
			msgs[0].addr  = at24c02_client->addr;
			msgs[0].flags = 0; /* 写 */
			msgs[0].len   = 1;
			msgs[0].buf   = &addr;

			msgs[1].addr  = at24c02_client->addr;
			msgs[1].flags = I2C_M_RD; /* 读 */
			msgs[1].len   = ker_buf[1];
			msgs[1].buf   = &ker_buf[2];
		
			i2c_transfer(at24c02_client->adapter, msgs, 2);

			// ker_buf[1] = data;
			err = copy_to_user(usr_buf, ker_buf, 2 + ker_buf[1]);
			
			break;
		}
		case IOC_AT24C02_WRITE:
		{
			/* 写AT24C02 */
			
			// msgs[0].addr  = at24c02_client->addr;
			// msgs[0].flags = 0; /* 写 */
			// msgs[0].len   = 1;
			// msgs[0].buf   = &addr;
			
			size = ker_buf[1];
			ker_buf[1] = addr;
			msgs[0].addr  = at24c02_client->addr;
			msgs[0].flags = 0; /* 写 */
			msgs[0].len   = size + 1;
			msgs[0].buf   = &ker_buf[1];

			i2c_transfer(at24c02_client->adapter, msgs, 1);

			mdelay(20);
			
			break;
		}
	}

	return 0;
}


static ssize_t at24c02_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	return 0;
}
 
static unsigned int at24c02_poll(struct file *file, struct poll_table_struct *poll)
{
	return 0;
}

static int at24c02_open(struct inode *inode, struct file *file)
{
	return 0;
}


static struct file_operations at24c02_ops = {
	.owner	=THIS_MODULE,
	.open 	=at24c02_open,
	.read 	=at24c02_read,
	.poll 	=at24c02_poll,
	.unlocked_ioctl = at24c02_ioctl,
};

//匹配回调函数
static int at24c02_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	at24c02_client = client;

	//注册设备
	major = register_chrdev(0, "xf_at24c02", &at24c02_ops);

	//创建类，自动分配设备结点信息
	at24c02_class = class_create(THIS_MODULE, "xf_at24c02_class");
	if (IS_ERR(at24c02_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "xf_at24c02");
		return PTR_ERR(at24c02_class);
	}
	
	device_create(at24c02_class, NULL, MKDEV(major, 0), NULL, "xf_at24c02"); 

	return 0;
}

static int at24c02_remove(struct i2c_client *client)
{
	//删除设备信息
	//卸载驱动结构体
	unregister_chrdev(major, "xf_at24c02");
	device_destroy(at24c02_class, MKDEV(major, 0));
	class_destroy(at24c02_class);
	
	return 0;
}


static const struct of_device_id at24c02_of_match[] = {
	{.compatible = "xf,at24c02"},
	{}
};

static const struct i2c_device_id at24c02_ids[] = {
	{ "xxxxyyy",	(kernel_ulong_t)NULL },
	{ /* END OF LIST */ }
};


//驱动程序注册结构体
static struct i2c_driver at24c02_driver = {
	.driver = {
		.name = "xf_24c02",
		.of_match_table	 = at24c02_of_match,
	},
	.probe = at24c02_probe,
	.remove = at24c02_remove,
	.id_table = at24c02_ids,	//必须要加
};


static int __init at24c02_init(void)
{
	int err;

	//打印运行到这里的代码段行数，用于调试
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = i2c_add_driver(&at24c02_driver);
	return err;
}

static void __exit at24c02_exit(void)
{
	//打印运行到这里的代码段行数，用于调试
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	i2c_del_driver(&at24c02_driver);
}


module_init(at24c02_init);
module_exit(at24c02_exit);

MODULE_LICENSE("GPL");



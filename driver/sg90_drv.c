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
#include <linux/acpi.h>
#include <asm/uaccess.h>
#include <linux/pwm.h>
#include <linux/of.h>
#include <linux/of_platform.h>

static int major=0;	//主设备号
static struct class *sg90_class;	//类
static struct pwm_device *pwm_test; //pwm子系统，pwm设备

static ssize_t sg90_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
	int res;
	unsigned char data[1];
	if(size != 1)
		return -1;

	res = copy_from_user(data, buf, size);

    if( (data[0]>180) | (data[0]<0))   return -1;
	/* 配置PWM：旋转任意角度(单位1度) */
	pwm_config(pwm_test, 500000 + data[0] * 100000 / 9, 20000000);   
	pwm_enable(pwm_test);    /* 使能PWM输出 */
	return 1;
}

static int sg90_open(struct inode *inode, struct file *file)
{
	return 0;
}

static struct file_operations sg90_ops = {
	.owner	=THIS_MODULE,
	.open 	=sg90_open,
    .write  =sg90_write,
};

//匹配回调函数
static int sg90_probe(struct platform_device *pdev)
{
    struct device_node *node = pdev->dev.of_node;

    printk("sg90 match success \n");
    if (node){
        /* 从子节点中获取PWM设备 */
        pwm_test = devm_of_pwm_get(&pdev->dev, node, NULL);  
        if (IS_ERR(pwm_test)){
            printk(" pwm_test,get pwm  error!!\n");
            return -1;
        }
    }
    else{
        printk(" pwm_test of_get_next_child  error!!\n");
        return -1;
    }

    //pwm子系统
    pwm_config(pwm_test, 1500000, 20000000);   /* 配置PWM：1.5ms，90度，周期：20000000ns = 20ms */
    pwm_set_polarity(pwm_test, PWM_POLARITY_NORMAL); /* 设置输出极性：占空比为高电平 */
    pwm_enable(pwm_test);    /* 使能PWM输出 */

	device_create(sg90_class, NULL, MKDEV(major, 0), NULL, "xf_sg90"); 

	return 0;
}

static int sg90_remove(struct platform_device *pdev)
{
    pwm_config(pwm_test, 500000, 20000000);  /* 配置PWM：0.5ms，0度 */
	pwm_free(pwm_test);

	//删除设备信息
	device_destroy(sg90_class, MKDEV(major, 0));
	return 0;
}

//多个设备资源共用的驱动
static const struct of_device_id sg90s[] = {
	{.compatible = "xf,sg90"},
	{ },
};

//驱动程序注册结构体
static struct platform_driver sg90_driver = {
	.probe		=sg90_probe,
	.remove		=sg90_remove,
	.driver		={
		.name	="sg90_xf",
		.of_match_table = sg90s,
	},
};

static int __init sg90_init(void)
{
	int err;

	//打印运行到这里的代码段行数，用于调试
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	//注册设备
	major = register_chrdev(0, "xf_sg90", &sg90_ops);

	//创建类，自动分配设备结点信息
	sg90_class = class_create(THIS_MODULE, "xf_sg90_class");
	if (IS_ERR(sg90_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "xf_sg90");
		return PTR_ERR(sg90_class);
	}

	//注册驱动结构体
	err = platform_driver_register(&sg90_driver);

	return err;
}

static void __exit sg90_exit(void)
{
	//打印运行到这里的代码段行数，用于调试
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	//卸载驱动结构体
	platform_driver_unregister(&sg90_driver);
	class_destroy(sg90_class);
	unregister_chrdev(major, "xf_sg90");
}


module_init(sg90_init);
module_exit(sg90_exit);

MODULE_LICENSE("GPL");



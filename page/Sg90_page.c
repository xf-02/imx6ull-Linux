#include <page_manager.h>
#include <stdio.h>
#include <ui.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/input.h>

static lv_obj_t * sg90_page = NULL;
static int Sg90_fd;
static int Hs0038_fd;
pthread_t Read_tid;					//遥控器读取距离值线程的tid
uint8_t sg90_anglea_data = 90;

#define LABEL_DATA_CHILD_ID		1		//代表屏幕父对象的第3个子对象

static void Sg90PageCreate()
{
	sg90_page = lv_obj_create(NULL);
}

static void *Hs0038_Read_thread_func(void *data)
{
    struct input_event hs0038_data;
    uint8_t sg90_angle_buf;
    
    while(1)
	{
		if (read(Hs0038_fd, &hs0038_data, sizeof(hs0038_data)) == sizeof(hs0038_data))
		{
            if(hs0038_data.code == 0x08)
            {
                if(hs0038_data.value == 0x01)   //左边按下
                {
                    sg90_angle_buf = lv_slider_get_value(data);
                    if(sg90_angle_buf > 30) 
                    {
                        sg90_angle_buf -=30;
                        lv_slider_set_value(data, sg90_angle_buf, LV_ANIM_OFF);
                        lv_event_send(data, LV_EVENT_VALUE_CHANGED, NULL);
                    }
                }
            }

            if(hs0038_data.code == 0x5a)
            {
                if(hs0038_data.value == 0x01)   //右边按下
                {
                    sg90_angle_buf = lv_slider_get_value(data);
                    if(sg90_angle_buf < 150) 
                    {
                        sg90_angle_buf+=30;
                        lv_slider_set_value(data, sg90_angle_buf, LV_ANIM_OFF);
                        lv_event_send(data, LV_EVENT_VALUE_CHANGED, NULL);
                    }
                }
            }
		}
		else	//驱动程序执行错误，可能是没有接入模块
		{
            printf("get IR code: -1\n");
		}
		usleep(50000);	//50ms测量一次
	}
	return NULL;
}

static void my_btn_return_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
        int ret = pthread_cancel(Read_tid);
        if (ret != 0) {
            // 处理错误
            perror("线程销毁失败！");
            return ;
        }

        ret = pthread_join(Read_tid, NULL);		//等待读取线程销毁完毕释放空间
        if (ret != 0)
        {
            printf("[join thread: error]\n");
            return ;
        }

        close(Sg90_fd);
		Page("main")->Run(NULL);
	}
}

static void my_btn_value_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码
    lv_obj_t * obj_e = lv_event_get_target(e);        // 获取触发事件的部件(对象)
    lv_obj_t *Label_data2 = lv_obj_get_child(sg90_page, LABEL_DATA_CHILD_ID);	//获取到数值地址

	if(code_e == LV_EVENT_VALUE_CHANGED)
	{
        uint8_t sg90_value = lv_slider_get_value(obj_e);
        lv_label_set_text_fmt(Label_data2, "#519ABA 舵机当前角度：%d°#", sg90_value);
        int ret;
        ret = write(Sg90_fd, &sg90_value, 1);
        if(ret != 1) 
        {
            perror("写入设备失败");
            return ;
        }
	}
}

static void Sg90PageInit(void *User_data)
{ 
	//创建组
	lv_group_t * g = lv_group_create();
	lv_group_set_default(g);								//添加到默认组
	lv_indev_set_group(mouse_indev, g);	//输入设备添加进组

	//创建返回按钮		id_0
    lv_obj_t * btn_return = lv_btn_create(sg90_page);
    lv_obj_set_size(btn_return, 100, 100);
	lv_obj_align(btn_return, LV_ALIGN_TOP_LEFT, 20, 20);
	lv_obj_t * label_return = lv_label_create(btn_return);
	FT_font_Set(28);
	lv_obj_set_style_text_font(label_return, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_text(label_return, "返回");
	lv_obj_align(label_return, LV_ALIGN_CENTER, 0, -4);

	//距离数据显示文本	id_1
    lv_obj_t * Label_data2 = lv_label_create(sg90_page);
	lv_obj_set_style_text_font(Label_data2, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_recolor(Label_data2, 1);
	lv_label_set_text(Label_data2, "#519ABA 舵机当前角度：0°#");
    lv_obj_align(Label_data2, LV_ALIGN_CENTER, 0, 10);

    //舵机控制滑块	id_2
    lv_obj_t* slider1 = lv_slider_create(sg90_page);
    lv_obj_align_to(slider1, Label_data2, LV_ALIGN_CENTER, 0, 30);
    lv_slider_set_value(slider1, 0, LV_ANIM_OFF);
    lv_obj_add_flag(slider1, LV_OBJ_FLAG_ADV_HITTEST);
    lv_slider_set_range(slider1, 0, 180);

    //1.打开舵机设备
    Sg90_fd = open("/dev/xf_sg90", O_RDWR);
    if(Sg90_fd < 0)
    {
        perror("打开设备失败");
        return ;
    }

    int ret;
    ret = write(Sg90_fd, &sg90_anglea_data, 1);
    if(ret != 1) 
    {
        close(Sg90_fd);
        perror("写入设备失败");
        return ;
    }

    //2.打开红外遥控器设备
    Hs0038_fd = open("/dev/input/event4", O_RDWR);
    if(Hs0038_fd < 0)
    {
        perror("打开红外遥控器设备失败");
        return ;
    }

    /* 创建"遥控器数据读取线程" */
    ret = pthread_create(&Read_tid, NULL, Hs0038_Read_thread_func, slider1);
    if (ret)
    {
        perror("Sr04_Read_pthread_create err!");
        return ;
    }

    // ret = pthread_detach(Read_tid);		//进行线程分离，线程结束自动回收资源
    // if (ret != 0) {
    // 	// 处理错误
    // 	perror("线程分离失败！");
    // 	return ;
    // }


	lv_obj_add_event_cb(btn_return, my_btn_return_event_cb, LV_EVENT_CLICKED, NULL);	//注册鼠标松开按钮触发事件
    lv_obj_add_event_cb(slider1, my_btn_value_event_cb, LV_EVENT_VALUE_CHANGED, NULL);	//注册鼠标松开按钮触发事件
}

static void Sg90PageRun(void *pParams)
{
    lv_obj_clean(lv_scr_act());
	Sg90PageInit(NULL);
    lv_scr_load(sg90_page);
}

static PageAction Sg90_page = {
	.name = "sg90",
	.Create = Sg90PageCreate,
	.Run  = Sg90PageRun,
};

void Sg90_page_Registered(void)
{
	Registered_Page(&Sg90_page);
}
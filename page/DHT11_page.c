#include <page_manager.h>
#include <stdio.h>
#include <ui.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>

static lv_obj_t * dht11_page = NULL;
static int DHT11_fd;
pthread_t Read_tid;					//超声波读取距离值线程的tid
uint32_t distance_mm;

#define LABEL_STATE_CHILD_ID	5
#define DHT11_sw_CHILD_ID		4
#define LABEL_DATA_CHILD_ID		2
#define LABEL_DATA2_CHILD_ID	3

static void DHT11pageCreate()
{
	dht11_page = lv_obj_create(NULL);
}

static void my_btn_return_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码
	if(code_e == LV_EVENT_CLICKED)
	{
		lv_obj_t *DHT11_sw = lv_obj_get_child(dht11_page, DHT11_sw_CHILD_ID);	//获取到开关对象的地址
		if(lv_obj_has_state(DHT11_sw, LV_STATE_CHECKED))	//如果用户按下返回时仍然是打开摄像头的状态则清除摄像头数据
		{
			int ret = pthread_cancel(Read_tid);
			if (ret != 0) {
				// 处理错误
				perror("线程销毁失败！");
				return ;
			}
			close(DHT11_fd);	//关闭设备

			int s = pthread_join(Read_tid, NULL);		//等待读取线程销毁完毕释放空间
			if (s != 0)
			{
				printf("[join thread: error]\n");
				return ;
			}
		}

		Page("main")->Run(NULL);
	}
}

static void *DHT11_Read_thread_func(void *data)
{
	lv_obj_t * Label_data = lv_obj_get_child(dht11_page, LABEL_DATA_CHILD_ID);
	lv_obj_t * Label_data2 = lv_obj_get_child(dht11_page, LABEL_DATA2_CHILD_ID);

	unsigned char dht11_data[4]; 

	while(1)
	{
		if (read(DHT11_fd, dht11_data, 4) == 4)
		{
			printf("get humidity  : %d.%d\n", dht11_data[0], dht11_data[1]);
			printf("get temprature: %d.%d\n", dht11_data[2], dht11_data[3]);

			lv_label_set_text_fmt(Label_data, "#519ABA 当前室内温度：%d.%d℃#",dht11_data[2], dht11_data[3]);
			lv_label_set_text_fmt(Label_data2, "#519ABA 当前室内湿度：%d.%d%%#",dht11_data[0], dht11_data[1]);
		}
		else	//驱动程序执行错误，可能是没有接入模块或者距离太长
		{
			printf("get temprature: -1\n");  /* mm */
			// lv_label_set_text_fmt(Label_data, "#B20000 发生错误，请检查模块#");

		}
		usleep(100000);	//100ms测量一次
	}
	return 0;
}

static void my_Sw_DHT11_event_cb(lv_event_t * e)
{
	lv_obj_t * obj_e = lv_event_get_target(e);        // 获取触发事件的部件(对象)
	lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_VALUE_CHANGED)
	{
		lv_obj_t * Label_state = lv_obj_get_child(dht11_page, LABEL_STATE_CHILD_ID);		//获取到画面背景对象的地址

		if(lv_obj_has_state(obj_e, LV_STATE_CHECKED))	//状态“1” ，打开超声波测距模块
		{
			//1.打开超声波设备
			DHT11_fd = open("/dev/xf_dht11", O_RDWR);
			if(DHT11_fd < 0)
			{
				perror("打开设备失败");
				lv_label_set_text(Label_state, "#B20000 打开DHT11失败！请检查驱动程序!#");
				lv_obj_clear_state(obj_e, LV_STATE_CHECKED);		//设置为关状态
				return ;
			}

			/* 创建"超声波数据读取线程" */
			int ret = pthread_create(&Read_tid, NULL, DHT11_Read_thread_func, Label_state);
			if (ret)
			{
				perror("DHT11_Read_pthread_create err!");
				return ;
			}

			lv_label_set_text(Label_state, "#0EB16C 正在测温#");
		}
		else
		{
			int ret = pthread_cancel(Read_tid);
			if (ret != 0) {
				// 处理错误
				perror("线程销毁失败！");
				return ;
			}
			close(DHT11_fd);	//关闭设备

			int s = pthread_join(Read_tid, NULL);		//等待读取线程销毁完毕释放空间
			if (s != 0)
			{
				printf("[join thread: error]\n");
				return ;
			}

			lv_obj_t * Label_data = lv_obj_get_child(dht11_page, LABEL_DATA_CHILD_ID);		//获取到画面背景对象的地址
			lv_obj_t * Label_data2 = lv_obj_get_child(dht11_page, LABEL_DATA2_CHILD_ID);

			lv_label_set_text(Label_state, "#519ABA 停止测温#");
			lv_label_set_text(Label_data, "#519ABA 当前室内温度：NULL#");
			lv_label_set_text(Label_data2, "#519ABA 当前室内湿度：NULL#");
		}
	}
}

static void DHT11pageInit(void *User_data)
{ 
	//创建组
	lv_group_t * g = lv_group_create();
	lv_group_set_default(g);								//添加到默认组
	lv_indev_set_group(mouse_indev, g);	//输入设备添加进组

	//创建返回按钮		id_0
    lv_obj_t * btn_return = lv_btn_create(dht11_page);
    lv_obj_set_size(btn_return, 100, 100);
	lv_obj_align(btn_return, LV_ALIGN_TOP_LEFT, 20, 20);

	lv_obj_t * label_return = lv_label_create(btn_return);
	FT_font_Set(28);
	lv_obj_set_style_text_font(label_return, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_text(label_return, "返回");
	lv_obj_align(label_return, LV_ALIGN_CENTER, 0, -4);

	//创建测温画面显示的区域	id_1
	lv_obj_t * Frame_obj= lv_btn_create(dht11_page);
	lv_obj_set_size(Frame_obj, 372, 308);													//设置画面显示大小
	lv_obj_set_style_opa(Frame_obj, LV_OPA_60, LV_STATE_DEFAULT);							//设置画面背景透明度60%
	lv_obj_set_style_bg_color(Frame_obj, lv_color_hex(0xEDEDF5), LV_STATE_DEFAULT);			//设置画面背景颜色，偏灰
	lv_obj_set_style_outline_width(Frame_obj, 2, LV_STATE_DEFAULT);							//设置画面边框厚度
	lv_obj_set_style_outline_color(Frame_obj, lv_color_hex(0XBDBDBD), LV_STATE_DEFAULT);	//设置画面边框颜色，灰色
	lv_obj_set_style_outline_pad(Frame_obj, 2, LV_STATE_DEFAULT);							//设置画面边框内边距
	lv_obj_align(Frame_obj, LV_ALIGN_CENTER, 0, 0);											//居中对齐

	//距离数据显示文本	id_2、3
	lv_obj_t * Label_data = lv_label_create(dht11_page);
	lv_obj_set_style_text_font(Label_data, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_recolor(Label_data, 1);
	lv_label_set_text(Label_data, "#519ABA 当前室内温度：NULL#");
	lv_obj_align_to(Label_data, Frame_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_t * Label_data2 = lv_label_create(dht11_page);
	lv_obj_set_style_text_font(Label_data2, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_recolor(Label_data2, 1);
	lv_label_set_text(Label_data2, "#519ABA 当前室内湿度：NULL#");
	lv_obj_align_to(Label_data2, Label_data, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

	//创建开始DHT11的开关	id_4
	lv_obj_t * DHT11_sw = lv_switch_create(dht11_page);
	lv_obj_set_size(DHT11_sw, 100, 60);
	lv_obj_align_to(DHT11_sw, Frame_obj, LV_ALIGN_OUT_TOP_LEFT, 0, -10);

	//设备状态显示文本	id_5
	lv_obj_t * Label_state = lv_label_create(dht11_page);
	lv_obj_set_style_text_font(Label_state, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_recolor(Label_state, 1);
	lv_label_set_text(Label_state, "#519ABA 停止测温#");
	lv_obj_align_to(Label_state, DHT11_sw, LV_ALIGN_CENTER, 160, 0);


	lv_obj_add_event_cb(btn_return, my_btn_return_event_cb, LV_EVENT_CLICKED, NULL);	//注册鼠标松开按钮触发事件
	lv_obj_add_event_cb(DHT11_sw, my_Sw_DHT11_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

static void DHT11pageRun(void *pParams)
{
    lv_obj_clean(lv_scr_act());
	DHT11pageInit(NULL);
    lv_scr_load(dht11_page);
}

static PageAction DHT11_page = {
	.name = "DHT11",
	.Create = DHT11pageCreate,
	.Run  = DHT11pageRun,
};

void DHT11_page_Registered(void)
{
	Registered_Page(&DHT11_page);
}
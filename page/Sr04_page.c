#include <page_manager.h>
#include <stdio.h>
#include <ui.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>

static lv_obj_t * Sr04_page = NULL;
static int Sr04_fd;
pthread_t Read_tid;					//超声波读取距离值线程的tid
uint32_t distance_mm;

// #define FRAME_OBJ_CHILD_ID		1		//代表屏幕父对象的第2个子对象
#define LABEL_STATE_CHILD_ID	4		//代表屏幕父对象的第5个子对象
#define SR04_SW_CHILD_ID		3		//代表屏幕父对象的第4个子对象
#define LABEL_DATA_CHILD_ID		2		//代表屏幕父对象的第3个子对象
#define Bar_CHILD_ID			5		//代表屏幕父对象的第6个子对象

static void Sr04PageCreate()
{
	Sr04_page = lv_obj_create(NULL);
}

static void my_btn_return_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码
	if(code_e == LV_EVENT_CLICKED)
	{
		lv_obj_t *Sr04_sw = lv_obj_get_child(Sr04_page, SR04_SW_CHILD_ID);	//获取到开关对象的地址
		if(lv_obj_has_state(Sr04_sw, LV_STATE_CHECKED))	//如果用户按下返回时仍然是打开摄像头的状态则清除摄像头数据
		{
			int ret = pthread_cancel(Read_tid);
			if (ret != 0) {
				// 处理错误
				perror("线程销毁失败！");
				return ;
			}
			close(Sr04_fd);	//关闭设备

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

static void *Sr04_Read_thread_func(void *data)
{
	lv_obj_t * Label_data = lv_obj_get_child(Sr04_page, LABEL_DATA_CHILD_ID);		//获取到画面背景对象的地址
	lv_obj_t * Bar_Distance = lv_obj_get_child(Sr04_page, Bar_CHILD_ID);	//获取到开关对象的地址

	while(1)
	{
		if (read(Sr04_fd, &distance_mm, 4) == 4)
		{
			printf("get distance: %dmm\n", distance_mm);  /* mm */

			lv_label_set_text_fmt(Label_data, "#519ABA 距离：%dmm#",distance_mm);
			if(distance_mm<300) lv_bar_set_value(Bar_Distance, distance_mm/3, LV_ANIM_OFF);  //设置初始值
			else lv_bar_set_value(Bar_Distance, 100, LV_ANIM_OFF);  //设置初始值
		}
		else	//驱动程序执行错误，可能是没有接入模块或者距离太长
		{
			printf("get distance: -1mm\n");  /* mm */
			lv_label_set_text_fmt(Label_data, "#B20000 发生错误，请检查模块#");
			lv_bar_set_value(Bar_Distance, 0, LV_ANIM_OFF);  //设置初始值
		}
		usleep(50000);	//50ms测量一次
	}
	return 0;
}

static void my_Sw_Sr04_event_cb(lv_event_t * e)
{
	lv_obj_t * obj_e = lv_event_get_target(e);        // 获取触发事件的部件(对象)
	lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_VALUE_CHANGED)
	{
		lv_obj_t * Label_state = lv_obj_get_child(Sr04_page, LABEL_STATE_CHILD_ID);		//获取到画面背景对象的地址

		if(lv_obj_has_state(obj_e, LV_STATE_CHECKED))	//状态“1” ，打开超声波测距模块
		{
			//1.打开超声波设备
			Sr04_fd = open("/dev/xf_sr04", O_RDWR);
			if(Sr04_fd < 0)
			{
				perror("打开设备失败");
				lv_label_set_text(Label_state, "#B20000 打开超声波失败！请检查驱动程序!#");
				lv_obj_clear_state(obj_e, LV_STATE_CHECKED);		//设置为关状态
				return ;
			}

			/* 创建"超声波数据读取线程" */
			int ret = pthread_create(&Read_tid, NULL, Sr04_Read_thread_func, Label_state);
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

			lv_label_set_text(Label_state, "#0EB16C 正在测距#");
		}
		else
		{
			int ret = pthread_cancel(Read_tid);
			if (ret != 0) {
				// 处理错误
				perror("线程销毁失败！");
				return ;
			}
			close(Sr04_fd);	//关闭设备

			int s = pthread_join(Read_tid, NULL);		//等待读取线程销毁完毕释放空间
			if (s != 0)
			{
				printf("[join thread: error]\n");
				return ;
			}

			lv_obj_t * Label_data = lv_obj_get_child(Sr04_page, LABEL_DATA_CHILD_ID);		//获取到画面背景对象的地址
			lv_obj_t * Bar_Distance = lv_obj_get_child(Sr04_page, Bar_CHILD_ID);	//获取到开关对象的地址
			lv_bar_set_value(Bar_Distance, 0, LV_ANIM_OFF);  //设置初始值
			lv_label_set_text(Label_state, "#519ABA 停止测距#");
			lv_label_set_text(Label_data, "#519ABA 距离：NULL#");
		}
	}
}

static void Sr04PageInit(void *User_data)
{ 
	//创建组
	lv_group_t * g = lv_group_create();
	lv_group_set_default(g);								//添加到默认组
	lv_indev_set_group(mouse_indev, g);	//输入设备添加进组

	//创建返回按钮		id_0
    lv_obj_t * btn_return = lv_btn_create(Sr04_page);
    lv_obj_set_size(btn_return, 100, 100);
	lv_obj_align(btn_return, LV_ALIGN_TOP_LEFT, 20, 20);

	lv_obj_t * label_return = lv_label_create(btn_return);
	FT_font_Set(28);
	lv_obj_set_style_text_font(label_return, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_text(label_return, "返回");
	lv_obj_align(label_return, LV_ALIGN_CENTER, 0, -4);

	//创建测距画面显示的区域	id_1
	lv_obj_t * Frame_obj= lv_btn_create(Sr04_page);
	lv_obj_set_size(Frame_obj, 372, 308);													//设置画面显示大小
	lv_obj_set_style_opa(Frame_obj, LV_OPA_60, LV_STATE_DEFAULT);							//设置画面背景透明度60%
	lv_obj_set_style_bg_color(Frame_obj, lv_color_hex(0xEDEDF5), LV_STATE_DEFAULT);			//设置画面背景颜色，偏灰
	lv_obj_set_style_outline_width(Frame_obj, 2, LV_STATE_DEFAULT);							//设置画面边框厚度
	lv_obj_set_style_outline_color(Frame_obj, lv_color_hex(0XBDBDBD), LV_STATE_DEFAULT);	//设置画面边框颜色，灰色
	lv_obj_set_style_outline_pad(Frame_obj, 2, LV_STATE_DEFAULT);							//设置画面边框内边距
	lv_obj_align(Frame_obj, LV_ALIGN_CENTER, 0, 0);											//居中对齐

	//距离数据显示文本	id_2
	lv_obj_t * Label_data = lv_label_create(Sr04_page);
	lv_obj_set_style_text_font(Label_data, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_recolor(Label_data, 1);
	lv_label_set_text(Label_data, "#519ABA 距离：NULL#");
	lv_obj_align_to(Label_data, Frame_obj, LV_ALIGN_TOP_LEFT, 0, 0);


	//创建开始超声波测距的开关	id_3
	lv_obj_t * Sr04_sw = lv_switch_create(Sr04_page);
	lv_obj_set_size(Sr04_sw, 100, 60);
	lv_obj_align_to(Sr04_sw, Frame_obj, LV_ALIGN_OUT_TOP_LEFT, 0, -10);

	//设备状态显示文本	id_4
	lv_obj_t * Label_state = lv_label_create(Sr04_page);
	lv_obj_set_style_text_font(Label_state, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_recolor(Label_state, 1);
	lv_label_set_text(Label_state, "#519ABA 停止测距#");
	lv_obj_align_to(Label_state, Sr04_sw, LV_ALIGN_CENTER, 160, 0);

	//距离值显示的滑条	id_5
	lv_obj_t * Bar_Distance = lv_bar_create(Sr04_page);  				//创建bar对象
	lv_obj_set_size(Bar_Distance,300,20);                   			//设置尺寸
	lv_obj_align_to(Bar_Distance, Label_data, LV_ALIGN_CENTER, 90, 40);
	lv_bar_set_value(Bar_Distance,0,LV_ANIM_OFF);  //设置初始值


	lv_obj_add_event_cb(btn_return, my_btn_return_event_cb, LV_EVENT_CLICKED, NULL);	//注册鼠标松开按钮触发事件
	lv_obj_add_event_cb(Sr04_sw, my_Sw_Sr04_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

static void Sr04PageRun(void *pParams)
{
    lv_obj_clean(lv_scr_act());
	Sr04PageInit(NULL);
    lv_scr_load(Sr04_page);
}

static PageAction Sr04_Page = {
	.name = "Sr04",
	.Create = Sr04PageCreate,
	.Run  = Sr04PageRun,
};

void Sr04_Page_Registered(void)
{
	Registered_Page(&Sr04_Page);
}
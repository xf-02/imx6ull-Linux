#include <page_manager.h>
#include <stdio.h>
#include <ui.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>     // 目录操作相关的头文件

static lv_obj_t * album_Page = NULL;

uint16_t file_name_num_id;
uint16_t file_name_num_max;
uint16_t file_umg_cnt;

#define LABEL_DIR_CHILD_ID		2	//获取目录的标签地址，这里是对应主界面子对象的id
#define LABEL_IMG_CHILD_ID		3	//获取当前显示图像的标签地址

extern uint16_t list_files(const char *path);

static void AlbumPageCreate()
{
	album_Page = lv_obj_create(NULL);
}

static void my_btn_return_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码
	if(code_e == LV_EVENT_CLICKED)
	{
		Page("main")->Run(NULL);
	}
}

static void my_btn_up_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码
	lv_obj_t * Label_name = lv_obj_get_child(album_Page, LABEL_DIR_CHILD_ID);		//获取到画面背景对象的地址
	lv_obj_t * img = lv_obj_get_child(album_Page, LABEL_IMG_CHILD_ID);		//获取到画面背景对象的地址
	char file_name[50];

	if(code_e == LV_EVENT_CLICKED)
	{
		if(file_umg_cnt < (file_name_num_max-2))	//如果还有图片
		{
			sprintf(file_name, "/usr/share/lvgl_user/jpg/%03d.jpg", ++file_name_num_id);
			FILE *file = fopen(file_name, "r");
			while(1)		//开始循环遍历到第一个非空文件
			{
				if(file == NULL) 
				{
					sprintf(file_name, "/usr/share/lvgl_user/jpg/%03d.jpg", ++file_name_num_id);
					file = fopen(file_name, "r");
				}
				else	//遍历到非空文件，退出
				{
					fclose(file);
					break;
				}

				if(file_name_num_id >= (file_name_num_max-2) ) 	//文件命名格式不对
				{
					lv_label_set_text(Label_name, "#519ABA 图片文件命名格式不对#");
					return;
				}
			}

			file_umg_cnt++;
			sprintf(file_name, "S:/usr/share/lvgl_user/jpg/%03d.jpg", file_name_num_id);
			lv_label_set_text_fmt(Label_name, "#519ABA 当前查看的文件:/usr/share/lvgl_user/jpg/%03d.jpg#", file_name_num_id);
    		lv_img_set_src(img, file_name); 	// 设置图像数据源
		}
		else
		{
			file_umg_cnt = 1;
			file_name_num_id = 0;

			sprintf(file_name, "/usr/share/lvgl_user/jpg/%03d.jpg", file_name_num_id);
			FILE *file = fopen(file_name, "r");
			while(1)		//开始循环遍历到第一个非空文件
			{
				if(file == NULL) 
				{
					sprintf(file_name, "/usr/share/lvgl_user/jpg/%03d.jpg", --file_name_num_id);
					file = fopen(file_name, "r");
				}
				else	//遍历到非空文件，退出
				{
					fclose(file);
					break;
				}

				if(file_name_num_id < 0 ) 	//文件命名格式不对
				{
					lv_label_set_text(Label_name, "#519ABA 图片文件命名格式不对#");
					return;
				}
			}

			sprintf(file_name, "S:/usr/share/lvgl_user/jpg/%03d.jpg", file_name_num_id);
			lv_label_set_text_fmt(Label_name, "#519ABA 当前查看的文件:/usr/share/lvgl_user/jpg/%03d.jpg#", file_name_num_id);
			lv_img_set_src(img, file_name); 	// 设置图像数据源
		}
	}
}

static void my_btn_down_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码
	lv_obj_t * Label_name = lv_obj_get_child(album_Page, LABEL_DIR_CHILD_ID);		//获取到画面背景对象的地址
	lv_obj_t * img = lv_obj_get_child(album_Page, LABEL_IMG_CHILD_ID);		//获取到画面背景对象的地址
	char file_name[50];
	
	if(code_e == LV_EVENT_CLICKED)
	{
		if(file_umg_cnt > 1)	//如果还有图片
		{
			sprintf(file_name, "/usr/share/lvgl_user/jpg/%03d.jpg", --file_name_num_id);
			FILE *file = fopen(file_name, "r");
			while(1)		//开始循环遍历到第一个非空文件
			{
				if(file == NULL) 
				{
					sprintf(file_name, "/usr/share/lvgl_user/jpg/%03d.jpg", --file_name_num_id);
					file = fopen(file_name, "r");
				}
				else	//遍历到非空文件，退出
				{
					fclose(file);
					break;
				}

				if(file_name_num_id < 0 ) 	//文件命名格式不对
				{
					lv_label_set_text(Label_name, "#519ABA 图片文件命名格式不对#");
					return;
				}
			}

			file_umg_cnt--;
			sprintf(file_name, "S:/usr/share/lvgl_user/jpg/%03d.jpg", file_name_num_id);
			lv_label_set_text_fmt(Label_name, "#519ABA 当前查看的文件:/usr/share/lvgl_user/jpg/%03d.jpg#", file_name_num_id);
    		lv_img_set_src(img, file_name); 	// 设置图像数据源
		}
		else
		{
			file_umg_cnt = file_name_num_max;
			file_name_num_id = file_name_num_max;

			sprintf(file_name, "/usr/share/lvgl_user/jpg/%03d.jpg", file_name_num_id);
			FILE *file = fopen(file_name, "r");
			while(1)		//开始循环遍历到第一个非空文件
			{
				if(file == NULL) 
				{
					sprintf(file_name, "/usr/share/lvgl_user/jpg/%03d.jpg", --file_name_num_id);
					file = fopen(file_name, "r");
				}
				else	//遍历到非空文件，退出
				{
					fclose(file);
					break;
				}

				if(file_name_num_id < 0 ) 	//文件命名格式不对
				{
					lv_label_set_text(Label_name, "#519ABA 图片文件命名格式不对#");
					return;
				}
			}

			sprintf(file_name, "S:/usr/share/lvgl_user/jpg/%03d.jpg", file_name_num_id);
			lv_label_set_text_fmt(Label_name, "#519ABA 当前查看的文件:/usr/share/lvgl_user/jpg/%03d.jpg#", file_name_num_id);
			lv_img_set_src(img, file_name); 	// 设置图像数据源
		}
	}
}

static void AlbumPageInit(void *User_data)
{ 
	//创建组
	lv_group_t * g = lv_group_create();
	lv_group_set_default(g);								//添加到默认组
	lv_indev_set_group(mouse_indev, g);	//输入设备添加进组

	//创建返回按钮		id_0
    lv_obj_t * btn_return = lv_btn_create(album_Page);
    lv_obj_set_size(btn_return, 100, 100);
	lv_obj_align(btn_return, LV_ALIGN_TOP_LEFT, 20, 20);

	lv_obj_t * label_return = lv_label_create(btn_return);
	FT_font_Set(28);
	lv_obj_set_style_text_font(label_return, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_text(label_return, "返回");
	lv_obj_align(label_return, LV_ALIGN_CENTER, 0, -4);

	//创建相册图片显示的区域	id_1
	lv_obj_t * Frame_obj= lv_btn_create(album_Page);
	lv_obj_set_size(Frame_obj, 372, 308);													//设置画面显示大小
	lv_obj_set_style_opa(Frame_obj, LV_OPA_60, LV_STATE_DEFAULT);							//设置画面背景透明度60%
	lv_obj_set_style_bg_color(Frame_obj, lv_color_hex(0xEDEDF5), LV_STATE_DEFAULT);			//设置画面背景颜色，偏灰
	lv_obj_set_style_outline_width(Frame_obj, 2, LV_STATE_DEFAULT);							//设置画面边框厚度
	lv_obj_set_style_outline_color(Frame_obj, lv_color_hex(0XBDBDBD), LV_STATE_DEFAULT);	//设置画面边框颜色，灰色
	lv_obj_set_style_outline_pad(Frame_obj, 2, LV_STATE_DEFAULT);							//设置画面边框内边距
	lv_obj_align(Frame_obj, LV_ALIGN_CENTER, 0, 0);											//居中对齐

	//当前浏览的文件名称	id_2
	lv_obj_t * Label_name = lv_label_create(album_Page);
	lv_obj_set_style_text_font(Label_name, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_recolor(Label_name, 1);
	// lv_label_set_text(Label_name, "#519ABA 当前查看的文件:/usr/share/lvgl_user/jpg/001.jpg#");
	lv_obj_align_to(Label_name, Frame_obj, LV_ALIGN_TOP_LEFT, -100, -50);

	char file_name[50];
	file_name_num_max = list_files("/usr/share/lvgl_user/jpg/");
	if(file_name_num_max <0 ){
		perror("目录获取异常！");
		return ;
	}

	if(file_name_num_max == 2)
	{
		LV_LOG_USER("当前目录没有图片文件");
		lv_label_set_text(Label_name, "#519ABA 当前目录没有图片文件#");
	}
	else
	{
		file_name_num_id = 0;	//从图片000.jpg开始遍历
		file_umg_cnt = 0;
		sprintf(file_name, "/usr/share/lvgl_user/jpg/%03d.jpg", file_name_num_id);
		FILE *file = fopen(file_name, "r");
		while(1)		//开始循环遍历到第一个非空文件
		{
			if(file == NULL) 
			{
				sprintf(file_name, "/usr/share/lvgl_user/jpg/%03d.jpg", ++file_name_num_id);
				file = fopen(file_name, "r");
			}
			else	//遍历到非空文件，退出
			{
				fclose(file);
				break;
			}

			if(file_name_num_id >= (file_name_num_max-2) ) 	//文件命名格式不对
			{
				lv_label_set_text(Label_name, "#519ABA 图片文件命名格式不对#");
				return;
			}
		}
		file_umg_cnt++;
		lv_label_set_text_fmt(Label_name, "#519ABA 当前查看的文件:/usr/share/lvgl_user/jpg/%03d.jpg#", file_name_num_id);
	}

    //图片文件	id_3
    lv_obj_t *img= lv_img_create(album_Page); // 创建img对象
	sprintf(file_name, "S:/usr/share/lvgl_user/jpg/%03d.jpg", file_name_num_id);
    lv_img_set_src(img, file_name); 	// 设置图像数据源，文件类型可以是：1.jpg, 1.sjpg, 1.png, 1.bmp
    lv_obj_center(img);  // 居中显示

    //切换图片按钮	id_4、5
	lv_obj_t * Btn_Up = lv_btn_create(album_Page);
    lv_obj_set_size(Btn_Up, 120, 60);
	lv_obj_set_style_radius(Btn_Up, 15, LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_Up, Frame_obj, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
	lv_obj_t * Label_Up = lv_label_create(Btn_Up);
	lv_obj_set_style_text_font(Label_Up, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_text(Label_Up, "下一张");
	lv_obj_align(Label_Up, LV_ALIGN_CENTER, 0, -3);

    lv_obj_t * Btn_Down = lv_btn_create(album_Page);
    lv_obj_set_size(Btn_Down, 120, 60);
	lv_obj_set_style_radius(Btn_Down, 15, LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_Down, Frame_obj, LV_ALIGN_OUT_LEFT_MID, -20, 0);
	lv_obj_t * Label_Down = lv_label_create(Btn_Down);
	lv_obj_set_style_text_font(Label_Down, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_text(Label_Down, "上一张");
	lv_obj_align(Label_Down, LV_ALIGN_CENTER, 0, -3);

	

	lv_obj_add_event_cb(btn_return, my_btn_return_event_cb, LV_EVENT_CLICKED, NULL);	//注册鼠标松开按钮触发事件
	lv_obj_add_event_cb(Btn_Up, my_btn_up_event_cb, LV_EVENT_CLICKED, NULL);	//注册鼠标松开按钮触发事件
	lv_obj_add_event_cb(Btn_Down, my_btn_down_event_cb, LV_EVENT_CLICKED, NULL);	//注册鼠标松开按钮触发事件
	// lv_obj_add_event_cb(Album_sw, my_Sw_Album_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

static void AlbumPageRun(void *pParams)
{
    lv_obj_clean(lv_scr_act());
	AlbumPageInit(NULL);
    lv_scr_load(album_Page);
}

static PageAction Album_Page = {
	.name = "album",
	.Create = AlbumPageCreate,
	.Run  = AlbumPageRun,
};

void Album_Page_Registered(void)
{
	Registered_Page(&Album_Page);
}
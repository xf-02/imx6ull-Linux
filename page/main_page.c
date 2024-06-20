#include <page_manager.h>
#include <stdio.h>
#include <ui.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

static lv_obj_t * main_page = NULL;

LV_FONT_DECLARE(my_t02);
#define  FONTAWESOME_SYMBOL_image     		"\xef\x80\xbe"
#define  FONTAWESOME_SYMBOL_house     		"\xef\x80\x95"
#define  FONTAWESOME_SYMBOL_phone     		"\xef\x82\x95"
#define  FONTAWESOME_SYMBOL_envelope     	"\xef\x83\xa0"
#define  FONTAWESOME_SYMBOL_camera_retro    "\xef\x82\x83"
#define  FONTAWESOME_SYMBOL_comment     	"\xef\x81\xb5"
#define  FONTAWESOME_SYMBOL_film     		"\xef\x80\x88"
#define  FONTAWESOME_SYMBOL_wifi     		"\xef\x87\xab"
#define  FONTAWESOME_SYMBOL_music     		"\xef\x80\x81"
#define  FONTAWESOME_SYMBOL_twitter     	"\xef\x82\x99"

static void my_Sg90_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
		Page("sg90")->Run(NULL);
	}
}

static void my_Album_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
		Page("album")->Run(NULL);
	}
}


static void my_Sr04_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
		Page("Sr04")->Run(NULL);
	}
}

static void my_At24c02_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
		Page("At24c02")->Run(NULL);
	}
}

static void my_Camera_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
		Page("Camera")->Run(NULL);
		// LV_LOG_USER("hello");
	}
}

static void my_DHT11_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
		Page("DHT11")->Run(NULL);
		// LV_LOG_USER("hello");
	}
}

static void MainPageCreate()
{
	//创建主界面
	// main_page = lv_scr_act();
	main_page = lv_obj_create(NULL);
}

static void MainPageInit(void *User_data)
{
	LV_IMG_DECLARE(bg1);
	lv_obj_t * img = lv_img_create(main_page);
	lv_img_set_src(img, &bg1);

	//创建组
	lv_group_t * g = lv_group_create();
	lv_group_set_default(g);								//添加到默认组
	lv_indev_set_group(mouse_indev, g);	//输入设备添加进组

	static lv_style_t style;         	//创建样式
	lv_style_init(&style);           	//初始化样式
	lv_style_set_radius(&style,8);   	//设置样式的圆角
	lv_style_set_width(&style,80);  	//设置样式的宽度
	lv_style_set_height(&style, 80); 	//设置样式的高度
	lv_style_set_pad_all(&style, 3);
    // lv_style_set_align(&style, LV_ALIGN_CENTER);
	lv_style_set_bg_color(&style, lv_color_hex(0x009688));
	lv_style_set_text_color(&style, lv_color_hex(0xffffff));

	static lv_style_t style_image;         							//创建样式
	lv_style_init(&style_image);           							//初始化图标样式
	lv_style_set_text_font(&style_image, &my_t02);					//初始化图标样式字体
	lv_style_set_text_color(&style_image, lv_color_hex(0xffffff));	//初始化图标样式颜色，白色

	//设置freetype字体大小24
	FT_font_Set(24);

	//第一个APP
	lv_obj_t * Btn_album = lv_btn_create(main_page);
	lv_obj_align(Btn_album, LV_ALIGN_TOP_LEFT, 100, 120);
	lv_obj_add_style(Btn_album,&style,LV_STATE_DEFAULT);
	lv_obj_t * Label_album = lv_label_create(main_page);			// 使用新添加的图标（symbol）
    lv_label_set_text(Label_album, FONTAWESOME_SYMBOL_image);			//添加图标
	lv_obj_add_style(Label_album,&style_image,LV_STATE_DEFAULT);		//添加样式，白色图标以及字体设置
	lv_obj_align_to(Label_album, Btn_album, LV_ALIGN_CENTER, 0, 0);			//居中
	lv_obj_t * Label_album_str = lv_label_create(main_page);				//添加APP名称
	lv_obj_set_style_text_color(Label_album_str, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
	lv_label_set_recolor(Label_album_str, 1);
	lv_label_set_text_fmt(Label_album_str, "#ffffff 相册#");							//添加名字
	lv_obj_set_style_text_font(Label_album_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_album_str, Btn_album, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方


	//第二个APP
	lv_obj_t * Btn_Sr04 = lv_btn_create(main_page);
	lv_obj_add_style(Btn_Sr04,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_Sr04, Btn_album, LV_ALIGN_CENTER, 180, 0);
	lv_obj_t * Label_Sr04 = lv_label_create(main_page);
    lv_label_set_text(Label_Sr04, FONTAWESOME_SYMBOL_house);
	lv_obj_add_style(Label_Sr04,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(Label_Sr04, Btn_Sr04, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * Label_Sr04_str = lv_label_create(main_page);				//添加APP名称 
	lv_label_set_recolor(Label_Sr04_str, 1);
	lv_label_set_text(Label_Sr04_str, "#ffffff 超声波#");						//添加名字
	lv_obj_set_style_text_font(Label_Sr04_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_Sr04_str, Btn_Sr04, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	//第三个APP
	lv_obj_t * Btn_At24 = lv_btn_create(main_page);
	lv_obj_add_style(Btn_At24,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_At24, Btn_Sr04, LV_ALIGN_CENTER, 180, 0);
	lv_obj_t * Label_At24 = lv_label_create(main_page);
    lv_label_set_text(Label_At24, FONTAWESOME_SYMBOL_phone);
	lv_obj_add_style(Label_At24,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(Label_At24, Btn_At24, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * Label_AT24_str = lv_label_create(main_page);				//添加APP名称
	lv_label_set_recolor(Label_AT24_str, 1);
	lv_label_set_text(Label_AT24_str, "#ffffff AT24C02#");						//添加名字
	lv_obj_set_style_text_font(Label_AT24_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_AT24_str, Btn_At24, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	//第四个APP
	lv_obj_t * Btn_DHT11 = lv_btn_create(main_page);
	lv_obj_add_style(Btn_DHT11,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_DHT11, Btn_At24, LV_ALIGN_CENTER, 180, 0);
	lv_obj_t * Label_DHT11 = lv_label_create(main_page);
    lv_label_set_text(Label_DHT11, FONTAWESOME_SYMBOL_envelope);
	lv_obj_add_style(Label_DHT11,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(Label_DHT11, Btn_DHT11, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * Label_DHT11_str = lv_label_create(main_page);				//添加APP名称
	lv_label_set_recolor(Label_DHT11_str, 1);
	lv_label_set_text(Label_DHT11_str, "#ffffff DHT11温湿度#");						//添加名字
	lv_obj_set_style_text_font(Label_DHT11_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_DHT11_str, Btn_DHT11, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	//第五个APP
	lv_obj_t * Btn_Sg90 = lv_btn_create(main_page);
	lv_obj_add_style(Btn_Sg90,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_Sg90, Btn_DHT11, LV_ALIGN_CENTER, 180, 0);
	lv_obj_t * Label_Sg90 = lv_label_create(main_page);
    lv_label_set_text(Label_Sg90, FONTAWESOME_SYMBOL_film);
	lv_obj_add_style(Label_Sg90,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(Label_Sg90, Btn_Sg90, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * Label_Sg90_str = lv_label_create(main_page);				//添加APP名称
	lv_label_set_recolor(Label_Sg90_str, 1);
	lv_label_set_text(Label_Sg90_str, "#ffffff SG90舵机#");						//添加名字
	lv_obj_set_style_text_font(Label_Sg90_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_Sg90_str, Btn_Sg90, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	//第六个APP
	lv_obj_t * btn6 = lv_btn_create(main_page);
	lv_obj_add_style(btn6,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(btn6, Btn_Sr04, LV_ALIGN_CENTER, 0, 200);
	lv_obj_t * label6 = lv_label_create(main_page);
    lv_label_set_text(label6, FONTAWESOME_SYMBOL_comment);
	lv_obj_add_style(label6,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(label6, btn6, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * labelstr6 = lv_label_create(main_page);				//添加APP名称
	lv_label_set_recolor(labelstr6, 1);
	lv_label_set_text(labelstr6, "#ffffff comment#");						//添加名字
	lv_obj_align_to(labelstr6, btn6, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	//第七个APP
	lv_obj_t * Btn_Camera = lv_btn_create(main_page);
	lv_obj_add_style(Btn_Camera,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_Camera, btn6, LV_ALIGN_CENTER, 180, 0);
	lv_obj_t * Label_Camera = lv_label_create(main_page);
    lv_label_set_text(Label_Camera, FONTAWESOME_SYMBOL_camera_retro);
	lv_obj_add_style(Label_Camera,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(Label_Camera, Btn_Camera, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * Label_Camera_str = lv_label_create(main_page);				//添加APP名称
	lv_label_set_recolor(Label_Camera_str, 1);
	// FT_font_Set(24);
	lv_label_set_text(Label_Camera_str, "#ffffff 摄像头#");						//添加名字
	lv_obj_set_style_text_font(Label_Camera_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_Camera_str, Btn_Camera, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	//第八个APP
	lv_obj_t * btn8 = lv_btn_create(main_page);
	lv_obj_add_style(btn8,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(btn8, Btn_Camera, LV_ALIGN_CENTER, 180, 0);
	lv_obj_t * label8 = lv_label_create(main_page);
    lv_label_set_text(label8, FONTAWESOME_SYMBOL_twitter);
	lv_obj_add_style(label8,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(label8, btn8, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * labelstr8 = lv_label_create(main_page);				//添加APP名称
	lv_label_set_recolor(labelstr8, 1);
	lv_label_set_text(labelstr8, "#ffffff twitter");						//添加名字
	lv_obj_align_to(labelstr8, btn8, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	lv_obj_add_event_cb(Btn_album, my_Album_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(Btn_Sr04, my_Sr04_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(Btn_At24, my_At24c02_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(Btn_DHT11, my_DHT11_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(Btn_Sg90, my_Sg90_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(Btn_Camera, my_Camera_event_cb, LV_EVENT_CLICKED, NULL);
}

static void MainPageRun(void *pParams)
{
	lv_obj_clean(lv_scr_act());
	MainPageInit(NULL);
	lv_scr_load(main_page);
}

static PageAction Main_Page = {
	.name = "main",
	.Create = MainPageCreate,
	.Run  = MainPageRun,
};

void Main_Page_Registered(void)
{
	Registered_Page(&Main_Page);
}



#include <page_manager.h>
#include <stdio.h>
#include <ui.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>


static lv_obj_t * At24c02_page = NULL;
// static int At24c02_fd;
pthread_t tid;					//超声波读取距离值线程的tid
uint32_t distance_mm;
lv_obj_t *BtnM_indev;        //输入键盘
static char *btnMap[49];    //储存矩阵按钮的文本
uint32_t *addr_id[2];       //存储按下的按钮的对象指针和id指针
uint32_t id;                //存储按下的按钮的id
static int At24_fd;         //

// #define FRAME_OBJ_CHILD_ID		1		//代表屏幕父对象的第2个子对象
#define LABEL_STATE_CHILD_ID	4		//代表屏幕父对象的第5个子对象
#define At24c02_SW_CHILD_ID		3		//代表屏幕父对象的第4个子对象
#define LABEL_DATA_CHILD_ID		2		//代表屏幕父对象的第3个子对象
#define FRAM_CHILD_ID			1		//代表屏幕父对象的第6个子对象

#define Btn_ADDR_CHILD_ID		2		//代表屏幕父对象的第3个子对象

#define IOC_AT24C02_READ  100
#define IOC_AT24C02_WRITE 101

static void my_btn_indev_event_cb(lv_event_t * e);
static void my_btn_addr_event_cb(lv_event_t * e);

static void At24c02PageCreate()
{
	At24c02_page = lv_obj_create(NULL);
}

static void my_btn_return_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码
	if(code_e == LV_EVENT_CLICKED)
	{
		Page("main")->Run(NULL);
	}
}

static void my_btn_write_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码
    uint8_t usr_buf[100];
    
    lv_obj_t * Label_W = lv_label_create(At24c02_page);
    lv_obj_set_style_text_font(Label_W, ft_info.font, LV_STATE_DEFAULT);
    lv_label_set_recolor(Label_W, 1);
    lv_obj_align_to(Label_W, At24c02_page, LV_ALIGN_CENTER, 360, 40);
    
    if(code_e == LV_EVENT_CLICKED)
	{

        At24_fd = open("/dev/xf_at24c02", O_RDWR);
        if(At24_fd < 0)
		{
            LV_LOG_ERROR("打开设备失败");
            lv_obj_align_to(Label_W, At24c02_page, LV_ALIGN_CENTER, 300, 40);
            lv_label_set_text(Label_W, "#B20000 打开EEPROM失败！\n#B20000 请检查驱动程序！#");
            lv_obj_del_delayed(Label_W, 1500);
            // lv_obj_clear_state(obj_e, LV_STATE_CHECKED);		//设置为关状态
            return ;
		}

        //打开设备成功
        //开始写入字节
        char buf[10];
        for(uint8_t i=0; i<49; i++)
        {
            if(strlen(btnMap[i]) >6 )
            {
                if(strncmp(&btnMap[i][13], "NULL", 4) == 0) 
                {
                    usr_buf[2+i] = 0xff;
                }
                else 
                {
                    buf[2] = '\0';
                    buf[3] = '\0';
                    strncpy(buf, &btnMap[i][15], 3);
                    usr_buf[2+i] = atoi(buf);
                }
            }
            else
            {
                //碰到换行符
                usr_buf[2+i] = 0xff;
            }
        }

        for(int i=1; i<49; i++)
        {
            if( i % 8 == 0)
            {
                usr_buf[i-8] = 0 + i - 8;
                usr_buf[i-8+1] = 8;
                ioctl(At24_fd, IOC_AT24C02_WRITE, &usr_buf[i-8]);   //按8字节页写入数据
            }
        }

        close(At24_fd);	//关闭设备
        lv_label_set_text(Label_W, "#0EB16C 写入成功！#");
        lv_obj_del_delayed(Label_W, 1500);
    }
}

static void my_btn_read_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

    uint8_t usr_buf[100];

    lv_obj_t * Label_W = lv_label_create(At24c02_page);
    lv_obj_set_style_text_font(Label_W, ft_info.font, LV_STATE_DEFAULT);
    lv_label_set_recolor(Label_W, 1);
    lv_obj_align_to(Label_W, At24c02_page, LV_ALIGN_CENTER, 360, 40);

    if(code_e == LV_EVENT_CLICKED)
	{

        At24_fd = open("/dev/xf_at24c02", O_RDWR);
        if(At24_fd < 0)
		{
            LV_LOG_ERROR("打开设备失败");
            lv_obj_align_to(Label_W, At24c02_page, LV_ALIGN_CENTER, 300, 40);
            lv_label_set_text(Label_W, "#B20000 打开EEPROM失败！\n#B20000 请检查驱动程序！#");
            lv_obj_del_delayed(Label_W, 1500);
            return ;
		}

        //打开设备成功
        //开始读取字节
        usr_buf[0] = 0x00;
        usr_buf[1] = 49;
        ioctl(At24_fd, IOC_AT24C02_READ, usr_buf);      //整片读出数据
        close(At24_fd);	//关闭设备

        char p[20];
        for(uint8_t i=0, j=0; i<49 ;i++)
        {
            if(usr_buf[2+i] == 0xff) sprintf(p, "0x%02X\n#0078FF NULL", i-j);
            else sprintf(p, "0x%02X\n#0078FF   %d", i-j, usr_buf[2+i]);

            strcpy(btnMap[i], p);
            if((i+1)%7==0)  
            {
                if(i == 48) break;
                strcpy(btnMap[i], "\n");

                if(usr_buf[2+i+1] == 0xff) sprintf(p, "0x%02X\n#0078FF NULL", i);
                else sprintf(p, "0x%02X\n#0078FF   %d", i, usr_buf[2+i+1]);

                strcpy(btnMap[i+1], p);
                if(i == 6) i++;
                j++;
            }
        }
        strcpy(btnMap[48], "");
        lv_obj_t * Btn_addr = lv_obj_get_child(lv_obj_get_child(At24c02_page, FRAM_CHILD_ID), 0);		//获取到画面背景对象的地址
        lv_btnmatrix_set_map(Btn_addr, (const char **)btnMap);

        lv_label_set_text(Label_W, "#0EB16C 读取成功！#");
        lv_obj_del_delayed(Label_W, 1500);
    }

}




static void my_btn_indev_set_value(void *btn, int32_t value)
{
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 400-3*value);
}

static void my_btn_indev_del_set_value(void *btn, int32_t value)
{
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 100+3*value);
    if(value == 100)
    {
        lv_obj_t * Btn_addr = lv_obj_get_child(lv_obj_get_child(At24c02_page, FRAM_CHILD_ID), 0);		//获取到画面背景对象的地址
        lv_obj_add_event_cb(Btn_addr, my_btn_addr_event_cb, LV_EVENT_CLICKED, NULL);	//注册鼠标松开按钮触发事件
    }
}

static void my_btn_addr_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码
    lv_obj_t* obj = lv_event_get_target(e);

	if(code_e == LV_EVENT_CLICKED)
	{
        id = lv_btnmatrix_get_selected_btn(obj);

        //创建矩阵键盘输入画面
        BtnM_indev = lv_btnmatrix_create(At24c02_page);
        static const char* btnMap2[] = {
            "aaa","\n",
            "1", "2", "3", "Del","\n",
            "4", "5", "6", "Return","\n",
            "7", "8", "9", "OK","\n",
            "0", ""
        };
        lv_obj_set_size(BtnM_indev, 400, 380);
        lv_btnmatrix_set_map(BtnM_indev, (const char **)btnMap2);
        lv_btnmatrix_set_btn_ctrl(BtnM_indev, 0, LV_BTNMATRIX_CTRL_HIDDEN);
        lv_obj_align(BtnM_indev, LV_ALIGN_CENTER, 0, 100+300);

        lv_obj_t* Textare = lv_textarea_create(BtnM_indev);
        lv_obj_set_size(Textare, 350, 60);
        lv_obj_align(Textare, LV_ALIGN_TOP_MID, 0, 0);
        lv_textarea_set_max_length(Textare, 3);
        lv_textarea_set_one_line(Textare, true);
        lv_textarea_set_placeholder_text(Textare, "data:0-255");

        lv_anim_t anim;         //创建动画
        lv_anim_init(&anim);
        lv_anim_set_exec_cb(&anim, my_btn_indev_set_value);
        lv_anim_set_values(&anim, 0, 100);
        lv_anim_set_path_cb(&anim, lv_anim_path_linear);
        lv_anim_set_time(&anim, 300);
        lv_anim_set_var(&anim, BtnM_indev);
	    lv_anim_start(&anim);

        addr_id[0] = (uint32_t *)obj;
        addr_id[1] = &id;

        lv_obj_add_event_cb(BtnM_indev, my_btn_indev_event_cb, LV_EVENT_CLICKED, (void *)addr_id);
        lv_obj_t * Btn_addr = lv_obj_get_child(lv_obj_get_child(At24c02_page, FRAM_CHILD_ID), 0);		//获取到画面背景对象的地址
        lv_obj_remove_event_cb(Btn_addr, my_btn_addr_event_cb);
	}
}

static void my_btn_indev_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码
    lv_obj_t * obj = lv_event_get_target(e);

    uint32_t *addr_id; 
    addr_id = lv_event_get_user_data(e);   // 获取添加事件时传递的用户数据
    lv_obj_t * obj2 = (lv_obj_t *)(addr_id[0]);

    if(code_e == LV_EVENT_CLICKED)
	{
        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        const char* txt_indev = lv_btnmatrix_get_btn_text(obj, id);
        lv_obj_t* Textare = lv_obj_get_child(BtnM_indev, 0);

        if(strcmp(txt_indev, "Return") == 0)    //返回按键
        {
            lv_anim_t anim;         //创建动画
            lv_anim_init(&anim);
            lv_anim_set_exec_cb(&anim, my_btn_indev_del_set_value);
            lv_anim_set_values(&anim, 0, 100);
            lv_anim_set_path_cb(&anim, lv_anim_path_linear);
            lv_anim_set_time(&anim, 300);
            lv_anim_set_var(&anim, BtnM_indev);
            lv_anim_start(&anim);
            lv_obj_del_delayed(BtnM_indev, 320);
        }
        else if(strcmp(txt_indev, "aaa") == 0)
        {

        }
        else if(strcmp(txt_indev, "Del") == 0)
        {
            lv_textarea_del_char(Textare);
        }
        else if(strcmp(txt_indev, "OK") == 0)
        {
            const char *p = lv_textarea_get_text(Textare);
            int data;
            sscanf(p, "%d", (int *)&data);

            //检查输入是否合法
            if( (data > 0) && (data < 256))
            {
                //合法
                uint32_t id2 = *((uint32_t *)addr_id[1]);
                char* txt2 = (char*)lv_btnmatrix_get_btn_text(obj2, id2);
                txt2[4] = '\0';

                char p[20];
                lv_obj_t * Btn_addr = lv_obj_get_child(lv_obj_get_child(At24c02_page, FRAM_CHILD_ID), 0);		//获取到画面背景对象的地址
                sprintf(p, "%s\n#0078FF   %d", txt2, data);
                strcpy(btnMap[id2 + (id2/6)], p);
                lv_btnmatrix_set_map(Btn_addr, (const char **)btnMap);      //更新按下的按钮的值

                lv_anim_t anim;         //创建动画
                lv_anim_init(&anim);
                lv_anim_set_exec_cb(&anim, my_btn_indev_del_set_value);
                lv_anim_set_values(&anim, 0, 100);
                lv_anim_set_path_cb(&anim, lv_anim_path_linear);
                lv_anim_set_time(&anim, 300);
                lv_anim_set_var(&anim, BtnM_indev);
                lv_anim_start(&anim);
                lv_obj_del_delayed(BtnM_indev, 320);
            }
            else    //输入值超出一个字节大小
            {
                lv_obj_t * Label_write = lv_label_create(At24c02_page);
                lv_obj_set_style_text_font(Label_write, ft_info.font, LV_STATE_DEFAULT);
                lv_label_set_recolor(Label_write, 1);
                lv_obj_align_to(Label_write, Textare, LV_ALIGN_CENTER, -40, -80);
                lv_label_set_text(Label_write, "#B20000 非法输入！#");
                lv_obj_del_delayed(Label_write, 1500);

                LV_LOG_ERROR("非法输入！");
            }
        }
        else    //数字
        {
            lv_textarea_add_char(Textare, txt_indev[0]);
        }
    }
}

static void At24c02PageInit(void *User_data)
{ 
	//创建组
	lv_group_t * g = lv_group_create();
	lv_group_set_default(g);								//添加到默认组
	lv_indev_set_group(mouse_indev, g);	//输入设备添加进组

	//创建返回按钮		id_0
    lv_obj_t * btn_return = lv_btn_create(At24c02_page);
    lv_obj_set_size(btn_return, 100, 100);
	lv_obj_align(btn_return, LV_ALIGN_TOP_LEFT, 20, 20);

	lv_obj_t * label_return = lv_label_create(btn_return);
	FT_font_Set(28);
	lv_obj_set_style_text_font(label_return, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_text(label_return, "返回");
	lv_obj_align(label_return, LV_ALIGN_CENTER, 0, -4);

	//创建存储画面显示的区域	id_1
	lv_obj_t * Frame_obj= lv_btn_create(At24c02_page);
	lv_obj_set_size(Frame_obj, 640, 460);													//设置画面显示大小
	lv_obj_set_style_opa(Frame_obj, LV_OPA_60, LV_STATE_DEFAULT);							//设置画面背景透明度60%
	lv_obj_set_style_bg_color(Frame_obj, lv_color_hex(0xEDEDF5), LV_STATE_DEFAULT);			//设置画面背景颜色，偏灰
	lv_obj_set_style_outline_width(Frame_obj, 2, LV_STATE_DEFAULT);							//设置画面边框厚度
	lv_obj_set_style_outline_color(Frame_obj, lv_color_hex(0XBDBDBD), LV_STATE_DEFAULT);	//设置画面边框颜色，灰色
	lv_obj_set_style_outline_pad(Frame_obj, 2, LV_STATE_DEFAULT);							//设置画面边框内边距
	lv_obj_align(Frame_obj, LV_ALIGN_CENTER, 0, 0);											//居中对齐

    //创建存储画面	id_2
    lv_obj_t *BtnM_addr = lv_btnmatrix_create(Frame_obj);
    

    for(uint8_t i=0; i<49 ;i++)
    {
        btnMap[i] = malloc(sizeof(char) * 20);
        if(btnMap[i] == NULL)
        {
            printf("分配内存失败！\n");
            return ;
        }
    }
    char p[20];
    for(uint8_t i=0, j=0; i<49 ;i++)
    {
        sprintf(p, "0x%02X\n#0078FF NULL", i-j);
        strcpy(btnMap[i], p);
        if((i+1)%7==0)  
        {
            if(i == 48) break;
            strcpy(btnMap[i], "\n");
            sprintf(p, "0x%02X\n#0078FF NULL", i);
            strcpy(btnMap[i+1], p);
            if(i == 6) i++;
            j++;
        }
    }
    strcpy(btnMap[48], "");
    lv_obj_set_size(BtnM_addr, 620, 440);
    lv_obj_set_style_border_opa(BtnM_addr, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(BtnM_addr, LV_OPA_0, LV_STATE_DEFAULT);
    lv_btnmatrix_set_map(BtnM_addr, (const char **)btnMap);
    lv_btnmatrix_set_btn_ctrl_all(BtnM_addr, LV_BTNMATRIX_CTRL_RECOLOR);
    lv_obj_align(BtnM_addr, LV_ALIGN_CENTER, 0, 0);

    // LV_LOG_USER("%p", BtnM_addr);

    //写入按钮	id_3
	lv_obj_t * Btn_write = lv_btn_create(At24c02_page);
    lv_obj_set_size(Btn_write, 120, 60);
	lv_obj_set_style_radius(Btn_write, 15, LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_write, Frame_obj, LV_ALIGN_TOP_RIGHT, 160, 40);
	lv_obj_t * Label_write = lv_label_create(Btn_write);
	lv_obj_set_style_text_font(Label_write, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_text(Label_write, "一键写入");
	lv_obj_align(Label_write, LV_ALIGN_CENTER, 0, -3);

    //读取按钮	id_4
	lv_obj_t * Btn_read = lv_btn_create(At24c02_page);
    lv_obj_set_size(Btn_read, 120, 60);
	lv_obj_set_style_radius(Btn_read, 15, LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_read, Btn_write, LV_ALIGN_TOP_RIGHT, 20, 80);
	lv_obj_t * Label_read = lv_label_create(Btn_read);
	lv_obj_set_style_text_font(Label_read, ft_info.font, LV_STATE_DEFAULT);
	lv_label_set_text(Label_read, "一键读取");
	lv_obj_align(Label_read, LV_ALIGN_CENTER, 0, -3);

    //文本提示	id_5
    lv_obj_t * Label_Str = lv_label_create(At24c02_page);
    lv_obj_set_style_text_font(Label_Str, ft_info.font, LV_STATE_DEFAULT);
    lv_label_set_recolor(Label_Str, 1);
    lv_obj_align_to(Label_Str, Frame_obj, LV_ALIGN_OUT_TOP_MID, -40, -10);
    lv_label_set_text(Label_Str, "#0000FF EEPROM区域#");


	lv_obj_add_event_cb(btn_return, my_btn_return_event_cb, LV_EVENT_CLICKED, NULL);	//注册点击松开按钮触发事件
    lv_obj_add_event_cb(BtnM_addr, my_btn_addr_event_cb, LV_EVENT_CLICKED, NULL);       //注册点击松开按钮触发事件
    lv_obj_add_event_cb(Btn_write, my_btn_write_event_cb, LV_EVENT_CLICKED, NULL);       //注册点击松开按钮触发事件
    lv_obj_add_event_cb(Btn_read, my_btn_read_event_cb, LV_EVENT_CLICKED, NULL);       //注册点击松开按钮触发事件
}

static void At24c02PageRun(void *pParams)
{
    lv_obj_clean(lv_scr_act());
	At24c02PageInit(NULL);
    lv_scr_load(At24c02_page);
}

static PageAction At24c02_Page = {
	.name = "At24c02",
	.Create = At24c02PageCreate,
	.Run  = At24c02PageRun,
};

void At24_Page_Registered(void)
{
	Registered_Page(&At24c02_Page);
}
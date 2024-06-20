#include <ui.h>

#define DISP_BUF_SIZE (128 * 1024)
#define FREETYPE_FONT_FILE ("/usr/share/lvgl_user/simsun.ttc")

lv_indev_t *mouse_indev;
lv_ft_info_t ft_info;
lv_fs_drv_t drv;

static void* ffopen(lv_fs_drv_t* drv, const char* fn, lv_fs_mode_t mode)
{
    (void)drv; /*Unused*/

    const char* flags = "";

    if (mode == LV_FS_MODE_WR) flags = "w";
    else if (mode == LV_FS_MODE_RD) flags = "r";
    else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD)) flags = "r+";

    /*Make the path relative to the current directory (the projects root folder)*/
    // char buf[256];
    // sprintf(buf, "./%s", fn);
    // LV_LOG_USER("name:%s_ff", fn);
    return fopen(fn, flags);  
}

static lv_fs_res_t ffclose(lv_fs_drv_t* drv, void* file_p)
{
    (void)drv; /*Unused*/

    return fclose(file_p);
}

static lv_fs_res_t ffread(lv_fs_drv_t* drv, void* file_p, void* buf, uint32_t btr, uint32_t* br)
{
    (void)drv; /*Unused*/
    
    //pc_file_t* fp = file_p;        /*Just avoid the confusing casings*/
    // LV_LOG_USER("name:%s_ff", fn);
    *br = (uint32_t)fread(buf, 1, btr, file_p);
    return LV_FS_RES_OK;
}


static lv_fs_res_t ffwrite(struct _lv_fs_drv_t* drv, void* file_p, const void* buf, uint32_t btw, uint32_t* bw)
{
    (void)drv; /*Unused*/

    *bw = (uint32_t)fwrite(buf, 1, btw, file_p);

    return LV_FS_RES_OK;
}

static lv_fs_res_t ffseek(lv_fs_drv_t* drv, void* file_p, uint32_t pos,lv_fs_whence_t whence)
{
    (void)drv; /*Unused*/

    return fseek(file_p, pos, whence);
}

static lv_fs_res_t fftell(lv_fs_drv_t* drv, void* file_p, uint32_t* pos_p)
{
    (void)drv; /*Unused*/
    *pos_p = ftell(file_p);
    return LV_FS_RES_OK;
}

bool ffready(lv_fs_drv_t* drv)
{
    (void)drv; /*Unused*/
    return true;  //这里仅返回true,如果是嵌入式，则是返回嵌入式文件系统挂载成功与否的标志
}

int UI_Init(void)
{
    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    lv_freetype_init(8,64,64);

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = fbdev_flush;
    disp_drv.hor_res    = 1024;
    disp_drv.ver_res    = 600;
    lv_disp_drv_register(&disp_drv);

    evdev_init();
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv_1.read_cb = evdev_read;
    mouse_indev = lv_indev_drv_register(&indev_drv_1);

    /*FreeType uses C standard file system, so no driver letter is required.*/
    ft_info.name = FREETYPE_FONT_FILE;
    ft_info.weight = 24;
    ft_info.style = FT_FONT_STYLE_NORMAL;
    ft_info.mem = NULL;
    if(!lv_ft_font_init(&ft_info)) {
        printf("create failed.");
    }

    // static lv_fs_drv_t drv;                   /* 需要是静态的或全局的 */
    lv_fs_drv_init(&drv);                     /* 基本初始化 */
    drv.letter = 'S';                         /* 用一个大写字母来标识驱动器 */
    drv.cache_size = NULL;           /* 读取缓存大小（以字节为单位）。0 表示不进行缓存。*/
    drv.ready_cb = ffready;               /* 通知驱动器是否可以使用的回调函数 */
    drv.open_cb = ffopen;                 /* 打开文件的回调函数 */
    drv.close_cb = ffclose;               /* 关闭文件的回调函数 */
    drv.read_cb = ffread;                 /* 读取文件的回调函数 */
    drv.write_cb = ffwrite;               /* 写入文件的回调函数 */
    drv.seek_cb = ffseek;                 /* 在文件中寻找（移动游标）的回调函数 */
    drv.tell_cb = fftell;                 /* 获取游标位置的回调函数 */
    drv.dir_open_cb = NULL;         /* 打开目录以读取其中内容的回调函数 */
    drv.dir_read_cb = NULL;         /* 读取目录内容的回调函数 */
    drv.dir_close_cb = NULL;       /* 关闭目录的回调函数 */
    drv.user_data = NULL;             /* 如有需要，可设置任意自定义数据 */

    lv_fs_drv_register(&drv);                 /* 最后注册驱动程序 */

    lv_split_jpeg_init();

	// /*Create style with the new font*/
	// static lv_style_t style22;
	// lv_style_init(&style22);
	// lv_style_set_text_font(&style22, info.font);


    // /*Set a cursor for the mouse*/
    // LV_IMG_DECLARE(mouse_cursor_icon)
    // lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
    // lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
    // lv_indev_set_cursor(mouse_indev, cursor_obj);             /*Connect the image  object to the driver*/


    // /*Create a Demo*/
    // lv_demo_widgets();

    return 0;
}

void FT_font_Set(uint8_t size)
{
    ft_info.weight = size;
    lv_ft_font_init(&ft_info);
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}

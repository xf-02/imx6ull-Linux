#ifndef _UI_H
#define _UI_H

#include "stdio.h"
#include "stdint.h"
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

extern lv_indev_t *mouse_indev;
extern lv_ft_info_t ft_info;

int UI_Init(void);
uint32_t custom_tick_get(void);
void FT_font_Set(uint8_t size);

#endif


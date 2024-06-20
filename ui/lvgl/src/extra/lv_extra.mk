CSRCS += $(shell find -L $(LVGL_DIR)/$(LVGL_DIR_NAME)/src/extra -name "*.c")

CSRCS +=$(LVGL_DIR)/$(LVGL_DIR_NAME)/src/extra/libs/freetype/lv_freetype.c
CSRCS +=$(LVGL_DIR)/$(LVGL_DIR_NAME)/src/extra/libs/sjpg/lv_sjpg.c
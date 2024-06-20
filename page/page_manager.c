#include <page_manager.h>
#include <string.h>

static PPageAction g_ptPages = NULL;

void Registered_Page(PPageAction ptPageAction)
{
	ptPageAction->Create();

	ptPageAction->ptNext = g_ptPages;
	g_ptPages = ptPageAction;

}

PPageAction Page(char *name)
{
	PPageAction ptTmp = g_ptPages;

	while (ptTmp)
	{
		if (strcmp(name, ptTmp->name) == 0)
			return ptTmp;
		ptTmp = ptTmp->ptNext;
	}

	return NULL;
}

void PageSystemInit(void)
{
	//LVGL图形库初始化
	UI_Init();

	extern void Main_Page_Registered(void);
	Main_Page_Registered();

	extern void Sr04_Page_Registered(void);
	Sr04_Page_Registered();

	extern void Camera_Page_Registered(void);
	Camera_Page_Registered();

	extern void At24_Page_Registered(void);
	At24_Page_Registered();

	extern void DHT11_page_Registered(void);
	DHT11_page_Registered();

	extern void Album_Page_Registered(void);
	Album_Page_Registered();

	extern void Sg90_page_Registered(void);
	Sg90_page_Registered();
}

void lv_PageInit(lv_obj_t * page)
{
	page = lv_obj_create(NULL);
}



#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

#include <ui.h>

typedef struct PageAction {
	char *name;
	void (*Run)(void *pParams);
	void (*Create)(void);
	struct PageAction *ptNext;
}PageAction, *PPageAction;

void Registered_Page(PPageAction ptPageAction);
void PageSystemInit(void);
PPageAction Page(char *name);
void lv_PageInit(lv_obj_t * page);

#endif





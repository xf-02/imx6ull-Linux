#include <pthread.h>
#include <page_manager.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <ui.h>

int main(int argc, char **argv)
{
    PageSystemInit();

    Page("main")->Run(NULL);

    while(1) {
        lv_timer_handler();
        usleep(5000);
        // sleep(10);
    }

    return 0;
}


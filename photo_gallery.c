#include <raylib.h>
#include <stdio.h>
#include "photo_db.h"

#define DEFAULT_SCREEN_WIDTH 800
#define DEFAULT_SCREEN_HEIGHT 600


static void init_raylib(void);


int main(void)
{
    FILES files = build_photo_db(".");
    init_raylib();

    for(int count = 0; count < files.file_count; count++)
    {
        printf("%s\n", files.files[count]);
    }

    return 0;
}

static void init_raylib(void)
{
    InitWindow(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, "Photo Gallery");

    int monitor = GetCurrentMonitor();
    int x = GetMonitorWidth(monitor);
    int y = GetMonitorHeight(monitor);
    int hz = GetMonitorRefreshRate(monitor);

    SetWindowSize(x, y);
    SetTargetFPS(hz);
}


#include <raylib.h>
#include <stdio.h>
#include <stdbool.h>

#include <transition_handler.h>

#include "photo_gallery.h"
#include "photo_db.h"

#define DEFAULT_SCREEN_WIDTH 800
#define DEFAULT_SCREEN_HEIGHT 600


Texture2D display_photo;
DISPLAY_TYPE display_type = DISPLAY_FIXED_RATIO;

static void init_raylib(void);
static bool run_loop(FILES *files);
static void set_random_photo(FILES *files);
static void next_photo(void);
static bool show_photo(void);


int main(int argc, char *argv[])
{
    char *initial_dir = (argc > 1) ? argv[1] : ".";
    FILES files = build_photo_db(initial_dir);

    init_raylib();

    for(int count = 0; count < files.file_count; count++)
    {
        printf("%s\n", files.files[count]);
    }

    while(run_loop(&files));
    return 0;
}

void render_photo(void)
{
    int monitor = GetCurrentMonitor();
    float dest_width = (float)GetMonitorWidth(monitor);
    float dest_height = (float)GetMonitorHeight(monitor);
    float dest_x = 0.0;
    float dest_y = 0.0;

    switch(display_type)
    {
        case DISPLAY_STRETCH:
            break;

        case DISPLAY_FIXED_RATIO:
        {
            float width_var = dest_width / (float)display_photo.width;
            float height_var = dest_height / (float)display_photo.height;

            if (width_var < height_var)
            {
                float full_height = dest_height;

                dest_height = display_photo.height * width_var;
                dest_y = (full_height - dest_height) / 2.0f;
            }
            else
            {
                float full_width = dest_width;

                dest_width = display_photo.width * height_var;
                dest_x = (full_width - dest_width) / 2.0f;
            }

            break;
        }
    }

    Rectangle source_rect = { 0.0f, 0.0f, (float)display_photo.width, (float)display_photo.height};
    Rectangle dest_rect = { dest_x, dest_y, dest_width, dest_height};

    ClearBackground(BLACK);
    DrawTexturePro(display_photo, source_rect, dest_rect, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
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

static bool run_loop(FILES *files)
{
    if (files->current_selection == NO_VALUE)
    {
        set_random_photo(files);
        next_photo();
    }

    if (is_transition_active())
    {
        run_transition();
        return true;
    }

    return show_photo();
}

static void set_random_photo(FILES *files)
{
    char *file = get_random_path_name(files);

    display_photo = LoadTexture(file);
}

static void next_photo(void)
{
    TRANSITION_TYPE transition_type = TRANSITION_SLIDE_LEFT;

    if (transition_type != TRANSITION_NONE)
    {
        set_transition_start_screen();
    }

    if (transition_type != TRANSITION_NONE)
    {
        set_transition_end_screen(render_photo);
        start_transition(transition_type);
    }
}

static bool show_photo(void)
{
    bool exitTitle = false;

    BeginDrawing();
        render_photo();
    EndDrawing();

    if (GetKeyPressed())
    {
        exitTitle = true;
    }

    return !exitTitle;
}


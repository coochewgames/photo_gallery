#include <raylib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <transition_handler.h>

#include "photo_gallery.h"
#include "photo_db.h"
#include "set_config.h"

#define DEFAULT_SCREEN_WIDTH 800
#define DEFAULT_SCREEN_HEIGHT 600


static DISPLAY_TYPE display_type = DISPLAY_FIXED_RATIO;
static int display_width = DEFAULT_SCREEN_WIDTH;
static int display_height = DEFAULT_SCREEN_HEIGHT;
static GET_TYPE get_type = GET_RANDOM_PHOTO;
static double display_time = 10.0;
static int num_attempts_to_find_valid_photo = 5;

static Texture2D display_photo;
static double last_display_time = 0.0f;
static char initial_dir[PATH_MAX_LEN];

static void init_raylib(void);
static bool run_loop(FILES *files);
static bool load_photo(FILES *files);
static void next_photo(void);
static bool show_photo(void);
static void render_photo(void);


int main(int argc, char *argv[])
{
    FILES files;

    read_config_file();

    if (strlen(initial_dir) == 0)
    {
        if (argc > 1)
        {
            set_initial_dir(argv[1]);
        }
        else
        {
            set_initial_dir(".");
        }
    }

    init_raylib();
    files = build_photo_db(initial_dir);

    if (files.file_count == 0)
    {
        printf("Unable to locate photos in %s\nExiting...\n", initial_dir);
        return 1;
    }

    while(run_loop(&files));
    return 0;
}

void set_display_type(DISPLAY_TYPE type)
{
    display_type = type;
}

void set_display_width(int width)
{
    display_width = width;
}

void set_display_height(int height)
{
    display_height = height;
}

void set_display_time(double time)
{
    display_time = time;
}

void set_gallery_transition_duration(double duration)
{
    set_transition_duration((float)duration);
}

void set_get_type(GET_TYPE type)
{
    get_type = type;
}

void set_initial_dir(char *dir)
{
    strncpy(initial_dir, dir, PATH_MAX_LEN);
}

void show_message(const char *message)
{
    BeginDrawing();
        ClearBackground(BLACK);
        DrawText(message, 0, 0, 18, GREEN); 
    EndDrawing();
}

static void init_raylib(void)
{
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(display_width, display_height, "Photo Gallery");
}

static bool run_loop(FILES *files)
{
    if (is_transition_active())
    {
        run_transition();
        return true;
    }

    if (files->current_selection == NO_VALUE || (GetTime() - last_display_time) > display_time)
    {
        UnloadTexture(display_photo);

        if (load_photo(files) == true)
        {
            next_photo();

            last_display_time = GetTime();
        }
        else
        {
            //  Unable to load photo
            return false;
        }
    }

    return show_photo();
}

static bool load_photo(FILES *files)
{
    int num_attempts = 0;
    bool photo_loaded = false;

    do
    {
        char *file;

        switch(get_type)
        {
            case GET_RANDOM_PHOTO:
                file = get_random_path_name(files);
                break;

            case GET_SEQUENTIAL_PHOTO:
            default:
                file = get_next_path_name(files);
                break;
        }

        display_photo = LoadTexture(file);

        if (display_photo.id > 0)
        {
            photo_loaded = true;
        }
        else
        {
            num_attempts++;
        }
    } while (photo_loaded == false && num_attempts < num_attempts_to_find_valid_photo);
    
    return photo_loaded;
}

static void next_photo(void)
{
    TRANSITION_TYPE transition_type = get_random_transition();

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

static void render_photo(void)
{
    float dest_width = display_width;
    float dest_height = display_height;
    float dest_x = 0.0;
    float dest_y = 0.0;

    switch(display_type)
    {
        case DISPLAY_NO_SCALE:
        case DISPLAY_NO_SCALE_CENTRE:
        case DISPLAY_NO_SCALE_CENTRE_X:
        case DISPLAY_NO_SCALE_CENTRE_Y:
            dest_width = (float)display_photo.width;
            dest_height = (float)display_photo.height;

            if (display_type == DISPLAY_NO_SCALE_CENTRE ||
                display_type == DISPLAY_NO_SCALE_CENTRE_X)
            {
                dest_x = (display_width - dest_width) / 2.0f;
            }

            if (display_type == DISPLAY_NO_SCALE_CENTRE ||
                display_type == DISPLAY_NO_SCALE_CENTRE_Y)
            {
                dest_y = (display_height - dest_height) / 2.0f;
            }

            break;

        case DISPLAY_STRETCH:
            break;

        case DISPLAY_FIXED_RATIO:
        default:
        {
            float width_var = dest_width / (float)display_photo.width;
            float height_var = dest_height / (float)display_photo.height;

            if (width_var < height_var)
            {
                dest_height = display_photo.height * width_var;
                dest_y = (display_height - dest_height) / 2.0f;
            }
            else
            {
                dest_width = display_photo.width * height_var;
                dest_x = (display_width - dest_width) / 2.0f;
            }

            break;
        }
    }

    Rectangle source_rect = { 0.0f, 0.0f, (float)display_photo.width, (float)display_photo.height};
    Rectangle dest_rect = { dest_x, dest_y, dest_width, dest_height};

    ClearBackground(BLACK);
    DrawTexturePro(display_photo, source_rect, dest_rect, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
}

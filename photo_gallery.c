#include <raylib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include <transition_handler.h>

#include "photo_gallery.h"
#include "photo_db.h"
#include "set_config.h"

#define DEFAULT_SCREEN_WIDTH 800
#define DEFAULT_SCREEN_HEIGHT 600
#define DEFAULT_DISPLAY_TIME 10


static DISPLAY_TYPE display_type = DISPLAY_FIXED_RATIO;
static int display_width = DEFAULT_SCREEN_WIDTH;
static int display_height = DEFAULT_SCREEN_HEIGHT;
static GET_TYPE get_type = GET_RANDOM_PHOTO;
static double display_time = DEFAULT_DISPLAY_TIME;
static int num_attempts_to_find_valid_photo = 5;

static bool is_loading_image = false;
static bool is_next_image_loaded = false;
static Image scaled_image;

static Texture2D display_photo;
static double last_display_time = 0.0f;
static char initial_dir[PATH_MAX_LEN];

static void init_raylib(void);
static bool run_loop(FILES *files);
static void *load_image(void *pfiles);
static void next_photo(void);
static bool show_photo(void);
static void render_photo(void);
static void scale_image(Image *image);


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
        TraceLog(LOG_ERROR, "Unable to locate photos in %s\nExiting...", initial_dir);
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
    SetTraceLogLevel(LOG_INFO);
    InitWindow(display_width, display_height, "Photo Gallery");
}

static bool run_loop(FILES *files)
{
    static pthread_t thread_id;
    static bool initial_run = true;

    if (is_transition_active())
    {
        run_transition();
        return true;
    }

    if (is_next_image_loaded == false)
    {
        if (is_loading_image == false)
        {
            pthread_create(&thread_id, NULL, load_image, files);
            is_loading_image = true;
        }
    }

    if (initial_run || (GetTime() - last_display_time) > display_time)
    {
        initial_run = false;

        if (is_loading_image == true)
        {
            TraceLog(LOG_INFO, "Awaiting image loading...");
            pthread_join(thread_id, NULL);
        }

        UnloadTexture(display_photo);
        display_photo = LoadTextureFromImage(scaled_image);

        next_photo();

        is_next_image_loaded = false;
        last_display_time = GetTime();

        return true;
    }

    return show_photo();
}

static void *load_image(void *pfiles)
{
    FILES *files = (FILES *)pfiles;
    int num_attempts = 0;
    bool image_loaded = false;
    Image image = { NULL };

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

        //  A texture can be limited by the GPU, so load in as an image and scale prior to loading
        //  into the GPU as a texture.
        image = LoadImage(file);

        if (image.data != NULL)
        {
            double start = GetTime();
            TraceLog(LOG_DEBUG, "Starting resize...");
            scale_image(&image);
            TraceLog(LOG_DEBUG, "Ending resize (%lf secs)", (GetTime() - start));

            scaled_image = image;
            image_loaded = true;
        }
        else
        {
            num_attempts++;
        }
    } while (image_loaded == false && num_attempts < num_attempts_to_find_valid_photo);

    is_next_image_loaded = true;
    is_loading_image = false;

    pthread_exit(NULL);
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
    float dest_x = 0.0;
    float dest_y = 0.0;

    switch(display_type)
    {
        case DISPLAY_NO_SCALE:
        case DISPLAY_NO_SCALE_CENTRE:
        case DISPLAY_NO_SCALE_CENTRE_X:
        case DISPLAY_NO_SCALE_CENTRE_Y:
            if (display_type == DISPLAY_NO_SCALE_CENTRE ||
                display_type == DISPLAY_NO_SCALE_CENTRE_X)
            {
                dest_x = (display_width - display_width) / 2.0f;
            }

            if (display_type == DISPLAY_NO_SCALE_CENTRE ||
                display_type == DISPLAY_NO_SCALE_CENTRE_Y)
            {
                dest_y = (display_height - display_height) / 2.0f;
            }

            break;

        case DISPLAY_STRETCH:
            break;

        case DISPLAY_FIXED_RATIO:
        default:
        {
            if (display_photo.width < display_photo.height)
            {
                dest_y = (display_height - display_photo.height) / 2.0f;
            }
            else
            {
                dest_x = (display_width - display_photo.width) / 2.0f;
            }

            break;
        }
    }

    ClearBackground(BLACK);
    DrawTexture(display_photo, dest_x, dest_y, WHITE);
}

static void scale_image(Image *image)
{
    float dest_width = display_width;
    float dest_height = display_height;
    bool resize_required = true;

    switch(display_type)
    {
        case DISPLAY_NO_SCALE:
        case DISPLAY_NO_SCALE_CENTRE:
        case DISPLAY_NO_SCALE_CENTRE_X:
        case DISPLAY_NO_SCALE_CENTRE_Y:
            resize_required = false;
            break;

        case DISPLAY_STRETCH:
            break;

        case DISPLAY_FIXED_RATIO:
        default:
        {
            float width_var = display_width / (float)image->width;
            float height_var = display_height / (float)image->height;

            if (width_var < height_var)
            {
                dest_height = image->height * width_var;
            }
            else
            {
                dest_width = image->width * height_var;
            }

            break;
        }
    }

    if (resize_required == true)
    {
        ImageResize(image, dest_width, dest_height);
    }
}

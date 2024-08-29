#include <raylib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>

#include <transition_handler.h>

#include "photo_gallery.h"
#include "photo_db.h"
#include "set_config.h"
#include "exif.h"

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
static FILES build_db(const char *initial_dir);
static bool run_loop(FILES *files);
static void *get_image(void *pfiles);
static void next_photo(void);
static bool show_photo(void);
static void render_photo(void);
static void scale_image(Image *image);

static double get_delta_time(double start_time);
static double get_current_time(void);

static Image load_image(const char *fileName);



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

    if (argc > 1)
    {
        for(int i = 0; argv[1][i] != '\0'; i++)
        {
            argv[1][i] = tolower(argv[1][i]);
        }

        if (strcmp(argv[1], "build") == 0)
        {
            show_message("Building photos db file...");
            (void)build_db(initial_dir);

            exit(0);
        }

        if (strcmp(argv[1], "db") == 0)
        {
            show_message("Loading files from db...");

            files = read_files_from_file();

            if (files.file_count == 0)
            {
                TraceLog(LOG_WARNING, "Unable to read in photos from db");
            }
            else
            {
                char message[256];

                sprintf(message, "%d photos found in db file\nPre-loading initial image...", files.file_count);
                TraceLog(LOG_INFO, message);
                show_message(message);
            }
        }
    }

    if (files.file_count == 0)
    {
        show_message("Locating photos for display...");
        files = build_db(initial_dir);
    }

    if (files.file_count > 0)
    {
        while(run_loop(&files));
    }

    exit(0);
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
        DrawText(message, 0, 0, 32, GREEN); 
    EndDrawing();
}

static void init_raylib(void)
{
    SetTraceLogLevel(LOG_INFO);
    InitWindow(display_width, display_height, "Photo Gallery");
}

static FILES build_db(const char *initial_dir)
{
    FILES files = build_photo_db(initial_dir);

    if (files.file_count == 0)
    {
        TraceLog(LOG_ERROR, "Unable to locate photos to build db in %s\nExiting...", initial_dir);
    }
    else
    {
        TraceLog(LOG_INFO, "%d photos found in %s", files.file_count, initial_dir);

        write_files_to_file(&files);
    }

    return files;
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
            pthread_create(&thread_id, NULL, get_image, files);
            is_loading_image = true;
        }
    }

    if (initial_run || get_delta_time(last_display_time) > display_time)
    {
        initial_run = false;

        if (is_loading_image == true)
        {
            TraceLog(LOG_INFO, "Awaiting image loading...");
            pthread_join(thread_id, NULL);
        }

        UnloadTexture(display_photo);
        display_photo = LoadTextureFromImage(scaled_image);
        UnloadImage(scaled_image);

        next_photo();

        is_next_image_loaded = false;
        last_display_time = get_current_time();

        return true;
    }

    return show_photo();
}

static void *get_image(void *pfiles)
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
        image = load_image(file);

        if (image.data != NULL)
        {
            double start = get_current_time();
            TraceLog(LOG_DEBUG, "Starting resize...");
            scale_image(&image);
            TraceLog(LOG_DEBUG, "Ending resize (%lf secs)", (get_delta_time(start)));

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
            if (display_photo.height < display_height)
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

static double get_delta_time(double start_time)
{
    return get_current_time() - start_time;
}

static double get_current_time(void)
{
    struct timeval start;

    gettimeofday(&start, NULL);
    return (double)start.tv_sec + ((double)start.tv_usec / 1000000.0);
}

static Image load_image(const char *file_name)
{
    Image image = { 0 };
    int rotation = 0;

    unsigned int file_size = 0;
    unsigned char *file_data = LoadFileData(file_name, &file_size);

    if (file_data != NULL)
    {
        rotation = get_exif_rotation(file_data, file_size);
        image = LoadImageFromMemory(GetFileExtension(file_name), file_data, file_size);
    }

    RL_FREE(file_data);

    if (rotation == 90)
    {
        ImageRotateCW(&image);
    }
    else if (rotation == 270)
    {
        ImageRotateCCW(&image);
    }

    return image;
}


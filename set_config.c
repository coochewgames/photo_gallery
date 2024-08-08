#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "set_config.h"
#include "photo_gallery.h"

#define MAX_LINE_LEN 256

static void set_config_display_type(char *type_value);
static void set_config_display_time(char *time_value);
static void set_config_get_type(char *type_value);
static void set_config_initial_dir(char *dir_value);


void read_config_file(void)
{
    FILE *config_file = fopen("photo_gallery.ini", "r");
    char line[MAX_LINE_LEN];

    if (config_file == NULL)
    {
        return;
    }

    while(fgets(line, MAX_LINE_LEN, config_file))
    {
        if (line[0] == '#' || line[0] == ' ' || line[0] == '\n')
        {
            continue;
        }

        char *key = strtok(line, "=");
        char *value = strtok(NULL, "\n");

        if (strcmp(key, "display_type") == 0)
        {
            set_config_display_type(value);
        }
        else if (strcmp(key, "display_time") == 0)
        {
            set_config_display_time(value);
        }
        else if (strcmp(key, "get_type") == 0)
        {
            set_config_get_type(value);
        }
        else if (strcmp(key, "initial_dir") == 0)
        {
            set_config_initial_dir(value);
        }
    }

    fclose(config_file);
}

static void set_config_display_type(char *type_value)
{
    if (strcmp(type_value, "NO_SCALE") == 0)
    {
        set_display_type(DISPLAY_NO_SCALE);
    }
    else if (strcmp(type_value, "NO_SCALE_CENTRE") == 0)
    {
        set_display_type(DISPLAY_NO_SCALE_CENTRE);
    }
    else if (strcmp(type_value, "NO_SCALE_CENTRE_X") == 0)
    {
        set_display_type(DISPLAY_NO_SCALE_CENTRE_X);
    }
    else if (strcmp(type_value, "NO_SCALE_CENTRE_Y") == 0)
    {
        set_display_type(DISPLAY_NO_SCALE_CENTRE_Y);
    }
    else if (strcmp(type_value, "STRETCH") == 0)
    {
        set_display_type(DISPLAY_STRETCH);
    }
    else if (strcmp(type_value, "FIXED_RATIO") == 0)
    {
        set_display_type(DISPLAY_FIXED_RATIO);
    }
}

static void set_config_display_time(char *time_value)
{
    double time = strtod(time_value, NULL);

    if (time > 0.0)
    {
        set_display_time(time);
    }
}

static void set_config_get_type(char *type_value)
{
    if (strcmp(type_value, "RANDOM") == 0)
    {
        set_get_type(GET_RANDOM_PHOTO);
    }
    else if (strcmp(type_value, "SEQUENTIAL") == 0)
    {
        set_get_type(GET_SEQUENTIAL_PHOTO);
    }
}

static void set_config_initial_dir(char *dir_value)
{
    if (strlen(dir_value) > 0)
    {
        set_initial_dir(dir_value);
    }
}

#ifndef PHOTO_GALLERY_H
#define PHOTO_GALLERY_H

typedef enum
{
    DISPLAY_NO_SCALE,
    DISPLAY_NO_SCALE_CENTRE,
    DISPLAY_NO_SCALE_CENTRE_X,
    DISPLAY_NO_SCALE_CENTRE_Y,
    DISPLAY_STRETCH,
    DISPLAY_FIXED_RATIO,
    DISPLAY_ALL
} DISPLAY_TYPE;

typedef enum
{
    GET_RANDOM_PHOTO,
    GET_SEQUENTIAL_PHOTO,
    GET_ALL
} GET_TYPE;

void set_display_type(DISPLAY_TYPE type);
void set_display_width(int width);
void set_display_height(int height);
void set_display_time(double time);
void set_gallery_transition_duration(double duration);
void set_get_type(GET_TYPE type);
void set_initial_dir(char *dir);
void show_message(const char *message);

#endif

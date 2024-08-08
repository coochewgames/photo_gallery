#ifndef PHOTO_DB_H
#define PHOTO_DB_H

#define PATH_MAX_LEN 256

typedef struct
{
    char **files;
    int file_count;
} FILES;


FILES build_photo_db(const char *dir_path);

#endif

#ifndef PHOTO_DB_H
#define PHOTO_DB_H

#define PATH_MAX_LEN 256
#define NO_VALUE -1

typedef struct
{
    char **files;
    int file_count;
    int current_selection;
} FILES;


FILES read_files_from_file();
void write_files_to_file(FILES *files);

FILES build_photo_db(const char *dir_path);

char *get_random_path_name(FILES *files);
char *get_next_path_name(FILES *files);

#endif

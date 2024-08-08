#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "photo_db.h"

static int file_limit = 0;
static int file_inc = 100;

static void search_photos(FILES *files, const char *dir_path);
static void add_path_name_to_files(FILES *files, const char *path_name);



FILES build_photo_db(const char *dir_path)
{
    FILES files = { NULL, 0 };

    search_photos(&files, dir_path);
    return(files);
}

static void search_photos(FILES *files, const char *dir_path)
{
    DIR *dir;
    struct dirent *dir_entry;
    struct stat path_stat;

    if ((dir = opendir(dir_path)) == NULL)
    {
        return;
    }

    while((dir_entry = readdir(dir)) != NULL)
    {
        char new_path_name[PATH_MAX_LEN];
        int name_len = snprintf(new_path_name, sizeof(new_path_name), "%s/%s", dir_path, dir_entry->d_name);

        //  Get platform indepent file details
        if (stat(new_path_name, &path_stat) == -1)
        {
            continue;
        }

        if (S_ISREG(path_stat.st_mode))
        {
            add_path_name_to_files(files, new_path_name);
        }
        else if (S_ISDIR(path_stat.st_mode))
        {
            //  Ignore ., .. and hidden directories
            if (strncmp(dir_entry->d_name, ".", 1) == 0)
            {
                continue;
            }

            search_photos(files, new_path_name);
        }
    }

    closedir(dir);
}

static void add_path_name_to_files(FILES *files, const char *path_name)
{
    if (files->file_count == file_limit)
    {
        if (file_limit == 0)
        {
            files->files = calloc(sizeof(char *), file_inc);
        }
        else
        {
            files->files = realloc(files->files, file_inc * sizeof(char *));
        }

        if (files->files == NULL)
        {
            // Error
            return;
        }

        file_limit += file_limit;
    }

    *(files->files + files->file_count) = (char *)malloc(strlen(path_name));
    memcpy(*(files->files + files->file_count), path_name, strlen(path_name));
    files->file_count++;
}

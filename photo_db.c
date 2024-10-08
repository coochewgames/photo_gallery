#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#include "photo_db.h"
#include "photo_gallery.h"

static int file_limit = 0;
static int file_inc = 100;
static const char *db_filename = "./photos.db";

static void search_photos(FILES *files, const char *dir_path);
static void add_path_name_to_files(FILES *files, const char *path_name);
static const char *get_filename_ext(const char *file_name);
static int is_ext_image(const char *ext);


FILES read_files_from_file()
{
    FILES files = { NULL, 0, NO_VALUE };
    FILE *db_file;
    char path[PATH_MAX_LEN];
    int error_number = 0;

    db_file = fopen(db_filename, "rt");

    while(fgets(path, PATH_MAX_LEN, db_file) != NULL)
    {
        if (strlen(path) > 0)
        {
            if (path[strlen(path) - 1] == '\n')
            {
                path[strlen(path) - 1] = '\0';
            }

            add_path_name_to_files(&files, path);
        }
    }

    if ((error_number = ferror(db_file)) != 0)
    {
        printf("Error returned from reading paths from data file\n");
    }

    fclose(db_file);
    return files;
}

void write_files_to_file(FILES *files)
{
    FILE *db_file;

    db_file = fopen(db_filename, "wt");

    for(int pos = 0; pos < files->file_count; pos++)
    {
        fprintf(db_file, "%s\n", files->files[pos]);
    }

    fclose(db_file);
}

FILES build_photo_db(const char *dir_path)
{
    FILES files = { NULL, 0, NO_VALUE };
    srand((unsigned int)time(NULL));

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
            if (is_ext_image(get_filename_ext(new_path_name)))
            {
                add_path_name_to_files(files, new_path_name);
            }
        }
        else if (S_ISDIR(path_stat.st_mode))
        {
            //  Ignore ., .. and hidden directories
            if (strncmp(dir_entry->d_name, ".", 1) == 0)
            {
                continue;
            }

            char message[128];
            snprintf(message, 128, "New directory found: %s", new_path_name);
            show_message(message);

            search_photos(files, new_path_name);
        }
    }

    closedir(dir);
}

char *get_random_path_name(FILES *files)
{
    int new_selection = rand() % files->file_count;

    if (new_selection == files->current_selection)
    {
        return get_next_path_name(files);
    }

    files->current_selection = new_selection;
    return files->files[files->current_selection];
}

char *get_next_path_name(FILES *files)
{
    if (++files->current_selection >= files->file_count)
    {
        files->current_selection = 0;
    }

    return files->files[files->current_selection];
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
            files->files = realloc(files->files, (file_limit + file_inc) * sizeof(char *));
        }

        if (files->files == NULL)
        {
            // Error
            return;
        }

        file_limit += file_inc;
    }

    *(files->files + files->file_count) = (char *)malloc(strlen(path_name) + 1);
    memset(*(files->files + files->file_count), 0x0, strlen(path_name) + 1);
    memcpy(*(files->files + files->file_count), path_name, strlen(path_name));
    files->file_count++;
}

static const char *get_filename_ext(const char *file_name)
{
    const char *dot = strrchr(file_name, '.');

    if (dot == NULL || dot == file_name)
    {
        return NULL;
    }

    return(dot + 1);
}

static int is_ext_image(const char *ext)
{
    char *lc_ext;
    int is_image = 0;


    if (ext == NULL)
    {
        return is_image;
    }

    lc_ext = malloc(strlen(ext) + 1);
    memset(lc_ext, 0, strlen(ext) + 1);

    for(int i = 0; ext[i] != '\0'; i++)
    {
        lc_ext[i] = tolower(ext[i]);
    }

    if (strcmp(lc_ext, "png") == 0 ||
        strcmp(lc_ext, "jpg") == 0 ||
        strcmp(lc_ext, "jpeg") == 0 ||
        strcmp(lc_ext, "bmp") == 0)
    {
        is_image = 1;
    }

    free(lc_ext);
    return is_image;
}

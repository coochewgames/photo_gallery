#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIF_TAG_ORIENTATION 0x0112

static const int BUFFER_LEN = 2;
static const int EXIF_HEADER_LEN = 6;

static unsigned short read_u16(const unsigned char* data, int big_endian);


int get_exif_rotation(const unsigned char *file_data, unsigned int file_size)
{
    unsigned int file_pos = 0;

    if (file_size < BUFFER_LEN)
    {
        fprintf(stderr, "Not a valid JPG file - less than 2 bytes long\n");
        return -1;
    }

    if (file_data[0] != 0xFF || file_data[1] != 0xD8)
    {
        fprintf(stderr, "Not a valid JPG file - markers not found\n");
        return -2;
    }

    file_pos += BUFFER_LEN;

    while((file_pos + BUFFER_LEN) < file_size)
    {
        unsigned char buffer[BUFFER_LEN];

        memcpy(buffer, (file_data + file_pos), BUFFER_LEN);
        file_pos += BUFFER_LEN;

        if (buffer[0] != 0xFF)
        {
            fprintf(stderr, "Invalid marker found in buffer at position: %u\n", file_pos - BUFFER_LEN);
            break;
        }

        if (buffer[1] == 0xE1)
        {
            memcpy(buffer, (file_data + file_pos), BUFFER_LEN);
            file_pos += BUFFER_LEN;

            unsigned short length = read_u16(buffer, 1);
            unsigned char *exif_data = (unsigned char*)malloc(length);
            int rotation = -1;

            if (!exif_data)
            {
                //  Memory allocation failed
                fprintf(stderr, "Failed to allocate memory for %u bytes:\n", length);
                return -4;
            }

            memcpy(exif_data, (file_data + file_pos), (length - BUFFER_LEN));
            file_pos += (length - BUFFER_LEN);

            if (memcmp(exif_data, "Exif\0\0", EXIF_HEADER_LEN) == 0)
            {
                int exif_pos = EXIF_HEADER_LEN;
                int big_endian = memcmp(exif_data + exif_pos, "MM", BUFFER_LEN) == 0;

                exif_pos += (BUFFER_LEN * 2);

                unsigned int offset = read_u16(exif_data + exif_pos, big_endian);
                unsigned int num_entries = read_u16(exif_data + EXIF_HEADER_LEN + offset, big_endian);

                for (unsigned int i = 0; i < num_entries; ++i)
                {
                    unsigned int entry_offset = EXIF_HEADER_LEN + offset + BUFFER_LEN + (i * 12);
                    unsigned short tag = read_u16(exif_data + entry_offset, big_endian);

                    if (tag == EXIF_TAG_ORIENTATION)
                    {
                        unsigned short value = read_u16(exif_data + entry_offset + 8, big_endian);
                    
                        switch (value)
                        {
                            case 3:
                                rotation = 180;
                                break;
                                
                            case 6:
                                rotation = 90;
                                break;
                                
                            case 8:
                                rotation = 270;
                                break;

                            case 1:
                            default:
                                rotation = 0;
                                break;
                        }
                    }
                }
            }

            free(exif_data);
            return rotation;
        }
        else
        {
            memcpy(buffer, (file_data + file_pos), BUFFER_LEN);
            file_pos += BUFFER_LEN;

            unsigned short length = read_u16(buffer, 1);
            file_pos += length - BUFFER_LEN;
        }
    }

    fprintf(stderr, "No EXIF orientation tag found\n");
    return -5;
}

static unsigned short read_u16(const unsigned char* data, int big_endian)
{
    return (big_endian) ? (data[0] << 8) | data[1] : (data[1] << 8) | data[0];
}

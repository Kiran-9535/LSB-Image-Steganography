#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>
#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    
    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX];        
    char secret_data[MAX_SECRET_BUF_SIZE];
    int size_secret_file;

    /* Stego Image Info */      //write mode
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output file info */
    char *output_fname;
    FILE *fptr_output;


    char d_magic_string[20];//to read magic string
    int d_magic_str_len;
    

} DecodeInfo;


/* Read and validate decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *deInfo);

/* Perform the encoding */
Status do_decoding(DecodeInfo *deInfo);

/* Get File pointers for i/p and o/p files */
Status de_open_files(DecodeInfo *deInfo);


/* Store Magic String */
Status decode_magic_string(DecodeInfo *deInfo, char *magic_string);
//Status decode_magic_string_len(int len, DecodeInfo *deInfo);

/* Encode secret file extenstion */
Status decode_secret_file_extn(DecodeInfo *deInfo, int extn_len);
Status decode_secret_file_extn_len(DecodeInfo *deInfo, int *extn_len);

Status decode_secret_file_size(DecodeInfo *deInfo);

/* Encode secret file data*/
Status decode_secret_file_data(DecodeInfo *deInfo);
/* Encode a byte into LSB of image data array */
Status decode_byte_from_lsb(char *image_buffer, char *data);
Status decode_size_from_lsb(char *image_buffer, int *size);
#endif
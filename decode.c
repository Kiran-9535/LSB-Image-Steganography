#include <stdio.h>
#include "decode.h"
#include <string.h>
#include "types.h"
Status read_and_validate_decode_args(char *argv[], DecodeInfo *deInfo)
{
   if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("ERROR: Please provide a .bmp file for decoding.\n");
        return d_failure;
    }
    deInfo->stego_image_fname = argv[2];    
    deInfo->output_fname = argv[3];
    return d_success;
}
Status de_open_files(DecodeInfo *deInfo)
{
    // Stego Image file
    deInfo->fptr_stego_image = fopen(deInfo->stego_image_fname, "r");
    // Do Error handling
    if (deInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", deInfo->stego_image_fname);

    	return d_failure;
    }

    return d_success;
}
Status do_decoding(DecodeInfo *deInfo)
{
   
    /* Step 1: Open the files */
    if (de_open_files(deInfo) == d_failure)
    {
        printf("ERROR: Opening required files failed\n");
        return d_failure;
    }
    printf("INFO: Files opened successfully\n");
    char ms[10];
    printf("Enter the magic string : ");
    scanf("%s",ms);
    if (decode_magic_string(deInfo, ms) == d_failure) {
        printf("ERROR: Magic string decoding failed\n");
        return d_failure;
    }
    printf("INFO: Magic string decoded successfully: %s\n", ms);
    int extn_len;
    decode_secret_file_extn_len(deInfo, &extn_len);
    decode_secret_file_extn(deInfo, extn_len);
    decode_secret_file_size(deInfo);
    decode_secret_file_data(deInfo);

    fclose(deInfo->fptr_stego_image);
    fclose(deInfo->fptr_output);
    return d_success;
}
Status decode_magic_string(DecodeInfo *deInfo, char *magic_string) 
{
    int len = strlen(magic_string);
    fseek(deInfo->fptr_stego_image, 86, SEEK_SET);
    char image_buffer[8];
    for (int i = 0; i < len; i++)
    {
        fread(image_buffer, 8, 1, deInfo->fptr_stego_image);
        decode_byte_from_lsb(image_buffer, &deInfo->d_magic_string[i]);
    }
    deInfo->d_magic_string[len] = '\0';
    if (strcmp(deInfo->d_magic_string,magic_string) != 0)
    {
        printf("ERROR: Magic string mismatch!\n");
        return d_failure;
    }
    printf("INFO: Magic string verified: %s\n", deInfo->d_magic_string);
    return d_success;
}
Status decode_secret_file_extn_len(DecodeInfo *deInfo, int *extn_len)
{
    char image_buffer[32];
    fread(image_buffer, 32, 1, deInfo->fptr_stego_image);
    decode_size_from_lsb(image_buffer, extn_len);
    return d_success;
}
Status decode_secret_file_extn(DecodeInfo *deInfo, int extn_len)
{
    char image_buffer[8];
    for (int i = 0; i < extn_len; i++)
    {
        fread(image_buffer, 8, 1, deInfo->fptr_stego_image);
        decode_byte_from_lsb(image_buffer, &deInfo->extn_secret_file[i]);
    }
    deInfo->extn_secret_file[extn_len] = '\0';
    printf("INFO: File extension decoded: %s\n", deInfo->extn_secret_file);
    strcat(deInfo->output_fname, deInfo->extn_secret_file);

    // Open the output file
    deInfo->fptr_output = fopen(deInfo->output_fname, "w");
    if (!deInfo->fptr_output)
    {
        perror("fopen");
        return d_failure;
    }
    printf("INFO: Output file created: %s\n", deInfo->output_fname);

    return d_success;
}
Status decode_secret_file_size(DecodeInfo *deInfo)
{
    char image_buffer[32];

    // Read 32 bytes = 32 LSBs → size of secret file
    fread(image_buffer, 32, 1, deInfo->fptr_stego_image);

    // Decode integer from LSB
    decode_size_from_lsb(image_buffer, &deInfo->size_secret_file);

    printf("INFO: Secret file size decoded: %d bytes\n", deInfo->size_secret_file);

    return d_success;
}
Status decode_secret_file_data(DecodeInfo *deInfo)
{
    char image_buffer[8];
    char ch;
    for (long i = 0; i < deInfo->size_secret_file; i++)
    {
        fread(image_buffer, 8, 1, deInfo->fptr_stego_image);
        decode_byte_from_lsb(image_buffer, &ch);
        fputc(ch, deInfo->fptr_output);
    }
    printf("INFO: Secret file data decoded and written successfully\n");
    return d_success;
}
Status decode_byte_from_lsb(char *image_buffer, char *data) 
{

    *data = 0;
    for (int i = 0; i < 8; i++) 
    {
        *data |= ((image_buffer[i] & 0x01) << (7 - i));
    }

    return d_success;
}
Status decode_size_from_lsb(char *image_buffer, int *size) 
{
   
    *size = 0;
    for (int i = 0; i < 32; i++) 
    {
        *size |= ((image_buffer[i] & 0x01) << (31 - i));
    }

    return d_success;
}





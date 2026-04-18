#include <stdio.h>
#include "encode.h"
#include <string.h>
#include "types.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */


uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}
/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

uint get_file_size(	FILE *fptr)
{
	fseek(fptr,0,SEEK_END);         
	int len = ftell(fptr);
	return len;
}

OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1], "-e") == 0)
    {
        
        return e_encode;
    }
    else if(strcmp(argv[1], "-d") == 0)
    {
        
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}


Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char arr[5];
    if(strstr(argv[2], ".bmp") != NULL)
    {
        printf("Source image file validated : %s\n", argv[2]);
        encInfo -> src_image_fname = argv[2];
    }
    else
    {
        printf("Source image file validated : beautiful.bmp is not prsent \n");
        return e_failure;
    }

    if(argv[3] != 0)
    {
        if((strstr(argv[3], ".txt")) || ((strstr(argv[3], ".c")) != NULL) )
        {
            printf("Secret file validated : %s\n", argv[3]);
            encInfo -> secret_fname = argv[3];
        }
        else
        {
            printf("Secret file validated : secret.txt not present\n");
  
            return e_failure;
        }
    }
    else
    {
        printf("Please pass screte file\n");
        return e_failure;

    }
    if(argv[4] != 0)
	{
		strcpy(arr,strstr(argv[4],"."));
		if(strcmp(arr,".bmp") == 0)
		{
			encInfo->stego_image_fname = argv[4];  //Copying output file name if passed
		}
		else
		{
			printf("Extension should be .bmp\n");  //Prompt user to give extension
			return e_failure;
		}
	}
	else
	{
		encInfo->stego_image_fname = "stego_img.bmp";  //Giving default name 
	}
    return e_success;
}


Status do_encoding(EncodeInfo *encInfo)
{
   if(open_files(encInfo) == e_success) // to open files
   {
       printf("SUCCESS: %s function COMPLETED\n", "open_files" );  //Printing success message
       int ret =  check_capacity(encInfo);
       if(ret != e_success)
       {
         printf("check capacity is Failure\n");  //Printing error mesage
		 return e_failure;
       }
       else
       {
          printf("Sufficient Capacity in the image for Encoding\n");  //Printing success message
       }
       if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
        {
            printf("SUCCESS: %s function COMPLETED\n", "copy_bmp_header");
        }
        else
        {
            printf("ERROR: %s function FAILED\n", "copy_bmp_header");
            return e_failure;
        }
          if (encode_magic_string_len(encInfo->magic_str_len, encInfo) == e_failure)
        {
            return e_failure;
        }
        if (encode_magic_string(encInfo->magic_string, encInfo) == e_failure)
        {
            return e_failure;
        }
        char *ret_ptr = strstr(encInfo->secret_fname, ".");
        if (ret_ptr == NULL)
        {
            printf("ERROR: Secret file has no extension\n");
            return e_failure;
        }
        int extn_len = strlen(ret_ptr);
        if (encode_secret_file_extn_len(extn_len, encInfo) == e_failure)
        {
            return e_failure;
        }

        if (encode_secret_file_extn(ret_ptr, encInfo) == e_failure)
        {
            return e_failure;
        }

        fseek(encInfo->fptr_secret, 0, SEEK_END);
        long file_size = ftell(encInfo->fptr_secret);
        rewind(encInfo->fptr_secret);

        if (encode_secret_file_size(file_size, encInfo) == e_failure)
        {
            return e_failure;
        }

        if (encode_secret_file_data(encInfo) == e_failure)
        {
            return e_failure;
        }

        if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
        {
            return e_failure;
        }
        return e_success;
    }
}
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];

    // Read 54 bytes from source
    if (fread(buffer, 54, 1, fptr_src_image) != 1)
    {
        return e_failure;
    }
    //write 54 bytes
    if (fwrite(buffer, 54, 1, fptr_dest_image) != 1)
    {
        return e_failure;
    }

    return e_success;
}
Status check_capacity(EncodeInfo *encInfo)
{
    printf("Enter magic string: ");
    scanf("%s", encInfo->magic_string);  
    // Get length of magic string
    int magic_str_len = strlen(encInfo->magic_string);

    // Get length of secret file extension
    int extn_len = strlen(encInfo->extn_secret_file);
    
    // Calculate total bytes to encode (header + data)
    long no_bytes_to_encode = (sizeof(int) + magic_str_len + sizeof(int) + extn_len + sizeof(int) +  encInfo->size_secret_file)  * 8; 

    // Check capacity (exclude BMP header = 54 bytes)
    if ((encInfo->image_capacity - 54) >= no_bytes_to_encode)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    int n=7;
    for (int i = 0; i < 8; i++) // Loop over 7 bits
    {
        int mask = 1 << n;
        int bit = data & mask; 
        image_buffer[i] = image_buffer[i] & 0xFE;
        bit = bit >> n;
        image_buffer[i] = image_buffer[i] | bit;
        n--;
    }

    return e_success;
}
Status encode_size_to_lsb(int data, char *image_buffer)
{
    int n=31;
    for (int i = 0; i < 32; i++) // Loop over 32 bits
    {
        int mask = 1 << n;
        int bit = data & mask; 
        image_buffer[i] = image_buffer[i] & 0xFE;
        bit = bit >> n;
        image_buffer[i] = image_buffer[i] | bit;
        n--;
    }

    return e_success;
}
Status encode_magic_string_len(int len, EncodeInfo *encInfo)
{
    char buffer[32];
    if (fread(buffer, 32, 1, encInfo->fptr_src_image) != 1)
        return e_failure;

    encode_size_to_lsb(len, buffer);

    if (fwrite(buffer, 32, 1, encInfo->fptr_stego_image) != 1)
        return e_failure;

    return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    for (int i = 0; i < strlen(magic_string); i++)
    {
        char buffer[8];
        if (fread(buffer, 8, 1, encInfo->fptr_src_image) != 1)
            return e_failure;
        
        encode_byte_to_lsb(magic_string[i], buffer);

        if (fwrite(buffer, 8, 1, encInfo->fptr_stego_image) != 1)
            return e_failure;
    }
    return e_success;
}
Status encode_secret_file_extn_len(int len, EncodeInfo *encInfo)
{
    char buffer[32];
    if (fread(buffer, 32, 1, encInfo->fptr_src_image) != 1)
        return e_failure;

    encode_size_to_lsb(len, buffer);

    if (fwrite(buffer, 32, 1, encInfo->fptr_stego_image) != 1)
        return e_failure;

    return e_success;
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    for (int i = 0; i < strlen(file_extn); i++)
    {
        char buffer[8];
        if (fread(buffer, 8, 1, encInfo->fptr_src_image) != 1)
            return e_failure;

        encode_byte_to_lsb(file_extn[i], buffer);

        if (fwrite(buffer, 8, 1, encInfo->fptr_stego_image) != 1)
            return e_failure;
    }
    return e_success;
}
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];
    if (fread(buffer, 32, 1, encInfo->fptr_src_image) != 1)
        return e_failure;

    encode_size_to_lsb(file_size, buffer);

    if (fwrite(buffer, 32, 1, encInfo->fptr_stego_image) != 1)
        return e_failure;

    return e_success;
}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    while (fread(&ch, 1, 1, encInfo->fptr_secret) == 1)
    {
        char buffer[8];
        if (fread(buffer, 8, 1, encInfo->fptr_src_image) != 1)
            return e_failure;

        encode_byte_to_lsb(ch, buffer);

        if (fwrite(buffer, 8, 1, encInfo->fptr_stego_image) != 1)
            return e_failure;
    }
    return e_success;
}
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}

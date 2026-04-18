#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
int main(int argc, char *argv[])
{
    if(check_operation_type(argv) == e_encode)
    {
        printf("You have choose enconding\n");
        EncodeInfo encInfo;
        if(read_and_validate_encode_args(argv, &encInfo)==e_success)
        {
            if(do_encoding(&encInfo)==e_success)
            {
                printf("Encoding is done...\n");
            }
            else
            {
                printf("Encoding is failed...\n");
            }
        }
    }
    else if(check_operation_type(argv) == e_decode)
    {
        printf("You have choosen deconding\n");
        DecodeInfo deInfo;
        if(read_and_validate_decode_args(argv, &deInfo)==d_success)
        {
            if(do_decoding(&deInfo)==d_success)
            {
                printf("decoding is done...\n");
            }
            else
            {
                printf("decoding is failed...\n");
            }
        }
        return d_failure;
    }
    else
    {
        printf("enter correct argument\n");
        return e_failure;
    }
}
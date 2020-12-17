#include "bmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

char *input_filename = "./input.bmp";
char *output_filename = "./output.bmp";

int main(){
    FILE *inBmp = fopen(input_filename, "rb");
    FILE *outBmp = fopen(output_filename, "wb+");
    struct image image;
    struct image rotated_image;

    enum read_status read_status = from_bmp(inBmp, &image);
    if (read_status != READ_OK) print_error_read(read_status);
    rotated_image = sepiaFilterAsm(image);
    enum write_status write_status = to_bmp(outBmp, &rotated_image);
    if (write_status != WRITE_OK) print_error_write(write_status);

    fclose(inBmp);
    fclose(outBmp);
    return 0;
}

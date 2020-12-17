#ifndef _BMP_H_
#define _BMP_H_

#include <stdint.h>
#include <stdio.h>


#define HEADER_TYPE 0x4d42
#define HEADER_RESERVED 0
#define HEADER_OFFBITS 54
#define HEADER_SIZE 40
#define HEADER_PLANES 1
#define HEADER_BITCOUNT 24
#define HEADER_COMPRESSION 0
#define HEADER_XMETER 0
#define HEADER_YMETER 0
#define HEADER_CLRUSED 0
#define HEADER_CLRIMPORTANT 0


struct __attribute__((packed)) bmp_header {
    uint16_t bfType;
    uint32_t bfileSize;
    uint32_t bfReserved;
    uint32_t bOffBits;
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};

struct __attribute__((packed)) pixel {
    uint8_t b, g, r;
};
struct __attribute__((packed)) image {
    uint32_t width, height;
    struct pixel *pixels;
};

enum read_status {
    READ_OK = 0,
    READ_INVALID_SIGNATURE,
    READ_INVALID_BITS,
    READ_INVALID_HEADER,
    READ_IO_ERROR
};

enum write_status {
    WRITE_OK = 0,
    WRITE_IO_ERROR
};

struct image create_image(uint32_t width, uint32_t height);
enum read_status from_bmp(FILE *in_bmp, struct image *in_image);
enum write_status to_bmp(FILE *out_bmp, struct image *out_image);


struct image rotate90(struct image source);
struct image sepiaFilter(struct image source);
struct image sepiaFilterAsm(struct image source);


struct bmp_header create_header(struct image *img);


void print_error_read(enum read_status status);
void print_error_write(enum write_status status);
void packed_mul(float *result, float const *mul1, float const *mul2);
#endif

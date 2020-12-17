#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bmp.h"

void bmp_header_print(struct bmp_header header) {
    printf("bfType: %#x\n", header.bfType);
    printf("bfileSize: %u\n", header.bfileSize);
    printf("bfReserved: %u\n", header.bfReserved);
    printf("bOffBits: %u\n", header.bOffBits);
    printf("biSize: %u\n", header.biSize);
    printf("biWidth: %u\n", header.biWidth);
    printf("biHeight: %u\n", header.biHeight);
    printf("biPlanes: %u\n", header.biPlanes);
    printf("biBitCount: %u\n", header.biBitCount);
    printf("biCompression: %u\n", header.biCompression);
    printf("biSizeImage: %u\n", header.biSizeImage);
    printf("biXPelsPerMeter: %u\n", header.biXPelsPerMeter);
    printf("biYPelsPerMeter: %u\n", header.biYPelsPerMeter);
    printf("biClrUsed: %u\n", header.biClrUsed);
    printf("biClrImportant: %u\n", header.biClrImportant);
}

struct image create_image(uint32_t width, uint32_t height) {
    struct image img;
    img.width = width;
    img.height = height;
    img.pixels = malloc(img.height * img.width * sizeof(struct pixel));
    return img;
}

struct bmp_header create_header(struct image *img) {
    struct bmp_header header;
    uint64_t remainder;
    header.bfType = HEADER_TYPE;
    header.bfReserved = HEADER_RESERVED;
    header.bOffBits =  HEADER_OFFBITS;
    header.biSize = HEADER_SIZE;
    header.biPlanes = HEADER_PLANES;
    header.biBitCount = HEADER_BITCOUNT;
    header.biCompression = HEADER_COMPRESSION;
    header.biXPelsPerMeter = HEADER_XMETER;
    header.biYPelsPerMeter = HEADER_YMETER;
    header.biClrUsed = HEADER_CLRUSED;
    header.biClrImportant = HEADER_CLRIMPORTANT;
    header.biHeight = img->height;
    header.biWidth = img->width;
    remainder = (img->width * 3) % 4;
    remainder = (remainder == 0) ? 0 : (4 - remainder);
    header.biSizeImage = (img->width * 3 + remainder) * img->height;
    header.bfileSize = header.biSizeImage + header.bOffBits;
    return header;
}


enum read_status from_bmp(FILE *in_bmp, struct image *in_image) {
    struct bmp_header header;
    if (!fread(&header, sizeof(header), 1, in_bmp)) return READ_IO_ERROR;
    bmp_header_print(header);
    uint8_t spare[4];
    int64_t remainder, row;
    *in_image = create_image(header.biWidth, header.biHeight);
    remainder = (in_image->width * 3) % 4;
    remainder = (remainder == 0) ? 0 : (4 - remainder);
    for (row = in_image->height - 1; row >= 0; row--) {
        uint64_t row_bits = fread(&in_image->pixels[row * in_image->width], sizeof(struct pixel), in_image->width, in_bmp);
        uint64_t rem_bits = fread(spare, sizeof(uint8_t), remainder, in_bmp);
        if (!row_bits || (remainder && !rem_bits)) {
            free(in_image->pixels);
            return READ_IO_ERROR;
        }
    }
    return READ_OK;
}


enum write_status to_bmp(FILE *out_bmp, struct image *out_image) {
    int64_t remainder, row;
    const uint8_t spare[4] = {0};
    struct bmp_header header;
    header = create_header(out_image);
    if (!fwrite(&header, sizeof(header), 1, out_bmp)) return WRITE_IO_ERROR;
    remainder = (out_image->width * 3) % 4;
    remainder = (remainder == 0) ? 0 : (4 - remainder);
    for (row = out_image->height - 1; row >= 0; row--) {
        uint64_t row_bits = fwrite(&out_image->pixels[row * out_image->width], sizeof(struct pixel), out_image->width,
                                   out_bmp);
        uint64_t rem_bits = fwrite(spare, sizeof(uint8_t), remainder, out_bmp);
        if (!row_bits || (remainder && !rem_bits)) return WRITE_IO_ERROR;
    }
    return WRITE_OK;
}
struct image rotate90(struct image const source) {
    struct image new_image = create_image(source.height, source.width);
    for (uint32_t x = 0; x < new_image.width; x++)
        for (uint32_t y = 0; y < new_image.height; y++)
            new_image.pixels[y * new_image.width + x] = source.pixels[y + new_image.height*(new_image.width - x -1)];
    return new_image;
}
 static const float SepiaCoef[3][3] = {
            { .393f, .769f, .189f },
            { .349f, .686f, .168f },
            { .272f, .543f, .131f } };

static unsigned char sat( uint64_t x) {
    if (x < 256) return x; return 255;
}

struct pixel sepiaPixel(struct pixel origin){
	struct pixel newpic = origin;
	newpic.r = sat(origin.r * SepiaCoef[0][0] + origin.g * SepiaCoef[0][1] + origin.b * SepiaCoef[0][2]);
	newpic.g = sat(origin.r * SepiaCoef[1][0] + origin.g * SepiaCoef[1][1] + origin.b * SepiaCoef[1][2]);
	newpic.b = sat(origin.r * SepiaCoef[2][0] + origin.g * SepiaCoef[2][1] + origin.b * SepiaCoef[2][2]);
	return newpic;
}

struct image sepiaFilter(struct image const source) {
    struct image new_image = create_image(source.width, source.height);

    for (uint32_t i = 0; i < new_image.width; i++)
        for (uint32_t j = 0; j < new_image.height; j++)
            new_image.pixels[i * new_image.height + j] = sepiaPixel(source.pixels[i * new_image.height + j]);

    return new_image;
}



static const float byte_to_float[256] = {
    0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
    16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f,
    32.0f, 33.0f, 34.0f, 35.0f, 36.0f, 37.0f, 38.0f, 39.0f, 40.0f, 41.0f, 42.0f, 43.0f, 44.0f, 45.0f, 46.0f, 47.0f,
    48.0f, 49.0f, 50.0f, 51.0f, 52.0f, 53.0f, 54.0f, 55.0f, 56.0f, 57.0f, 58.0f, 59.0f, 60.0f, 61.0f, 62.0f, 63.0f,
    64.0f, 65.0f, 66.0f, 67.0f, 68.0f, 69.0f, 70.0f, 71.0f, 72.0f, 73.0f, 74.0f, 75.0f, 76.0f, 77.0f, 78.0f, 79.0f,
    80.0f, 81.0f, 82.0f, 83.0f, 84.0f, 85.0f, 86.0f, 87.0f, 88.0f, 89.0f, 90.0f, 91.0f, 92.0f, 93.0f, 94.0f, 95.0f,
    96.0f, 97.0f, 98.0f, 99.0f, 100.0f, 101.0f, 102.0f, 103.0f, 104.0f, 105.0f, 106.0f, 107.0f, 108.0f, 109.0f, 110.0f, 111.0f,
    112.0f, 113.0f, 114.0f, 115.0f, 116.0f, 117.0f, 118.0f, 119.0f, 120.0f, 121.0f, 122.0f, 123.0f, 124.0f, 125.0f, 126.0f, 127.0f,
    128.0f, 129.0f, 130.0f, 131.0f, 132.0f, 133.0f, 134.0f, 135.0f, 136.0f, 137.0f, 138.0f, 139.0f, 140.0f, 141.0f, 142.0f, 143.0f,
    144.0f, 145.0f, 146.0f, 147.0f, 148.0f, 149.0f, 150.0f, 151.0f, 152.0f, 153.0f, 154.0f, 155.0f, 156.0f, 157.0f, 158.0f, 159.0f,
    160.0f, 161.0f, 162.0f, 163.0f, 164.0f, 165.0f, 166.0f, 167.0f, 168.0f, 169.0f, 170.0f, 171.0f, 172.0f, 173.0f, 174.0f, 175.0f,
    176.0f, 177.0f, 178.0f, 179.0f, 180.0f, 181.0f, 182.0f, 183.0f, 184.0f, 185.0f, 186.0f, 187.0f, 188.0f, 189.0f, 190.0f, 191.0f,
    192.0f, 193.0f, 194.0f, 195.0f, 196.0f, 197.0f, 198.0f, 199.0f, 200.0f, 201.0f, 202.0f, 203.0f, 204.0f, 205.0f, 206.0f, 207.0f,
    208.0f, 209.0f, 210.0f, 211.0f, 212.0f, 213.0f, 214.0f, 215.0f, 216.0f, 217.0f, 218.0f, 219.0f, 220.0f, 221.0f, 222.0f, 223.0f,
    224.0f, 225.0f, 226.0f, 227.0f, 228.0f, 229.0f, 230.0f, 231.0f, 232.0f, 233.0f, 234.0f, 235.0f, 236.0f, 237.0f, 238.0f, 239.0f,
    240.0f, 241.0f, 242.0f, 243.0f, 244.0f, 245.0f, 246.0f, 247.0f, 248.0f, 249.0f, 250.0f, 251.0f, 252.0f, 253.0f, 254.0f, 255.0f
};



struct image sepiaFilterAsm(struct image const source) {
	struct image new_image = create_image(source.width, source.height);
	int pixelcount =  source.width * source.height;
	int fastIters = pixelcount/4;

	 float fastSepiaCoef[3][12] = {
		{.131f, .168f, .189f, .131f, //b row
		.543f, .686f, .769f, .543f, //g row
		.272f, .349f, .393f, .272f}, // r row

		{.168f, .189f, .131f, .168f, //b row
		.686f, .769f, .543f, .686f, //g row
		.349f, .393f, .272f, .349f}, // r row

		{.189f, .131f, .168f, .189f, //b row
		.769f, .543f, .686f, .769f, //g row
		.393f, .272f, .349f, .393f} // r row
	 };
	uint8_t * resultPointer = (uint8_t*) new_image.pixels;
	float rawResult[4];
	float pixelsToMul[12];
	struct pixel *sourcePixelPtr = source.pixels;
	for(int i = 0; i < fastIters; i++){
		for(int d = 0; d < 3; d++){
			for(int px = 0; px < 4; px++){
				pixelsToMul[px] = byte_to_float[sourcePixelPtr->b];
				pixelsToMul[px+4] = byte_to_float[sourcePixelPtr->g];
				pixelsToMul[px+8] = byte_to_float[sourcePixelPtr->r];
				if((d*4 + px)%3 == 2)sourcePixelPtr++;
			}
			 packed_mul(rawResult, pixelsToMul, fastSepiaCoef[d]);
			for(int px = 0; px < 4; px++, resultPointer++){
				*resultPointer = sat((uint64_t) roundf(rawResult[px]));
			}
		}
	}
	for(int i  = fastIters*4; i < pixelcount; i++){
		new_image.pixels[i] = sepiaPixel(source.pixels[i]);
	}
	return new_image;
}


void print_error_read(enum read_status status) {
    if (status == READ_OK) return;
    char *err_msg;

    switch (status) {

        case READ_INVALID_SIGNATURE:
            err_msg = "Invalid signature";
            break;
        case READ_INVALID_BITS:
            err_msg = "Invalid bit count";
            break;
        case READ_INVALID_HEADER:
            err_msg = "Invalid header";
            break;
        case READ_IO_ERROR:
            err_msg = "Read IO error";
            break;
        default:
            err_msg = "Read error";
            break;
    }

    fprintf(stderr, "%s\n", err_msg);
}


void print_error_write(enum write_status status) {
    if (status == WRITE_OK) return;
    char *err_msg;

    switch (status) {
        case WRITE_IO_ERROR:
            err_msg = "Write IO error";
            break;
        default:
            err_msg = "Write error";
            break;
    }

    fprintf(stderr, "%s\n", err_msg);
}

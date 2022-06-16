#include "dds_loader.h"
#define BCDEC_IMPLEMENTATION 1
#include "../libs/bcdec/bcdec.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include "../libs/bcdec/stb_image_write.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <string>

constexpr int kRuns = 10;

static std::chrono::steady_clock::time_point get_time()
{
    return std::chrono::high_resolution_clock::now();
}
static double get_duration(std::chrono::steady_clock::time_point since)
{
    std::chrono::duration<double> dur = get_time() - since;
    return dur.count();
}

static void print_time(const char* title, double t, int width, int height)
{
    double mpix = width * height / 1000000.0 * kRuns;
    printf("  %-18s %6.1f ms %8.1f Mpix/s\n", title, t * 1000.0, mpix/t);
}

static void decode_bcdec(int width, int height, unsigned int format, const void* input, void* output)
{
    const char* src = (const char*)input;
    char* dst = (char*)output;
    for (int i = 0; i < height; i += 4)
    {
        for (int j = 0; j < width; j += 4)
        {
            if (format == FORMAT_DXT1) {
                bcdec_bc1(src, dst + (i*width+j)*4, width * 4);
                src += BCDEC_BC1_BLOCK_SIZE;
            } else if (format == FORMAT_DXT3) {
                bcdec_bc2(src, dst + (i*width+j)*4, width * 4);
                src += BCDEC_BC2_BLOCK_SIZE;
            } else if (format == FORMAT_DXT5) {
                bcdec_bc3(src, dst + (i*width+j)*4, width * 4);
                src += BCDEC_BC3_BLOCK_SIZE;
            } else if (format == FORMAT_BC7_UNORM) {
                bcdec_bc7(src, dst + (i*width+j)*4, width * 4);
                src += BCDEC_BC7_BLOCK_SIZE;
            } else if (format == FORMAT_BC4_UNORM) {
                bcdec_bc4(src, dst + (i*width+j)*1, width * 1);
                src += BCDEC_BC4_BLOCK_SIZE;                
            } else if (format == FORMAT_BC5_UNORM) {
                bcdec_bc5(src, dst + (i*width+j)*2, width * 2);
                src += BCDEC_BC5_BLOCK_SIZE;                
            } else if (format == FORMAT_BC6H_UF16 || format == FORMAT_BC6H_SF16) {
                bcdec_bc6h(src, dst + (i*width+j)*12, width * 3, format == FORMAT_BC6H_SF16);
                src += BCDEC_BC5_BLOCK_SIZE;                
            }
        }
    }
}

typedef void (DecodeFunc)(int width, int height, unsigned int format, const void* input, void* output);

struct Decoder
{
    const char* name;
    DecodeFunc* func;
};

static Decoder s_Decoders[] =
{
    {"bcdec", decode_bcdec},
};


int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("USAGE: bcn_decode_tester <path/to/input.dds> ...\n");
        return -1;
    }
    printf("Doing %i runs on each image\n", kRuns);
    for (int fi = 1; fi < argc; ++fi)
    {
        std::string filename = argv[fi];
        int width = 0, height = 0;
        unsigned int format = 0;
        void* input_data = nullptr;
        if (!load_dds(filename.c_str(), &width, &height, &format, &input_data))
        {
            printf("ERROR: failed to read dds file '%s'\n", filename.c_str());
            return 1;
        }
        printf("%s, %ix%i, %s\n", filename.c_str(), width, height, get_format_name(format));
        // remove ".dds"
        filename.pop_back();
        filename.pop_back();
        filename.pop_back();
        filename.pop_back();

        void* output_data = malloc(width * height * 12);
        
        for (const Decoder& dec : s_Decoders)
        {
            memset(output_data, 0, width * height * 12);
            auto t0 = get_time();
            for (int run = 0; run < kRuns; ++run)
                dec.func(width, height, format, input_data, output_data);
            auto dur = get_duration(t0);
            print_time(dec.name, dur, width, height);

            if (format == FORMAT_BC6H_SF16 || format == FORMAT_BC6H_UF16)
                stbi_write_hdr((filename+"-"+dec.name+".hdr").c_str(), width, height, 3, (const float*)output_data);
            else if (format == FORMAT_BC5_UNORM)
                stbi_write_tga((filename+"-"+dec.name+".tga").c_str(), width, height, 2, output_data);
            else if (format == FORMAT_BC4_UNORM)
                stbi_write_tga((filename+"-"+dec.name+".tga").c_str(), width, height, 1, output_data);
            else
                stbi_write_tga((filename+"-"+dec.name+".tga").c_str(), width, height, 4, output_data);
        }

        free(input_data);
        free(output_data);
    }
    return 0;
}

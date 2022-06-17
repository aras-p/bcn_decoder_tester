#include "dds_loader.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include "../libs/bcdec/stb_image_write.h"

#define BCDEC_IMPLEMENTATION 1
#include "../libs/bcdec/bcdec.h"

#define RGBCX_IMPLEMENTATION 1
#include "../libs/bc7enc/rgbcx.h"
#include "../libs/bc7enc/bc7decomp.h"

#include "../libs/DirectXTex/DirectXTex/BC.h"

#include "../libs/swiftshader/BC_Decoder.hpp"

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

static bool decode_bcdec(int width, int height, unsigned int format, const void* input, void* output)
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
            } else {
                return false;
            }
        }
    }
    return true;
}

static bool decode_bc7dec(int width, int height, unsigned int format, const void* input, void* output)
{
    const char* src = (const char*)input;
    char* dst = (char*)output;
    uint32_t rgba[16];
    for (int i = 0; i < height; i += 4)
    {
        for (int j = 0; j < width; j += 4)
        {
            if (format == FORMAT_DXT1) {
                rgbcx::unpack_bc1(src, rgba);
                src += BCDEC_BC1_BLOCK_SIZE;
                for (int r = 0; r < 4; ++r) {
                    memcpy(dst + (i*width+j)*4 + width * 4 * r, rgba + 4 * r, 4 * 4);
                }
            } else if (format == FORMAT_DXT5) {
                rgbcx::unpack_bc3(src, rgba);
                src += BCDEC_BC3_BLOCK_SIZE;
                for (int r = 0; r < 4; ++r) {
                    memcpy(dst + (i*width+j)*4 + width * 4 * r, rgba + 4 * r, 4 * 4);
                }
            } else if (format == FORMAT_BC7_UNORM) {
                bc7decomp::unpack_bc7(src, (bc7decomp::color_rgba*)rgba);
                src += BCDEC_BC7_BLOCK_SIZE;
                for (int r = 0; r < 4; ++r) {
                    memcpy(dst + (i*width+j)*4 + width * 4 * r, rgba + 4 * r, 4 * 4);
                }
            } else if (format == FORMAT_BC4_UNORM) {
                rgbcx::unpack_bc4(src, (uint8_t*)rgba, 1);
                src += BCDEC_BC4_BLOCK_SIZE;
                for (int r = 0; r < 4; ++r) {
                    memcpy(dst + (i*width+j)*1 + width * 1 * r, rgba + r, 4);
                }
            } else if (format == FORMAT_BC5_UNORM) {
                rgbcx::unpack_bc5(src, rgba, 0, 1, 2);
                src += BCDEC_BC5_BLOCK_SIZE;
                for (int r = 0; r < 4; ++r) {
                    memcpy(dst + (i*width+j)*2 + width * 2 * r, rgba + 2 * r, 4 * 2);
                }
            } else {
                return false;
            }
        }
    }
    return true;
}

static bool decode_dxtex(int width, int height, unsigned int format, const void* input, void* output)
{
    using namespace DirectX;
    const uint8_t* src = (const uint8_t*)input;
    uint8_t* dst = (uint8_t*)output;
    XMVECTOR rgbaf[NUM_PIXELS_PER_BLOCK];
    PackedVector::XMUBYTEN4 rgbai[NUM_PIXELS_PER_BLOCK];
    for (int i = 0; i < height; i += 4)
    {
        for (int j = 0; j < width; j += 4)
        {
            if (format == FORMAT_DXT1) {
                D3DXDecodeBC1(rgbaf, src);
                src += BCDEC_BC1_BLOCK_SIZE;
                for (int c = 0; c < 16; ++c) XMStoreUByteN4(rgbai+c, rgbaf[c]);
                for (int r = 0; r < 4; ++r) memcpy(dst + (i*width+j)*4 + width * 4 * r, rgbai + 4 * r, 4 * 4);
            } else if (format == FORMAT_DXT3) {
                D3DXDecodeBC2(rgbaf, src);
                src += BCDEC_BC2_BLOCK_SIZE;
                for (int c = 0; c < 16; ++c) XMStoreUByteN4(rgbai+c, rgbaf[c]);
                for (int r = 0; r < 4; ++r) memcpy(dst + (i*width+j)*4 + width * 4 * r, rgbai + 4 * r, 4 * 4);
            } else if (format == FORMAT_DXT5) {
                D3DXDecodeBC3(rgbaf, src);
                src += BCDEC_BC3_BLOCK_SIZE;
                for (int c = 0; c < 16; ++c) XMStoreUByteN4(rgbai+c, rgbaf[c]);
                for (int r = 0; r < 4; ++r) memcpy(dst + (i*width+j)*4 + width * 4 * r, rgbai + 4 * r, 4 * 4);
            } else if (format == FORMAT_BC7_UNORM) {
                D3DXDecodeBC7(rgbaf, src);
                src += BCDEC_BC7_BLOCK_SIZE;
                for (int c = 0; c < 16; ++c) XMStoreUByteN4(rgbai+c, rgbaf[c]);
                for (int r = 0; r < 4; ++r) memcpy(dst + (i*width+j)*4 + width * 4 * r, rgbai + 4 * r, 4 * 4);
            } else if (format == FORMAT_BC4_UNORM) {
                D3DXDecodeBC4U(rgbaf, src);
                src += BCDEC_BC4_BLOCK_SIZE;
                for (int c = 0; c < 16; ++c) {
                    XMStoreUByteN4(rgbai+c, rgbaf[c]);
                    dst[(i*width+j) + width*(c/4) + (c&3)] = rgbai[c].x;
                }
            } else if (format == FORMAT_BC5_UNORM) {
                D3DXDecodeBC5U(rgbaf, src);
                src += BCDEC_BC5_BLOCK_SIZE;
                for (int c = 0; c < 16; ++c) {
                    XMStoreUByteN4(rgbai+c, rgbaf[c]);
                    dst[(i*width+j)*2 + width*2*(c/4) + (c&3)*2 + 0] = rgbai[c].x;
                    dst[(i*width+j)*2 + width*2*(c/4) + (c&3)*2 + 1] = rgbai[c].y;
                }
            } else if (format == FORMAT_BC6H_UF16 || format == FORMAT_BC6H_SF16) {
                if (format == FORMAT_BC6H_UF16)
                    D3DXDecodeBC6HU(rgbaf, src);
                else
                    D3DXDecodeBC6HS(rgbaf, src);
                src += BCDEC_BC6H_BLOCK_SIZE;
                for (int c = 0; c < 16; ++c) {
                    memcpy(dst + (i*width+j)*12 + width*(c/4)*12 + (c&3)*12, rgbaf+c, 12);
                }
            } else {
                return false;
            }
        }
    }
    return true;
}

static bool decode_swiftshader(int width, int height, unsigned int format, const void* input, void* output)
{
    const unsigned char* src = (const unsigned char*)input;
    unsigned char* dst = (unsigned char*)output;
    int n = 0;
    int dst_bpp = 4;
    switch (format)
    {
        case FORMAT_DXT1: n = 1; break;
        case FORMAT_DXT3: n = 2; break;
        case FORMAT_DXT5: n = 3; break;
        case FORMAT_BC4_UNORM: n = 4; dst_bpp = 1; break;
        case FORMAT_BC5_UNORM: n = 5; dst_bpp = 2; break;
        case FORMAT_BC6H_UF16:
        case FORMAT_BC6H_SF16: n = 6; dst_bpp = 8; break;
        case FORMAT_BC7_UNORM: n = 7; break;
        default: return false;
    }
    bool ok = BC_Decoder::Decode(src, dst, width, height, width * dst_bpp, dst_bpp, n, format!=FORMAT_BC6H_SF16);
    if (format == FORMAT_DXT1 || format == FORMAT_DXT3 || format == FORMAT_DXT5 || format == FORMAT_BC7_UNORM)
    {
        // swizzle BGRA -> RGBA
        for (int i = 0; i < width * height; ++i) {
            unsigned char c = dst[i*4+0];
            dst[i*4+0] = dst[i*4+2];
            dst[i*4+2] = c;
        }
    }
    //@TODO
    return ok;
}

typedef bool (DecodeFunc)(int width, int height, unsigned int format, const void* input, void* output);

struct Decoder
{
    const char* name;
    DecodeFunc* func;
};

static Decoder s_Decoders[] =
{
    {"bcdec", decode_bcdec},
    {"bc7dec", decode_bc7dec},
    {"dxtex", decode_dxtex},
    {"swiftshader", decode_swiftshader},
};


int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("USAGE: bcn_decode_tester <path/to/input.dds> ...\n");
        return -1;
    }
    printf("Doing %i runs on each image\n", kRuns);
    rgbcx::init();
    
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
            bool ok = true;
            for (int run = 0; run < kRuns; ++run)
                ok &= dec.func(width, height, format, input_data, output_data);
            auto dur = get_duration(t0);
            if (ok)
            {
                print_time(dec.name, dur, width, height);

                if (format == FORMAT_BC6H_SF16 || format == FORMAT_BC6H_UF16)
                    stbi_write_hdr((filename+"-"+dec.name+".hdr").c_str(), width, height, 3, (const float*)output_data);
                else if (format == FORMAT_BC5_UNORM)
                {
                    char* src = (char*)output_data;
                    char* dst = src + width * height * 2;
                    for (int ip = 0; ip < width * height; ++ip) {
                        dst[ip * 3 + 0] = src[ip * 2 + 0];
                        dst[ip * 3 + 1] = src[ip * 2 + 1];
                        dst[ip * 3 + 2] = 0;
                    }
                    stbi_write_tga((filename+"-"+dec.name+".tga").c_str(), width, height, 3, dst);
                }
                else if (format == FORMAT_BC4_UNORM)
                    stbi_write_tga((filename+"-"+dec.name+".tga").c_str(), width, height, 1, output_data);
                else
                    stbi_write_tga((filename+"-"+dec.name+".tga").c_str(), width, height, 4, output_data);
            }
        }

        free(input_data);
        free(output_data);
    }
    return 0;
}

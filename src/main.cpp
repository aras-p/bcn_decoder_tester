
constexpr int kRuns = 20;
constexpr bool kWriteOutputImages = true;

#define USE_BCDEC 1
#define USE_BC7ENC_RDO 1
#define USE_DXTEX 1
#define USE_SWIFTSHADER 1
#define USE_ICBC 1
#define USE_ETCPAK 1
#define USE_SQUISH 1
#define USE_CONVECTION 1

#include "dds_loader.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include "../libs/bcdec/stb_image_write.h"
#include "../libs/xxHash/xxhash.h"

#if USE_BCDEC
#   define BCDEC_IMPLEMENTATION 1
#   include "../libs/bcdec/bcdec.h"
#endif
#if USE_BC7ENC_RDO
#   define RGBCX_IMPLEMENTATION 1
#   include "../libs/bc7enc_rdo/rgbcx.h"
#   include "../libs/bc7enc_rdo/bc7decomp.h"
#endif
#if USE_DXTEX
#   include "../libs/DirectXTex/DirectXTex/BC.h"
#endif
#if USE_SWIFTSHADER
#   include "../libs/swiftshader/BC_Decoder.hpp"
#endif
#if USE_ICBC
#   define ICBC_IMPLEMENTATION 1
#   define ICBC_SIMD 0 // scalar version for testing
#   include "../libs/icbc/icbc.h"
#endif
#if USE_ETCPAK
#   include "../libs/etcpak/BlockData.hpp"
#endif
#if USE_SQUISH
#   include "../libs/libsquish/squish.h"
#endif
#if USE_CONVECTION
#   include "../libs/ConvectionKernels/ConvectionKernels_BC67.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <string>
#include <filesystem>
#include <map>

static std::chrono::steady_clock::time_point get_time()
{
    return std::chrono::high_resolution_clock::now();
}
static double get_duration(std::chrono::steady_clock::time_point since)
{
    std::chrono::duration<double> dur = get_time() - since;
    return dur.count();
}

static void print_time(const char* title, double t, size_t pixelCount)
{
    double mpix = pixelCount / 1000000.0 * kRuns;
    printf("  %-12s %6.1f ms %8.1f Mpix/s\n", title, t * 1000.0, mpix/t);
}

#if USE_BCDEC
static bool decode_bcdec(int width, int height, DDSFormat format, const void* input, void* output)
{
    const char* src = (const char*)input;
    char* dst = (char*)output;
    for (int i = 0; i < height; i += 4)
    {
        for (int j = 0; j < width; j += 4)
        {
            if (format == DDSFormat::BC1) {
                bcdec_bc1(src, dst + (i*width+j)*4, width * 4);
                src += BCDEC_BC1_BLOCK_SIZE;
            } else if (format == DDSFormat::BC2) {
                bcdec_bc2(src, dst + (i*width+j)*4, width * 4);
                src += BCDEC_BC2_BLOCK_SIZE;
            } else if (format == DDSFormat::BC3) {
                bcdec_bc3(src, dst + (i*width+j)*4, width * 4);
                src += BCDEC_BC3_BLOCK_SIZE;
            } else if (format == DDSFormat::BC7) {
                bcdec_bc7(src, dst + (i*width+j)*4, width * 4);
                src += BCDEC_BC7_BLOCK_SIZE;
            } else if (format == DDSFormat::BC4) {
                bcdec_bc4(src, dst + (i*width+j)*1, width * 1);
                src += BCDEC_BC4_BLOCK_SIZE;                
            } else if (format == DDSFormat::BC5) {
                bcdec_bc5(src, dst + (i*width+j)*2, width * 2);
                src += BCDEC_BC5_BLOCK_SIZE;                
            } else if (format == DDSFormat::BC6HU || format == DDSFormat::BC6HS) {
                bcdec_bc6h(src, dst + (i*width+j)*12, width * 3, format == DDSFormat::BC6HS);
                src += BCDEC_BC5_BLOCK_SIZE;                
            } else {
                return false;
            }
        }
    }
    return true;
}
#endif

#if USE_BC7ENC_RDO
static bool decode_bc7dec_rdo(int width, int height, DDSFormat format, const void* input, void* output)
{
    const char* src = (const char*)input;
    char* dst = (char*)output;
    uint32_t rgba[16];
    for (int i = 0; i < height; i += 4)
    {
        for (int j = 0; j < width; j += 4)
        {
            if (format == DDSFormat::BC1) {
                rgbcx::unpack_bc1(src, rgba);
                src += BCDEC_BC1_BLOCK_SIZE;
                for (int r = 0; r < 4; ++r) {
                    memcpy(dst + (i*width+j)*4 + width * 4 * r, rgba + 4 * r, 4 * 4);
                }
            } else if (format == DDSFormat::BC3) {
                rgbcx::unpack_bc3(src, rgba);
                src += BCDEC_BC3_BLOCK_SIZE;
                for (int r = 0; r < 4; ++r) {
                    memcpy(dst + (i*width+j)*4 + width * 4 * r, rgba + 4 * r, 4 * 4);
                }
            } else if (format == DDSFormat::BC7) {
                bc7decomp::unpack_bc7(src, (bc7decomp::color_rgba*)rgba);
                src += BCDEC_BC7_BLOCK_SIZE;
                for (int r = 0; r < 4; ++r) {
                    memcpy(dst + (i*width+j)*4 + width * 4 * r, rgba + 4 * r, 4 * 4);
                }
            } else if (format == DDSFormat::BC4) {
                rgbcx::unpack_bc4(src, (uint8_t*)rgba, 1);
                src += BCDEC_BC4_BLOCK_SIZE;
                for (int r = 0; r < 4; ++r) {
                    memcpy(dst + (i*width+j)*1 + width * 1 * r, rgba + r, 4);
                }
            } else if (format == DDSFormat::BC5) {
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
#endif

#if USE_DXTEX
static bool decode_dxtex(int width, int height, DDSFormat format, const void* input, void* output)
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
            if (format == DDSFormat::BC1) {
                D3DXDecodeBC1(rgbaf, src);
                src += BCDEC_BC1_BLOCK_SIZE;
                for (int c = 0; c < 16; ++c) XMStoreUByteN4(rgbai+c, rgbaf[c]);
                for (int r = 0; r < 4; ++r) memcpy(dst + (i*width+j)*4 + width * 4 * r, rgbai + 4 * r, 4 * 4);
            } else if (format == DDSFormat::BC2) {
                D3DXDecodeBC2(rgbaf, src);
                src += BCDEC_BC2_BLOCK_SIZE;
                for (int c = 0; c < 16; ++c) XMStoreUByteN4(rgbai+c, rgbaf[c]);
                for (int r = 0; r < 4; ++r) memcpy(dst + (i*width+j)*4 + width * 4 * r, rgbai + 4 * r, 4 * 4);
            } else if (format == DDSFormat::BC3) {
                D3DXDecodeBC3(rgbaf, src);
                src += BCDEC_BC3_BLOCK_SIZE;
                for (int c = 0; c < 16; ++c) XMStoreUByteN4(rgbai+c, rgbaf[c]);
                for (int r = 0; r < 4; ++r) memcpy(dst + (i*width+j)*4 + width * 4 * r, rgbai + 4 * r, 4 * 4);
            } else if (format == DDSFormat::BC7) {
                D3DXDecodeBC7(rgbaf, src);
                src += BCDEC_BC7_BLOCK_SIZE;
                for (int c = 0; c < 16; ++c) XMStoreUByteN4(rgbai+c, rgbaf[c]);
                for (int r = 0; r < 4; ++r) memcpy(dst + (i*width+j)*4 + width * 4 * r, rgbai + 4 * r, 4 * 4);
            } else if (format == DDSFormat::BC4) {
                D3DXDecodeBC4U(rgbaf, src);
                src += BCDEC_BC4_BLOCK_SIZE;
                for (int c = 0; c < 16; ++c) {
                    XMStoreUByteN4(rgbai+c, rgbaf[c]);
                    dst[(i*width+j) + width*(c/4) + (c&3)] = rgbai[c].x;
                }
            } else if (format == DDSFormat::BC5) {
                D3DXDecodeBC5U(rgbaf, src);
                src += BCDEC_BC5_BLOCK_SIZE;
                for (int c = 0; c < 16; ++c) {
                    XMStoreUByteN4(rgbai+c, rgbaf[c]);
                    dst[(i*width+j)*2 + width*2*(c/4) + (c&3)*2 + 0] = rgbai[c].x;
                    dst[(i*width+j)*2 + width*2*(c/4) + (c&3)*2 + 1] = rgbai[c].y;
                }
            } else if (format == DDSFormat::BC6HU || format == DDSFormat::BC6HS) {
                if (format == DDSFormat::BC6HU)
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
#endif

#if USE_SWIFTSHADER || USE_CONVECTION
// from https://gist.github.com/rygorous/2144712
union FP32
{
    uint32_t u;
    float f;
};
static float half_to_float_fast5(uint16_t h)
{
    static const FP32 magic = { (254 - 15) << 23 };
    static const FP32 was_infnan = { (127 + 16) << 23 };
    FP32 o;

    o.u = (h & 0x7fff) << 13;       // exponent/mantissa bits
    o.f *= magic.f;                 // exponent adjust
    if (o.f >= was_infnan.f)        // make sure Inf/NaN survive
        o.u |= 255 << 23;
    o.u |= (h & 0x8000) << 16;      // sign bit
    return o.f;
}
#endif

#if USE_SWIFTSHADER
static bool decode_swiftshader(int width, int height, DDSFormat format, const void* input, void* output)
{
    const unsigned char* src = (const unsigned char*)input;
    unsigned char* dst = (unsigned char*)output;
    unsigned char* dst_tmp = dst + width * height * 12;
    int n = 0;
    int dst_bpp = 4;
    switch (format)
    {
        case DDSFormat::BC1: n = 1; break;
        case DDSFormat::BC2: n = 2; break;
        case DDSFormat::BC3: n = 3; break;
        case DDSFormat::BC4: n = 4; dst_bpp = 1; break;
        case DDSFormat::BC5: n = 5; dst_bpp = 2; break;
        case DDSFormat::BC6HU:
        case DDSFormat::BC6HS: n = 6; dst_bpp = 8; break;
        case DDSFormat::BC7: n = 7; break;
        default: return false;
    }
    bool ok = BC_Decoder::Decode(src, n == 6 ? dst_tmp : dst, width, height, width * dst_bpp, dst_bpp, n, format!=DDSFormat::BC6HS);
    if (format == DDSFormat::BC1 || format == DDSFormat::BC2 || format == DDSFormat::BC3 || format == DDSFormat::BC7)
    {
        // swizzle BGRA -> RGBA
        for (int i = 0; i < width * height; ++i) {
            unsigned char c = dst[i*4+0];
            dst[i*4+0] = dst[i*4+2];
            dst[i*4+2] = c;
        }
    }
    if (n == 6)
    {
        // FP16 RGBA -> FP32 RGB
        const uint16_t* src_u = (const uint16_t*)dst_tmp;
        float* dst_f = (float*)dst;
        for (int i = 0; i < width * height; ++i) {
            dst_f[i*3+0] = half_to_float_fast5(src_u[i*4+0]);
            dst_f[i*3+1] = half_to_float_fast5(src_u[i*4+1]);
            dst_f[i*3+2] = half_to_float_fast5(src_u[i*4+2]);
        }
    }
    return ok;
}
#endif

#if USE_ICBC
static bool decode_icbc(int width, int height, DDSFormat format, const void* input, void* output)
{
    const char* src = (const char*)input;
    char* dst = (char*)output;
    uint32_t rgba[16];
    for (int i = 0; i < height; i += 4)
    {
        for (int j = 0; j < width; j += 4)
        {
            if (format == DDSFormat::BC1) {
                icbc::decode_bc1(src, (unsigned char*)rgba);
                src += 8;
                for (int r = 0; r < 4; ++r) {
                    memcpy(dst + (i*width+j)*4 + width * 4 * r, rgba + 4 * r, 4 * 4);
                }
            } else if (format == DDSFormat::BC3) {
                icbc::decode_bc3(src, (unsigned char*)rgba);
                src += 16;
                for (int r = 0; r < 4; ++r) {
                    memcpy(dst + (i*width+j)*4 + width * 4 * r, rgba + 4 * r, 4 * 4);
                }
            } else {
                return false;
            }
        }
    }
    return true;
}
#endif

#if USE_ETCPAK
static bool decode_etcpak(int width, int height, DDSFormat format, const void* input, void* output)
{
    const unsigned char* src = (const unsigned char*)input;
    unsigned char* dst = (unsigned char*)output;
    if (format == DDSFormat::BC1)
    {
        etcpak_BlockData_DecodeDxt1(input, width, height, output);
        return true;
    }
    if (format == DDSFormat::BC3)
    {
        etcpak_BlockData_DecodeDxt5(input, width, height, output);
        return true;
    }
    return false;
}
#endif

#if USE_SQUISH
static bool decode_squish(int width, int height, DDSFormat format, const void* input, void* output)
{
    const unsigned char* src = (const unsigned char*)input;
    unsigned char* dst = (unsigned char*)output;
    int flags = -1;
    switch(format) {
        case DDSFormat::BC1: flags = squish::kDxt1; break;
        case DDSFormat::BC2: flags = squish::kDxt3; break;
        case DDSFormat::BC3: flags = squish::kDxt5; break;
        case DDSFormat::BC4: flags = squish::kBc4; break;
        case DDSFormat::BC5: flags = squish::kBc5; break;
        default:
            return false;
    }
    squish::DecompressImage(dst, width, height, src, flags);
    return true;
}
#endif

#if USE_CONVECTION
static bool decode_convection(int width, int height, DDSFormat format, const void* input, void* output)
{
    const uint8_t* src = (const uint8_t*)input;
    uint8_t* dst = (uint8_t*)output;
    if (format == DDSFormat::BC6HU || format == DDSFormat::BC6HS)
    {
        cvtt::PixelBlockF16 rgba;
        for (int i = 0; i < height; i += 4)
        {
            for (int j = 0; j < width; j += 4)
            {
                cvtt::Internal::BC6HComputer::UnpackOne(rgba, src, format == DDSFormat::BC6HS);
                src += 16;
                for (int c = 0; c < 16; ++c) {
                    float* dst_f = (float*)(dst + (i*width+j)*12 + width*(c/4)*12 + (c&3)*12);
                    dst_f[0] = half_to_float_fast5(rgba.m_pixels[c][0]);
                    dst_f[1] = half_to_float_fast5(rgba.m_pixels[c][1]);
                    dst_f[2] = half_to_float_fast5(rgba.m_pixels[c][2]);
                }
            }
        }
        return true;
    }
    if (format == DDSFormat::BC7)
    {
        cvtt::PixelBlockU8 rgba;
        for (int i = 0; i < height; i += 4)
        {
            for (int j = 0; j < width; j += 4)
            {
                cvtt::Internal::BC7Computer::UnpackOne(rgba, src);
                src += 16;
                for (int r = 0; r < 4; ++r)
                {
                    memcpy(dst + (i*width+j)*4 + width * 4 * r, rgba.m_pixels + 4 * r, 4 * 4);
                }
            }
        }
        return true;
    }
    return false;
}
#endif

typedef bool (DecodeFunc)(int width, int height, DDSFormat format, const void* input, void* output);

struct Decoder
{
    const char* name;
    DecodeFunc* func;
};

static Decoder s_Decoders[] =
{
#if USE_BCDEC
    {"bcdec", decode_bcdec},
#endif
#if USE_BC7ENC_RDO
    {"bc7dec_rdo", decode_bc7dec_rdo},
#endif
#if USE_DXTEX
    {"dxtex", decode_dxtex},
#endif
#if USE_SWIFTSHADER
    {"swiftshader", decode_swiftshader},
#endif
#if USE_ICBC
    {"icbc", decode_icbc},
#endif
#if USE_ETCPAK
    {"etcpak", decode_etcpak},
#endif
#if USE_SQUISH
    {"squish", decode_squish},
#endif
#if USE_CONVECTION
    {"convection", decode_convection},
#endif
};

struct FormatResult
{
    size_t imageCount = 0;
    size_t pixelCount = 0;
    std::map<std::string, double> times;
};
static std::map<DDSFormat, FormatResult> s_Results;

static std::map<std::string, uint32_t> s_ExpectedHashes;

static void ReadExpectedHashes(const char* inputFolder)
{
    std::string hashesPath = std::string(inputFolder) + "/_hashes.txt";
    FILE *f = fopen(hashesPath.c_str(), "rt");
    if (!f)
        return;
    while (true) {
        char path[1000];
        path[0] = 0;
        uint32_t hash = 0;
        if (fscanf(f, "%s %x\n", path, &hash) != 2)
            break;
        if (path[0] == 0)
            break;
        s_ExpectedHashes[path] = hash;
    }
    fclose(f);
}

int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        printf("USAGE: bcn_decode_tester <input_folder> <output_folder>\n");
        return -1;
    }
    ReadExpectedHashes(argv[1]);
    printf("Input folder %s, output folder %s, %i runs, expected hashes %zi\n", argv[1], argv[2], kRuns, s_ExpectedHashes.size());
#if USE_BC7ENC_RDO
    rgbcx::init();
#endif
#if USE_ICBC
    icbc::init();
#endif
    
    namespace fs = std::filesystem;
    fs::path outputdir = argv[2];
    fs::create_directories(outputdir);
    for (auto const& de : fs::recursive_directory_iterator(argv[1]))
    {
        if (!de.is_regular_file())
            continue;
        if (de.path().extension() != ".dds")
            continue;
        fs::path filename = de.path();
        int width = 0, height = 0;
        DDSFormat format = DDSFormat::Unknown;
        void* input_data = nullptr;
        if (!load_dds(filename.string().c_str(), &width, &height, &format, &input_data))
        {
            printf("ERROR: failed to read dds file '%s'\n", filename.string().c_str());
            return 1;
        }
        printf("%s, %ix%i, %s\n", filename.stem().string().c_str(), width, height, get_format_name(format));

        void* output_data = malloc(width * height * 12 * 2);

        auto& format_res = s_Results[format];
        format_res.imageCount++;
        format_res.pixelCount += width * height;
        
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
                format_res.times[dec.name] += dur;
                
                const bool bc6 = format == DDSFormat::BC6HS || format == DDSFormat::BC6HU;
                
                // check if this matches expected hash, if we have it
                {
                    auto hit = s_ExpectedHashes.find(filename.stem().string());
                    if (hit != s_ExpectedHashes.end())
                    {
                        uint32_t hash = XXH3_64bits(output_data, width * height * (bc6 ? 12 : 4)) & 0xffffff;
                        if (hash != hit->second)
                        {
                            printf("error: expected result hash does not match for %s: expected %06x got %06x\n", dec.name, hit->second, hash);
                            //return 1;
                        }
                    }
                }

                if (kWriteOutputImages)
                {
                    const char *ext = bc6 ? ".hdr" : ".tga";
                    std::string outputpath = (outputdir / (filename.stem().string()+"-"+dec.name+ext)).string();
                    if (bc6)
                        stbi_write_hdr(outputpath.c_str(), width, height, 3, (const float*)output_data);
                    else if (format == DDSFormat::BC5)
                    {
                        char* src = (char*)output_data;
                        char* dst = src + width * height * 2;
                        for (int ip = 0; ip < width * height; ++ip) {
                            dst[ip * 3 + 0] = src[ip * 2 + 0];
                            dst[ip * 3 + 1] = src[ip * 2 + 1];
                            dst[ip * 3 + 2] = 0;
                        }
                        stbi_write_tga(outputpath.c_str(), width, height, 3, dst);
                    }
                    else if (format == DDSFormat::BC4)
                        stbi_write_tga(outputpath.c_str(), width, height, 1, output_data);
                    else
                        stbi_write_tga(outputpath.c_str(), width, height, 4, output_data);
                }
            }
        }

        free(input_data);
        free(output_data);
    }
    
    // print stats
    for (const auto& fres : s_Results)
    {
        printf("%s (%zi images, %.1fMpix):\n", get_format_name(fres.first), fres.second.imageCount, fres.second.pixelCount / 1000000.0);
        for (const auto& tres : fres.second.times)
        {
            print_time(tres.first.c_str(), tres.second, fres.second.pixelCount);
        }
    }
    
    // write stats as csv
    printf("CSV results:\n");
    for (const auto& fres : s_Results)
    {
        if (fres.first == DDSFormat::BC6HS)
            continue;
        printf(",%s", get_format_name(fres.first));
    }
    printf("\n");
    for (const Decoder& dec : s_Decoders)
    {
        printf("%s", dec.name);
        for (const auto& fres : s_Results)
        {
            if (fres.first == DDSFormat::BC6HS)
                continue;
            printf(",");
            auto it = fres.second.times.find(dec.name);
            if (it != fres.second.times.end())
            {
                double mpix = fres.second.pixelCount / 1000000.0 * kRuns / it->second;
                printf("%i", (int)mpix);
            }
        }
        printf("\n");
    }

    return 0;
}

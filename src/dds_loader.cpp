#include "dds_loader.h"

#include <stdio.h>
#include <stdlib.h>

#define FOURCC_DDS    FOURCC('D', 'D', 'S', ' ')
#define FOURCC_DX10   FOURCC('D', 'X', '1', '0')

#define BC1_BLOCK_SIZE    8
#define BC2_BLOCK_SIZE    16
#define BC3_BLOCK_SIZE    16
#define BC4_BLOCK_SIZE    8
#define BC5_BLOCK_SIZE    16
#define BC6H_BLOCK_SIZE   16
#define BC7_BLOCK_SIZE    16

#define BC1_COMPRESSED_SIZE(w, h)     ((((w)>>2)*((h)>>2))*BC1_BLOCK_SIZE)
#define BC2_COMPRESSED_SIZE(w, h)     ((((w)>>2)*((h)>>2))*BC2_BLOCK_SIZE)
#define BC3_COMPRESSED_SIZE(w, h)     ((((w)>>2)*((h)>>2))*BC3_BLOCK_SIZE)
#define BC4_COMPRESSED_SIZE(w, h)     ((((w)>>2)*((h)>>2))*BC4_BLOCK_SIZE)
#define BC5_COMPRESSED_SIZE(w, h)     ((((w)>>2)*((h)>>2))*BC5_BLOCK_SIZE)
#define BC6H_COMPRESSED_SIZE(w, h)    ((((w)>>2)*((h)>>2))*BC6H_BLOCK_SIZE)
#define BC7_COMPRESSED_SIZE(w, h)     ((((w)>>2)*((h)>>2))*BC7_BLOCK_SIZE)


typedef struct DDCOLORKEY {
    unsigned int    dwUnused0;
    unsigned int    dwUnused1;
} DDCOLORKEY_t;

typedef struct DDPIXELFORMAT {
    unsigned int    dwSize;
    unsigned int    dwFlags;
    unsigned int    dwFourCC;
    unsigned int    dwRGBBitCount;
    unsigned int    dwRBitMask;
    unsigned int    dwGBitMask;
    unsigned int    dwBBitMask;
    unsigned int    dwRGBAlphaBitMask;
} DDPIXELFORMAT_t;

typedef struct DDSCAPS2 {
    unsigned int    dwCaps;
    unsigned int    dwCaps2;
    unsigned int    dwCaps3;
    unsigned int    dwCaps4;
} DDSCAPS2_t;

typedef struct DDSURFACEDESC2 {
    unsigned int    dwSize;
    unsigned int    dwFlags;
    unsigned int    dwHeight;
    unsigned int    dwWidth;
    union {
        int         lPitch;
        unsigned int dwLinearSize;
    };
    unsigned int    dwBackBufferCount;
    unsigned int    dwMipMapCount;
    unsigned int    dwAlphaBitDepth;
    unsigned int    dwUnused0;
    unsigned int    lpSurface;
    DDCOLORKEY_t    unused0;
    DDCOLORKEY_t    unused1;
    DDCOLORKEY_t    unused2;
    DDCOLORKEY_t    unused3;
    DDPIXELFORMAT_t ddpfPixelFormat;
    DDSCAPS2_t      ddsCaps;
    unsigned int    dwUnused1;
} DDSURFACEDESC2_t;

typedef struct DDS_HEADER_DXT10 {
    unsigned int    dxgiFormat;
    unsigned int    resourceDimension;
    unsigned int    miscFlag;
    unsigned int    arraySize;
    unsigned int    miscFlags2;
} DDS_HEADER_DXT10_t;

bool load_dds(const char* filePath, int* w, int* h, DDSFormat* format, void** compressedData)
{
    unsigned int magic, compressedSize;
    DDSURFACEDESC2_t ddsDesc;
    DDS_HEADER_DXT10_t dx10Desc;

    FILE* f = fopen(filePath, "rb");
    if (!f) {
        return false;
    }

    fread(&magic, 1, 4, f);
    if (FOURCC_DDS != magic) {
        return false;
    }

    fread(&ddsDesc, 1, sizeof(ddsDesc), f);

    *w = ddsDesc.dwWidth;
    *h = ddsDesc.dwHeight;
    *format = DDSFormat::Unknown;
    unsigned int fourcc = 0;
    fourcc = ddsDesc.ddpfPixelFormat.dwFourCC;

    if (ddsDesc.ddpfPixelFormat.dwFourCC == FORMAT_DXT1) {
        compressedSize = BC1_COMPRESSED_SIZE(ddsDesc.dwWidth, ddsDesc.dwHeight);
    } else if (ddsDesc.ddpfPixelFormat.dwFourCC == FORMAT_DXT3 || ddsDesc.ddpfPixelFormat.dwFourCC == FORMAT_DXT5) {
        compressedSize = BC3_COMPRESSED_SIZE(ddsDesc.dwWidth, ddsDesc.dwHeight);
    } else if (ddsDesc.ddpfPixelFormat.dwFourCC == FOURCC_DX10) {
        fread(&dx10Desc, 1, sizeof(dx10Desc), f);

        fourcc = dx10Desc.dxgiFormat;

        if (dx10Desc.dxgiFormat == FORMAT_BC4_UNORM) {
            compressedSize = BC4_COMPRESSED_SIZE(ddsDesc.dwWidth, ddsDesc.dwHeight);
        } else if (dx10Desc.dxgiFormat == FORMAT_BC5_UNORM) {
            compressedSize = BC5_COMPRESSED_SIZE(ddsDesc.dwWidth, ddsDesc.dwHeight);
        } else if (dx10Desc.dxgiFormat == FORMAT_BC6H_UF16 || dx10Desc.dxgiFormat == FORMAT_BC6H_SF16) {
            compressedSize = BC6H_COMPRESSED_SIZE(ddsDesc.dwWidth, ddsDesc.dwHeight);
        } else if (dx10Desc.dxgiFormat == FORMAT_BC7_UNORM) {
            compressedSize = BC7_COMPRESSED_SIZE(ddsDesc.dwWidth, ddsDesc.dwHeight);
        } else {
            return false;
        }
    } else {
        return false;
    }

    *compressedData = malloc(compressedSize);
    fread(*compressedData, 1, compressedSize, f);
    fclose(f);
    
    switch (fourcc) {
        case FORMAT_DXT1: *format = DDSFormat::BC1; break;
        case FORMAT_DXT3: *format = DDSFormat::BC2; break;
        case FORMAT_DXT5: *format = DDSFormat::BC3; break;
        case FORMAT_BC4_UNORM: *format = DDSFormat::BC4; break;
        case FORMAT_BC5_UNORM: *format = DDSFormat::BC5; break;
        case FORMAT_BC6H_UF16: *format = DDSFormat::BC6HU; break;
        case FORMAT_BC6H_SF16: *format = DDSFormat::BC6HS; break;
        case FORMAT_BC7_UNORM: *format = DDSFormat::BC7; break;
    }

    return true;
}

const char* get_format_name(DDSFormat format)
{
    switch (format) {
        case DDSFormat::BC1: return "BC1";
        case DDSFormat::BC2: return "BC2";
        case DDSFormat::BC3: return "BC3";
        case DDSFormat::BC4: return "BC4";
        case DDSFormat::BC5: return "BC5";
        case DDSFormat::BC6HU: return "BC6Hu";
        case DDSFormat::BC6HS: return "BC6Hs";
        case DDSFormat::BC7: return "BC7";
        default: return "Unknown";
    }
}

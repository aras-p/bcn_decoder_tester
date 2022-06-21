#pragma once

enum class DDSFormat
{
    Unknown,
    BC1,
    BC2,
    BC3,
    BC4,
    BC5,
    BC6HU,
    BC6HS,
    BC7,
};

#define FOURCC(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

#define FORMAT_DXT1   FOURCC('D', 'X', 'T', '1')
#define FORMAT_DXT3   FOURCC('D', 'X', 'T', '3')
#define FORMAT_DXT5   FOURCC('D', 'X', 'T', '5')

#define FORMAT_BC4_UNORM   80
#define FORMAT_BC5_UNORM   83
#define FORMAT_BC6H_UF16   95
#define FORMAT_BC6H_SF16   96
#define FORMAT_BC7_UNORM   98

bool load_dds(const char* filePath, int* w, int* h, DDSFormat* format, void** compressedData);
const char* get_format_name(DDSFormat format);


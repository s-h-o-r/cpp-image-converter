#include "bmp_image.h"
#include "pack_defines.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

static const std::array<char, 2> BMP_SIG = {'B', 'M'};

PACKED_STRUCT_BEGIN BitmapFileHeader {
    std::array<char, 2> caption;
    std::uint32_t header_and_data_size;
    std::uint32_t reserved;
    std::uint32_t data_padding;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    std::uint32_t header_size;
    std::int32_t width;
    std::int32_t height;
    std::uint16_t planes_amount;
    std::uint16_t bits_per_pixel;
    std::uint32_t compression_type;
    std::uint32_t bits_in_data;
    std::int32_t h_quality;
    std::int32_t v_quality;
    std::int32_t using_colors_amount;
    std::int32_t signifcant_colors_amount;
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {
    ofstream ofs(file, ios::binary);
    if (!ofs || !image) {
        return false;
    }
    int bmp_stride = GetBMPStride(image.GetWidth());
    BitmapFileHeader file_header = {
        .caption = BMP_SIG,
        .header_and_data_size = static_cast<uint32_t>(54 /*BitmapFileHeader size + BitmapInfoHeader size*/
                                                      + (bmp_stride * image.GetHeight())),
        .reserved = 0,
        .data_padding = 54 /*BitmapFileHeader size + BitmapInfoHeader size*/
    };
    BitmapInfoHeader info_header = {
        .header_size = 40,
        .width = image.GetWidth(),
        .height = image.GetHeight(),
        .planes_amount = 1,
        .bits_per_pixel = 24,
        .compression_type = 0,
        .bits_in_data = static_cast<uint32_t>(bmp_stride * image.GetHeight()),
        .h_quality = 11811,
        .v_quality = 11811,
        .using_colors_amount = 0,
        .signifcant_colors_amount = 0x1000000
    };

    ofs.write(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    ofs.write(reinterpret_cast<char*>(&info_header), sizeof(info_header));

    std::vector<char> buff(bmp_stride);

    for (int y = 0; y < info_header.height; ++y) {
        const Color* line = image.GetLine(info_header.height - 1 - y);
        for (int x = 0; x < info_header.width; ++x) {
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
        }
        std::fill(buff.begin() + info_header.width * 3, buff.end(), 0);
        if (!ofs.write(buff.data(), buff.size())) {
            return false;
        }
    }

    return true;
}

// напишите эту функцию
Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;
    ifs.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    ifs.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));
    
    if (file_header.caption != BMP_SIG) {
        return {};
    }

    img_lib::Image result(info_header.width, info_header.height, Color::Black());
    int bmp_stride = GetBMPStride(info_header.width);
    std::vector<char> buff(bmp_stride);

    for (int y = 0; y < info_header.height; ++y) {
        Color* line = result.GetLine(info_header.height - 1 - y);
        ifs.read(buff.data(), bmp_stride);

        for (int x = 0; x < info_header.width; ++x) {
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
        }
    }

    return result;
}

}  // namespace img_lib

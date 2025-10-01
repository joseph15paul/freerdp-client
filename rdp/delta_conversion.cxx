#include "delta_conversion.hxx"

void agbr_to_rgba(uint8_t *rgba_data, const uint8_t *agbr_data, size_t width, size_t height)
{
    for (size_t i = 0; i < width * height; ++i)
    {
        rgba_data[i * 4 + 0] = agbr_data[i * 4 + 3]; // R
        rgba_data[i * 4 + 1] = agbr_data[i * 4 + 2]; // G
        rgba_data[i * 4 + 2] = agbr_data[i * 4 + 1]; // B
        rgba_data[i * 4 + 3] = 0xff - agbr_data[i * 4 + 0]; // A
    }
}

std::vector<Pixel> rgba_data_to_pixels(const uint8_t *rgba_data, size_t width, size_t height)
{
    std::vector<Pixel> pixels;

    for (size_t i = 0; i < width; ++i)
    {
        for (size_t j = 0; j < height; ++j)
        {
            size_t index = j * width + i;
            Pixel pixel;
            pixel.x = static_cast<uint16_t>(i);
            pixel.y = static_cast<uint16_t>(j);
            pixel.rgba = (rgba_data[index * 4 + 0] << 24) | (rgba_data[index * 4 + 1] << 16) |
                          (rgba_data[index * 4 + 2] << 8) | rgba_data[index * 4 + 3];
            pixels.push_back(pixel);
        }
    }
    return pixels;
    
}

std::vector<Pixel> compute_deltas(const std::vector<Pixel> &old_pixels, const std::vector<Pixel> &new_pixels)
{
    std::vector<Pixel> deltas;

    for (size_t i = 0; i < new_pixels.size(); ++i)
    {
        if (i >= old_pixels.size() || old_pixels[i].rgba != new_pixels[i].rgba)
        {
            deltas.push_back(new_pixels[i]);
        }
    }
    return deltas;
}


#include <cstdint>
#include <cstddef>
#include <vector>

struct DeltaPatch {
    uint16_t top_left_x, top_left_y;
    uint16_t bottom_right_x, bottom_right_y;
    uint8_t* rgba;
};

struct Pixel
{
    uint16_t x,y;
    uint32_t rgba;
};

void agbr_to_rgba(uint8_t *rgba_data, const uint8_t *agbr_data, size_t width, size_t height);

std::vector<Pixel> rgba_data_to_pixels(const uint8_t *rgba_data, size_t width, size_t height);

std::vector<Pixel> compute_deltas(const std::vector<Pixel> &old_pixels, const std::vector<Pixel> &new_pixels);
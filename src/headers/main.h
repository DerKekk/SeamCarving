#ifndef SEAMCARVING_MAIN_H
#define SEAMCARVING_MAIN_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>

int compute_offset(int x, int y, int width, int channels) {
    return (y * width + x) * channels;
}

//Takes image as unsigned char array and converts to unsigned int array
unsigned int* convert_to_int(const unsigned char* img, int width, int height, int channels) {
    auto* ret = (unsigned int*) malloc(width*height* sizeof(unsigned int));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int position = compute_offset(x, y, width, channels);
            unsigned int value = img[position + 3] * 256;

            value += img[position + 2];
            value *= 256;

            value += img[position + 1];
            value *= 256;

            value += img[position];

            ret[position/channels] = value;
        }
    }

    return ret;
}

//Takes image as unsigned int array and converts to unsigned char array
unsigned char* convert_to_char(const unsigned int* img, int width, int height, int targetChannels) {
    auto* ret = (unsigned char*) malloc(width*height*targetChannels*sizeof(unsigned char));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int position = compute_offset(x, y, width, 1);
            unsigned int value = img[position];

            ret[position*targetChannels] = value % 256;
            value /= 256;
            ret[position*targetChannels + 1] = value % 256;
            value /= 256;
            ret[position*targetChannels + 2] = value % 256;
            value /= 256;
            ret[position*targetChannels + 3] = value % 256;
        }
    }

    return ret;
}

//returns grayscale value of pixel at input coordinates x, y
unsigned char get_grayscale_value(int x, int y, const unsigned int* img, int raw_width) {
    int position = compute_offset(x, y, raw_width, 1);
    unsigned int value = img[position];

    int red = value % 256;
    value /= 256;
    int green = value % 256;
    value /= 256;
    int blue = value % 256;

    return static_cast<unsigned char>(0.299 * red + 0.587 * green + 0.114 * blue);
}

//returns unsigned char array with grayscale pixel values from input int array
unsigned char* grayscale(unsigned int* img, int width, int height, int raw_width) {
    auto* grayscale = (unsigned char*) malloc(width*height*sizeof(unsigned char));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int position = compute_offset(x, y, width, 1);
            grayscale[position] = get_grayscale_value(x, y, img, raw_width);
        }
    }

    return grayscale;
}

unsigned short gradient_magnitude(int x, int y, const unsigned char* grayscale, int raw_width) {
    unsigned char left_pixel_grayscale = grayscale[compute_offset(x-1, y, raw_width, 1)];
    unsigned char right_pixel_grayscale = grayscale[compute_offset(x+1, y, raw_width, 1)];
    unsigned char upper_pixel_grayscale = grayscale[compute_offset(x, y-1, raw_width, 1)];
    unsigned char lower_pixel_grayscale = grayscale[compute_offset(x, y+1, raw_width, 1)];

    unsigned char horizontal_gradient = std::abs(right_pixel_grayscale - left_pixel_grayscale);
    unsigned char vertical_gradient = std::abs(upper_pixel_grayscale - lower_pixel_grayscale);

    return horizontal_gradient + vertical_gradient;
}

//Takes image as one-channel unsigned char array in grayscale
//returns vector describing the images energy map using a simple gradient function, with higher value meaning more significant pixel.
//note: The resulting vector will consist of only one channel
void generate_energy_map(std::vector<unsigned short>& energy, unsigned char* grayscale, int width, int height, int raw_width) {

    //Calculate gradient magnitude for all non-border pixels
    for (int y = 1; y < height-1; y++) {
        for (int x = 1; x < width-1; x++) {
            energy[compute_offset(x, y, width, 1)] = gradient_magnitude(x, y, grayscale, raw_width);
        }
    }

    //Set borders to max_value
    for (int y = 0; y < height; y++) {
        energy[compute_offset(0, y, raw_width, 1)] = UINT16_MAX;
        energy[compute_offset(width-1, y, raw_width, 1)] = UINT16_MAX;
    }
}

//Take the same arguments as generate_energy_map as well as a seam.
//Only recalculates energy for pixels affected by the removal of given seam
void recalculate_energy_at_seam(std::vector<unsigned short>& energy, unsigned char* grayscale, const int width, const int height, const int raw_width, std::vector<int>& seam) {
    for (int y = 1; y < height - 1; y++) {
        int x = seam[y];
        int left_pixel_position = compute_offset(x-1, y, raw_width, 1);
        int shifted_pixel_position = compute_offset(x, y, raw_width, 1);

        if (x > 1) {
            energy[left_pixel_position] = gradient_magnitude(x-1, y, grayscale, raw_width);
        }
        else {
            energy[left_pixel_position] = UINT16_MAX;
        }

        if (x < width - 1) {
            energy[shifted_pixel_position] = gradient_magnitude(x, y, grayscale, raw_width);
        }
        else {
            energy[shifted_pixel_position] = UINT16_MAX;
        }
    }
}

//Builds a seam by deciding on the lowest energy path through the image
void build_seam(std::vector<int>& seam, std::vector<int>& seam_weights, const std::vector<unsigned short>& energy, int width, int height, const int raw_width) {
    int seamNo = seam[0];

    for (int y = 1; y < height; y++) {
        int current_x = seam[y-1];
        int position = compute_offset(current_x, y, raw_width, 1);

        int left = current_x > 0 ? energy[position - 1] : INT32_MAX;
        int middle = energy[position];
        int right = current_x < width - 1 ? energy[position + 1] : INT32_MAX;

        if (middle <= left && middle <= right) {
            seam[y] = current_x;
            seam_weights[seamNo] += middle;
            continue;
        }
        if (left <= middle && left <= right) {
            seam[y] = current_x-1;
            seam_weights[seamNo] += left;
            continue;
        }
        seam[y] = current_x+1;
        seam_weights[seamNo] += right;
    }
}

void generate_seams(std::vector<std::vector<int>>& seams, std::vector<int>& seam_weights, std::vector<unsigned short>& energy, int width, int height, int raw_width, int seam_count) {

    //Init vector and first row
    for (int x = 0; x < width; x++) {
        seams[x] = std::vector<int>(height);
        seams[x][0] = x;
        seam_weights[x] = 0;
    }
    seam_weights.resize(width);

    int seam_spacing = width/seam_count;
    if (seam_spacing < 1) {
        seam_spacing = 1;
    }

    for (int x = 0; x < width; x++) {
        if (x % seam_spacing == 0) {
            build_seam(seams[x], seam_weights, energy, width, height, raw_width);
        }
        else {
            seam_weights[x] = INT32_MAX;
        }
    }
}

//Removes seam with the least importance and recalculates energy map at affected pixels
void remove_seam(unsigned int* img, std::vector<std::vector<int>>& seams, const std::vector<int>& seamWeights, std::vector<unsigned short>& energy, int& width, int& height, const int raw_width, unsigned char* grayscale) {
    int index = std::distance(
            std::begin(seamWeights), std::min_element(std::begin(seamWeights), std::end(seamWeights)));

    for (int y = 0; y < height; y++) {
        for (int x = seams[index][y]; x < width - 1; x++) {
            int position = compute_offset(x, y, raw_width, 1);
            img[position] = img[position+1];
            energy[position] = energy[position+1];
            grayscale[position] = grayscale[position+1];
        }
    }
    width--;
    std::cout << "Removed seam no. " << index << ", new width: " << width << std::endl;

    recalculate_energy_at_seam(energy, grayscale, width, height, raw_width, seams[index]);
}

unsigned int* postprocess (unsigned int* img, int width, int height, int raw_width) {
    auto* new_img = (unsigned int*) malloc(width*height*sizeof(int));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int position_old = compute_offset(x, y, raw_width, 1);
            int position_new = compute_offset(x, y, width, 1);
            new_img[position_new] = img[position_old];
        }
    }

    free(img);
    return new_img;
}

#endif //SEAMCARVING_MAIN_H

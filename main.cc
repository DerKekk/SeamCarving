#include "headers/main.h"

int save_energy_map(std::string path) {
    int width, height, channels;

    // Load the image --> will eventually be replaced by cli arguments
    unsigned char* raw_img = stbi_load(path.c_str(), &width, &height, &channels, 4);
    channels = 4;
    if (raw_img == nullptr) {
        std::cerr << "Error" << std::endl;
        return 1;
    }
    std::cout << "Loaded image with width of " << width << ", height of " << height << ", and " << channels << " channels." << std::endl;

    int raw_width = width;

    std::cout << "Converting image" << std::endl;
    unsigned int* img = convert_to_int(raw_img, width, height, channels);
    unsigned char* grayscale_img = grayscale(img, width, height, raw_width);
    channels = 1;
    std::cout << "Convert successful" << std::endl << "-------------" << std::endl;

    std::vector<unsigned short> energy(width*height);

    generate_energy_map(energy, grayscale_img, width, height, channels, raw_width);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int position = compute_offset(x, y, width, 4);

            raw_img[position] = energy[position/4];
            raw_img[position+1] = energy[position/4];
            raw_img[position+2] = energy[position/4];
        }
    }

    channels = 4;
    if (!stbi_write_png("../assets/energy_map.png", width, height, channels, raw_img, width * channels)) {
        std::cerr << "Error in saving the image" << std::endl;
        stbi_image_free(img);
        stbi_image_free(raw_img);
        return 1;
    }

    std::cout << "image saved successfully." << std::endl;

    // Free the image memory
    stbi_image_free(img);
    stbi_image_free(raw_img);

    return 0;


}

int remove_seams(const std::string& path, int n) {
    int width, height, channels;

    // Load the image --> will eventually be replaced by cli arguments
    unsigned char* raw_img = stbi_load(path.c_str(), &width, &height, &channels, 4);
    channels = 4;
    if (raw_img == nullptr) {
        std::cerr << "Error" << std::endl;
        return 1;
    }
    std::cout << "Loaded image with width of " << width << ", height of " << height << ", and " << channels << " channels." << std::endl;

    int raw_width = width;

    std::cout << "Converting image" << std::endl;
    unsigned int* img = convert_to_int(raw_img, width, height, channels);
    unsigned char* grayscale_img = grayscale(img, width, height, raw_width);
    stbi_image_free(raw_img);
    channels = 1;
    std::cout << "Convert successful" << std::endl << "-------------" << std::endl;

    std::vector<unsigned short> energy(width*height);
    std::vector<std::vector<int>> seams(width);
    std::vector<int> seam_weights(width);

    std::cout << "Generating energy map" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    generate_energy_map(energy, grayscale_img, width, height, channels, raw_width);
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = end - start;
    std::cout << "Took: " << dur.count()/1000000 << "ms" << std::endl;

    auto start_total = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < n; i++) {
        std::cout << "Seam no. " << i+1 << std::endl;

        std::cout << "Generated map, building seams" << std::endl;
        start = std::chrono::high_resolution_clock::now();
        generate_seams(seams, seam_weights, energy, width, height, raw_width);
        end = std::chrono::high_resolution_clock::now();
        dur = end - start;
        std::cout << "Took: " << dur.count()/1000000 << "ms" << std::endl;

        std::cout << "Seams built, removing seam" << std::endl;
        start = std::chrono::high_resolution_clock::now();
        remove_seam(img, seams, seam_weights, energy, width, height, raw_width);
        end = std::chrono::high_resolution_clock::now();
        dur = end - start;
        std::cout << "Took: " << dur.count()/1000000 << "ms" << std::endl;

        std::cout << "******" << std::endl;
    }

    std::cout << std::endl;
    auto end_total = std::chrono::high_resolution_clock::now();
    auto dur_total = end_total - start_total;
    std::cout << "Total: " << dur_total.count()/1000000 << "ms" << std::endl;
    std::cout << std::endl;

    std::cout << "Converting image back and saving" << std::endl;
    img = postprocess(img, width, height, raw_width);
    raw_img = convert_to_char(img, width, height, 4);
    channels = 4;

    if (!stbi_write_png("../assets/output.png", width, height, channels, raw_img, width * channels)) {
        std::cerr << "Error in saving the image" << std::endl;
        stbi_image_free(img);
        stbi_image_free(raw_img);
        return 1;
    }

    std::cout << "image saved successfully." << std::endl;

    // Free the image memory
    free(img);
    stbi_image_free(raw_img);

    return 0;
}



int main() {
    return remove_seams("../assets/car.png", 400);

}
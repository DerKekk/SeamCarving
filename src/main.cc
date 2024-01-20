#include "headers/main.h"


int remove_seams(const std::string& path, const std::string& out, int n, int seam_count) {
    int width, height, channels;

    // Load the image
    unsigned char* raw_img = stbi_load(path.c_str(), &width, &height, &channels, 4);
    channels = 4;
    if (raw_img == nullptr) {
        std::cerr << "Error" << std::endl;
        return 1;
    }
    std::cout << "Loaded image with width of " << width << ", height of " << height << ", and " << channels << " channels." << std::endl;

    //Edge case
    if (n > width - 1) {
        std::cout << std::endl << "####" << std::endl << "Can only remove " << width-1 << " pixels." << std::endl;
        std::cout << "Setting to max: " << width - 1 << std::endl << "####" << std::endl << std::endl;

        n = width - 1;
    }


    //Since arrays will not be resized during operations, raw_width must be kept to calculate correct index
    int raw_width = width;

    //Convert image from separate channels as char-array to combined channel int array for efficiency
    std::cout << "Converting image" << std::endl;
    unsigned int* img = convert_to_int(raw_img, width, height, channels);
    unsigned char* grayscale_img = grayscale(img, width, height, raw_width);
    stbi_image_free(raw_img);
    channels = 1;
    std::cout << "Convert successful" << std::endl;

    std::vector<unsigned short> energy(width*height);
    std::vector<std::vector<int>> seams(width);
    std::vector<int> seam_weights(width);

    //Energy map must only be calculated once, and will only be partially recalculated (see main.h: remove_seam())
    std::cout << "Generating energy map" << std::endl;
    generate_energy_map(energy, grayscale_img, width, height, raw_width);
    std::cout << "Energy map generated, commencing seam removal" << std::endl << "-------------" << std::endl << std::endl;

    auto start_total = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < n; i++) {
        std::cout << "Seam no. " << i+1 << std::endl;

        std::cout << "Generated map, building seams" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        generate_seams(seams, seam_weights, energy, width, height, raw_width, seam_count);
        auto end = std::chrono::high_resolution_clock::now();
        auto dur = end - start;
        std::cout << "Took: " << dur.count()/1000000 << "ms" << std::endl;

        std::cout << "Seams built, removing seam" << std::endl;
        start = std::chrono::high_resolution_clock::now();
        remove_seam(img, seams, seam_weights, energy, width, height, raw_width, grayscale_img);
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

    //Convert image back to byte array with separate channels in order to save
    std::cout << "Converting image back and saving" << std::endl;
    img = postprocess(img, width, height, raw_width);
    raw_img = convert_to_char(img, width, height, 4);
    channels = 4;

    if (!stbi_write_png(out.c_str(), width, height, channels, raw_img, width * channels)) {
        std::cerr << "Error in saving the image" << std::endl;
        free(grayscale_img);
        free(img);
        stbi_image_free(raw_img);
        return 1;
    }

    std::cout << "image saved successfully." << std::endl;

    // Free the image memory
    free(grayscale_img);
    free(img);
    stbi_image_free(raw_img);

    return 0;
}



int main(int argc, char* argv[]) {
    if (argc == 5) {
        std::string src(argv[1]);
        std::string out(argv[2]);
        int remove;
        int seams;
        try {
            remove = std::stoi(std::string(argv[3]));
            seams = std::stoi(std::string(argv[4]));
        } catch (std::invalid_argument& invalidArgument){
            std::cout << "Error: invalid input number at <number of pixels to remove> or <number of seams>" << std::endl;
            return 1;
        }


        if (src.ends_with(".png") || src.ends_with(".jpg") || src.ends_with(".JPG")) {
            if (!out.ends_with(".png")) {
                out.append(".png");
            }
            return remove_seams(src, out, remove, seams);
        }
        else {
            std::cout << "Error: supported File formats are .png, .jpg" << std::endl;
            return 1;
        }
    }
    if (argc == 2) {
        std::string in = argv[1];
        if (in == "help") {
            std::cout << "SeamCarving.exe <input path> <output path> <number of pixels to remove> <number of seams>" << std::endl << std::endl;
            std::cout << "<input path>\tPath of input picture, can be absolute or relative." << std::endl;
            std::cout << "\t\tSupported file formats: .png, .jpg/.JPG." << std::endl << std::endl;
            std::cout << "<output path>\tPath to output picture, can be absolute or relative." << std::endl;
            std::cout << "\t\tOutput is always .png." << std::endl << std::endl;
            std::cout << "<number of pixels to remove>" << std::endl << std::endl;
            std::cout << "<output path>\t*advanced setting*" << std::endl;
            std::cout << "\t\tSpecifies the number of seams being calculated, trading accuracy for speed." << std::endl;
            std::cout << "\t\tValue 100 will be best for most cases." << std::endl;
            return 0;
        }
    }
    std::cout << "Error: please use SeamCarving.exe <input path> <output path> <number of pixels to remove> <number of seams>" << std::endl;
    std::cout << "Type 'SeamCarving.exe help' for more info" << std::endl;
    return 1;
}
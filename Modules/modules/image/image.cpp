#include <iostream>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"

#include "../../Vortex.hpp"

/* Implement Lib Functions */

VortexObj load(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    if (args[0]->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 1 string argument");
    }

    int width, height, channels;
    unsigned char *img = stbi_load(args[0]->String.value.c_str(), &width, &height, &channels, 0);
    if(img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);

    node_ptr img_pointer = new_vortex_obj(NodeType::POINTER);
    img_pointer->Pointer.value = img;
    return img_pointer;
}

VortexObj write(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 2) {
        error_and_exit("Function '" + name + "' expects 2 argument");
    }

    if (args[0]->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 1 String argument and one List argument");
    }

    if (args[1]->type != NodeType::LIST) {
        error_and_exit("Function '" + name + "' expects 1 String argument and one List argument");
    }

    if (args[1]->List.elements.size() == 0) {
        return new_vortex_obj(NodeType::NONE);
    }

    std::vector<VortexObj> colors = args[1]->List.elements;

    const int width = colors[0]->List.elements.size();
    const int height = colors.size();
    const int channel_num = 3;

    uint8_t* pixels = new uint8_t[width * height * channel_num];

    int index = 0;
    for (int j = height - 1; j >= 0; --j) {
        for (int i = 0; i < width; ++i) {
            VortexObj color = colors[j]->List.elements[i];
            pixels[index++] = color->Object.properties["r"]->Number.value;
            pixels[index++] = color->Object.properties["g"]->Number.value;
            pixels[index++] = color->Object.properties["b"]->Number.value;
        }
    }

    stbi_write_png(args[0]->String.value.c_str(), width, height, channel_num, pixels, width * channel_num);
    delete[] pixels;

    return new_vortex_obj(NodeType::NONE);
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "load") {
        return load(name, args);
    }
    if (name == "write") {
        return write(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}
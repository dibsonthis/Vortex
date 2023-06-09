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
    unsigned char *img = stbi_load(args[0]->_Node.String().value.c_str(), &width, &height, &channels, 0);
    if(img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);

    node_ptr img_pointer = new_vortex_obj(NodeType::POINTER);
    img_pointer->_Node.Pointer().value = img;
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

    if (args[1]->_Node.List().elements.size() == 0) {
        return new_vortex_obj(NodeType::NONE);
    }

    std::vector<VortexObj> colors = args[1]->_Node.List().elements;

    const int width = colors[0]->_Node.List().elements.size();
    const int height = colors.size();
    const int channel_num = 3;

    uint8_t* pixels = new uint8_t[width * height * channel_num];

    int index = 0;
    for (int j = height - 1; j >= 0; --j) {
        for (int i = 0; i < width; ++i) {
            VortexObj color = colors[j]->_Node.List().elements[i];
            if (color->type != NodeType::OBJECT) {
                error_and_exit("Malformed buffer - expecting color object");
            }
            
            VortexObj r_node = color->_Node.Object().properties["r"];
            VortexObj g_node = color->_Node.Object().properties["g"];
            VortexObj b_node = color->_Node.Object().properties["b"];

            if (!r_node || r_node->type != NodeType::NUMBER) {
                error_and_exit("Malformed buffer - missing 'r' value");
            }
            if (!g_node || g_node->type != NodeType::NUMBER) {
                error_and_exit("Malformed buffer - missing 'g' value");
            }
            if (!b_node || b_node->type != NodeType::NUMBER) {
                error_and_exit("Malformed buffer - missing 'b' value");
            }

            pixels[index++] = r_node->_Node.Number().value;
            pixels[index++] = g_node->_Node.Number().value;
            pixels[index++] = b_node->_Node.Number().value;
        }
    }

    stbi_write_png(args[0]->_Node.String().value.c_str(), width, height, channel_num, pixels, width * channel_num);
    delete[] pixels;

    return new_vortex_obj(NodeType::NONE);
}

VortexObj create(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 2) {
        error_and_exit("Function '" + name + "' expects 2 argument");
    }

    if (args[0]->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects argument 1 to be a Number");
    }

    if (args[1]->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects argument 2 to be a Number");
    }

    uint8_t* pixels = new uint8_t[args[0]->_Node.Number().value * args[1]->_Node.Number().value * 3];

    VortexObj canvas = new_vortex_obj(NodeType::OBJECT);
    canvas->_Node.Object().properties["pixels"] = new_vortex_obj(NodeType::POINTER);
    canvas->_Node.Object().properties["pixels"]->_Node.Pointer().value = pixels;
    canvas->_Node.Object().properties["width"] = args[0];
    canvas->_Node.Object().properties["height"] = args[1];

    return canvas;
}

VortexObj blit(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 4) {
        error_and_exit("Function '" + name + "' expects 2 argument");
    }

    if (args[0]->type != NodeType::OBJECT) {
        error_and_exit("Function '" + name + "' expects argument 1 to be a Object");
    }

    if (args[1]->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects argument 2 to be a Number");
    }

    if (args[2]->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects argument 3 to be a Number");
    }

    if (args[3]->type != NodeType::OBJECT) {
        error_and_exit("Function '" + name + "' expects argument 4 to be a Object");
    }
    
    VortexObj pixel_ptr = args[0]->_Node.Object().properties["pixels"];

    if (!pixel_ptr || (pixel_ptr && pixel_ptr->type != NodeType::POINTER)) {
        error_and_exit("Function '" + name + "' missing pixel pointer");
    }

    VortexObj width = args[0]->_Node.Object().properties["width"];

    if (!width || (width && width->type != NodeType::NUMBER)) {
        error_and_exit("Function '" + name + "' missing width property");
    }

    VortexObj height = args[0]->_Node.Object().properties["height"];

    if (!height || (height && height->type != NodeType::NUMBER)) {
        error_and_exit("Function '" + name + "' missing height property");
    }

    uint8_t* pixels = (uint8_t*)pixel_ptr->_Node.Pointer().value;

    VortexObj color = args[3];
    VortexObj r = color->_Node.Object().properties["r"];
    VortexObj g = color->_Node.Object().properties["g"];
    VortexObj b = color->_Node.Object().properties["b"];

    if (!r || (r && r->type != NodeType::NUMBER)) {
        error_and_exit("Function '" + name + "' has malformed color property");
    }

    if (!g || (g && g->type != NodeType::NUMBER)) {
        error_and_exit("Function '" + name + "' has malformed color property");
    }

    if (!b || (b && b->type != NodeType::NUMBER)) {
        error_and_exit("Function '" + name + "' has malformed color property");
    }

    int x = args[1]->_Node.Number().value;
    int y = args[2]->_Node.Number().value;
    int index_r = (3 * (y * width->_Node.Number().value + x));
    int index_g = (3 * (y * width->_Node.Number().value + x) + 1);
    int index_b = (3 * (y * width->_Node.Number().value + x) + 2);

    pixels[index_r] = r->_Node.Number().value;
    pixels[index_g] = g->_Node.Number().value;
    pixels[index_b] = b->_Node.Number().value;

    return new_vortex_obj(NodeType::NONE);
}

VortexObj save(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 2) {
        error_and_exit("Function '" + name + "' expects 2 argument");
    }

    if (args[0]->type != NodeType::OBJECT) {
        error_and_exit("Function '" + name + "' expects argument 1 to be a Object");
    }

    if (args[1]->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects argument 2 to be a String");
    }

    VortexObj pixel_ptr = args[0]->_Node.Object().properties["pixels"];

    if (!pixel_ptr || (pixel_ptr && pixel_ptr->type != NodeType::POINTER)) {
        error_and_exit("Function '" + name + "' missing pixel pointer");
    }

    VortexObj width = args[0]->_Node.Object().properties["width"];

    if (!width || (width && width->type != NodeType::NUMBER)) {
        error_and_exit("Function '" + name + "' missing width property");
    }

    VortexObj height = args[0]->_Node.Object().properties["height"];

    if (!height || (height && height->type != NodeType::NUMBER)) {
        error_and_exit("Function '" + name + "' missing height property");
    }

    uint8_t* pixels = (uint8_t*)pixel_ptr->_Node.Pointer().value;

    stbi_write_png(args[1]->_Node.String().value.c_str(), width->_Node.Number().value, height->_Node.Number().value, 3, pixels, width->_Node.Number().value * 3);
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
    if (name == "create") {
        return create(name, args);
    }
    if (name == "blit") {
        return blit(name, args);
    }
    if (name == "save") {
        return save(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}
#include "include/Vortex.hpp"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_Image.h"
#include "include/SDL2/SDL_ttf.h"

extern "C" Value initSDL(std::vector<Value>& args) {
    int num_required_args = 0;

    if (args.size() != num_required_args) {
        error("Function 'initSDL' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    TTF_Init();
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    int init = SDL_Init(SDL_INIT_EVERYTHING);

    return number_val(init);
}

extern "C" Value createWindow(std::vector<Value>& args) {
    int num_required_args = 6;

    if (args.size() != num_required_args) {
        error("Function 'createWindow' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value windowName = args[0];
    Value xPos = args[1];
    Value yPos = args[2];
    Value width = args[3];
    Value height = args[4];
    Value flags = args[5];

    if (!windowName.is_string()) {
        error("Function 'createWindow' expects arg 'windowName' to be a string");
    }

    if (!xPos.is_number()) {
        error("Function 'createWindow' expects arg 'xPos' to be a number");
    }

    if (!yPos.is_number()) {
        error("Function 'createWindow' expects arg 'yPos' to be a number");
    }

    if (!width.is_number()) {
        error("Function 'createWindow' expects arg 'width' to be a number");
    }

    if (!height.is_number()) {
        error("Function 'createWindow' expects arg 'height' to be a number");
    }

    if (!flags.is_number()) {
        error("Function 'createWindow' expects arg 'flags' to be a number");
    }

    SDL_Window* window = SDL_CreateWindow(windowName.get_string().c_str(), 
                                      (int)xPos.get_number(), 
                                      (int)yPos.get_number(), 
                                      (int)width.get_number(),
                                      (int)height.get_number(), 
                                      SDL_WINDOW_SHOWN | (int)flags.get_number());

    SDL_SetWindowResizable(window, SDL_TRUE);

    Value windowPointer = pointer_val();
    windowPointer.get_pointer()->value = window;

    auto sdl_error = std::string(SDL_GetError());
    if (sdl_error != "") {
        error("SDL Error (createWindow): " + sdl_error);
    }

    std::cout << "Initalized window: " << window << "\n";
    return windowPointer;
}

extern "C" Value createRenderer(std::vector<Value>& args) {
    int num_required_args = 2;

    if (args.size() != num_required_args) {
        error("Function 'createRenderer' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value window = args[0];
    Value index = args[1];

    if (!window.is_pointer()) {
        error("Function 'createRenderer' expects arg 'window' to be a pointer");
    }

    if (!index.is_number()) {
        error("Function 'createRenderer' expects arg 'index' to be a number");
    }

    SDL_Window* windowPtr = (SDL_Window*)window.get_pointer()->value;

    SDL_Renderer* renderer = SDL_CreateRenderer(windowPtr, (int)index.get_number(), SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    Value rendererPointer = pointer_val();
    rendererPointer.get_pointer()->value = renderer;

    auto sdl_error = std::string(SDL_GetError());
    if (sdl_error != "") {
        error("SDL Error (createRenderer): " + sdl_error);
    }

    std::cout << "Initalized renderer: " << renderer << "\n";
    return rendererPointer;
}

extern "C" Value pollEvent(std::vector<Value>& args) {

    SDL_Event e;
    int status = SDL_PollEvent(&e);

    Value event = object_val();
    auto& event_obj = event.get_object();
    event_obj->values["type"] = number_val(e.type);
    // Button
    event_obj->values["button"] = object_val();
    event_obj->values["button"].get_object()->values["button"] = number_val(e.button.button);
    event_obj->values["button"].get_object()->values["clicks"] = number_val(e.button.clicks);
    event_obj->values["button"].get_object()->values["x"] = number_val(e.button.x);
    event_obj->values["button"].get_object()->values["y"] = number_val(e.button.y);
    // Motion
    event_obj->values["motion"] = object_val();
    event_obj->values["motion"].get_object()->values["x"] = number_val(e.motion.x);
    event_obj->values["motion"].get_object()->values["y"] = number_val(e.button.y);
    // Keyboard
    event_obj->values["key"] = object_val();
    event_obj->values["key"].get_object()->values["type"] = number_val(e.key.type);
    event_obj->values["key"].get_object()->values["state"] = number_val(e.key.state);
    event_obj->values["key"].get_object()->values["repeat"] = number_val(e.key.repeat);
    event_obj->values["key"].get_object()->values["keysm"] = object_val();
    event_obj->values["key"].get_object()->values["keysm"].get_object()->values["mod"] = number_val(e.key.keysym.mod);
    event_obj->values["key"].get_object()->values["keysm"].get_object()->values["scancode"] = number_val(e.key.keysym.scancode);
    event_obj->values["key"].get_object()->values["keysm"].get_object()->values["sym"] = number_val(e.key.keysym.sym);
    // Window
    event_obj->values["window"] = object_val();
    event_obj->values["window"].get_object()->values["event"] = number_val(e.window.event);
    event_obj->values["window"].get_object()->values["data1"] = number_val(e.window.data1);
    event_obj->values["window"].get_object()->values["data2"] = number_val(e.window.data2);

    Value eventStruct = object_val();
    eventStruct.get_object()->values["status"] = number_val(status);
    eventStruct.get_object()->values["event"] = event;

    return eventStruct;
}

extern "C" Value renderClear(std::vector<Value>& args) {
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error("Function 'renderClear' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value renderer = args[0];

    if (!renderer.is_pointer()) {
        error("Function 'renderClear' expects arg 'renderer' to be a pointer");
    }

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer.get_pointer()->value;

    int status = SDL_RenderClear(rendererPtr);

    if (status != 0) {
        error("SDL Error (renderClear): " + std::string(SDL_GetError()) + "\n");
    }

    return none_val();
}

extern "C" Value renderPresent(std::vector<Value>& args) {
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error("Function 'renderPresent' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value renderer = args[0];

    if (!renderer.is_pointer()) {
        error("Function 'renderPresent' expects arg 'renderer' to be a pointer");
    }

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer.get_pointer()->value;

    SDL_RenderPresent(rendererPtr);

    return none_val();
}

extern "C" Value getKeyName(std::vector<Value>& args) {
    int num_required_args = 1;

    if (args.size() != num_required_args) {
        error("Function 'getKeyName' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value keyCode = args[0];

    if (!keyCode.is_number()) {
        error("Function 'getKeyName' expects arg 'keyCode' to be a number");
    }

    const char* key_name = SDL_GetKeyName(keyCode.get_number());
    Value name_node = string_val(std::string(key_name));
    return name_node;
}

// VortexObj destroy_renderer(std::string name, std::vector<VortexObj> args) {
//     if (args.size() != 1) {
//         error_and_exit("Function '" + name + "' expects 1 argument");
//     }

//     VortexObj rendererPointer = args[0];

//     if (rendererPointer->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects 1 pointer argument");
//     }

//     SDL_Renderer* renderer = (SDL_Renderer*)rendererPointer->_Node.Pointer().value;

//     SDL_DestroyRenderer(renderer);

//     auto error = std::string(SDL_GetError());
//     if (error != "") {
//         std::cout << "SDL Error (" << name << "): " << error << "\n";
//         return new_vortex_obj(NodeType::NONE);
//     }

//     std::cout << "Renderer destroyed\n";

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj set_render_draw_color(std::string name, std::vector<VortexObj> args) {
//     if (args.size() != 5) {
//         error_and_exit("Function '" + name + "' expects 5 arguments");
//     }

//     VortexObj renderer = args[0];
//     VortexObj r = args[1];
//     VortexObj g = args[2];
//     VortexObj b = args[3];
//     VortexObj a = args[4];

//     if (renderer->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects arg 1 to be a pointer");
//     }

//     if (r->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects arg 2 to be a number");
//     }

//     if (g->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects arg 3 to be a number");
//     }

//     if (b->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects arg 4 to be a number");
//     }

//     if (a->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects arg 5 to be a number");
//     }

//     SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

//     int status = SDL_SetRenderDrawColor(rendererPtr, r->_Node.Number().value, g->_Node.Number().value, b->_Node.Number().value, a->_Node.Number().value);

//     if (status != 0) {
//         std::cout << "SDL Error (" << name << "): " << SDL_GetError() << "\n";
//         return new_vortex_obj(NodeType::NONE);
//     }

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj draw_point(std::string name, std::vector<VortexObj> args) {
//     if (args.size() != 3) {
//         error_and_exit("Function '" + name + "' expects 2 arguments");
//     }

//     VortexObj renderer = args[0];
//     VortexObj xPos = args[1];
//     VortexObj yPos = args[2];

//     if (renderer->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects first argument to be a pointer");
//     }

//     if (xPos->type != NodeType::NUMBER || yPos->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects 2 number arguments");
//     }

//     SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

//     int draw = SDL_RenderDrawPoint(rendererPtr, xPos->_Node.Number().value, yPos->_Node.Number().value);

//     if (draw != 0) {
//         error_and_exit("Function '" + name + "' failed");
//     }

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj draw_line(std::string name, std::vector<VortexObj> args) {
//     if (args.size() != 5) {
//         error_and_exit("Function '" + name + "' expects 5 arguments");
//     }

//     VortexObj renderer = args[0];
//     VortexObj x1 = args[1];
//     VortexObj y1 = args[2];
//     VortexObj x2 = args[3];
//     VortexObj y2 = args[4];

//     if (renderer->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects first argument to be a pointer");
//     }

//     if (x1->type != NodeType::NUMBER || y1->type != NodeType::NUMBER || x2->type != NodeType::NUMBER || y2->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects 4 number arguments");
//     }

//     SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

//     int draw = SDL_RenderDrawLineF(rendererPtr, x1->_Node.Number().value, y1->_Node.Number().value, x2->_Node.Number().value, y2->_Node.Number().value);

//     if (draw != 0) {
//         error_and_exit("Function '" + name + "' failed");
//     }

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj draw_rect(std::string name, std::vector<VortexObj> args) {
//     if (args.size() != 5) {
//         error_and_exit("Function '" + name + "' expects 5 arguments");
//     }

//     VortexObj renderer = args[0];
//     VortexObj x = args[1];
//     VortexObj y = args[2];
//     VortexObj w = args[3];
//     VortexObj h = args[4];

//     if (renderer->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects first argument to be a pointer");
//     }

//     if (x->type != NodeType::NUMBER || y->type != NodeType::NUMBER || w->type != NodeType::NUMBER || h->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects 4 number arguments");
//     }

//     SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

//     SDL_FRect rect;
//     rect.x = x->_Node.Number().value;
//     rect.y = y->_Node.Number().value;
//     rect.w = w->_Node.Number().value;
//     rect.h = h->_Node.Number().value;

//     int draw = SDL_RenderDrawRectF(rendererPtr, &rect);
//     int fill = SDL_RenderFillRectF(rendererPtr, &rect);

//     if (draw != 0) {
//         error_and_exit("Function '" + name + "' failed");
//     }

//     if (fill != 0) {
//         error_and_exit("Function '" + name + "' failed");
//     }

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj draw_geometry(std::string name, std::vector<VortexObj> args) {
//     int args_count = 3;

//     if (args.size() != args_count) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(args_count) + " argument(s)");
//     }

//     VortexObj renderer = args[0];
//     VortexObj vertices = args[1];
//     VortexObj texture = args[2];

//     if (renderer->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects 'renderer' argument to be a pointer");
//     }

//     if (texture->type != NodeType::POINTER && texture->type != NodeType::NONE) {
//         error_and_exit("Function '" + name + "' expects 'texture' argument to be a pointer or None");
//     }

//     if (vertices->type != NodeType::LIST) {
//         error_and_exit("Function '" + name + "' expects 'vertices' argument to be a List");
//     }

//     SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;
//     SDL_Texture* texturePtr = NULL;
//     if (texture->type == NodeType::POINTER) {
//         texturePtr = (SDL_Texture*)texture->_Node.Pointer().value;
//     }
//     std::vector<SDL_Vertex> sdl_vertices;

//     for (VortexObj vertex : vertices->_Node.List().elements) {
//         SDL_Vertex v;
//         v.color.r = vertex->_Node.Object().properties["color"]->_Node.Object().properties["r"]->_Node.Number().value;
//         v.color.g = vertex->_Node.Object().properties["color"]->_Node.Object().properties["g"]->_Node.Number().value;
//         v.color.b = vertex->_Node.Object().properties["color"]->_Node.Object().properties["b"]->_Node.Number().value;
//         v.color.a = vertex->_Node.Object().properties["color"]->_Node.Object().properties["a"]->_Node.Number().value;

//         v.position.x = vertex->_Node.Object().properties["position"]->_Node.Object().properties["x"]->_Node.Number().value;
//         v.position.y = vertex->_Node.Object().properties["position"]->_Node.Object().properties["y"]->_Node.Number().value;

//         sdl_vertices.push_back(v);
//     }

//     int render = SDL_RenderGeometry(rendererPtr, texturePtr, sdl_vertices.data(), sdl_vertices.size(), NULL, 0);

//     if (render != 0) {
//         std::cout << "SDL Error (" << name << "): " << SDL_GetError() << "\n";
//     }

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj load_texture(std::string name, std::vector<VortexObj> args) {
//     if (args.size() != 2) {
//         error_and_exit("Function '" + name + "' expects 2 arguments");
//     }

//     VortexObj renderer = args[0];
//     VortexObj file_path = args[1];

//     if (renderer->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects argument 1 to be a pointer");
//     }

//     if (file_path->type != NodeType::STRING) {
//         error_and_exit("Function '" + name + "' expects argument 2 to be a string");
//     }

//     SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

//     SDL_Texture* texture = IMG_LoadTexture(rendererPtr, file_path->_Node.String().value.c_str());
    
//     if (!texture) {
//         //error_and_exit("Problem loading texture: " + std::string(SDL_GetError()));
//         std::cout << "SDL Error (" << name << "): " << SDL_GetError() << "\n";
//         //return new_vortex_obj(NodeType::NONE);
//     }

//     VortexObj textureNode = new_vortex_obj(NodeType::POINTER);
//     textureNode->_Node.Pointer().value = texture;

//     return textureNode;
// }

// VortexObj load_text(std::string name, std::vector<VortexObj> args) {
//     // loadText(renderer, font, size, r, g, b, text)

//     if (args.size() != 7) {
//         error_and_exit("Function '" + name + "' expects 7 arguments");
//     }

//     VortexObj renderer = args[0];
//     VortexObj font_path = args[1];
//     VortexObj font_size = args[2];
//     VortexObj color_r = args[3];
//     VortexObj color_g = args[4];
//     VortexObj color_b = args[5];
//     VortexObj text = args[6];

//     if (renderer->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects argument 1 to be a pointer");
//     }

//     if (font_path->type != NodeType::STRING) {
//         error_and_exit("Function '" + name + "' expects argument 2 to be a string");
//     }

//     if (font_size->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects argument 3 to be a number");
//     }

//     if (color_r->type != NodeType::NUMBER || color_g->type != NodeType::NUMBER || color_b->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects arguments 4-6 to be a number");
//     }

//     if (text->type != NodeType::STRING) {
//         error_and_exit("Function '" + name + "' expects argument 7 to be a string");
//     }

//     SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

//     SDL_Color color = {
//         color_r->_Node.Number().value,
//         color_g->_Node.Number().value,
//         color_b->_Node.Number().value 
//     };
    
//     TTF_Font* font = TTF_OpenFont(font_path->_Node.String().value.c_str(), font_size->_Node.Number().value);

//     if (!font) {
//         //error_and_exit("Problem loading texture: " + std::string(SDL_GetError()));
//         std::cout << "SDL Error (" << name << "): " << SDL_GetError() << "\n";
//         //return new_vortex_obj(NodeType::NONE);
//     }

//     SDL_Surface* surfaceMessage =
//     TTF_RenderText_Solid(font, text->_Node.String().value.c_str(), color); 

//     if (!surfaceMessage) {
//         //error_and_exit("Problem loading texture: " + std::string(SDL_GetError()));
//         std::cout << "SDL Error (" << name << "): " << SDL_GetError() << "\n";
//         //return new_vortex_obj(NodeType::NONE);
//     }

//     SDL_Texture* texture = SDL_CreateTextureFromSurface(rendererPtr, surfaceMessage);

//     if (!texture) {
//         //error_and_exit("Problem loading texture: " + std::string(SDL_GetError()));
//         std::cout << "SDL Error (" << name << "): " << SDL_GetError() << "\n";
//         //return new_vortex_obj(NodeType::NONE);
//     }

//     VortexObj textureNode = new_vortex_obj(NodeType::POINTER);
//     textureNode->_Node.Pointer().value = texture;

//     SDL_FreeSurface(surfaceMessage);
//     TTF_CloseFont(font);

//     return textureNode;
// }

// VortexObj destroy_texture(std::string name, std::vector<VortexObj> args) {
//     if (args.size() != 1) {
//         error_and_exit("Function '" + name + "' expects 1 argument");
//     }

//     VortexObj texturePtr = args[0];

//     if (texturePtr->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects 1 pointer argument");
//     }

//     SDL_Texture* texture = (SDL_Texture*)texturePtr->_Node.Pointer().value;

//     SDL_DestroyTexture(texture);

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj render_copy(std::string name, std::vector<VortexObj> args) {
//     if (args.size() != 10) {
//         error_and_exit("Function '" + name + "' expects 9 arguments");
//     }

//     VortexObj renderer = args[0];
//     VortexObj texture = args[1];

//     VortexObj src_x = args[2];
//     VortexObj src_y = args[3];
//     VortexObj src_w = args[4];
//     VortexObj src_h = args[5];

//     VortexObj dest_x = args[6];
//     VortexObj dest_y = args[7];
//     VortexObj dest_w = args[8];
//     VortexObj dest_h = args[9];

//     if (renderer->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects first argument to be a pointer");
//     }

//     if (texture->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects second argument to be a pointer");
//     }

//     if (src_x->type != NodeType::NUMBER || src_y->type != NodeType::NUMBER || src_w->type != NodeType::NUMBER || src_h->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects 4 number arguments");
//     }

//     if (dest_x->type != NodeType::NUMBER || dest_y->type != NodeType::NUMBER || dest_w->type != NodeType::NUMBER || dest_h->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects 4 number arguments");
//     }

//     SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

//     SDL_Rect src_rect;
//     src_rect.x = src_x->_Node.Number().value;
//     src_rect.y = src_y->_Node.Number().value;
//     src_rect.w = src_w->_Node.Number().value;
//     src_rect.h = src_h->_Node.Number().value;

//     bool src_rect_null = src_rect.x == -1 && src_rect.y == -1 && src_rect.w == -1 && src_rect.h == -1;

//     SDL_Rect dest_rect;
//     dest_rect.x = dest_x->_Node.Number().value;
//     dest_rect.y = dest_y->_Node.Number().value;
//     dest_rect.w = dest_w->_Node.Number().value;
//     dest_rect.h = dest_h->_Node.Number().value;

//     bool dest_rect_null = dest_rect.x == -1 && dest_rect.y == -1 && dest_rect.w == -1 && dest_rect.h == -1;

//     SDL_Texture* texturePtr = (SDL_Texture*)texture->_Node.Pointer().value;
//     SDL_RenderCopy(rendererPtr, texturePtr, src_rect_null ? NULL : &src_rect, dest_rect_null ? NULL : &dest_rect);

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj render_copy_ex(std::string name, std::vector<VortexObj> args) {
//     if (args.size() != 14) {
//         error_and_exit("Function '" + name + "' expects 14 arguments");
//     }

//     VortexObj renderer = args[0];
//     VortexObj texture = args[1];

//     VortexObj src_x = args[2];
//     VortexObj src_y = args[3];
//     VortexObj src_w = args[4];
//     VortexObj src_h = args[5];

//     VortexObj dest_x = args[6];
//     VortexObj dest_y = args[7];
//     VortexObj dest_w = args[8];
//     VortexObj dest_h = args[9];

//     VortexObj angle = args[10];

//     VortexObj rot_x = args[11];
//     VortexObj rot_y = args[12];

//     VortexObj flip = args[13];

//     if (renderer->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects first argument to be a pointer");
//     }

//     if (texture->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects second argument to be a pointer");
//     }

//     if (src_x->type != NodeType::NUMBER || src_y->type != NodeType::NUMBER || src_w->type != NodeType::NUMBER || src_h->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects 4 number arguments");
//     }

//     if (dest_x->type != NodeType::NUMBER || dest_y->type != NodeType::NUMBER || dest_w->type != NodeType::NUMBER || dest_h->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects 4 number arguments");
//     }

//     if (angle->type != NodeType::NUMBER || rot_x->type != NodeType::NUMBER || rot_y->type != NodeType::NUMBER || flip->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects 4 number arguments");
//     }

//     SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

//     SDL_Rect src_rect;
//     src_rect.x = src_x->_Node.Number().value;
//     src_rect.y = src_y->_Node.Number().value;
//     src_rect.w = src_w->_Node.Number().value;
//     src_rect.h = src_h->_Node.Number().value;

//     bool src_rect_null = src_rect.x == -1 && src_rect.y == -1 && src_rect.w == -1 && src_rect.h == -1;

//     SDL_Rect dest_rect;
//     dest_rect.x = dest_x->_Node.Number().value;
//     dest_rect.y = dest_y->_Node.Number().value;
//     dest_rect.w = dest_w->_Node.Number().value;
//     dest_rect.h = dest_h->_Node.Number().value;

//     bool dest_rect_null = dest_rect.x == -1 && dest_rect.y == -1 && dest_rect.w == -1 && dest_rect.h == -1;

//     SDL_Point center;
//     int rotX = rot_x->_Node.Number().value;
//     int rotY = rot_y->_Node.Number().value;
    
//     if (rotX == -1) {
//         center.x = dest_rect.w / 2;
//     } else {
//         center.x = rotX;
//     }

//     if (rotY == -1) {
//         center.y = dest_rect.h / 2;
//     } else {
//         center.y = rotY;
//     }

//     SDL_RendererFlip renderFlip = (SDL_RendererFlip)flip->_Node.Number().value;

//     SDL_Texture* texturePtr = (SDL_Texture*)texture->_Node.Pointer().value;
//     SDL_RenderCopyEx(rendererPtr, texturePtr, src_rect_null ? NULL : &src_rect, dest_rect_null ? NULL : &dest_rect, angle->_Node.Number().value, &center, renderFlip);

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj get_scancode(std::string name, std::vector<VortexObj> args) {
//     if (args.size() != 1) {
//         error_and_exit("Function '" + name + "' expects 1 argument");
//     }

//     VortexObj key_name = args[0];

//     if (key_name->type != NodeType::STRING) {
//         error_and_exit("Function '" + name + "' expects 1 string argument");
//     }

//     int scancode = SDL_GetScancodeFromName(key_name->_Node.String().value.c_str());
//     VortexObj scancode_node = new_number_node(scancode);
//     return scancode_node;
// }

// VortexObj sdl_delay(std::string name, std::vector<VortexObj> args) {

//     if (args.size() != 1) {
//         error_and_exit("Function '" + name + "' expects 1 argument");
//     }

//     VortexObj time = args[0];

//     if (time->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects 1 number argument");
//     }

//     SDL_Delay(time->_Node.Number().value);

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj sdl_quit(std::string name, std::vector<VortexObj> args) {

//     if (args.size() != 0) {
//         error_and_exit("Function '" + name + "' expects 0 arguments");
//     }

//     SDL_Quit();

//     auto error = std::string(SDL_GetError());
//     if (error != "") {
//         std::cout << "SDL Error (" << name << "): " << error << "\n";
//         return new_vortex_obj(NodeType::NONE);
//     }

//     std::cout << "SDL Quit\n";

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj sdl_get_performance_counter(std::string name, std::vector<VortexObj> args) {

//     if (args.size() != 0) {
//         error_and_exit("Function '" + name + "' expects 0 arguments");
//     }

//     node_ptr counter = new_number_node(SDL_GetPerformanceCounter());

//     return counter;
// }

// VortexObj sdl_get_performance_frequency(std::string name, std::vector<VortexObj> args) {

//     if (args.size() != 0) {
//         error_and_exit("Function '" + name + "' expects 0 arguments");
//     }

//     node_ptr frequency = new_number_node(SDL_GetPerformanceFrequency());

//     return frequency;
// }

// VortexObj sdl_get_keyboard_state(std::string name, std::vector<VortexObj> args) {

//     if (args.size() != 0) {
//         error_and_exit("Function '" + name + "' expects 0 arguments");
//     }

//     int len;
//     const Uint8* keyboard_state = SDL_GetKeyboardState(&len);

//     node_ptr keyboard_state_list = new_vortex_obj(NodeType::LIST);
//     for (int i = 0; i < len; i++) {
//         keyboard_state_list->_Node.List().elements.push_back(new_boolean_node(keyboard_state[i]));
//     }

//     return keyboard_state_list;
// }

// VortexObj sdl_get_window_size(std::string name, std::vector<VortexObj> args) {

//     if (args.size() != 1) {
//         error_and_exit("Function '" + name + "' expects 1 argument");
//     }

//     node_ptr windowPointer = args[0];

//     if (windowPointer->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects 1 pointer argument");
//     }

//     SDL_Window* window = (SDL_Window*)windowPointer->_Node.Pointer().value;

//     int w, h;

//     SDL_GetWindowSize(window, &w, &h);

//     node_ptr sizeObj = new_vortex_obj(NodeType::OBJECT);
//     sizeObj->_Node.Object().properties["w"] = new_number_node(w);
//     sizeObj->_Node.Object().properties["h"] = new_number_node(h);

//     return sizeObj;
// }

// VortexObj sdl_show_cursor(std::string name, std::vector<VortexObj> args) {

//     if (args.size() != 1) {
//         error_and_exit("Function '" + name + "' expects 1 argument");
//     }

//     if (args[0]->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects 1 number argument");
//     }

//     int state = SDL_ShowCursor((int)args[0]->_Node.Number().value);
//     return new_number_node(state);
// }

// // GL Functions

// VortexObj gl_enable(std::string name, std::vector<VortexObj> args) {

//     int args_count = 1;

//     if (args.size() != args_count) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(args_count) + " argument(s)");
//     }

//     if (args[0]->type != NodeType::NUMBER) {
//         error_and_exit("Argument 'glEnum' should be a number"); 
//     }

//     GLenum gl_enum = args[0]->_Node.Number().value;

//     glEnable(gl_enum);

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj gl_clear(std::string name, std::vector<VortexObj> args) {

//     int args_count = 1;

//     if (args.size() != args_count) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(args_count) + " argument(s)");
//     }

//     if (args[0]->type != NodeType::NUMBER) {
//         error_and_exit("Argument 'GLbitfield' should be a number"); 
//     }

//     GLenum gl_bit_field = args[0]->_Node.Number().value;

//     glClear(gl_bit_field);

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj gl_viewport(std::string name, std::vector<VortexObj> args) {

//     int args_count = 4;

//     if (args.size() != args_count) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(args_count) + " argument(s)");
//     }

//     if (args[0]->type != NodeType::NUMBER || args[1]->type != NodeType::NUMBER || args[2]->type != NodeType::NUMBER || args[3]->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(args_count) + " number argument(s)");
//     }

//     int x = args[0]->_Node.Number().value;
//     int y = args[1]->_Node.Number().value;
//     int w = args[2]->_Node.Number().value;
//     int h = args[3]->_Node.Number().value;

//     glViewport(x, y, w, h);

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj gl_create_context(std::string name, std::vector<VortexObj> args) {

//     int args_count = 1;

//     if (args.size() != args_count) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(args_count) + " argument(s)");
//     }

//     if (args[0]->type != NodeType::POINTER) {
//         error_and_exit("Argument 'window' should be a pointer"); 
//     }

//     SDL_Window* window = (SDL_Window*)args[0]->_Node.Pointer().value;

//     SDL_GLContext context = SDL_GL_CreateContext(window);

//     VortexObj context_ptr = new_vortex_obj(NodeType::POINTER);
//     context_ptr->_Node.Pointer().value = context;

//     return context_ptr;
// }

// VortexObj gl_swap_window(std::string name, std::vector<VortexObj> args) {

//     int args_count = 1;

//     if (args.size() != args_count) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(args_count) + " argument(s)");
//     }

//     if (args[0]->type != NodeType::POINTER) {
//         error_and_exit("Argument 'GLbitfield' should be a pointer"); 
//     }

//     SDL_Window* window = (SDL_Window*)args[0]->_Node.Pointer().value;

//     SDL_GL_SwapWindow(window);

//     return new_vortex_obj(NodeType::NONE);
// }

// VortexObj gl_clear_color(std::string name, std::vector<VortexObj> args) {

//     int args_count = 4;

//     if (args.size() != args_count) {
//         error_and_exit("Function '" + name + "' expects " + std::to_string(args_count) + " argument(s)");
//     }

//     VortexObj r = args[0];
//     VortexObj g = args[1];
//     VortexObj b = args[2];
//     VortexObj a = args[3];

//     if (r->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects arg 1 to be a number");
//     }

//     if (g->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects arg 2 to be a number");
//     }

//     if (b->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects arg 3 to be a number");
//     }

//     if (a->type != NodeType::NUMBER) {
//         error_and_exit("Function '" + name + "' expects arg 4 to be a number");
//     }

//     glClearColor(r->_Node.Number().value, g->_Node.Number().value, b->_Node.Number().value, a->_Node.Number().value);

//     return new_vortex_obj(NodeType::NONE);
// }

// /* Implement call_function */

// extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
//     if (name == "sdl_init") {
//         return sdl_init(name, args);
//     }
//     if (name == "create_window") {
//         return create_window(name, args);
//     }
//     if (name == "destroy_window") {
//         return destroy_window(name, args);
//     }
//     if (name == "create_renderer") {
//         return create_renderer(name, args);
//     }
//     if (name == "destroy_renderer") {
//         return destroy_renderer(name, args);
//     }
//     if (name == "set_render_draw_color") {
//         return set_render_draw_color(name, args);
//     }
//     if (name == "render_clear") {
//         return render_clear(name, args);
//     }
//     if (name == "render_present") {
//         return render_present(name, args);
//     }
//     if (name == "poll_event") {
//         return poll_event(name, args);
//     }
//     if (name == "get_key_name") {
//         return get_key_name(name, args);
//     }
//     if (name == "get_scancode") {
//         return get_scancode(name, args);
//     }
//     if (name == "sdl_quit") {
//         return sdl_quit(name, args);
//     }
//     if (name == "draw_point") {
//         return draw_point(name, args);
//     }
//     if (name == "draw_line") {
//         return draw_line(name, args);
//     }
//     if (name == "draw_rect") {
//         return draw_rect(name, args);
//     }
//     if (name == "draw_geometry") {
//         return draw_geometry(name, args);
//     }
//     if (name == "sdl_delay") {
//         return sdl_delay(name, args);
//     }
//     if (name == "sdl_get_performance_counter") {
//         return sdl_get_performance_counter(name, args);
//     }
//     if (name == "sdl_get_performance_frequency") {
//         return sdl_get_performance_frequency(name, args);
//     }
//     if (name == "sdl_get_keyboard_state") {
//         return sdl_get_keyboard_state(name, args);
//     }
//     if (name == "load_texture") {
//         return load_texture(name, args);
//     }
//     if (name == "load_text") {
//         return load_text(name, args);
//     }
//     if (name == "destroy_texture") {
//         return destroy_texture(name, args);
//     }
//     if (name == "render_copy") {
//         return render_copy(name, args);
//     }
//     if (name == "render_copy_ex") {
//         return render_copy_ex(name, args);
//     }
//     if (name == "sdl_get_window_size") {
//         return sdl_get_window_size(name, args);
//     }
//     if (name == "sdl_show_cursor") {
//         return sdl_show_cursor(name, args);
//     }
//     // GL Functuons
//     if (name == "gl_enable") {
//         return gl_enable(name, args);
//     }
//     if (name == "gl_clear") {
//         return gl_clear(name, args);
//     }
//     if (name == "gl_viewport") {
//         return gl_viewport(name, args);
//     }
//     if (name == "gl_create_context") {
//         return gl_create_context(name, args);
//     }
//     if (name == "gl_swap_window") {
//         return gl_swap_window(name, args);
//     }
//     if (name == "gl_clear_color") {
//         return gl_clear_color(name, args);
//     }

//     error_and_exit("Function '" + name + "' is undefined");

//     return new_vortex_obj(NodeType::NONE);
// }
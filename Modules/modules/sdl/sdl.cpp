#include "../../Vortex.hpp"
#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_Image.h"
#include "include/SDL2/SDL_ttf.h"

/* Define Vars */

/* Declare Lib Functions */

/* Implement Lib Functions */

VortexObj sdl_init(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 0) {
        error_and_exit("Function '" + name + "' expects 0 arguments");
    }
    int init = SDL_Init(SDL_INIT_EVERYTHING);
    return new_number_node(init);
}

VortexObj create_window(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 5) {
        error_and_exit("Function '" + name + "' expects 5 arguments");
    }

    VortexObj windowName = args[0];
    VortexObj xPos = args[1];
    VortexObj yPos = args[2];
    VortexObj width = args[3];
    VortexObj height = args[4];

    if (windowName->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects arg 1 to be a string");
    }

    if (xPos->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects arg 2 to be a number");
    }

    if (yPos->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects arg 3 to be a number");
    }

    if (width->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects arg 4 to be a number");
    }

    if (height->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects arg 5 to be a number");
    }

    SDL_Window* window = SDL_CreateWindow(windowName->_Node.String().value.c_str(), 
                                      (int)xPos->_Node.Number().value, 
                                      (int)yPos->_Node.Number().value, 
                                      (int)width->_Node.Number().value,
                                      (int)height->_Node.Number().value, 
                                      SDL_WINDOW_SHOWN);

    SDL_SetWindowResizable(window, SDL_TRUE);

    VortexObj windowNode = new_vortex_obj(NodeType::POINTER);
    windowNode->_Node.Pointer().value = window;

    auto error = std::string(SDL_GetError());
    if (error != "") {
        std::cout << "SDL Error (" << name << "): " << error << "\n";
        return new_vortex_obj(NodeType::NONE);
    }

    std::cout << "Initalized window: " << window << "\n";
    return windowNode;
}

VortexObj destroy_window(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    VortexObj windowPointer = args[0];

    if (windowPointer->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects 1 pointer argument");
    }

    SDL_Window* window = (SDL_Window*)windowPointer->_Node.Pointer().value;

    SDL_DestroyWindow(window);

    std::cout << "Window destroyed\n";

    return new_vortex_obj(NodeType::NONE);
}

VortexObj create_renderer(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 2) {
        error_and_exit("Function '" + name + "' expects 2 arguments");
    }

    VortexObj window = args[0];
    VortexObj index = args[1];

    if (window->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects arg 1 to be a pointer");
    }

    if (index->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects arg 2 to be a number");
    }

    SDL_Window* windowPtr = (SDL_Window*)window->_Node.Pointer().value;

    SDL_Renderer* renderer = SDL_CreateRenderer(windowPtr, (int)index->_Node.Number().value, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    VortexObj rendererNode = new_vortex_obj(NodeType::POINTER);
    rendererNode->_Node.Pointer().value = renderer;

    auto error = std::string(SDL_GetError());
    if (error != "") {
        std::cout << "SDL Error (" << name << "): " << error << "\n";
        return new_vortex_obj(NodeType::NONE);
    }

    std::cout << "Initalized renderer: " << renderer << "\n";
    return rendererNode;
}

VortexObj destroy_renderer(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    VortexObj rendererPointer = args[0];

    if (rendererPointer->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects 1 pointer argument");
    }

    SDL_Renderer* renderer = (SDL_Renderer*)rendererPointer->_Node.Pointer().value;

    SDL_DestroyRenderer(renderer);

    auto error = std::string(SDL_GetError());
    if (error != "") {
        std::cout << "SDL Error (" << name << "): " << error << "\n";
        return new_vortex_obj(NodeType::NONE);
    }

    std::cout << "Renderer destroyed\n";

    return new_vortex_obj(NodeType::NONE);
}

VortexObj set_render_draw_color(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 5) {
        error_and_exit("Function '" + name + "' expects 5 arguments");
    }

    VortexObj renderer = args[0];
    VortexObj r = args[1];
    VortexObj g = args[2];
    VortexObj b = args[3];
    VortexObj a = args[4];

    if (renderer->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects arg 1 to be a pointer");
    }

    if (r->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects arg 2 to be a number");
    }

    if (g->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects arg 3 to be a number");
    }

    if (b->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects arg 4 to be a number");
    }

    if (a->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects arg 5 to be a number");
    }

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

    int status = SDL_SetRenderDrawColor(rendererPtr, r->_Node.Number().value, g->_Node.Number().value, b->_Node.Number().value, a->_Node.Number().value);

    if (status != 0) {
        std::cout << "SDL Error (" << name << "): " << SDL_GetError() << "\n";
        return new_vortex_obj(NodeType::NONE);
    }

    return new_vortex_obj(NodeType::NONE);
}

VortexObj render_clear(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    VortexObj renderer = args[0];

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

    int status = SDL_RenderClear(rendererPtr);

    if (status != 0) {
        std::cout << "SDL Error (" << name << "): " << SDL_GetError() << "\n";
        return new_vortex_obj(NodeType::NONE);
    }

    return new_vortex_obj(NodeType::NONE);
}

VortexObj render_present(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    VortexObj renderer = args[0];

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

    SDL_RenderPresent(rendererPtr);

    return new_vortex_obj(NodeType::NONE);
}

VortexObj draw_point(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 3) {
        error_and_exit("Function '" + name + "' expects 2 arguments");
    }

    VortexObj renderer = args[0];
    VortexObj xPos = args[1];
    VortexObj yPos = args[2];

    if (renderer->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects first argument to be a pointer");
    }

    if (xPos->type != NodeType::NUMBER || yPos->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects 2 number arguments");
    }

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

    int draw = SDL_RenderDrawPoint(rendererPtr, xPos->_Node.Number().value, yPos->_Node.Number().value);

    if (draw != 0) {
        error_and_exit("Function '" + name + "' failed");
    }

    return new_vortex_obj(NodeType::NONE);
}

VortexObj draw_line(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 5) {
        error_and_exit("Function '" + name + "' expects 5 arguments");
    }

    VortexObj renderer = args[0];
    VortexObj x1 = args[1];
    VortexObj y1 = args[2];
    VortexObj x2 = args[3];
    VortexObj y2 = args[4];

    if (renderer->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects first argument to be a pointer");
    }

    if (x1->type != NodeType::NUMBER || y1->type != NodeType::NUMBER || x2->type != NodeType::NUMBER || y2->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects 4 number arguments");
    }

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

    int draw = SDL_RenderDrawLineF(rendererPtr, x1->_Node.Number().value, y1->_Node.Number().value, x2->_Node.Number().value, y2->_Node.Number().value);

    if (draw != 0) {
        error_and_exit("Function '" + name + "' failed");
    }

    return new_vortex_obj(NodeType::NONE);
}

VortexObj draw_rect(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 5) {
        error_and_exit("Function '" + name + "' expects 5 arguments");
    }

    VortexObj renderer = args[0];
    VortexObj x = args[1];
    VortexObj y = args[2];
    VortexObj w = args[3];
    VortexObj h = args[4];

    if (renderer->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects first argument to be a pointer");
    }

    if (x->type != NodeType::NUMBER || y->type != NodeType::NUMBER || w->type != NodeType::NUMBER || h->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects 4 number arguments");
    }

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

    SDL_FRect rect;
    rect.x = x->_Node.Number().value;
    rect.y = y->_Node.Number().value;
    rect.w = w->_Node.Number().value;
    rect.h = h->_Node.Number().value;

    int draw = SDL_RenderDrawRectF(rendererPtr, &rect);
    int fill = SDL_RenderFillRectF(rendererPtr, &rect);

    if (draw != 0) {
        error_and_exit("Function '" + name + "' failed");
    }

    if (fill != 0) {
        error_and_exit("Function '" + name + "' failed");
    }

    return new_vortex_obj(NodeType::NONE);
}

VortexObj load_texture(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 2) {
        error_and_exit("Function '" + name + "' expects 5 arguments");
    }

    VortexObj renderer = args[0];
    VortexObj file_path = args[1];

    if (renderer->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects argument 1 to be a pointer");
    }

    if (file_path->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects argument 2 to be a string");
    }

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

    SDL_Texture* texture = IMG_LoadTexture(rendererPtr, file_path->_Node.String().value.c_str());

    if (!texture) {
        error_and_exit("Problem loading texture");
    }

    VortexObj textureNode = new_vortex_obj(NodeType::POINTER);
    textureNode->_Node.Pointer().value = texture;

    return textureNode;
}

VortexObj render_copy(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 10) {
        error_and_exit("Function '" + name + "' expects 9 arguments");
    }

    VortexObj renderer = args[0];
    VortexObj texture = args[1];

    VortexObj src_x = args[2];
    VortexObj src_y = args[3];
    VortexObj src_w = args[4];
    VortexObj src_h = args[5];

    VortexObj dest_x = args[6];
    VortexObj dest_y = args[7];
    VortexObj dest_w = args[8];
    VortexObj dest_h = args[9];

    if (renderer->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects first argument to be a pointer");
    }

    if (texture->type != NodeType::POINTER) {
        error_and_exit("Function '" + name + "' expects second argument to be a pointer");
    }

    if (src_x->type != NodeType::NUMBER || src_y->type != NodeType::NUMBER || src_w->type != NodeType::NUMBER || src_h->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects 4 number arguments");
    }

    if (dest_x->type != NodeType::NUMBER || dest_y->type != NodeType::NUMBER || dest_w->type != NodeType::NUMBER || dest_h->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects 4 number arguments");
    }

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->_Node.Pointer().value;

    SDL_Rect src_rect;
    src_rect.x = src_x->_Node.Number().value;
    src_rect.y = src_y->_Node.Number().value;
    src_rect.w = src_w->_Node.Number().value;
    src_rect.h = src_h->_Node.Number().value;

    bool src_rect_null = src_rect.x == -1 && src_rect.y == -1 && src_rect.w == -1 && src_rect.h == -1;

    SDL_Rect dest_rect;
    dest_rect.x = dest_x->_Node.Number().value;
    dest_rect.y = dest_y->_Node.Number().value;
    dest_rect.w = dest_w->_Node.Number().value;
    dest_rect.h = dest_h->_Node.Number().value;

    bool dest_rect_null = dest_rect.x == -1 && dest_rect.y == -1 && dest_rect.w == -1 && dest_rect.h == -1;

    SDL_Texture* texturePtr = (SDL_Texture*)texture->_Node.Pointer().value;
    SDL_RenderCopy(rendererPtr, texturePtr, src_rect_null ? NULL : &src_rect, dest_rect_null ? NULL : &dest_rect);

    return new_vortex_obj(NodeType::NONE);
}

VortexObj get_key_name(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    VortexObj key_code = args[0];

    if (key_code->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects 1 number argument");
    }

    const char* key_name = SDL_GetKeyName(key_code->_Node.Number().value);
    VortexObj name_node = new_string_node(std::string(key_name));
    return name_node;
}

VortexObj get_scancode(std::string name, std::vector<VortexObj> args) {
    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    VortexObj key_name = args[0];

    if (key_name->type != NodeType::STRING) {
        error_and_exit("Function '" + name + "' expects 1 string argument");
    }

    int scancode = SDL_GetScancodeFromName(key_name->_Node.String().value.c_str());
    VortexObj scancode_node = new_number_node(scancode);
    return scancode_node;
}

VortexObj poll_event(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 0) {
        error_and_exit("Function '" + name + "' expects 0 arguments");
    }

    SDL_Event e;
    int status = SDL_PollEvent(&e);

    VortexObj event = new_vortex_obj(NodeType::OBJECT);
    event->_Node.Object().properties["type"] = new_number_node(e.type);
    // Button
    event->_Node.Object().properties["button"] = new_vortex_obj(NodeType::OBJECT);
    event->_Node.Object().properties["button"]->_Node.Object().properties["button"] = new_number_node(e.button.button);
    event->_Node.Object().properties["button"]->_Node.Object().properties["clicks"] = new_number_node(e.button.clicks);
    event->_Node.Object().properties["button"]->_Node.Object().properties["x"] = new_number_node(e.button.x);
    event->_Node.Object().properties["button"]->_Node.Object().properties["y"] = new_number_node(e.button.y);
    // Motion
    event->_Node.Object().properties["motion"] = new_vortex_obj(NodeType::OBJECT);
    event->_Node.Object().properties["motion"]->_Node.Object().properties["x"] = new_number_node(e.motion.x);
    event->_Node.Object().properties["motion"]->_Node.Object().properties["y"] = new_number_node(e.button.y);
    // Keyboard
    event->_Node.Object().properties["key"] = new_vortex_obj(NodeType::OBJECT);
    event->_Node.Object().properties["key"]->_Node.Object().properties["type"] = new_number_node(e.key.type);
    event->_Node.Object().properties["key"]->_Node.Object().properties["state"] = new_number_node(e.key.state);
    event->_Node.Object().properties["key"]->_Node.Object().properties["repeat"] = new_number_node(e.key.repeat);
    event->_Node.Object().properties["key"]->_Node.Object().properties["keysm"] = new_vortex_obj(NodeType::OBJECT);
    event->_Node.Object().properties["key"]->_Node.Object().properties["keysm"]->_Node.Object().properties["mod"] = new_number_node(e.key.keysym.mod);
    event->_Node.Object().properties["key"]->_Node.Object().properties["keysm"]->_Node.Object().properties["scancode"] = new_number_node(e.key.keysym.scancode);
    event->_Node.Object().properties["key"]->_Node.Object().properties["keysm"]->_Node.Object().properties["sym"] = new_number_node(e.key.keysym.sym);

    VortexObj eventStruct = new_vortex_obj(NodeType::OBJECT);
    eventStruct->_Node.Object().properties["status"] = new_number_node(status);
    eventStruct->_Node.Object().properties["event"] = event;

    return eventStruct;
}

VortexObj sdl_delay(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 1) {
        error_and_exit("Function '" + name + "' expects 1 argument");
    }

    VortexObj time = args[0];

    if (time->type != NodeType::NUMBER) {
        error_and_exit("Function '" + name + "' expects 1 number argument");
    }

    SDL_Delay(time->_Node.Number().value);

    return new_vortex_obj(NodeType::NONE);
}

VortexObj sdl_quit(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 0) {
        error_and_exit("Function '" + name + "' expects 0 arguments");
    }

    SDL_Quit();

    auto error = std::string(SDL_GetError());
    if (error != "") {
        std::cout << "SDL Error (" << name << "): " << error << "\n";
        return new_vortex_obj(NodeType::NONE);
    }

    std::cout << "SDL Quit\n";

    return new_vortex_obj(NodeType::NONE);
}

VortexObj sdl_get_performance_counter(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 0) {
        error_and_exit("Function '" + name + "' expects 0 arguments");
    }

    node_ptr counter = new_number_node(SDL_GetPerformanceCounter());

    return counter;
}

VortexObj sdl_get_performance_frequency(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 0) {
        error_and_exit("Function '" + name + "' expects 0 arguments");
    }

    node_ptr frequency = new_number_node(SDL_GetPerformanceFrequency());

    return frequency;
}

VortexObj sdl_get_keyboard_state(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 0) {
        error_and_exit("Function '" + name + "' expects 0 arguments");
    }

    int len;
    const Uint8* keyboard_state = SDL_GetKeyboardState(&len);

    node_ptr keyboard_state_list = new_vortex_obj(NodeType::LIST);
    for (int i = 0; i < len; i++) {
        keyboard_state_list->_Node.List().elements.push_back(new_boolean_node(keyboard_state[i]));
    }

    return keyboard_state_list;
}

/* Implement call_function */

extern "C" VortexObj call_function(std::string name, std::vector<VortexObj> args) {
    if (name == "sdl_init") {
        return sdl_init(name, args);
    }
    if (name == "create_window") {
        return create_window(name, args);
    }
    if (name == "destroy_window") {
        return destroy_window(name, args);
    }
    if (name == "create_renderer") {
        return create_renderer(name, args);
    }
    if (name == "destroy_renderer") {
        return destroy_renderer(name, args);
    }
    if (name == "set_render_draw_color") {
        return set_render_draw_color(name, args);
    }
    if (name == "render_clear") {
        return render_clear(name, args);
    }
    if (name == "render_present") {
        return render_present(name, args);
    }
    if (name == "poll_event") {
        return poll_event(name, args);
    }
    if (name == "get_key_name") {
        return get_key_name(name, args);
    }
    if (name == "get_scancode") {
        return get_scancode(name, args);
    }
    if (name == "sdl_quit") {
        return sdl_quit(name, args);
    }
    if (name == "draw_point") {
        return draw_point(name, args);
    }
    if (name == "draw_line") {
        return draw_line(name, args);
    }
    if (name == "draw_rect") {
        return draw_rect(name, args);
    }
    if (name == "sdl_delay") {
        return sdl_delay(name, args);
    }
    if (name == "sdl_get_performance_counter") {
        return sdl_get_performance_counter(name, args);
    }
    if (name == "sdl_get_performance_frequency") {
        return sdl_get_performance_frequency(name, args);
    }
    if (name == "sdl_get_keyboard_state") {
        return sdl_get_keyboard_state(name, args);
    }
    if (name == "load_texture") {
        return load_texture(name, args);
    }
    if (name == "render_copy") {
        return render_copy(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}
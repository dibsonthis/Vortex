#include "../../Vortex.hpp"
#include "include/SDL2/SDL.h"

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

    SDL_Window* window = SDL_CreateWindow(windowName->String.value.c_str(), 
                                      (int)xPos->Number.value, 
                                      (int)yPos->Number.value, 
                                      (int)width->Number.value,
                                      (int)height->Number.value, 
                                      SDL_WINDOW_SHOWN);

    SDL_SetWindowResizable(window, SDL_TRUE);

    VortexObj windowNode = new_vortex_obj(NodeType::POINTER);
    windowNode->Pointer.value = window;

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

    SDL_Window* window = (SDL_Window*)windowPointer->Pointer.value;

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

    SDL_Window* windowPtr = (SDL_Window*)window->Pointer.value;

    SDL_Renderer* renderer = SDL_CreateRenderer(windowPtr, (int)index->Number.value, SDL_RENDERER_ACCELERATED);

    VortexObj rendererNode = new_vortex_obj(NodeType::POINTER);
    rendererNode->Pointer.value = renderer;

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

    SDL_Renderer* renderer = (SDL_Renderer*)rendererPointer->Pointer.value;

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

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->Pointer.value;

    int status = SDL_SetRenderDrawColor(rendererPtr, r->Number.value, g->Number.value, b->Number.value, a->Number.value);

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

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->Pointer.value;

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

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->Pointer.value;

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

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->Pointer.value;

    int draw = SDL_RenderDrawPoint(rendererPtr, xPos->Number.value, yPos->Number.value);

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

    SDL_Renderer* rendererPtr = (SDL_Renderer*)renderer->Pointer.value;

    int draw = SDL_RenderDrawLineF(rendererPtr, x1->Number.value, y1->Number.value, x2->Number.value, y2->Number.value);

    if (draw != 0) {
        error_and_exit("Function '" + name + "' failed");
    }

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

    const char* key_name = SDL_GetKeyName(key_code->Number.value);
    VortexObj name_node = new_string_node(std::string(key_name));
    return name_node;
}

VortexObj poll_event(std::string name, std::vector<VortexObj> args) {

    if (args.size() != 0) {
        error_and_exit("Function '" + name + "' expects 0 arguments");
    }

    SDL_Event e;
    int status = SDL_PollEvent(&e);

    VortexObj event = new_vortex_obj(NodeType::OBJECT);
    event->Object.properties["type"] = new_number_node(e.type);
    // Button
    event->Object.properties["button"] = new_vortex_obj(NodeType::OBJECT);
    event->Object.properties["button"]->Object.properties["button"] = new_number_node(e.button.button);
    event->Object.properties["button"]->Object.properties["clicks"] = new_number_node(e.button.clicks);
    event->Object.properties["button"]->Object.properties["x"] = new_number_node(e.button.x);
    event->Object.properties["button"]->Object.properties["y"] = new_number_node(e.button.y);
    // Motion
    event->Object.properties["motion"] = new_vortex_obj(NodeType::OBJECT);
    event->Object.properties["motion"]->Object.properties["x"] = new_number_node(e.motion.x);
    event->Object.properties["motion"]->Object.properties["y"] = new_number_node(e.button.y);
    // Keyboard
    event->Object.properties["key"] = new_vortex_obj(NodeType::OBJECT);
    event->Object.properties["key"]->Object.properties["type"] = new_number_node(e.key.type);
    event->Object.properties["key"]->Object.properties["state"] = new_number_node(e.key.state);
    event->Object.properties["key"]->Object.properties["repeat"] = new_number_node(e.key.repeat);
    event->Object.properties["key"]->Object.properties["keysm"] = new_vortex_obj(NodeType::OBJECT);
    event->Object.properties["key"]->Object.properties["keysm"]->Object.properties["mod"] = new_number_node(e.key.keysym.mod);
    event->Object.properties["key"]->Object.properties["keysm"]->Object.properties["scancode"] = new_number_node(e.key.keysym.scancode);
    event->Object.properties["key"]->Object.properties["keysm"]->Object.properties["sym"] = new_number_node(e.key.keysym.sym);

    VortexObj eventStruct = new_vortex_obj(NodeType::OBJECT);
    eventStruct->Object.properties["status"] = new_number_node(status);
    eventStruct->Object.properties["event"] = event;

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

    SDL_Delay(time->Number.value);

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
    if (name == "sdl_quit") {
        return sdl_quit(name, args);
    }
    if (name == "draw_point") {
        return draw_point(name, args);
    }
    if (name == "draw_line") {
        return draw_line(name, args);
    }
    if (name == "sdl_delay") {
        return sdl_delay(name, args);
    }

    error_and_exit("Function '" + name + "' is undefined");

    return new_vortex_obj(NodeType::NONE);
}
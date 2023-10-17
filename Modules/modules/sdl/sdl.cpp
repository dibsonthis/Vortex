#include "include/Vortex.hpp"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_Image.h"
#include "include/SDL2/SDL_ttf.h"
#include "include/SDL2/imgui.h"
#include "include/SDL2/imgui_impl_sdl2.h"
#include "include/SDL2/imgui_impl_sdlrenderer2.h"

extern "C" Value initSDL(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'initSDL' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    TTF_Init();
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    int init = SDL_Init(SDL_INIT_EVERYTHING);

    return number_val(init);
}

extern "C" Value createWindow(std::vector<Value> &args)
{
    int num_required_args = 6;

    if (args.size() != num_required_args)
    {
        error("Function 'createWindow' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value windowName = args[0];
    Value xPos = args[1];
    Value yPos = args[2];
    Value width = args[3];
    Value height = args[4];
    Value flags = args[5];

    if (!windowName.is_string())
    {
        error("Function 'createWindow' expects arg 'windowName' to be a string");
    }

    if (!xPos.is_number())
    {
        error("Function 'createWindow' expects arg 'xPos' to be a number");
    }

    if (!yPos.is_number())
    {
        error("Function 'createWindow' expects arg 'yPos' to be a number");
    }

    if (!width.is_number())
    {
        error("Function 'createWindow' expects arg 'width' to be a number");
    }

    if (!height.is_number())
    {
        error("Function 'createWindow' expects arg 'height' to be a number");
    }

    if (!flags.is_number())
    {
        error("Function 'createWindow' expects arg 'flags' to be a number");
    }

    SDL_Window *window = SDL_CreateWindow(windowName.get_string().c_str(),
                                          (int)xPos.get_number(),
                                          (int)yPos.get_number(),
                                          (int)width.get_number(),
                                          (int)height.get_number(),
                                          SDL_WINDOW_SHOWN | (int)flags.get_number());

    SDL_SetWindowResizable(window, SDL_TRUE);

    Value windowPointer = pointer_val();
    windowPointer.get_pointer()->value = window;

    auto sdl_error = std::string(SDL_GetError());
    if (sdl_error != "")
    {
        error("SDL Error (createWindow): " + sdl_error);
    }

    std::cout << "Initalized window: " << window << "\n";
    return windowPointer;
}

extern "C" Value createRenderer(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'createRenderer' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value window = args[0];
    Value index = args[1];

    if (!window.is_pointer())
    {
        error("Function 'createRenderer' expects arg 'window' to be a pointer");
    }

    if (!index.is_number())
    {
        error("Function 'createRenderer' expects arg 'index' to be a number");
    }

    SDL_Window *windowPtr = (SDL_Window *)window.get_pointer()->value;

    SDL_Renderer *renderer = SDL_CreateRenderer(windowPtr, (int)index.get_number(), SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    Value rendererPointer = pointer_val();
    rendererPointer.get_pointer()->value = renderer;

    auto sdl_error = std::string(SDL_GetError());
    if (sdl_error != "")
    {
        error("SDL Error (createRenderer): " + sdl_error);
    }

    std::cout << "Initalized renderer: " << renderer << "\n";
    return rendererPointer;
}

extern "C" Value pollEvent(std::vector<Value> &args)
{
    // auto e = std::make_shared<SDL_Event>();
    // int status = SDL_PollEvent(e.get());

    SDL_Event e = SDL_Event();
    int status = SDL_PollEvent(&e);

    Value event = object_val();
    auto &event_obj = event.get_object();

    event_obj->keys = {"type", "button", "motion", "key", "window"};

    event_obj->values["type"] = number_val(e.type);
    // Button
    event_obj->values["button"] = object_val();
    event_obj->values["button"].get_object()->keys = {"button", "clicks", "x", "y", "which"};
    event_obj->values["button"].get_object()->values["button"] = number_val(e.button.button);
    event_obj->values["button"].get_object()->values["clicks"] = number_val(e.button.clicks);
    event_obj->values["button"].get_object()->values["x"] = number_val(e.button.x);
    event_obj->values["button"].get_object()->values["y"] = number_val(e.button.y);
    event_obj->values["button"].get_object()->values["which"] = number_val(e.button.which);
    event_obj->values["button"].get_object()->values["windowID"] = number_val(e.button.windowID);
    // Motion
    event_obj->values["motion"] = object_val();
    event_obj->values["motion"].get_object()->keys = {"x", "y", "which", "windowID"};
    event_obj->values["motion"].get_object()->values["x"] = number_val(e.motion.x);
    event_obj->values["motion"].get_object()->values["y"] = number_val(e.motion.y);
    event_obj->values["motion"].get_object()->values["which"] = number_val(e.motion.which);
    event_obj->values["motion"].get_object()->values["windowID"] = number_val(e.motion.windowID);
    // Wheel
    event_obj->values["wheel"] = object_val();
    event_obj->values["wheel"].get_object()->keys = {"x", "y", "preciseX", "preciseY", "which", "windowID"};
    event_obj->values["wheel"].get_object()->values["x"] = number_val(e.wheel.x);
    event_obj->values["wheel"].get_object()->values["y"] = number_val(e.wheel.y);
    event_obj->values["wheel"].get_object()->values["preciseX"] = number_val(e.wheel.preciseX);
    event_obj->values["wheel"].get_object()->values["preciseY"] = number_val(e.wheel.preciseY);
    event_obj->values["wheel"].get_object()->values["which"] = number_val(e.wheel.which);
    event_obj->values["wheel"].get_object()->values["windowID"] = number_val(e.wheel.windowID);
    // Keyboard
    event_obj->values["key"] = object_val();
    event_obj->values["key"].get_object()->keys = {"type", "state", "repeat", "keysm"};
    event_obj->values["key"].get_object()->values["type"] = number_val(e.key.type);
    event_obj->values["key"].get_object()->values["state"] = number_val(e.key.state);
    event_obj->values["key"].get_object()->values["repeat"] = number_val(e.key.repeat);
    event_obj->values["key"].get_object()->values["keysm"] = object_val();
    event_obj->values["key"].get_object()->values["keysm"].get_object()->keys = {"mod", "scancode", "sym"};
    event_obj->values["key"].get_object()->values["keysm"].get_object()->values["mod"] = number_val(e.key.keysym.mod);
    event_obj->values["key"].get_object()->values["keysm"].get_object()->values["scancode"] = number_val(e.key.keysym.scancode);
    event_obj->values["key"].get_object()->values["keysm"].get_object()->values["sym"] = number_val(e.key.keysym.sym);
    // Text
    event_obj->values["text"] = object_val();
    event_obj->values["text"].get_object()->keys = {"text"};
    event_obj->values["text"].get_object()->values["text"] = string_val(std::string(e.text.text));
    // Window
    event_obj->values["window"] = object_val();
    event_obj->values["window"].get_object()->keys = {"event", "data1", "data2", "windowID"};
    event_obj->values["window"].get_object()->values["event"] = number_val(e.window.event);
    event_obj->values["window"].get_object()->values["data1"] = number_val(e.window.data1);
    event_obj->values["window"].get_object()->values["data2"] = number_val(e.window.data2);
    event_obj->values["window"].get_object()->values["windowID"] = number_val(e.window.windowID);

    Value eventStruct = object_val();
    eventStruct.get_object()->keys = {"status", "event", "event_ptr"};
    eventStruct.get_object()->values["status"] = number_val(status);
    eventStruct.get_object()->values["event"] = event;

    return eventStruct;
}

extern "C" Value renderClear(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'renderClear' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value renderer = args[0];

    if (!renderer.is_pointer())
    {
        error("Function 'renderClear' expects arg 'renderer' to be a pointer");
    }

    SDL_Renderer *rendererPtr = (SDL_Renderer *)renderer.get_pointer()->value;

    int status = SDL_RenderClear(rendererPtr);

    if (status != 0)
    {
        error("SDL Error (renderClear): " + std::string(SDL_GetError()) + "\n");
    }

    return none_val();
}

extern "C" Value renderPresent(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'renderPresent' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value renderer = args[0];

    if (!renderer.is_pointer())
    {
        error("Function 'renderPresent' expects arg 'renderer' to be a pointer");
    }

    SDL_Renderer *rendererPtr = (SDL_Renderer *)renderer.get_pointer()->value;

    SDL_RenderPresent(rendererPtr);

    return none_val();
}

extern "C" Value getKeyName(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'getKeyName' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value keyCode = args[0];

    if (!keyCode.is_number())
    {
        error("Function 'getKeyName' expects arg 'keyCode' to be a number");
    }

    const char *key_name = SDL_GetKeyName(keyCode.get_number());
    Value name_node = string_val(std::string(key_name));
    return name_node;
}

extern "C" Value setRenderDrawColor(std::vector<Value> &args)
{
    int num_required_args = 5;

    if (args.size() != num_required_args)
    {
        error("Function 'setRenderDrawColor' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value renderer = args[0];
    Value r = args[1];
    Value g = args[2];
    Value b = args[3];
    Value a = args[4];

    if (!renderer.is_pointer())
    {
        error("Function 'setRenderDrawColor' expects arg 'renderer' to be a pointer");
    }

    if (!(r.is_number() && g.is_number() && b.is_number() && a.is_number()))
    {
        error("Function 'setRenderDrawColor' expects args 'r', 'g', 'b', 'a' to be numbers");
    }

    SDL_Renderer *rendererPtr = (SDL_Renderer *)renderer.get_pointer()->value;

    int status = SDL_SetRenderDrawColor(rendererPtr, r.get_number(), g.get_number(), b.get_number(), a.get_number());

    if (status != 0)
    {
        error("SDL Error (setRenderDrawColor): " + std::string(SDL_GetError()) + "\n");
    }

    return none_val();
}

extern "C" Value delay(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'delay' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value ms = args[0];

    if (!ms.is_number())
    {
        error("Function 'delay' expects arg 'ms' to be a number");
    }

    SDL_Delay(ms.get_number());

    return none_val();
}

extern "C" Value drawPoint(std::vector<Value> &args)
{
    int num_required_args = 3;

    if (args.size() != num_required_args)
    {
        error("Function 'drawPoint' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value renderer = args[0];
    Value xPos = args[1];
    Value yPos = args[2];

    if (!renderer.is_pointer())
    {
        error("Function 'drawPoint' expects arg 'renderer' to be a pointer");
    }

    if (!(xPos.is_number() && yPos.is_number()))
    {
        error("Function 'drawPoint' expects args 'xPos', 'yPos' to be numbers");
    }

    SDL_Renderer *rendererPtr = (SDL_Renderer *)renderer.get_pointer()->value;

    int draw = SDL_RenderDrawPoint(rendererPtr, xPos.get_number(), yPos.get_number());

    if (draw != 0)
    {
        error("Function 'drawPoint' failed");
    }

    return none_val();
}

extern "C" Value drawLine(std::vector<Value> &args)
{
    int num_required_args = 5;

    if (args.size() != num_required_args)
    {
        error("Function 'drawLine' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value renderer = args[0];
    Value x1 = args[1];
    Value y1 = args[2];
    Value x2 = args[3];
    Value y2 = args[4];

    if (!renderer.is_pointer())
    {
        error("Function 'drawLine' expects arg 'renderer' to be a pointer");
    }

    if (!(x1.is_number() && y1.is_number() && x2.is_number() && y2.is_number()))
    {
        error("Function 'drawLine' expects args 'x1', 'y1', 'x2', 'y2' to be numbers");
    }

    SDL_Renderer *rendererPtr = (SDL_Renderer *)renderer.get_pointer()->value;

    int draw = SDL_RenderDrawLineF(rendererPtr, x1.get_number(), y1.get_number(), x2.get_number(), y2.get_number());

    if (draw != 0)
    {
        error("Function 'drawLine' failed");
    }

    return none_val();
}

extern "C" Value drawRect(std::vector<Value> &args)
{
    int num_required_args = 5;

    if (args.size() != num_required_args)
    {
        error("Function 'drawRect' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value renderer = args[0];
    Value x = args[1];
    Value y = args[2];
    Value w = args[3];
    Value h = args[4];

    if (!renderer.is_pointer())
    {
        error("Function 'drawRect' expects arg 'renderer' to be a pointer");
    }

    if (!(x.is_number() && y.is_number() && w.is_number() && h.is_number()))
    {
        error("Function 'drawRect' expects args 'x', 'y', 'w', 'h' to be numbers");
    }

    SDL_Renderer *rendererPtr = (SDL_Renderer *)renderer.get_pointer()->value;

    SDL_FRect rect;
    rect.x = x.get_number();
    rect.y = y.get_number();
    rect.w = w.get_number();
    rect.h = h.get_number();

    int draw = SDL_RenderDrawRectF(rendererPtr, &rect);
    int fill = SDL_RenderFillRectF(rendererPtr, &rect);

    if (draw != 0 || fill != 0)
    {
        error("Function 'drawRect' failed");
    }

    return none_val();
}

extern "C" Value drawGeometry(std::vector<Value> &args)
{
    int num_required_args = 3;

    if (args.size() != num_required_args)
    {
        error("Function 'drawGeometry' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value renderer = args[0];
    Value vertices = args[1];
    Value texture = args[2];

    if (!renderer.is_pointer())
    {
        error("Function 'drawGeometry' expects arg 'renderer' to be a pointer");
    }

    if (!texture.is_pointer() && !texture.is_none())
    {
        error("Function 'drawGeometry' expects arg 'texture' to be a pointer or none");
    }

    if (!vertices.is_list())
    {
        error("Function 'drawGeometry' expects arg 'vertices' to be a list");
    }

    SDL_Renderer *rendererPtr = (SDL_Renderer *)renderer.get_pointer()->value;
    SDL_Texture *texturePtr = NULL;

    if (texture.is_pointer())
    {
        texturePtr = (SDL_Texture *)texture.get_pointer()->value;
    }

    std::vector<SDL_Vertex> sdl_vertices;

    for (Value &vertex : *vertices.get_list())
    {
        SDL_Vertex v;
        v.color.r = vertex.get_object()->values["color"].get_object()->values["r"].get_number();
        v.color.g = vertex.get_object()->values["color"].get_object()->values["g"].get_number();
        v.color.b = vertex.get_object()->values["color"].get_object()->values["b"].get_number();
        v.color.a = vertex.get_object()->values["color"].get_object()->values["a"].get_number();

        v.position.x = vertex.get_object()->values["position"].get_object()->values["x"].get_number();
        v.position.y = vertex.get_object()->values["position"].get_object()->values["y"].get_number();

        sdl_vertices.push_back(v);
    }

    int render = SDL_RenderGeometry(rendererPtr, texturePtr, sdl_vertices.data(), sdl_vertices.size(), NULL, 0);

    if (render != 0)
    {
        error("SDL Error (drawGeometry): " + std::string(SDL_GetError()) + "\n");
    }

    return none_val();
}

extern "C" Value loadTexture(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'loadTexture' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value renderer = args[0];
    Value filePath = args[1];

    if (!renderer.is_pointer())
    {
        error("Function 'loadTexture' expects arg 'renderer' to be a pointer");
    }

    if (!filePath.is_string())
    {
        error("Function 'loadTexture' expects arg 'filePath' to be a string");
    }

    SDL_Renderer *rendererPtr = (SDL_Renderer *)renderer.get_pointer()->value;
    SDL_Texture *texture = IMG_LoadTexture(rendererPtr, filePath.get_string().c_str());

    if (!texture)
    {
        error("SDL Error (loadTexture): " + std::string(SDL_GetError()) + "\n");
    }

    Value texturePtr = pointer_val();
    texturePtr.get_pointer()->value = texture;

    return texturePtr;
}

extern "C" Value loadText(std::vector<Value> &args)
{
    int num_required_args = 7;

    if (args.size() != num_required_args)
    {
        error("Function 'loadText' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value renderer = args[0];
    Value fontPath = args[1];
    Value fontSize = args[2];
    Value r = args[3];
    Value g = args[4];
    Value b = args[5];
    Value text = args[6];

    if (!renderer.is_pointer())
    {
        error("Function 'loadText' expects arg 'renderer' to be a pointer");
    }

    if (!fontPath.is_string())
    {
        error("Function 'loadText' expects arg 'fontPath' to be a string");
    }

    if (!fontSize.is_number() && !r.is_number() && !g.is_number() && !b.is_number())
    {
        error("Function 'loadText' expects args 'fontPath', 'r', 'g', 'b' to be numbers");
    }

    if (!text.is_string())
    {
        error("Function 'loadText' expects arg 'text' to be a string");
    }

    SDL_Renderer *rendererPtr = (SDL_Renderer *)renderer.get_pointer()->value;

    SDL_Color color = {
        r.get_number(),
        g.get_number(),
        b.get_number()};

    TTF_Font *font = TTF_OpenFont(fontPath.get_string().c_str(), fontSize.get_number());

    if (!font)
    {
        error("SDL Error (loadText): " + std::string(SDL_GetError()) + "\n");
    }

    SDL_Surface *surfaceMessage =
        TTF_RenderText_Solid(font, text.get_string().c_str(), color);

    if (!surfaceMessage)
    {
        error("SDL Error (loadText): " + std::string(SDL_GetError()) + "\n");
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(rendererPtr, surfaceMessage);

    if (!texture)
    {
        error("SDL Error (loadText): " + std::string(SDL_GetError()) + "\n");
    }

    Value texturePtr = pointer_val();
    texturePtr.get_pointer()->value = texture;

    SDL_FreeSurface(surfaceMessage);
    TTF_CloseFont(font);

    return texturePtr;
}

extern "C" Value getWindowSize(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'getWindowSize' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value window = args[0];

    if (!window.is_pointer())
    {
        error("Function 'getWindowSize' expects arg 'window' to be a pointer");
    }

    SDL_Window *windowPtr = (SDL_Window *)window.get_pointer()->value;

    int w, h;

    SDL_GetWindowSize(windowPtr, &w, &h);

    Value sizeObj = object_val();
    sizeObj.get_object()->values["w"] = number_val(w);
    sizeObj.get_object()->values["h"] = number_val(h);

    return sizeObj;
}

extern "C" Value showCursor(std::vector<Value> &args)
{
    int num_required_args = 3;

    if (args.size() != num_required_args)
    {
        error("Function 'showCursor' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value flag = args[0];

    if (!flag.is_number())
    {
        error("Function 'showCursor' expects arg 'flag' to be a number");
    }

    int state = SDL_ShowCursor((int)flag.get_number());
    return number_val(state);
}

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

// VortexObj destroy_window(std::string name, std::vector<VortexObj> args) {
//     if (args.size() != 1) {
//         error_and_exit("Function '" + name + "' expects 1 argument");
//     }

//     VortexObj windowPointer = args[0];

//     if (windowPointer->type != NodeType::POINTER) {
//         error_and_exit("Function '" + name + "' expects 1 pointer argument");
//     }

//     SDL_Window* window = (SDL_Window*)windowPointer->_Node.Pointer().value;

//     SDL_DestroyWindow(window);

//     std::cout << "Window destroyed\n";

//     return new_vortex_obj(NodeType::NONE);
// }

extern "C" Value imgui_check_version(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'check_version' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    bool result = IMGUI_CHECKVERSION();

    return boolean_val(result);
}

extern "C" Value imgui_create_context(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'create_context' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    auto context = ImGui::CreateContext();
    Value contextPointer = pointer_val();
    contextPointer.get_pointer()->value = context;

    return contextPointer;
}

extern "C" Value imgui_init_io(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'init_io' expects " + std::to_string(num_required_args) + " argument(s)");
    }
    auto &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Mouse Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking

    return boolean_val(true);
}

extern "C" Value imgui_get_io(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'get_io' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    auto &io = ImGui::GetIO();

    auto io_object = object_val();
    io_object.get_object()->values["WantCaptureMouse"] = boolean_val(io.WantCaptureMouse);
    io_object.get_object()->keys.push_back("WantCaptureMouse");
    io_object.get_object()->values["WantCaptureKeyboard"] = boolean_val(io.WantCaptureKeyboard);
    io_object.get_object()->keys.push_back("WantCaptureKeyboard");
    io_object.get_object()->values["Framerate"] = number_val(io.Framerate);
    io_object.get_object()->keys.push_back("Framerate");
    io_object.get_object()->values["DisplayFramebufferScale"] = object_val();
    io_object.get_object()->keys.push_back("DisplayFramebufferScale");
    io_object.get_object()->values["DisplayFramebufferScale"].get_object()->values["x"] = number_val(io.DisplayFramebufferScale.x);
    io_object.get_object()->values["DisplayFramebufferScale"].get_object()->values["y"] = number_val(io.DisplayFramebufferScale.y);
    io_object.get_object()->values["DisplayFramebufferScale"].get_object()->keys = {"x", "y"};

    return io_object;
}

extern "C" Value imgui_init_for_sdl(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'init_for_sdl' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value window = args[0];
    Value renderer = args[1];

    if (!renderer.is_pointer())
    {
        error("Function 'init_for_sdl' expects arg 'renderer' to be a pointer");
    }

    if (!window.is_pointer())
    {
        error("Function 'init_for_sdl' expects arg 'window' to be a pointer");
    }

    SDL_Renderer *rendererPtr = (SDL_Renderer *)renderer.get_pointer()->value;
    SDL_Window *windowPtr = (SDL_Window *)window.get_pointer()->value;

    bool result = ImGui_ImplSDL2_InitForSDLRenderer(windowPtr, rendererPtr);
    return boolean_val(result);
}

extern "C" Value imgui_sdl_render_init(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'sdl_render_init' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value &renderer = args[0];

    if (!renderer.is_pointer())
    {
        error("Function 'sdl_render_init' expects arg 'renderer' to be a pointer");
    }

    SDL_Renderer *rendererPtr = (SDL_Renderer *)renderer.get_pointer()->value;

    bool result = ImGui_ImplSDLRenderer2_Init(rendererPtr);
    return boolean_val(result);
}

extern "C" Value imgui_process_event(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'imgui_process_event' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value event = args[0];

    if (!event.is_object())
    {
        error("Function 'init_for_sdl' expects arg 'event' to be an object");
    }

    auto &event_obj = event.get_object();

    SDL_Event _event = SDL_Event();
    _event.type = event_obj->values["type"].get_number();

    _event.motion.x = event_obj->values["motion"].get_object()->values["x"].get_number();
    _event.motion.y = event_obj->values["motion"].get_object()->values["y"].get_number();
    _event.motion.windowID = event_obj->values["motion"].get_object()->values["windowID"].get_number();
    _event.motion.which = event_obj->values["motion"].get_object()->values["which"].get_number();

    _event.wheel.x = event_obj->values["wheel"].get_object()->values["x"].get_number();
    _event.wheel.y = event_obj->values["wheel"].get_object()->values["y"].get_number();
    _event.wheel.preciseX = event_obj->values["wheel"].get_object()->values["preciseX"].get_number();
    _event.wheel.preciseY = event_obj->values["wheel"].get_object()->values["preciseY"].get_number();
    _event.wheel.windowID = event_obj->values["wheel"].get_object()->values["windowID"].get_number();
    _event.wheel.which = event_obj->values["wheel"].get_object()->values["which"].get_number();

    _event.button.button = event_obj->values["button"].get_object()->values["button"].get_number();
    _event.button.which = event_obj->values["button"].get_object()->values["which"].get_number();

    strncpy(_event.text.text, event_obj->values["text"].get_object()->values["text"].get_string().c_str(), 32);

    _event.key.keysym.mod = event_obj->values["key"].get_object()->values["keysm"].get_object()->values["mod"].get_number();
    _event.key.keysym.sym = event_obj->values["key"].get_object()->values["keysm"].get_object()->values["sym"].get_number();
    _event.key.keysym.scancode = (SDL_Scancode)event_obj->values["key"].get_object()->values["keysm"].get_object()->values["scancode"].get_number();

    _event.window.event = event_obj->values["window"].get_object()->values["event"].get_number();
    _event.window.windowID = event_obj->values["window"].get_object()->values["windowID"].get_number();

    bool result = ImGui_ImplSDL2_ProcessEvent(_event);

    return boolean_val(result);
}

extern "C" Value imgui_new_frame(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'new_frame' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    return boolean_val(true);
}

extern "C" Value imgui_render(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'render' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    ImGui::Render();

    return boolean_val(true);
}

extern "C" Value imgui_get_draw_data(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'get_draw_data' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    ImDrawData *data = ImGui::GetDrawData();
    auto draw_data_ptr = pointer_val();
    draw_data_ptr.get_pointer()->value = data;

    return draw_data_ptr;
}

extern "C" Value imgui_render_draw_data(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'render_draw_data' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value data_ptr = args[0];

    if (!data_ptr.is_pointer())
    {
        error("Function 'render_draw_data' expects arg 'data' to be a pointer");
    }

    ImDrawData *data = (ImDrawData *)data_ptr.get_pointer()->value;

    ImGui_ImplSDLRenderer2_RenderDrawData(data);

    return boolean_val(true);
}

extern "C" Value imgui_show_demo_window(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'show_demo_window' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    ImGui::ShowDemoWindow();

    return boolean_val(true);
}

extern "C" Value imgui_begin(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'begin' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value name = args[0];

    if (!name.is_string())
    {
        error("Function 'begin' expects arg 'name' to be a string");
    }

    bool result = ImGui::Begin(name.get_string().c_str());

    return boolean_val(result);
}

extern "C" Value imgui_end(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'end' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    ImGui::End();

    return boolean_val(true);
}

extern "C" Value imgui_text(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'text' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value text = args[0];

    if (!text.is_string())
    {
        error("Function 'text' expects arg 'text' to be a string");
    }

    ImGui::Text(text.get_string().c_str());

    return boolean_val(true);
}

extern "C" Value imgui_button(std::vector<Value> &args)
{
    int num_required_args = 3;

    if (args.size() != num_required_args)
    {
        error("Function 'button' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value text = args[0];
    Value x = args[1];
    Value y = args[2];

    if (!text.is_string())
    {
        error("Function 'button' expects arg 'text' to be a string");
    }

    if (!x.is_number())
    {
        error("Function 'button' expects arg 'x' to be a number");
    }

    if (!y.is_number())
    {
        error("Function 'button' expects arg 'y' to be a number");
    }

    bool result = ImGui::Button(text.get_string().c_str(), ImVec2(x.get_number(), y.get_number()));

    return boolean_val(result);
}

extern "C" Value imgui_checkbox(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'checkbox' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value text = args[0];
    Value checked = args[1];

    if (!text.is_string())
    {
        error("Function 'checkbox' expects arg 'text' to be a string");
    }

    if (!checked.is_boolean())
    {
        error("Function 'checkbox' expects arg 'checked' to be a boolean");
    }

    bool result = ImGui::Checkbox(text.get_string().c_str(), checked.get_boolean());

    return boolean_val(result);
}
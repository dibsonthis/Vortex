#include "include/Vortex.hpp"
#include "include/imgui/SDL.h"
#include "include/imgui/imgui.h"
#include "include/imgui/imgui_impl_sdl2.h"
#include "include/imgui/imgui_impl_sdlrenderer2.h"

extern "C" Value check_version(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'check_version' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    bool result = IMGUI_CHECKVERSION();

    return boolean_val(result);
}

extern "C" Value create_context(std::vector<Value> &args)
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

extern "C" Value get_io(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'get_io' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    auto io = std::make_shared<ImGuiIO>(ImGui::GetIO());
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Mouse Controls

    Value ioPointer = pointer_val();
    ioPointer.get_pointer()->value = io.get();

    return ioPointer;
}

extern "C" Value init_for_sdl(std::vector<Value> &args)
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

extern "C" Value sdl_render_init(std::vector<Value> &args)
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
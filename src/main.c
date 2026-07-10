#define SDL_MAIN_USE_CALLBACKS 1 // Use the callbacks instead of main()

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cglm/cglm.h>

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif // __EMSCRIPTEN__

#include "file_utils.h"
#include "line_drawer.h"
#include "shader_program.h"

typedef struct
{
    SDL_Window *window;
    SDL_GLContext glContext;
    GLuint shaderProgram;
    LineDrawer *lineDrawer;
    bool projectionNeedsUpdate;
} App;

void update_projection(void *appState, int width, int height)
{
    App *app = (App *)appState;
    float aspect = (float)width / (float)height;

    mat4 projMatrix, viewMatrix, projViewMatrix;

    // glm_perspective(fovy, aspect, near, far, dest)
    // 45 degrees FOV is standard for most 3D applications
    glm_perspective(glm_rad(45.0f), aspect, 0.1f, 1000.0f, projMatrix);

    // In 3D, you usually move the camera back so objects are in front of it
    vec3 eye = { 50.f, 50.f, 230.f }; // Camera pulled back on the Z-axis
    vec3 center = { 0.f, 0.f, 0.f };
    vec3 up = { 0.f, 1.f, 0.f };
    glm_lookat(eye, center, up, viewMatrix);

    glm_mat4_mul(projMatrix, viewMatrix, projViewMatrix);
    LineDrawer_setProjViewMatrix(app->lineDrawer, projViewMatrix);
}

// This function runs once at startup
SDL_AppResult SDL_AppInit(void **appState, int argc, char *argv[])
{
    App *app = (App *)SDL_malloc(sizeof(App));
    *appState = app;

#ifndef __EMSCRIPTEN__
    if (!SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "60"))
    {
        SDL_Log("Failed to set a frame rate: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
#endif

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); // Enable MULTISAMPLE
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8); // Can be 2, 4, 8 or 16

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
    // Mobile and Web: Request OpenGL ES 3.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    // Windows/Desktop: Request OpenGL 3.3 Core Profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // Explicitly ask for forward compatibility for better driver support
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    int w = 480; // Default width for Windows
    int h = 480; // Default height for Windows

#if ANDROID
    flags |= SDL_WINDOW_FULLSCREEN;
    w = 0;
    h = 0;
#endif // ANDROID

    app->window = SDL_CreateWindow("C, SDL3, cglm, OpenGL", w, h, flags);
    if (!app->window)
    {
        SDL_Log("Couldn't create the window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    app->glContext = SDL_GL_CreateContext(app->window);
    if (!app->glContext)
    {
        SDL_Log("Couldn't create the glContext: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

#ifdef WIN32
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        SDL_Log("Failed to initialize OpenGL function pointers");
        return SDL_APP_FAILURE;
    }
#endif // WIN32

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);

    // Load your shader sources
#if defined(__ANDROID__)
    // On Android, the assets folder is the root
    char *vertexSource = readFile("shaders/shader.vert");
    char *fragmentSource = readFile("shaders/shader.frag");
#else
    // On Windows/Desktop, you might still use the "assets/" prefix
    // if your local folder structure keeps them there
    char *vertexSource = readFile("assets/shaders/shader.vert");
    char *fragmentSource = readFile("assets/shaders/shader.frag");
#endif

    // Validate that both loaded successfully
    if (vertexSource != NULL && fragmentSource != NULL)
    {
        // Pass the pointers to your creation function
        app->shaderProgram = createShaderProgram(vertexSource, fragmentSource);
        if (!app->shaderProgram)
        {
            return SDL_APP_FAILURE;
        }

        // Once the GPU has compiled the shaders, you can free the memory
        SDL_free(vertexSource);
        SDL_free(fragmentSource);
    }
    else
    {
        // Handle error: one or both shaders failed to load
        if (vertexSource)
            SDL_free(vertexSource);
        if (fragmentSource)
            SDL_free(fragmentSource);

        SDL_Log("Error: Could not load shader files.");
        return SDL_APP_FAILURE;
    }

    app->lineDrawer = LineDrawer_getInstance();
    mat4 dummyMatrix = GLM_MAT4_IDENTITY_INIT;
    LineDrawer_init(app->lineDrawer, app->shaderProgram, dummyMatrix);

    return SDL_APP_CONTINUE;
}

// This function runs when a new event (mouse input, keypresses, etc) occurs
SDL_AppResult SDL_AppEvent(void *appState, SDL_Event *event)
{
    App *app = (App *)appState;

    switch (event->type)
    {
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        {
            break;
        }
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        default:
            break;
    }

    return SDL_APP_CONTINUE;
}

// This function runs once per frame, and is the heart of the program
SDL_AppResult SDL_AppIterate(void *appState)
{
    App *app = (App *)appState;
    int winW, winH;
    SDL_GetWindowSizeInPixels(app->window, &winW, &winH);

    // Update projection every frame with the window's actual aspect ratio
    update_projection(app, winW, winH);

    // Fill the entire window
    glViewport(0, 0, winW, winH);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // First line
    vec3 start1 = { -60.f, 50.f, 0.f };
    vec3 end1 = { 40.f, 80.f, 0.f };
    vec3 color1 = { 1.f, 0.5f, 0.5f };
    LineDrawer_draw(app->lineDrawer, start1, end1, color1, 1.f);

    // Second line
    vec3 start2 = { -28.f, 37.f, 0.f };
    vec3 end2 = { 80.f, 25.f, 0.f };
    vec3 color2 = { 0.5f, 0.5f, 1.f };
    LineDrawer_draw(app->lineDrawer, start2, end2, color2, 3.f);

    // Polygonal chain
    vec3 polyColor = { 0.5f, 1.f, 0.5f };
    float polyThickness = 5.f;

    // We define the points to mirror the chain logic
    vec3 p0 = { -81.f, -73.f, 0.f };
    vec3 p1 = { -43.f, -20.f, 0.f };
    vec3 p2 = { 0.f, -5.f, 0.f };
    vec3 p3 = { 25.f, -10.f, 0.f };
    vec3 p4 = { 52.f, -70.f, 0.f };
    vec3 p5 = { 77.f, -5.f, 0.f };

    LineDrawer_draw(app->lineDrawer, p0, p1, polyColor, polyThickness);
    LineDrawer_draw(app->lineDrawer, p1, p2, polyColor, polyThickness);
    LineDrawer_draw(app->lineDrawer, p2, p3, polyColor, polyThickness);
    LineDrawer_draw(app->lineDrawer, p3, p4, polyColor, polyThickness);
    LineDrawer_draw(app->lineDrawer, p4, p5, polyColor, polyThickness);

    // Axis helper: X-axis (Red), Y-axis (Green), Z-axis (Blue)
    float axisLength = 80.0f;
    vec3 origin = { 0.f, 0.f, 0.f };

    // X-axis
    vec3 xAxis = { axisLength, 0.f, 0.f };
    vec3 xColor = { 1.f, 0.f, 0.f };
    LineDrawer_draw(app->lineDrawer, origin, xAxis, xColor, 1.f);

    // Y-axis
    vec3 yAxis = { 0.f, axisLength, 0.f };
    vec3 yColor = { 0.f, 1.f, 0.f };
    LineDrawer_draw(app->lineDrawer, origin, yAxis, yColor, 1.f);

    // Z-axis
    vec3 zAxis = { 0.f, 0.f, axisLength };
    vec3 zColor = { 0.f, 0.f, 1.f };
    LineDrawer_draw(app->lineDrawer, origin, zAxis, zColor, 1.f);

    // Update the screen
    SDL_GL_SwapWindow(app->window);
    return SDL_APP_CONTINUE;
}

// This function runs once at shutdown
void SDL_AppQuit(void *appState, SDL_AppResult result)
{
    App *app = (App *)appState;
    glDeleteProgram(app->shaderProgram);
    LineDrawer_cleanup();
    SDL_GL_DestroyContext(app->glContext);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
}

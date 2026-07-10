#ifndef LINE_DRAWER_H
#define LINE_DRAWER_H

#include <cglm/cglm.h>

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif // __EMSCRIPTEN__

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        GLuint shaderProgram;
        GLuint vao;
        GLuint vbo;
        GLint uColorLocation;
        GLint uMvpLocation;
        mat4 projViewMatrix;
        mat4 modelMatrix;
        mat4 mvpMatrix;
        mat4 rotationMatrix;
        int vertCount;
    } LineDrawer;

    // Initialization (constructor-like)
    void LineDrawer_init(LineDrawer *self, GLuint shaderProgram,
        mat4 projViewMatrix);

    // Get the line drawer instance
    LineDrawer *LineDrawer_getInstance(void);

    // Setters
    void LineDrawer_setProjViewMatrix(LineDrawer *self, mat4 projViewMatrix);

    // Draw function
    void LineDrawer_draw(LineDrawer *self, vec3 start, vec3 end,
        vec3 color, float thickness);

    // Cleanup (destructor-lie)
    void LineDrawer_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // LINE_DRAWER_H

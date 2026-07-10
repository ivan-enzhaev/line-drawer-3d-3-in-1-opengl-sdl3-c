#include "line_drawer.h"
#include "math_helper.h"

#include <SDL3/SDL.h>
#include <math.h>

static LineDrawer *instance = NULL;

// Constructor-like initialization
void LineDrawer_init(LineDrawer *self, GLuint shaderProgram, mat4 projViewMatrix)
{
    if (self)
    {
        self->shaderProgram = shaderProgram;
        glm_mat4_copy(projViewMatrix, self->projViewMatrix);

        // Set up vertex data and buffers
        // clang-format off
        float vertices[] = {
            -0.5f, -0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, 0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, 0.5f,
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            -0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f
        };
        // clang-format on
        self->vertCount = sizeof(vertices) / (sizeof(float) * 3);

        // Generate and bind VAO
        glGenVertexArrays(1, &self->vao);
        glBindVertexArray(self->vao);

        // Generate and bind VBO
        glGenBuffers(1, &self->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Setup Layout 0
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

        // Unbind to prevent accidental state changes
        glBindVertexArray(0);

        // Activate the current shader program to access shader variables
        glUseProgram(self->shaderProgram);

        // Uniforms
        self->uColorLocation = glGetUniformLocation(shaderProgram, "uColor");
        self->uMvpLocation = glGetUniformLocation(shaderProgram, "uMvpMatrix");
    }
}

LineDrawer *LineDrawer_getInstance(void)
{
    if (instance == NULL)
    {
        instance = malloc(sizeof(LineDrawer));
    }
    return instance;
}

void LineDrawer_setProjViewMatrix(LineDrawer *self, mat4 projViewMatrix)
{
    glm_mat4_copy(projViewMatrix, self->projViewMatrix);
}

void LineDrawer_draw(LineDrawer *self, vec3 start, vec3 end, vec3 color, float thickness)
{
    if (!self)
        return;

    // Compute vector v = end - start
    vec3 v;
    glm_vec3_sub(end, start, v);

    // centerPosition = start + v / 2
    vec3 center;
    glm_vec3_scale(v, 0.5f, center);
    glm_vec3_add(start, center, center);

    // length = ||v||
    float length = glm_vec3_norm(v);
    vec3 norm;
    if (length > 1e-8f)
    {
        glm_vec3_normalize_to(v, norm);
    }
    else
    {
        // Degenerate segment: point; pick X axis
        norm[0] = 1.0f;
        norm[1] = 0.0f;
        norm[2] = 0.0f;
        length = 0.0f;
    }

    versor rotation;
    vec3 from = { 1.0f, 0.0f, 0.0f };
    MathHelper_rotationTo(from, norm, rotation);

    // Build model matrix
    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, center);

    mat4 rotMat;
    glm_quat_mat4(rotation, rotMat);
    glm_mat4_mul(model, rotMat, model);

    mat4 scaleMat = GLM_MAT4_IDENTITY_INIT;
    scaleMat[0][0] = length;
    scaleMat[1][1] = thickness;
    scaleMat[2][2] = thickness;
    glm_mat4_mul(model, scaleMat, model);

    mat4 mvp;
    glm_mat4_mul(self->projViewMatrix, model, mvp);

    // Upload uniforms
    glUseProgram(self->shaderProgram);
    glUniformMatrix4fv(self->uMvpLocation, 1, GL_FALSE, (const GLfloat *)mvp);
    glUniform3fv(self->uColorLocation, 1, (const GLfloat *)color);

    // Bind VAO and Draw
    glBindVertexArray(self->vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, self->vertCount);

    // Clean up binding
    glBindVertexArray(0);
}

void LineDrawer_cleanup(void)
{
    free(instance);
    instance = NULL;
}

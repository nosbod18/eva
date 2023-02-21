#define EVA_IMPLEMENTATION
#define WTK_IMPLEMENTATION
#include "../eva.h"
#include "wtk/wtk.h"

#define GLSL(code) "#version 330 core\n" #code

float vertices[] = {
     0.5f,  0.5f, 0.8f, 0.0f, 0.0f,
     0.5f, -0.5f, 0.0f, 0.8f, 0.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 0.8f,
    -0.5f,  0.5f, 0.8f, 0.0f, 0.8f,
};

unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};

char const *vs_src = GLSL(
    layout (location = 0) in vec2 a_pos;
    layout (location = 1) in vec3 a_color;

    out vec4 v_color;

    void main() {
        gl_Position = vec4(a_pos, 0.0, 1.0);
        v_color = vec4(a_color, 1.0);
    }
);

char const *fs_src = GLSL(
    in vec4 v_color;
    out vec4 f_color;

    void main() {
        f_color = v_color;
    }
);

int main(void) {
    WtkWindow *wnd = WtkCreateWindow(&(WtkWindowDesc){0});
    WtkMakeCurrent(wnd);

    EvaBuffer *vbo = EvaCreateBuffer(&(EvaBufferDesc){
        .data   = vertices,
        .size   = sizeof vertices,
        .layout = {EVA_VERTEXFORMAT_FLOAT2, EVA_VERTEXFORMAT_FLOAT3},
    });

    EvaBuffer *ibo = EvaCreateBuffer(&(EvaBufferDesc){
        .data = indices,
        .size = sizeof indices,
    });

    EvaShader *shader = EvaCreateShader(&(EvaShaderDesc){
        .sources = {vs_src, fs_src}
    });

    EvaBindings bindings = {
        .vbos   = {vbo},
        .ibo    = ibo,
        .shader = shader
    };

    while (!WtkGetWindowShouldClose(wnd)) {
        int w, h;
        WtkGetWindowSize(wnd, &w, &h);

        EvaBeginPass(&(EvaPassDesc){.clear = {0.1f, 0.1f, 0.1f, 1.0f}, .viewport = {.w = w, .h = h}});
            EvaApplyBindings(&bindings);
            EvaDraw(6);
        EvaEndPass();

        WtkSwapBuffers(wnd);
        WtkPollEvents();
    }

    EvaDeleteShader(shader);
    EvaDeleteBuffer(ibo);
    EvaDeleteBuffer(vbo);
    WtkDeleteWindow(wnd);
}


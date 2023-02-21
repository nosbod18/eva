#define EVA_IMPLEMENTATION
#define WTK_IMPLEMENTATION
#include "../eva.h"
#include "wtk/wtk.h"

#define GLSL(code) "#version 330 core\n" #code

float vertices[] = {
     0.0f,  0.5f,
     0.5f, -0.5f,
    -0.5f, -0.5f,
};

char const *vs_src = GLSL(
    layout (location = 0) in vec2 a_pos;

    void main() {
        gl_Position = vec4(a_pos, 0.0, 1.0);
    }
);

char const *fs_src = GLSL(
    out vec4 f_color;

    void main() {
        f_color = vec4(0.0, 0.7, 0.8, 1.0);
    }
);

int main(void) {
    WtkWindow *wnd = WtkCreateWindow(&(WtkWindowDesc){0});
    WtkMakeCurrent(wnd);

    EvaBuffer *vbo = EvaCreateBuffer(&(EvaBufferDesc){
        .data   = vertices,
        .size   = sizeof vertices,
        .layout = {EVA_VERTEXFORMAT_FLOAT2},
    });

    EvaShader *shader = EvaCreateShader(&(EvaShaderDesc){
        .sources = {vs_src, fs_src}
    });

    EvaBindings bindings = {
        .vbos = {vbo},
        .shader = shader
    };

    while (!WtkGetWindowShouldClose(wnd)) {
        int w, h;
        WtkGetWindowSize(wnd, &w, &h);

        EvaBeginPass(&(EvaPassDesc){.clear = {0.1f, 0.1f, 0.1f, 1.0f}, .viewport = {.w = w, .h = h}});
            EvaApplyBindings(&bindings);
            EvaDraw(3);
        EvaEndPass();

        WtkSwapBuffers(wnd);
        WtkPollEvents();
    }

    EvaDeleteShader(shader);
    EvaDeleteBuffer(vbo);
    WtkDeleteWindow(wnd);
}


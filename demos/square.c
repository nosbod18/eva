#include "eva/eva.h"
#include "wtk/wtk.h"

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

char const *vs_src =
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos;\n"
    "layout (location = 1) in vec3 a_color;\n"
    "out vec4 v_color;\n"
    "void main() {\n"
    "   gl_Position = vec4(a_pos, 0.0, 1.0);\n"
    "   v_color = vec4(a_color, 1.0);\n"
    "}\n";

char const *fs_src =
    "#version 330 core\n"
    "in vec4 v_color;\n"
    "out vec4 f_color;\n"
    "void main() {\n"
    "   f_color = v_color;\n"
    "}\n";

int main(void) {
    WtkWindow *wnd = WtkCreateWindow(&(WtkWindowDesc){0});
    WtkMakeCurrent(wnd);

    EvaBuffer *vbo = EvaCreateBuffer(&(EvaBufferDesc){
        .data = vertices,
        .size = sizeof vertices,
        .layout = {EVA_FLOAT2, EVA_FLOAT3},
    });

    EvaBuffer *ibo = EvaCreateBuffer(&(EvaBufferDesc){
        .data = indices,
        .size = sizeof indices,
    });

    EvaShader *shader = EvaCreateShader(&(EvaShaderDesc){
        .vs_src = vs_src,
        .fs_src = fs_src,
    });

    EvaBindings bindings = {
        .vbos = {vbo},
        .ibo  = ibo,
    };

    while (!WtkGetWindowShouldClose(wnd)) {
        EvaClear(0.1f, 0.1f, 0.1f, 1.0f);
        EvaDraw(&bindings, shader, 6);

        WtkSwapBuffers(wnd);
        WtkPollEvents();
    }

    EvaDeleteShader(shader);
    EvaDeleteBuffer(ibo);
    EvaDeleteBuffer(vbo);
    WtkDeleteWindow(wnd);
}


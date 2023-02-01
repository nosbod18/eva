#include "eva/eva.h"
#include "wtk/wtk.h"

float vertices[] = {
     0.0f,  0.5f,
     0.5f, -0.5f,
    -0.5f, -0.5f,
};

char const *vs_src =
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos;\n"
    "void main() {\n"
    "   gl_Position = vec4(a_pos, 0.0, 1.0);\n"
    "}\n";

char const *fs_src =
    "#version 330 core\n"
    "out vec4 f_color;\n"
    "void main() {\n"
    "   f_color = vec4(0.0, 0.7, 0.8, 1.0);\n"
    "}\n";

int main(void) {
    WtkWindow *wnd = WtkCreateWindow(&(WtkWindowDesc){0});
    WtkMakeCurrent(wnd);

    EvaBuffer *vbo = EvaCreateBuffer(&(EvaBufferDesc){
        .data = vertices,
        .size = sizeof vertices,
        .layout = {EVA_FLOAT2},
    });

    EvaShader *shader = EvaCreateShader(&(EvaShaderDesc){
        .vs_src = vs_src,
        .fs_src = fs_src,
    });

    EvaBindings bindings = {
        .vbos = {vbo},
    };

    while (!WtkGetWindowShouldClose(wnd)) {
        EvaClear(0.1f, 0.1f, 0.1f, 1.0f);
        EvaDraw(&bindings, shader, 3);

        WtkSwapBuffers(wnd);
        WtkPollEvents();
    }

    EvaDeleteShader(shader);
    EvaDeleteBuffer(vbo);
    WtkDeleteWindow(wnd);
}


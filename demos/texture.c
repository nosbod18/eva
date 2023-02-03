#include "eva/eva.h"
#include "wtk/wtk.h"

float vertices[] = {
     0.5f,  0.5f, 1.0f, 1.0f,
     0.5f, -0.5f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f, 1.0f,
};

unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};

unsigned int pixels[] = {
    0x00000000, 0xffffffff, 0x00000000,
    0xffffffff, 0x00000000, 0xffffffff,
    0x00000000, 0xffffffff, 0x00000000,
};

char const *vs_src =
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos;\n"
    "layout (location = 1) in vec2 a_uv;\n"
    "out vec2 v_uv;\n"
    "void main() {\n"
    "   gl_Position = vec4(a_pos, 0.0, 1.0);\n"
    "   v_uv = a_uv;\n"
    "}\n";

char const *fs_src =
    "#version 330 core\n"
    "in vec2 v_uv;\n"
    "uniform sampler2D u_tex;\n"
    "out vec4 f_color;\n"
    "void main() {\n"
    "   f_color = texture(u_tex, v_uv);\n"
    "}\n";

int main(void) {
    WtkWindow *wnd = WtkCreateWindow(&(WtkWindowDesc){0});
    WtkMakeCurrent(wnd);

    EvaBuffer *vbo = EvaCreateBuffer(&(EvaBufferDesc){
        .data = vertices,
        .size = sizeof vertices,
        .layout = {EVA_VERTEXFORMAT_FLOAT2, EVA_VERTEXFORMAT_FLOAT2},
    });

    EvaBuffer *ibo = EvaCreateBuffer(&(EvaBufferDesc){
        .data = indices,
        .size = sizeof indices,
    });

    EvaShader *shader = EvaCreateShader(&(EvaShaderDesc){
        .vs_src   = vs_src,
        .fs_src   = fs_src,
        .uniforms = {{.name = "u_tex", .format = EVA_UNIFORMFORMAT_IMAGE2D}}
    });

    EvaImage *image = EvaCreateImage(&(EvaImageDesc){
        .data   = pixels,
        .width  = 3,
        .height = 3
    });

    EvaBindings bindings = {
        .vbos   = {vbo},
        .ibo    = ibo,
        .images = {image}
    };

    while (!WtkGetWindowShouldClose(wnd)) {
        EvaClear(0.1f, 0.1f, 0.1f, 1.0f);
        EvaApplyUniforms(shader, image);
        EvaDraw(&bindings, shader, 6);

        WtkSwapBuffers(wnd);
        WtkPollEvents();
    }

    EvaDeleteShader(shader);
    EvaDeleteBuffer(ibo);
    EvaDeleteBuffer(vbo);
    WtkDeleteWindow(wnd);
}


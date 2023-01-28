#include "../eva.h"
#include "wtk/wtk.h"

float const vertices[] = {
     0.5f,  0.5f,  0.8f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.0f, 0.8f, 0.0f,
    -0.5f, -0.5f,  0.0f, 0.0f, 0.8f,
    -0.5f,  0.5f,  0.8f, 0.0f, 0.8f,
};

unsigned int const indices[] = {
    0, 1, 2,
    2, 3, 0,
};

char const *vss =
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos;\n"
    "layout (location = 1) in vec3 a_color;\n"
    "out vec4 v_color;\n"
    "void main() {\n"
    "   gl_Position = vec4(a_pos, 0.0, 1.0);\n"
    "   v_color = vec4(a_color, 1.0);\n"
    "}\n";

char const *fss =
    "#version 330 core\n"
    "in vec4 v_color;\n"
    "out vec4 f_color;\n"
    "void main() {\n"
    "   f_color = v_color;\n"
    "}\n";

int main(void) {
    WtkWindow *window = WtkCreateWindow(&(WtkWindowDesc){0});
    WtkMakeCurrent(window);

    EvaBuffer *vbo = EvaCreateBuffer(&(EvaBufferDesc){
        .data = vertices,
        .size = sizeof vertices,
    });

    EvaBuffer *ibo = EvaCreateBuffer(&(EvaBufferDesc){
        .data = indices,
        .size = sizeof indices,
        .type = EVA_BUFFERTYPE_INDEX,
    });

    EvaShader *shader = EvaCreateShader(&(EvaShaderDesc){
        .vs.src = vss,
        .fs.src = fss,
    });

    EvaPipeline *pipeline = EvaCreatePipeline(&(EvaPipelineDesc){
        .layout = {
            [0].format = EVA_VERTEXFORMAT_FLOAT2,
            [1].format = EVA_VERTEXFORMAT_FLOAT3
        },
        .vertexBuffers  = {vbo},
        .indexBuffer    = ibo,
        .shader         = shader,
    });

    while (!WtkGetWindowShouldClose(window)) {
        EvaClear(0.1f, 0.1f, 0.1f, 1.0f);
        EvaDraw(pipeline, 6);

        WtkSwapBuffers(window);
        WtkPollEvents();
    }

    EvaDeletePipeline(pipeline);
    EvaDeleteShader(shader);
    EvaDeleteBuffer(vbo);
    WtkDeleteWindow(window);
}

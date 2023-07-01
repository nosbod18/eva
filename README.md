# eva
A tiny OpenGL wrapper to make life a little easier.

```c
#define EVA_IMPLEMENTATION
#define WTK_IMPLEMENTATION
#include "eva/eva.h"
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
    WtkWindow *window = WtkCreateWindow(&(WtkWindowDesc){0});
    WtkMakeCurrent(window);

    EvaBuffer *vbo = EvaCreateBuffer(&(EvaBufferDesc){
        .data   = vertices,
        .size   = sizeof vertices,
        .layout = {EVA_VERTEXFORMAT_FLOAT2}
    });

    EvaShader *shader = EvaCreateShader(&(EvaShaderDesc){
        .sources = {vs_src, fs_src}
    });

    EvaBindings bindings = {
        .vbos = {vbo}
    };

    while (!WtkGetWindowShouldClose(window)) {
        int w, h;
        WtkGetWindowSize(window, &w, &h);

        EvaBeginPass(&(EvaPassDesc){.clear = {0.1f, 0.1f, 0.1f, 1.0f}, .viewport = {.w = w, .h = h}});
            EvaApplyBindings(&bindings);
            EvaDraw(3);
        EvaEndPass();

        WtkSwapBuffers(window);
        WtkPollEvents();
    }

    EvaDeleteShader(shader);
    EvaDeleteBuffer(vbo);
    WtkDeleteWindow(window);
}
```

## License
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>

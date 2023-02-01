#include "eva.h"

#define GLAD_GL_IMPLEMENTATION
#include "glad.h"

#include <stdio.h>

///
/// Types
///

typedef struct VertexAttribute {
    int  format;
    int  count;
    int  normalized;
    long offset;
} VertexAttribute;

struct EvaBuffer {
    VertexAttribute attributes[EVA_BUFFER_MAX_ATTRIBUTES];
    int num_attributes;
    int usage;
    size_t size;
    size_t stride;
    unsigned int id;
};

typedef struct Uniform {
    int location;
    int format;
    int count;
} Uniform;

struct EvaShader {
    Uniform uniforms[EVA_SHADER_MAX_UNIFORMS];
    int num_uniforms;
    unsigned int id;
};

///
/// Global / Init
///

static struct {
    unsigned int vao;
} G = {0};

static void InitIfNeeded(void) {
    if (G.vao == 0) {
        gladLoaderLoadGL();
        glGenVertexArrays(1, &G.vao);
        glBindVertexArray(G.vao);
    }
}

///
/// Buffer
///

static int TranslateBufferUsage(int usage) {
    switch (usage) {
        case EVA_BUFFERUSAGE_STATIC:  return GL_STATIC_DRAW;
        case EVA_BUFFERUSAGE_DYNAMIC: return GL_DYNAMIC_DRAW;
    }
    return 0;
}

static VertexAttribute TranslateVertexFormat(int format, size_t *size) {
    switch (format) {
        case EVA_VERTEXFORMAT_INT:    *size = 4; return (VertexAttribute){.format = GL_INT,   .count = 1};
        case EVA_VERTEXFORMAT_INT2:   *size = 4; return (VertexAttribute){.format = GL_INT,   .count = 2};
        case EVA_VERTEXFORMAT_INT3:   *size = 4; return (VertexAttribute){.format = GL_INT,   .count = 3};
        case EVA_VERTEXFORMAT_INT4:   *size = 4; return (VertexAttribute){.format = GL_INT,   .count = 4};
        case EVA_VERTEXFORMAT_FLOAT:  *size = 4; return (VertexAttribute){.format = GL_FLOAT, .count = 1};
        case EVA_VERTEXFORMAT_FLOAT2: *size = 4; return (VertexAttribute){.format = GL_FLOAT, .count = 2};
        case EVA_VERTEXFORMAT_FLOAT3: *size = 4; return (VertexAttribute){.format = GL_FLOAT, .count = 3};
        case EVA_VERTEXFORMAT_FLOAT4: *size = 4; return (VertexAttribute){.format = GL_FLOAT, .count = 4};
    }
    *size = 0;
    return (VertexAttribute){0};
}

EvaBuffer *EvaCreateBuffer(EvaBufferDesc *desc) {
    InitIfNeeded();

    EvaBuffer *buffer = calloc(1, sizeof *buffer);
    buffer->size = desc->size;
    buffer->usage = TranslateBufferUsage(desc->usage);

    int type = (desc->layout[0] == EVA_VERTEXFORMAT_INVALID) ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;

    glGenBuffers(1, &buffer->id);
    glBindBuffer(type, buffer->id);
    glBufferData(type, buffer->size, desc->data, buffer->usage);
    glBindBuffer(type, 0);

    for (int i = 0; i < EVA_BUFFER_MAX_ATTRIBUTES && desc->layout[i] != EVA_VERTEXFORMAT_INVALID; i++) {
        size_t size;
        buffer->attributes[i] = TranslateVertexFormat(desc->layout[i], &size);
        buffer->attributes[i].offset = buffer->stride;
        buffer->stride += buffer->attributes[i].count * size;
        buffer->num_attributes++;
    }

    return buffer;
}

void EvaDeleteBuffer(EvaBuffer *buffer) {
    glDeleteBuffers(1, &buffer->id);
    free(buffer);
}

///
/// Shader
///

static Uniform TranslateUniformFormat(int format) {
    switch (format) {
        case EVA_UNIFORMFORMAT_INT:    return (Uniform){.format = GL_INT, .count = 1};
        case EVA_UNIFORMFORMAT_INT2:   return (Uniform){.format = GL_INT, .count = 2};
        case EVA_UNIFORMFORMAT_INT3:   return (Uniform){.format = GL_INT, .count = 3};
        case EVA_UNIFORMFORMAT_INT4:   return (Uniform){.format = GL_INT, .count = 4};
        case EVA_UNIFORMFORMAT_FLOAT:  return (Uniform){.format = GL_FLOAT, .count = 1};
        case EVA_UNIFORMFORMAT_FLOAT2: return (Uniform){.format = GL_FLOAT, .count = 2};
        case EVA_UNIFORMFORMAT_FLOAT3: return (Uniform){.format = GL_FLOAT, .count = 3};
        case EVA_UNIFORMFORMAT_FLOAT4: return (Uniform){.format = GL_FLOAT, .count = 4};
    }
    return (Uniform){0};
}

EvaShader *EvaCreateShader(EvaShaderDesc *desc) {
    InitIfNeeded();

    EvaShader *shader = calloc(1, sizeof *shader);

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &desc->vs_source, NULL);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &desc->fs_source, NULL);
    glCompileShader(fs);

    shader->id = glCreateProgram();
    glAttachShader(shader->id, vs);
    glAttachShader(shader->id, fs);
    glLinkProgram(shader->id);
    glDeleteShader(vs);
    glDeleteShader(fs);

    for (int i = 0; i < EVA_SHADER_MAX_UNIFORMS && desc->uniforms[i].name != NULL; i++) {
        shader->uniforms[i] = TranslateUniformFormat(desc->uniforms[i].format);
        shader->uniforms[i].location = glGetUniformLocation(shader->id, desc->uniforms[i].name);
        shader->num_uniforms++;
    }

    return shader;
}

void EvaDeleteShader(EvaShader *shader) {
    glDeleteProgram(shader->id);
    free(shader);
}

///
/// Commands
///

void EvaClear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void EvaSendUniforms(EvaShader *shader, void const *data) {
    glUseProgram(shader->id);

    for (int i = 0; i < shader->num_uniforms; i++) {
        Uniform u = shader->uniforms[i];
        switch (u.format) {
            case GL_INT:
                switch (u.count) {
                    case 1: glUniform1iv(u.location, 1, data); break;
                    case 2: glUniform2iv(u.location, 1, data); break;
                    case 3: glUniform3iv(u.location, 1, data); break;
                    case 4: glUniform4iv(u.location, 1, data); break;
                    default:                                   break;
                }
                data = (int const *)data + u.count;
            break;

            case GL_FLOAT:
                switch (u.count) {
                    case 1:  glUniform1fv(u.location, 1, data);          break;
                    case 2:  glUniform2fv(u.location, 1, data);          break;
                    case 3:  glUniform3fv(u.location, 1, data);          break;
                    case 4:  glUniform4fv(u.location, 1, data);          break;
                    case 12: glUniformMatrix3fv(u.location, 1, 0, data); break;
                    case 16: glUniformMatrix4fv(u.location, 1, 0, data); break;
                    default:                                             break;
                }
                data = (float const *)data + u.count;
            break;

            default:
            break;
        }
    }

    glUseProgram(0);
}

void EvaDraw(EvaBindings *bindings, EvaShader *shader, int count) {
    glUseProgram(shader->id);

    int attrib = 0;
    for (int i = 0; i < EVA_BINDINGS_MAX_VERTEX_BUFFERS && bindings->vbos[i] != NULL; i++) {
        EvaBuffer *vbo = bindings->vbos[i];
        glBindBuffer(GL_ARRAY_BUFFER, vbo->id);

        for (int j = 0; j < vbo->num_attributes; j++) {
            VertexAttribute va = vbo->attributes[j];
            glEnableVertexAttribArray(attrib);

            if (i == 0) {
                glVertexAttribPointer(attrib, va.count, va.format, va.normalized, vbo->stride, (void *)va.offset);
            } else {
                glVertexAttribPointer(attrib, va.count, va.format, va.normalized, 0, 0);
            }

            attrib++;
        }
    }

    if (bindings->ibo != NULL) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bindings->ibo->id);
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, NULL);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, count);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
}


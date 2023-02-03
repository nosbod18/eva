#include "eva.h"

#if defined(__linux__)
    #define GL_GLEXT_PROTOTYPES
    #include <GL/gl.h>
#endif

#include <stdio.h>
#include <stdlib.h>

///
/// Types
///

typedef struct VertexAttribute {
    int format;
    int count;
    int normalized;
    long offset;
} VertexAttribute;

struct EvaBuffer {
    VertexAttribute attributes[EVA_BUFFER_MAX_ATTRIBUTES];
    int num_attributes;
    size_t stride;
    unsigned int id;
};

typedef struct Uniform {
    int location;
    int format;
    size_t offset;
} Uniform;

struct EvaShader {
    Uniform uniforms[EVA_SHADER_MAX_UNIFORMS];
    int num_uniforms;
    unsigned int id;
};

struct EvaImage {
    unsigned int id;
    int slot;
    int width;
    int height;
};

///
/// Global / Init
///

static struct {
    unsigned int vao;
} _eva = {0};

static void InitIfNeeded(void) {
    if (_eva.vao == 0) {
        glGenVertexArrays(1, &_eva.vao);
        glBindVertexArray(_eva.vao);
    }
}

///
/// Buffer
///

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

    int type = (desc->layout[0] == 0) ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
    int usage = (desc->data == NULL) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glGenBuffers(1, &buffer->id);
    glBindBuffer(type, buffer->id);
    glBufferData(type, desc->size, desc->data, usage);
    glBindBuffer(type, 0);

    for (int i = 0; i < EVA_BUFFER_MAX_ATTRIBUTES && desc->layout[i] != 0; i++) {
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

static size_t GetUniformFormatSize(int format) {
    switch (format) {
        case EVA_UNIFORMFORMAT_INT:     return sizeof(int)   * 1;
        case EVA_UNIFORMFORMAT_INT2:    return sizeof(int)   * 2;
        case EVA_UNIFORMFORMAT_INT3:    return sizeof(int)   * 3;
        case EVA_UNIFORMFORMAT_INT4:    return sizeof(int)   * 4;
        case EVA_UNIFORMFORMAT_FLOAT:   return sizeof(float) * 1;
        case EVA_UNIFORMFORMAT_FLOAT2:  return sizeof(float) * 2;
        case EVA_UNIFORMFORMAT_FLOAT3:  return sizeof(float) * 3;
        case EVA_UNIFORMFORMAT_FLOAT4:  return sizeof(float) * 4;
        case EVA_UNIFORMFORMAT_MAT3:    return sizeof(float) * 12;
        case EVA_UNIFORMFORMAT_MAT4:    return sizeof(float) * 16;
        case EVA_UNIFORMFORMAT_IMAGE2D: return sizeof(EvaImage);
    }
    return 0;
}

static unsigned int CreateShaderStage(int stage, char const *src) {
    unsigned int shader = glCreateShader(stage);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == 0) {
        char log[512] = {0};
        glGetShaderInfoLog(shader, sizeof log, NULL, log);
        printf("%s\n", log);
    }

    return shader;
}

EvaShader *EvaCreateShader(EvaShaderDesc *desc) {
    InitIfNeeded();

    EvaShader *shader = calloc(1, sizeof *shader);
    shader->id = glCreateProgram();

    unsigned int vs = CreateShaderStage(GL_VERTEX_SHADER, desc->vs_src);
    unsigned int fs = CreateShaderStage(GL_FRAGMENT_SHADER, desc->fs_src);

    glAttachShader(shader->id, vs);
    glAttachShader(shader->id, fs);
    glLinkProgram(shader->id);

    glDeleteShader(vs);
    glDeleteShader(fs);

    for (int i = 0; i < EVA_SHADER_MAX_UNIFORMS && desc->uniforms[i].name != NULL; i++) {
        Uniform *u = &shader->uniforms[i];
        u->format = desc->uniforms[i].format;
        u->location = glGetUniformLocation(shader->id, desc->uniforms[i].name);

        if (i > 0) {
            u->offset = GetUniformFormatSize(desc->uniforms[i].format) + shader->uniforms[i - 1].offset;
        }

        shader->num_uniforms++;
    }

    return shader;
}

void EvaDeleteShader(EvaShader *shader) {
    glDeleteProgram(shader->id);
    free(shader);
}

///
/// Image
///

static int TranslateImageFormat(int format) {
    switch (format) {
        case EVA_IMAGEFORMAT_RGBA8: return GL_RGBA8;
        case EVA_IMAGEFORMAT_RGB8:  return GL_RGB8;
    }
    return 0;
}

static int TranslateImageWrap(int wrap) {
    switch (wrap) {
        case EVA_IMAGEWRAP_REPEAT:          return GL_REPEAT;
        case EVA_IMAGEWRAP_MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
        case EVA_IMAGEWRAP_CLAMP_TO_EDGE:   return GL_CLAMP_TO_EDGE;
        case EVA_IMAGEWRAP_CLAMP_TO_BORDER: return GL_CLAMP_TO_BORDER;
    }
    return 0;
}

static int TranslateImageFilter(int filter) {
    switch (filter) {
        case EVA_IMAGEFILTER_NEAREST:                   return GL_NEAREST;
        case EVA_IMAGEFILTER_LINEAR:                    return GL_LINEAR;
        case EVA_IMAGEFILTER_NEAREST_MIPMAP_NEAREST:    return GL_NEAREST_MIPMAP_NEAREST;
        case EVA_IMAGEFILTER_LINEAR_MIPMAP_NEAREST:     return GL_LINEAR_MIPMAP_NEAREST;
        case EVA_IMAGEFILTER_NEAREST_MIPMAP_LINEAR:     return GL_NEAREST_MIPMAP_LINEAR;
        case EVA_IMAGEFILTER_LINEAR_MIPMAP_LINEAR:      return GL_LINEAR_MIPMAP_LINEAR;
    }
    return 0;
}

EvaImage *EvaCreateImage(EvaImageDesc *desc) {
    EvaImage *image = calloc(1, sizeof *image);
    image->width = desc->width;
    image->height = desc->height;

    glGenTextures(1, &image->id);
    glBindTexture(GL_TEXTURE_2D, image->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TranslateImageWrap(desc->wrap.s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TranslateImageWrap(desc->wrap.t));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TranslateImageFilter(desc->filter.min));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TranslateImageFilter(desc->filter.mag));

    glTexImage2D(GL_TEXTURE_2D, 0, TranslateImageFormat(desc->format), desc->width, desc->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, desc->data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    return image;
}

void EvaDeleteImage(EvaImage *image) {
    glDeleteTextures(1, &image->id);
    free(image);
}

///
/// Commands
///

static void ApplyLayout(EvaBindings *bindings) {
    int attrib = 0;
    for (int i = 0; i < EVA_BINDINGS_MAX_VBOS && bindings->vbos[i] != NULL; i++) {
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
}

void EvaClear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void EvaApplyUniforms(EvaShader *shader, void *data) {
    glUseProgram(shader->id);

    int texture_slot = 0;
    for (int i = 0; i < shader->num_uniforms; i++) {
        Uniform u = shader->uniforms[i];
        void *ptr = (char *)data + u.offset;

        switch (u.format) {
            case EVA_UNIFORMFORMAT_INT:     glUniform1iv(u.location, 1, ptr);          break;
            case EVA_UNIFORMFORMAT_INT2:    glUniform2iv(u.location, 1, ptr);          break;
            case EVA_UNIFORMFORMAT_INT3:    glUniform3iv(u.location, 1, ptr);          break;
            case EVA_UNIFORMFORMAT_INT4:    glUniform4iv(u.location, 1, ptr);          break;
            case EVA_UNIFORMFORMAT_FLOAT:   glUniform1fv(u.location, 1, ptr);          break;
            case EVA_UNIFORMFORMAT_FLOAT2:  glUniform2fv(u.location, 1, ptr);          break;
            case EVA_UNIFORMFORMAT_FLOAT3:  glUniform3fv(u.location, 1, ptr);          break;
            case EVA_UNIFORMFORMAT_FLOAT4:  glUniform4fv(u.location, 1, ptr);          break;
            case EVA_UNIFORMFORMAT_MAT3:    glUniformMatrix3fv(u.location, 1, 0, ptr); break;
            case EVA_UNIFORMFORMAT_MAT4:    glUniformMatrix4fv(u.location, 1, 0, ptr); break;

            case EVA_UNIFORMFORMAT_IMAGE2D:
                glActiveTexture(GL_TEXTURE0 + texture_slot);
                glUniform1i(u.location, texture_slot);
                texture_slot++;
                break;

            default: break;
        }
    }

    glUseProgram(0);
}

void EvaDraw(EvaBindings *bindings, EvaShader *shader, int count) {
    ApplyLayout(bindings);

    glUseProgram(shader->id);

    for (int i = 0; i < EVA_BINDINGS_MAX_IMAGES && bindings->images[i] != NULL; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, bindings->images[i]->id);
    }

    if (bindings->ibo != NULL) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bindings->ibo->id);
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, NULL);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, count);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}


#include "eva.h"

#define GLAD_GL_IMPL
#include "glad.h"

#include <stdio.h>
#include <stdlib.h>

///
/// Types
///

typedef struct _eva_vertex_attr_desc_t {
    int format;
    int count;
    int normalized;
    size_t offset;
} _eva_vertex_attr_desc_t;

struct eva_buffer_t {
    _eva_vertex_attr_desc_t attributes[EVA_BUFFER_MAX_ATTRIBUTES];
    int nattributes;
    size_t stride;
    unsigned int id;
};

typedef struct _eva_uniform_desc_t {
    int location;
    int format;
    size_t offset;
} _eva_uniform_desc_t;

struct eva_shader_t {
    _eva_uniform_desc_t uniforms[EVA_SHADER_MAX_UNIFORMS];
    int nuniforms;
    unsigned int id;
};

struct eva_image_t {
    unsigned int id;
    int width;
    int height;
};

static struct {
    eva_bindings_desc_t bindings;
    eva_pipeline_desc_t pipeline;
    unsigned int vao;
    int initted;
} G = {0};

static void _eva_init(void) {
    gladLoaderLoadGL();
    glGenVertexArrays(1, &G.vao);
    glBindVertexArray(G.vao);
}

static _eva_vertex_attr_desc_t _eva_vertex_attr_translate(int format, size_t *size) {
    switch (format) {
        case EVA_VERTEXFORMAT_INT:    *size = 4; return (_eva_vertex_attr_desc_t){.format = GL_INT,   .count = 1};
        case EVA_VERTEXFORMAT_INT2:   *size = 4; return (_eva_vertex_attr_desc_t){.format = GL_INT,   .count = 2};
        case EVA_VERTEXFORMAT_INT3:   *size = 4; return (_eva_vertex_attr_desc_t){.format = GL_INT,   .count = 3};
        case EVA_VERTEXFORMAT_INT4:   *size = 4; return (_eva_vertex_attr_desc_t){.format = GL_INT,   .count = 4};
        case EVA_VERTEXFORMAT_FLOAT:  *size = 4; return (_eva_vertex_attr_desc_t){.format = GL_FLOAT, .count = 1};
        case EVA_VERTEXFORMAT_FLOAT2: *size = 4; return (_eva_vertex_attr_desc_t){.format = GL_FLOAT, .count = 2};
        case EVA_VERTEXFORMAT_FLOAT3: *size = 4; return (_eva_vertex_attr_desc_t){.format = GL_FLOAT, .count = 3};
        case EVA_VERTEXFORMAT_FLOAT4: *size = 4; return (_eva_vertex_attr_desc_t){.format = GL_FLOAT, .count = 4};
    }
    *size = 0;
    return (_eva_vertex_attr_desc_t){0};
}

static size_t _eva_uniform_format_size(int format) {
    switch (format) {
        case EVA_UNIFORMFORMAT_INT:     return  1 * sizeof(int);
        case EVA_UNIFORMFORMAT_INT2:    return  2 * sizeof(int);
        case EVA_UNIFORMFORMAT_INT3:    return  3 * sizeof(int);
        case EVA_UNIFORMFORMAT_INT4:    return  4 * sizeof(int);
        case EVA_UNIFORMFORMAT_FLOAT:   return  1 * sizeof(float);
        case EVA_UNIFORMFORMAT_FLOAT2:  return  2 * sizeof(float);
        case EVA_UNIFORMFORMAT_FLOAT3:  return  3 * sizeof(float);
        case EVA_UNIFORMFORMAT_FLOAT4:  return  4 * sizeof(float);
        case EVA_UNIFORMFORMAT_MAT3:    return 12 * sizeof(float);
        case EVA_UNIFORMFORMAT_MAT4:    return 16 * sizeof(float);
        case EVA_UNIFORMFORMAT_IMAGE2D: return  1 * sizeof(eva_image_t);
    }
    return 0;
}

static unsigned int _eva_shader_stage_create(int stage, char const *src) {
    unsigned int shader = glCreateShader(stage);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == 0) {
        char log[512] = {0};
        glGetShaderInfoLog(shader, sizeof log, NULL, log);
        fprintf(stderr, "%s\n", log);
    }

    return shader;
}

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

eva_buffer_t *eva_buffer_create(eva_buffer_desc_t *desc) {
    if (G.initted == 0)
        _eva_init();

    eva_buffer_t *buffer = calloc(1, sizeof *buffer);

    int type = (desc->layout[0] == 0) ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
    int usage = (desc->data == NULL) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glGenBuffers(1, &buffer->id);
    glBindBuffer(type, buffer->id);
    glBufferData(type, desc->size, desc->data, usage);
    glBindBuffer(type, 0);

    for (int i = 0; i < EVA_BUFFER_MAX_ATTRIBUTES && desc->layout[i] != 0; i++) {
        size_t size;
        buffer->attributes[i] = _eva_vertex_attr_translate(desc->layout[i], &size);
        buffer->attributes[i].offset = buffer->stride;
        buffer->stride += buffer->attributes[i].count * size;

        buffer->nattributes++;
    }

    return buffer;
}

eva_shader_t *eva_shader_create(eva_shader_desc_t *desc) {
    if (G.initted == 0)
        _eva_init();

    eva_shader_t *shader = calloc(1, sizeof *shader);
    shader->id = glCreateProgram();

    unsigned int vs = _eva_shader_stage_create(GL_VERTEX_SHADER, desc->sources[0].src);
    unsigned int fs = _eva_shader_stage_create(GL_FRAGMENT_SHADER, desc->sources[1].src);

    glAttachShader(shader->id, vs);
    glAttachShader(shader->id, fs);
    glLinkProgram(shader->id);

    glDeleteShader(vs);
    glDeleteShader(fs);

    size_t offset = 0;
    for (int i = 0; i < EVA_SHADER_MAX_UNIFORMS && desc->uniforms[i].name != NULL; i++) {
        _eva_uniform_desc_t *u = &shader->uniforms[i];
        u->format = desc->uniforms[i].format;
        u->location = glGetUniformLocation(shader->id, desc->uniforms[i].name);
        u->offset = offset;

        if (i > 0) {
            offset += _eva_uniform_format_size(desc->uniforms[i].format);
        }

        shader->nuniforms++;
    }

    return shader;
}

eva_image_t *eva_image_create(eva_image_desc_t *desc) {
    if (G.initted == 0)
        _eva_init();

    eva_image_t *image = calloc(1, sizeof *image);
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

void eva_buffer_delete(eva_buffer_t *buffer) {
    glDeleteBuffers(1, &buffer->id);
    free(buffer);
}

void eva_shader_delete(eva_shader_t *shader) {
    glDeleteProgram(shader->id);
    free(shader);
}

void eva_image_delete(eva_image_t *image) {
    glDeleteTextures(1, &image->id);
    free(image);
}

void eva_pass_begin(eva_pass_desc_t *desc) {
    glViewport(desc->viewport.x, desc->viewport.y, desc->viewport.w, desc->viewport.h);
    glClearColor(desc->clear.r, desc->clear.g, desc->clear.b, desc->clear.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void eva_uniforms_apply(void *data) {
    glUseProgram(G.pipeline.shader->id);

    int texture_slot = 0;
    for (int i = 0; i < G.pipeline.shader->nuniforms; i++) {
        _eva_uniform_desc_t u = G.pipeline.shader->uniforms[i];
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
            default:                                                                   break;
        }
    }

    glUseProgram(0);
}

void eva_bindings_apply(eva_bindings_desc_t *bindings) {
    int index = 0;
    for (int i = 0; i < EVA_BINDINGS_MAX_VBOS && bindings->vbos[i] != NULL; i++) {
        eva_buffer_t *vbo = bindings->vbos[i];
        glBindBuffer(GL_ARRAY_BUFFER, vbo->id);

        for (int j = 0; j < vbo->nattributes; j++) {
            _eva_vertex_attr_desc_t va = vbo->attributes[j];
            glEnableVertexAttribArray(index);

            if (i == 0) {
                glVertexAttribPointer(index, va.count, va.format, va.normalized, vbo->stride, (void *)va.offset);
            } else {
                glVertexAttribPointer(index, va.count, va.format, va.normalized, 0, 0);
            }

            index++;
        }
    }

    for (int i = 0; i < EVA_BINDINGS_MAX_IMAGES && bindings->images[i] != NULL; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, bindings->images[i]->id);
    }

    G.bindings = *bindings;
}

void eva_pipeline_apply(eva_pipeline_desc_t *pipeline) {
    if (pipeline->shader != G.pipeline.shader)
        glUseProgram(pipeline->shader->id);

    G.pipeline = *pipeline;
}

void eva_draw(int first, int count) {
    if (G.bindings.ibo != NULL) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, G.bindings.ibo->id);
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, NULL);
    } else {
        glDrawArrays(GL_TRIANGLES, first, count);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void eva_pass_end(void) {
    // Does nothing... for now...
}

// #endif // EVA_IMPL

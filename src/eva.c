#include "eva.h"

#define GLAD_GL_IMPLEMENTATION
#include "glad.h"

struct EvaBuffer {
    unsigned int id;
    unsigned int type;
    unsigned int usage;
    size_t       size;
};

struct EvaShader {
    unsigned int id;
};

struct EvaPipeline {
    unsigned int id;
    EvaBuffer *ibo;
    EvaShader *shader;
};

static int TranslateBufferType(int type) {
    switch (type) {
        case EVA_BUFFERTYPE_VERTEX: return GL_ARRAY_BUFFER;
        case EVA_BUFFERTYPE_INDEX:  return GL_ELEMENT_ARRAY_BUFFER;
    }
    return 0;
}

static int TranslateBufferUsage(int usage) {
    switch (usage) {
        case EVA_BUFFERUSAGE_STATIC:  return GL_STATIC_DRAW;
        case EVA_BUFFERUSAGE_DYNAMIC: return GL_DYNAMIC_DRAW;
    }
    return 0;
}

static size_t TranslateVertexFormat(int format, int *type, int *count) {
    switch (format) {
        case EVA_VERTEXFORMAT_INT:    *type = GL_INT;   *count = 1; return sizeof(int);
        case EVA_VERTEXFORMAT_INT2:   *type = GL_INT;   *count = 2; return sizeof(int);
        case EVA_VERTEXFORMAT_INT3:   *type = GL_INT;   *count = 3; return sizeof(int);
        case EVA_VERTEXFORMAT_INT4:   *type = GL_INT;   *count = 4; return sizeof(int);
        case EVA_VERTEXFORMAT_FLOAT:  *type = GL_FLOAT; *count = 1; return sizeof(float);
        case EVA_VERTEXFORMAT_FLOAT2: *type = GL_FLOAT; *count = 2; return sizeof(float);
        case EVA_VERTEXFORMAT_FLOAT3: *type = GL_FLOAT; *count = 3; return sizeof(float);
        case EVA_VERTEXFORMAT_FLOAT4: *type = GL_FLOAT; *count = 4; return sizeof(float);
    }
    return 0;
}

static void InitIfNeeded(void) {
    static int initted = 0;
    if (initted == 0) {
        gladLoaderLoadGL();
        initted = 1;
    }
}

EvaBuffer *EvaCreateBuffer(EvaBufferDesc *desc) {
    InitIfNeeded();

    EvaBuffer *buffer = calloc(1, sizeof *buffer);
    buffer->size = desc->size;
    buffer->type = TranslateBufferType(desc->type);
    buffer->usage = TranslateBufferUsage(desc->usage);

    glGenBuffers(1, &buffer->id);
    glBindBuffer(buffer->type, buffer->id);
    glBufferData(buffer->type, buffer->size, desc->data, buffer->usage);
    glBindBuffer(buffer->type, 0);

    return buffer;
}

void EvaDeleteBuffer(EvaBuffer *buffer) {
    glDeleteBuffers(1, &buffer->id);
    free(buffer);
}

EvaShader *EvaCreateShader(EvaShaderDesc *desc) {
    InitIfNeeded();

    EvaShader *shader = calloc(1, sizeof *shader);

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &desc->vs.src, 0);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &desc->fs.src, 0);
    glCompileShader(fs);

    shader->id = glCreateProgram();
    glAttachShader(shader->id, vs);
    glAttachShader(shader->id, fs);
    glLinkProgram(shader->id);
    glDeleteShader(vs);
    glDeleteShader(fs);

    return shader;
}

void EvaDeleteShader(EvaShader *shader) {
    glDeleteProgram(shader->id);
    free(shader);
}

EvaPipeline *EvaCreatePipeline(EvaPipelineDesc *desc) {
    EvaPipeline *pipeline = calloc(1, sizeof *pipeline);
    pipeline->ibo = desc->ibo;
    pipeline->shader = desc->shader;

    glGenVertexArrays(1, &pipeline->id);
    glBindVertexArray(pipeline->id);

    if (desc->ibo) {
        pipeline->ibo = desc->ibo;
        glBindBuffer(pipeline->ibo->type, pipeline->ibo->id);
    }

    int num_vbos = 0;
    while (num_vbos < EVA_PIPELINE_MAX_VERTEX_BUFFERS && desc->vbos[num_vbos] != NULL) {
        num_vbos++;
    }

    // Only calculate the stride if there's a single interleaved buffer
    size_t stride = 0;
    if (num_vbos == 1) {
        for (size_t i = 0; i < EVA_PIPELINE_MAX_LAYOUT_ATTRIBUTES && desc->layout[i].format != EVA_VERTEXFORMAT_INVALID; i++) {
            int type, count;
            stride += TranslateVertexFormat(desc->layout[i].format, &type, &count) * count;
        }
    }

    size_t offset = 0;
    for (size_t i = 0; i < EVA_PIPELINE_MAX_LAYOUT_ATTRIBUTES && desc->layout[i].format != EVA_VERTEXFORMAT_INVALID; i++) {
        EvaBuffer *vbo = desc->vbos[desc->layout[i].binding];
        glBindBuffer(vbo->type, vbo->id);

        int type, count;
        size_t size = TranslateVertexFormat(desc->layout[i].format, &type, &count);

        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, count, type, 0, stride, (void *)offset);

        // Only calculate the offset if there's a single interleaved buffer
        if (num_vbos == 1) {
            offset += size * count;
        }
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return pipeline;
}

void EvaDeletePipeline(EvaPipeline *pipeline) {
    glDeleteVertexArrays(1, &pipeline->id);
    free(pipeline);
}

void EvaClear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void EvaDraw(EvaPipeline *pipeline, int num_vertices) {
    glBindVertexArray(pipeline->id);
    glUseProgram(pipeline->shader->id);

    if (pipeline->ibo != NULL) {
        glDrawElements(GL_TRIANGLES, num_vertices, GL_UNSIGNED_INT, NULL);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }

    glUseProgram(0);
    glBindVertexArray(0);
}

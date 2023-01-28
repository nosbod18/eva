#pragma once
#include <stddef.h>

#define EVA_PIPELINE_MAX_LAYOUT_ATTRIBUTES 16
#define EVA_PIPELINE_MAX_VERTEX_BUFFERS    16

enum {
    EVA_VERTEXFORMAT_INVALID,
    EVA_VERTEXFORMAT_INT,   EVA_VERTEXFORMAT_INT2,   EVA_VERTEXFORMAT_INT3,   EVA_VERTEXFORMAT_INT4,
    EVA_VERTEXFORMAT_FLOAT, EVA_VERTEXFORMAT_FLOAT2, EVA_VERTEXFORMAT_FLOAT3, EVA_VERTEXFORMAT_FLOAT4,
};

enum {
    EVA_BUFFERTYPE_VERTEX,
    EVA_BUFFERTYPE_INDEX,
};

enum {
    EVA_BUFFERUSAGE_STATIC,
    EVA_BUFFERUSAGE_DYNAMIC
};

typedef struct EvaBuffer   EvaBuffer;
typedef struct EvaShader   EvaShader;
typedef struct EvaPipeline EvaPipeline;

typedef struct EvaBufferDesc {
    void const *data;
    size_t size;
    int type;
    int usage;
} EvaBufferDesc;

typedef struct EvaShaderDesc {
    struct {
        char const *src;
    } vs, fs;
} EvaShaderDesc;

typedef struct EvaPipelineDesc {
    struct {
        int format;
        int binding;
    } layout[EVA_PIPELINE_MAX_LAYOUT_ATTRIBUTES];
    EvaBuffer *vertexBuffers[EVA_PIPELINE_MAX_VERTEX_BUFFERS];
    EvaBuffer *indexBuffer;
    EvaShader *shader;
} EvaPipelineDesc;

EvaBuffer   *EvaCreateBuffer    (EvaBufferDesc *desc);
void         EvaDeleteBuffer    (EvaBuffer *buffer);

EvaShader   *EvaCreateShader    (EvaShaderDesc *desc);
void         EvaDeleteShader    (EvaShader *shader);

EvaPipeline *EvaCreatePipeline  (EvaPipelineDesc *desc);
void         EvaDeletePipeline  (EvaPipeline *pipeline);

void         EvaClear           (float r, float g, float b, float a);
void         EvaDraw            (EvaPipeline *pipeline, int num_vertices);

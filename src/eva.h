#pragma once
#include <stddef.h>

enum {
    EVA_BUFFERTYPE_VERTEX,
    EVA_BUFFERTYPE_INDEX,
};

enum {
    EVA_BUFFERUSAGE_STATIC,
    EVA_BUFFERUSAGE_DYNAMIC,
};

enum {
    EVA_VERTEXFORMAT_INT,   EVA_VERTEXFORMAT_INT2,   EVA_VERTEXFORMAT_INT3,   EVA_VERTEXFORMAT_INT4,
    EVA_VERTEXFORMAT_FLOAT, EVA_VERTEXFORMAT_FLOAT2, EVA_VERTEXFORMAT_FLOAT3, EVA_VERTEXFORMAT_FLOAT4,
};

typedef struct EvaBuffer EvaBuffer;
typedef struct EvaShader EvaShader;

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

EvaBuffer  *EvaCreateBuffer (EvaBufferDesc *desc);
void        EvaDeleteBuffer (EvaBuffer *buffer);

EvaShader  *EvaCreateShader (EvaShaderDesc *desc);
void        EvaDeleteShader (EvaShader *shader);

void        EvaClear        (float r, float g, float b, float a);
void        EvaDraw         (EvaBuffer *buffer, EvaShader *shader, int num_vertices);

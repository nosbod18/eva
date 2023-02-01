#pragma once
#include <stddef.h>

///
/// Defines
///

#define EVA_SHADER_MAX_UNIFORMS         32
#define EVA_BUFFER_MAX_ATTRIBUTES       16
#define EVA_BINDINGS_MAX_VERTEX_BUFFERS 16

///
/// Types
///

enum {
    EVA_BUFFERUSAGE_STATIC,
    EVA_BUFFERUSAGE_DYNAMIC
};

enum {
    EVA_SHADERSTAGE_VERTEX,
    EVA_SHADERSTAGE_FRAGMENT,
    EVA_SHADERSTAGE_MAX,
};

enum {
    EVA_VERTEXFORMAT_INVALID,
    EVA_VERTEXFORMAT_INT,   EVA_VERTEXFORMAT_INT2,   EVA_VERTEXFORMAT_INT3,   EVA_VERTEXFORMAT_INT4,
    EVA_VERTEXFORMAT_FLOAT, EVA_VERTEXFORMAT_FLOAT2, EVA_VERTEXFORMAT_FLOAT3, EVA_VERTEXFORMAT_FLOAT4,
};

enum {
    EVA_UNIFORMFORMAT_INVALID,
    EVA_UNIFORMFORMAT_INT,   EVA_UNIFORMFORMAT_INT2,   EVA_UNIFORMFORMAT_INT3,   EVA_UNIFORMFORMAT_INT4,
    EVA_UNIFORMFORMAT_FLOAT, EVA_UNIFORMFORMAT_FLOAT2, EVA_UNIFORMFORMAT_FLOAT3, EVA_UNIFORMFORMAT_FLOAT4,
    EVA_UNIFORMFORMAT_MAT3,  EVA_UNIFORMFORMAT_MAT4
};

typedef struct EvaBuffer EvaBuffer;
typedef struct EvaShader EvaShader;

typedef struct EvaBufferDesc {
    void const *data;
    size_t size;
    int usage;
    int layout[EVA_BUFFER_MAX_ATTRIBUTES];
} EvaBufferDesc;

typedef struct EvaShaderDesc {
    char const *vs_src;
    char const *fs_src;
    struct {
        char const *name;
        int format;
    } uniforms[EVA_SHADER_MAX_UNIFORMS];
} EvaShaderDesc;

typedef struct EvaBindings {
    EvaBuffer *vbos[EVA_BINDINGS_MAX_VERTEX_BUFFERS];
    EvaBuffer *ibo;
} EvaBindings;

///
/// Prototypes
///

EvaBuffer   *EvaCreateBuffer    (EvaBufferDesc *desc);
void         EvaDeleteBuffer    (EvaBuffer *buffer);

EvaShader   *EvaCreateShader    (EvaShaderDesc *desc);
void         EvaDeleteShader    (EvaShader *shader);

void         EvaClear           (float r, float g, float b, float a);
void         EvaSendUniforms    (EvaShader *shader, void const *data);
void         EvaDraw            (EvaBindings *bindings, EvaShader *shader, int count);

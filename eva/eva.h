#pragma once
#include <stddef.h>

///
/// Constants
///

#define EVA_SHADER_MAX_UNIFORMS         32
#define EVA_BUFFER_MAX_ATTRIBUTES       16
#define EVA_BINDINGS_MAX_VERTEX_BUFFERS 16

enum {
    EVA_INT = 1, EVA_INT2,   EVA_INT3,   EVA_INT4,
    EVA_FLOAT,   EVA_FLOAT2, EVA_FLOAT3, EVA_FLOAT4,
    EVA_MAT3,    EVA_MAT4
};

///
/// Types
///

typedef struct EvaBuffer EvaBuffer;
typedef struct EvaShader EvaShader;

typedef struct EvaBufferDesc {
    void const *data;
    size_t size;
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
void         EvaApplyUniforms   (EvaShader *shader, void const *data);
void         EvaDraw            (EvaBindings *bindings, EvaShader *shader, int count);

#pragma once
#include <stddef.h>

///
/// Constants
///

#define EVA_BUFFER_MAX_ATTRIBUTES   16
#define EVA_SHADER_MAX_UNIFORMS     32
#define EVA_BINDINGS_MAX_VBOS       16
#define EVA_BINDINGS_MAX_IMAGES     16

enum {
    EVA_VERTEXFORMAT_INVALID,
    EVA_VERTEXFORMAT_INT,   EVA_VERTEXFORMAT_INT2,   EVA_VERTEXFORMAT_INT3,   EVA_VERTEXFORMAT_INT4,
    EVA_VERTEXFORMAT_FLOAT, EVA_VERTEXFORMAT_FLOAT2, EVA_VERTEXFORMAT_FLOAT3, EVA_VERTEXFORMAT_FLOAT4,
};

enum {
    EVA_UNIFORMFORMAT_INVALID,
    EVA_UNIFORMFORMAT_INT,    EVA_UNIFORMFORMAT_INT2,   EVA_UNIFORMFORMAT_INT3,   EVA_UNIFORMFORMAT_INT4,
    EVA_UNIFORMFORMAT_FLOAT,  EVA_UNIFORMFORMAT_FLOAT2, EVA_UNIFORMFORMAT_FLOAT3, EVA_UNIFORMFORMAT_FLOAT4,
    EVA_UNIFORMFORMAT_MAT3,   EVA_UNIFORMFORMAT_MAT4,
    EVA_UNIFORMFORMAT_IMAGE2D
};

enum {
    EVA_IMAGEFORMAT_RGBA8,
    EVA_IMAGEFORMAT_RGB8,
};

enum {
    EVA_IMAGEWRAP_REPEAT,
    EVA_IMAGEWRAP_MIRRORED_REPEAT,
    EVA_IMAGEWRAP_CLAMP_TO_EDGE,
    EVA_IMAGEWRAP_CLAMP_TO_BORDER,
};

enum {
    EVA_IMAGEFILTER_NEAREST,
    EVA_IMAGEFILTER_LINEAR,
    EVA_IMAGEFILTER_NEAREST_MIPMAP_NEAREST,
    EVA_IMAGEFILTER_LINEAR_MIPMAP_NEAREST,
    EVA_IMAGEFILTER_NEAREST_MIPMAP_LINEAR,
    EVA_IMAGEFILTER_LINEAR_MIPMAP_LINEAR,
};

///
/// Types
///

typedef struct EvaBuffer EvaBuffer;
typedef struct EvaBufferDesc {
    void const *data;
    size_t size;
    int layout[EVA_BUFFER_MAX_ATTRIBUTES];
} EvaBufferDesc;

typedef struct EvaShader EvaShader;
typedef struct EvaShaderDesc {
    struct {
        char const *src;
    } sources[2];
    struct {
        char const *name;
        int format;
    } uniforms[EVA_SHADER_MAX_UNIFORMS];
} EvaShaderDesc;

typedef struct EvaImage EvaImage;
typedef struct EvaImageDesc {
    void const *data;
    int width;
    int height;
    int format;
    struct {
        int s, t;
    } wrap;
    struct {
        int min, mag;
    } filter;
} EvaImageDesc;

typedef struct EvaBindings {
    EvaBuffer *vbos[EVA_BINDINGS_MAX_VBOS];
    EvaBuffer *ibo;
    EvaShader *shader;
    EvaImage  *images[EVA_BINDINGS_MAX_IMAGES];
} EvaBindings;

typedef struct EvaPassDesc {
    struct { float r, g, b, a; } clear;
    struct { int x, y, w, h;   } viewport;
} EvaPassDesc;

///
/// Prototypes
///

EvaBuffer   *EvaCreateBuffer    (EvaBufferDesc *desc);
EvaShader   *EvaCreateShader    (EvaShaderDesc *desc);
EvaImage    *EvaCreateImage     (EvaImageDesc *desc);

void         EvaDeleteBuffer    (EvaBuffer *buffer);
void         EvaDeleteShader    (EvaShader *shader);
void         EvaDeleteImage     (EvaImage *image);

void         EvaBeginPass       (EvaPassDesc *pass);
void         EvaApplyBindings   (EvaBindings *bindings);
void         EvaApplyUniforms   (void *data);
void         EvaDraw            (int count);
void         EvaEndPass         (void);

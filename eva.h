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
    EVA_SHADER_STAGE_VERTEX,
    EVA_SHADER_STAGE_FRAGMENT,
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

typedef struct eva_buffer_t    eva_buffer_t;
typedef struct eva_shader_t    eva_shader_t;
typedef struct eva_image_t     eva_image_t;

typedef struct eva_buffer_desc_t {
    void const *data;
    size_t size;
    int layout[EVA_BUFFER_MAX_ATTRIBUTES];
} eva_buffer_desc_t;

typedef struct eva_shader_desc_t {
    struct {
        char const *src;
    } sources[2];
    struct {
        char const *name;
        int format;
    } uniforms[EVA_SHADER_MAX_UNIFORMS];
} eva_shader_desc_t;

typedef struct eva_image_desc_t {
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
} eva_image_desc_t;

typedef struct eva_bindings_desc_t {
    eva_buffer_t *vbos[EVA_BINDINGS_MAX_VBOS];
    eva_buffer_t *ibo;
    eva_image_t  *images[EVA_BINDINGS_MAX_IMAGES];
} eva_bindings_desc_t;

typedef struct eva_pipeline_desc_t {
    eva_shader_t *shader;
} eva_pipeline_desc_t;

typedef struct eva_pass_desc_t {
    struct { float r, g, b, a; } clear;
    struct { int   x, y, w, h; } viewport;
} eva_pass_desc_t;

///
/// Prototypes
///

eva_buffer_t   *eva_buffer_create   (eva_buffer_desc_t *desc);
eva_shader_t   *eva_shader_create   (eva_shader_desc_t *desc);
eva_image_t    *eva_image_create    (eva_image_desc_t *desc);

void            eva_buffer_delete   (eva_buffer_t *buffer);
void            eva_shader_delete   (eva_shader_t *shader);
void            eva_image_delete    (eva_image_t *image);

void            eva_pass_begin      (eva_pass_desc_t *pass);
void            eva_bindings_apply  (eva_bindings_desc_t *bindings);
void            eva_pipeline_apply  (eva_pipeline_desc_t *pipeline);
void            eva_uniforms_apply  (void *data);
void            eva_draw            (int first, int count);
void            eva_pass_end        (void);

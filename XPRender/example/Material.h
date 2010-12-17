#ifndef __EXAMPLE_MATERIAL_H__
#define __EXAMPLE_MATERIAL_H__

#include "../lib/xprender/Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprShader;
struct XprPipeline;

typedef enum MaterialFlag
{
	MaterialFlag_Ready = 1 << 0,
} MaterialFlag;

typedef struct Material
{
	struct XprShader** shaders;
	struct XprPipeline* pipeline;
	size_t flags;
} Material;

Material* Material_new(const char** args);

void Material_free(Material* self);

#ifdef __cplusplus
}
#endif

#endif // __EXAMPLE_MATERIAL_H__

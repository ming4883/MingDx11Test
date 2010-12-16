#ifndef __EXAMPLE_MATERIAL_H__
#define __EXAMPLE_MATERIAL_H__

#include "../lib/xprender/Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprShader;
struct XprPipeline;

typedef struct Material
{
	struct XprShader** shaders;
	struct XprPipeline* pipeline;
} Material;

Material* Material_new();

void Material_free(Material* self);

XprBool Material_load(Material* self, const char** args);

#ifdef __cplusplus
}
#endif

#endif // __EXAMPLE_MATERIAL_H__

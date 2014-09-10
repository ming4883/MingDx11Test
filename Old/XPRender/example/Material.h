#ifndef __EXAMPLE_MATERIAL_H__
#define __EXAMPLE_MATERIAL_H__

#include "../lib/xprender/Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprGpuShader;
struct XprGpuProgram;

typedef enum MaterialFlag
{
	MaterialFlag_Inited = 1 << 0,
} MaterialFlag;

typedef struct Material
{
	struct XprGpuShader* shaders[8];
	struct XprGpuProgram* program;
	size_t flags;
} Material;

Material* materialAlloc();

void materialFree(Material* self);

XprBool materialInitWithShaders(Material* self, const char** args);

#ifdef __cplusplus
}
#endif

#endif // __EXAMPLE_MATERIAL_H__

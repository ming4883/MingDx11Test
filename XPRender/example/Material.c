#include "Material.h"

#include "../lib/xprender/StrUtil.h"
#include "../lib/xprender/Shader.h"

#include <stdlib.h>
#include <string.h>

enum
{
	VS,
	TC,
	TE,
	GS,
	FS,
	ShaderCount
};

Material* Material_alloc()
{
	Material* self = malloc(sizeof(Material));
	memset(self, 0, sizeof(Material));

	self->shaders = malloc(sizeof(XprGpuShader)*ShaderCount);
	memset(self->shaders, 0, sizeof(XprGpuShader)*ShaderCount);

	return self;
}

void Material_initWithShaders(Material* self, const char** args)
{
	const char* key; const char* val;
	int i = 0;

	while(1) {
		key = args[i++];
		if(nullptr == key) break;

		val = args[i++];
		if(nullptr == val) break;

		if(0 == strcasecmp(key, "vs")) {
			self->shaders[VS] = xprGpuShaderAlloc();
			xprGpuShaderInit(self->shaders[VS], &val, 1, XprGpuShaderType_Vertex);
		}

		if(0 == strcasecmp(key, "tc")) {
			self->shaders[TC] = xprGpuShaderAlloc();
			xprGpuShaderInit(self->shaders[TC], &val, 1, XprGpuShaderType_TessControl);
		}

		if(0 == strcasecmp(key, "te")) {
			self->shaders[TE] = xprGpuShaderAlloc();
			xprGpuShaderInit(self->shaders[TE], &val, 1, XprGpuShaderType_TessEvaluation);
		}

		if(0 == strcasecmp(key, "gs")) {
			self->shaders[GS] = xprGpuShaderAlloc();
			xprGpuShaderInit(self->shaders[GS], &val, 1, XprGpuShaderType_Geometry);
		}

		if(0 == strcasecmp(key, "fs")) {
			self->shaders[FS] = xprGpuShaderAlloc();
			xprGpuShaderInit(self->shaders[FS], &val, 1, XprGpuShaderType_Fragment);
		}
	}

	// at least we need 1 vertex shader and 1 fragment shader
	if(nullptr == self->shaders[VS] || nullptr == self->shaders[FS])
		return;

	self->program = xprGpuProgramAlloc();
	xprGpuProgramInit(self->program, self->shaders, ShaderCount);

	if(!(self->program->flags & XprGpuProgramFlag_Linked))
		return;

	self->flags |= MaterialFlag_Inited;
}


void Material_free(Material* self)
{
	int i;

	if(nullptr == self)
		return;

	if(nullptr != self->program)
		xprGpuProgramFree(self->program);

	for(i=0; i<ShaderCount; ++i)
	{
		if(nullptr != self->shaders[i])
			xprGpuShaderFree(self->shaders[i]);
	}

	free(self->shaders);
	free(self);
}

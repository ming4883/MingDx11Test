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
			self->shaders[VS] = XprGpuShader_alloc();
			XprGpuShader_init(self->shaders[VS], &val, 1, XprGpuShaderType_Vertex);
		}

		if(0 == strcasecmp(key, "tc")) {
			self->shaders[TC] = XprGpuShader_alloc();
			XprGpuShader_init(self->shaders[TC], &val, 1, XprGpuShaderType_TessControl);
		}

		if(0 == strcasecmp(key, "te")) {
			self->shaders[TE] = XprGpuShader_alloc();
			XprGpuShader_init(self->shaders[TE], &val, 1, XprGpuShaderType_TessEvaluation);
		}

		if(0 == strcasecmp(key, "gs")) {
			self->shaders[GS] = XprGpuShader_alloc();
			XprGpuShader_init(self->shaders[GS], &val, 1, XprGpuShaderType_Geometry);
		}

		if(0 == strcasecmp(key, "fs")) {
			self->shaders[FS] = XprGpuShader_alloc();
			XprGpuShader_init(self->shaders[FS], &val, 1, XprGpuShaderType_Fragment);
		}
	}

	// at least we need 1 vertex shader and 1 fragment shader
	if(nullptr == self->shaders[VS] || nullptr == self->shaders[FS])
		return;

	self->program = XprGpuProgram_alloc();
	XprGpuProgram_init(self->program, self->shaders, ShaderCount);

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
		XprGpuProgram_free(self->program);

	for(i=0; i<ShaderCount; ++i)
	{
		if(nullptr != self->shaders[i])
			XprGpuShader_free(self->shaders[i]);
	}

	free(self->shaders);
	free(self);
}

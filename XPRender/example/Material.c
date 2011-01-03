#include "Material.h"

#include "../lib/xprender/StrUtil.h"
#include "../lib/xprender/Shader.h"

#include <stdlib.h>
#include <string.h>

enum
{
	VS,
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

		if(0 == strcasecmp(key, "fs")) {
			self->shaders[FS] = XprGpuShader_alloc();
			XprGpuShader_init(self->shaders[FS], &val, 1, XprGpuShaderType_Fragment);
		}
	}

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

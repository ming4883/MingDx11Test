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

Material* Material_new()
{
	Material* self = malloc(sizeof(Material));
	memset(self, 0, sizeof(Material));

	self->shaders = malloc(sizeof(XprShader)*ShaderCount);
	memset(self->shaders, 0, sizeof(XprShader)*ShaderCount);

	return self;
}

void Material_free(Material* self)
{
	int i;

	if(nullptr != self->pipeline)
		XprPipeline_free(self->pipeline);

	for(i=0; i<ShaderCount; ++i)
	{
		if(nullptr != self->shaders[i])
			XprShader_free(self->shaders[i]);
	}

	free(self->shaders);
	free(self);
}

XprBool Material_load(Material* self, const char** args)
{
	const char* key; const char* val;

	int i = 0;
	while(1)
	{
		key = args[i++];
		if(nullptr == key) break;

		val = args[i++];
		if(nullptr == val) break;

		if(0 == strcasecmp(key, "vs"))
			self->shaders[VS] = XprShader_new(&val, 1, XprShaderType_Vertex);

		if(0 == strcasecmp(key, "fs"))
			self->shaders[FS] = XprShader_new(&val, 1, XprShaderType_Fragment);
	}

	if(nullptr == self->shaders[VS] || nullptr == self->shaders[FS])
		return XprFalse;

	self->pipeline = XprPipeline_new(self->shaders, ShaderCount);

	return XprTrue;
}
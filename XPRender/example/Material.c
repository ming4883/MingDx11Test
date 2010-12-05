#include "Material.h"

#include "../lib/xprender/Shader.h"
#include <stdlib.h>
#include <string.h>

Material* Material_new()
{
	Material* self = malloc(sizeof(Material));
	memset(self, 0, sizeof(Material));
	return self;
}

void Material_free(Material* self)
{
	if(nullptr != self->pipeline)
		XprPipeline_free(self->pipeline);

	if(nullptr != self->ps)
		XprShader_free(self->ps);

	if(nullptr != self->vs)
		XprShader_free(self->vs);
}

void Material_loadFromFile(Material* self, const char* shader)
{
}
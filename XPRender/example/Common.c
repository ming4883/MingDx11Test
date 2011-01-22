#include "Common.h"

void RenderContext_apply(RenderContext* self, Material* material)
{
	XprGpuProgram_uniformMtx4fv(material->program, XPR_HASH("u_worldViewMtx"), 1, XprTrue, self->worldViewMtx.v);
	XprGpuProgram_uniformMtx4fv(material->program, XPR_HASH("u_worldViewProjMtx"), 1, XprTrue, self->worldViewProjMtx.v);
	XprGpuProgram_uniform4fv(material->program, XPR_HASH("u_matDiffuse"), 1, self->matDiffuse.v);
	XprGpuProgram_uniform4fv(material->program, XPR_HASH("u_matSpecular"), 1, self->matSpecular.v);
	XprGpuProgram_uniform1fv(material->program, XPR_HASH("u_matShininess"), 1, &self->matShininess);
}

Material* loadMaterial(const char* vsKey, const char* fsKey, const char* tcKey, const char* teKey, const char* gsKey)
{
	const char* args[11] = {nullptr};
	size_t idx = 0;
	Material* material = nullptr;

	if(nullptr != vsKey) {
		args[idx++] = "vs";
		args[idx++] = glswGetShader(vsKey);
	}

	if(nullptr != tcKey) {
		args[idx++] = "tc";
		args[idx++] = glswGetShader(tcKey);
	}

	if(nullptr != teKey) {
		args[idx++] = "te";
		args[idx++] = glswGetShader(teKey);
	}

	if(nullptr != gsKey) {
		args[idx++] = "gs";
		args[idx++] = glswGetShader(gsKey);
	}
	
	if(nullptr != fsKey) {
		args[idx++] = "fs";
		args[idx++] = glswGetShader(fsKey);
	}
	
	material = Material_alloc();
	Material_initWithShaders(material, args);

	if(0 == (material->flags & MaterialFlag_Inited))
		PezDebugString("failed to load material vs=%s,fs=%s!\n", vsKey, fsKey);

	return material;
}

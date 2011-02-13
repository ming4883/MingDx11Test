#include "Common.h"
#include "Mesh.h"
#include "Material.h"
#include "Label.h"

XprGpuState* _gpuState = nullptr;
Material* _textMaterial = nullptr;
Mesh* _bgMesh = nullptr;
Label* _label = nullptr;
XprVec4 _textColor = {1, 0, 0, 1};

void PezUpdate(unsigned int elapsedMilliseconds)
{
}

void PezHandleMouse(int x, int y, int action)
{
}

void PezRender()
{
	// render to texture
	xprRenderTargetClearDepth(1);
	xprRenderTargetClearColor(0.25f, 0.75f, 1.0f, 1.0f);

	// display the label
	xprGpuStateSetBlendEnabled(_gpuState, XprTrue);
	xprGpuStateSetBlendFactorRGB(_gpuState, XprGpuState_BlendFactor_SrcAlpha, XprGpuState_BlendFactor_OneMinusSrcAlpha);
	xprGpuStateSetBlendFactorA  (_gpuState, XprGpuState_BlendFactor_SrcAlpha, XprGpuState_BlendFactor_OneMinusSrcAlpha);
	xprGpuStatePreRender(_gpuState);
	
	xprGpuProgramPreRender(_textMaterial->program);
	xprGpuProgramUniformTexture(_textMaterial->program, XprHash("u_tex"), _label->texture);
	xprGpuProgramUniform4fv(_textMaterial->program, XprHash("u_textColor"), 1, _textColor.v);
	
	Mesh_preRender(_bgMesh, _textMaterial->program);
	Mesh_render(_bgMesh);

	xprGpuStateSetBlendEnabled(_gpuState, XprFalse);
}

void PezConfig()
{
	PEZ_VIEWPORT_WIDTH = 800;
	PEZ_VIEWPORT_HEIGHT = 600;
	PEZ_ENABLE_MULTISAMPLING = 0;
	PEZ_VERTICAL_SYNC = 0;
}

void PezExit(void)
{
	xprGpuStateFree(_gpuState);
	Material_free(_textMaterial);
	Label_free(_label);
}

const char* PezInitialize(int width, int height)
{
	char utf8[] = {0xEF, 0xBB, 0xBF, 0xE9, 0x80, 0x99, 0xE6, 0x98, 0xAF, 0x55, 0x54, 0x46, 0x38, 0x00};

	xprRenderTargetSetViewport(0, 0, (float)width, (float)height, -1, 1);

	_gpuState = xprGpuStateAlloc();
	xprGpuStateInit(_gpuState);
	
	// label
	_label = Label_alloc();
	Label_init(_label, width, height);
	Label_setText(_label, utf8);
	Label_commit(_label);

	// materials
	glswInit(&myFileSystem);
	glswSetPath("", ".glsl");
	glswAddDirectiveToken("","#version 150");

	_textMaterial = loadMaterial(
		"Common.Ui.Vertex",
		"Common.Text.Fragment",
		nullptr, nullptr, nullptr);

	glswShutdown();

	_bgMesh = Mesh_alloc();
	Mesh_initWithScreenQuad(_bgMesh);
	
	atexit(PezExit);

	return "Label";
}

#include "Common.h"
#include "Mesh.h"
#include "Material.h"
#include "Label.h"

AppContext* app = nullptr;
Material* mtlText = nullptr;
Mesh* meshBg = nullptr;
Label* label = nullptr;
XprVec4 textColor = {1, 0, 0, 1};

void xprAppUpdate(unsigned int elapsedMilliseconds)
{
}

void xprAppHandleMouse(int x, int y, int action)
{
}

void xprAppRender()
{
	XprGpuStateDesc* gpuState = &app->gpuState->desc;

	// render to texture
	xprRenderTargetClearDepth(1);
	xprRenderTargetClearColor(0.25f, 0.75f, 1.0f, 1.0f);

	// display the label
	gpuState->blend = XprTrue;
	gpuState->blendFactorSrcRGB = XprGpuState_BlendFactor_SrcAlpha;
	gpuState->blendFactorDestRGB = XprGpuState_BlendFactor_OneMinusSrcAlpha;
	gpuState->blendFactorSrcA = XprGpuState_BlendFactor_SrcAlpha;
	gpuState->blendFactorDestA = XprGpuState_BlendFactor_OneMinusSrcAlpha;
	xprGpuStatePreRender(app->gpuState);
	
	xprGpuProgramPreRender(mtlText->program);
	xprGpuProgramUniformTexture(mtlText->program, XprHash("u_tex"), label->texture);
	xprGpuProgramUniform4fv(mtlText->program, XprHash("u_textColor"), 1, textColor.v);
	
	meshPreRender(meshBg, mtlText->program);
	meshRenderTriangles(meshBg);

	gpuState->blend = XprFalse;
}

void xprAppConfig()
{
	xprAppContext.appName = "Label";
	xprAppContext.xres = 800;
	xprAppContext.yres = 600;
	xprAppContext.multiSampling = XprFalse;
	xprAppContext.vsync = XprFalse;
	xprAppContext.apiMajorVer = 3;
	xprAppContext.apiMinorVer = 3;
}

void xprAppFinalize()
{
	materialFree(mtlText);
	Label_free(label);
	appFree(app);
}

XprBool xprAppInitialize()
{
	char utf8[] = {0xEF, 0xBB, 0xBF, 0xE9, 0x80, 0x99, 0xE6, 0x98, 0xAF, 0x55, 0x54, 0x46, 0x38, 0x00};

	app = appAlloc();
	appInit(app);
	
	// label
	{
		label = Label_alloc();
		Label_init(label, xprAppContext.xres, xprAppContext.yres);
		Label_setText(label, utf8);
		Label_commit(label);
	}

	// materials
	{
		const char* directives[] = {nullptr};
		appLoadMaterialBegin(app, directives);

		mtlText = appLoadMaterial(
			"Common.Ui.Vertex",
			"Common.Text.Fragment",
			nullptr, nullptr, nullptr);

		appLoadMaterialEnd(app);
	}

	// mesh
	{
		meshBg = meshAlloc();
		meshInitWithScreenQuad(meshBg);
	}

	return XprTrue;
}

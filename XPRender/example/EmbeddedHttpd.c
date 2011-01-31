#include "Common.h"
#include "../lib/httpd/httpd.h"
#include "../lib/xprender/uthash/uthash.h"
#include "../lib/xprender/StrHash.h"
#include <winsock2.h>

typedef struct RemoteVar
{
	XprHashCode id;
	char* name;
	float* value;
	float lowerBound, upperBound;
	UT_hash_handle hh;
} RemoteVar;

RemoteVar* RemoteVar_alloc()
{
	RemoteVar* self;
	self = malloc(sizeof(RemoteVar));
	memset(self, 0, sizeof(RemoteVar));
	return self;
}

void RemoteVar_free(RemoteVar* self)
{
	free(self->name);
	free(self);
}

void RemoteVar_init(RemoteVar* self, const char* name, float* value, float lower, float upper)
{
	self->id = XPR_HASH(name);
	self->name = strdup(name);
	self->value = value;
	self->upperBound = upper;
	self->lowerBound = lower;
}

typedef struct RemoteObj
{
	struct RemoteVar* vars;
	char* report;
} RemoteObj;

RemoteObj* RemoteObj_alloc()
{
	RemoteObj* self;
	self = malloc(sizeof(RemoteObj));
	memset(self, 0, sizeof(RemoteObj));
	return self;
}

void RemoteObj_free(RemoteObj* self)
{
	struct RemoteVar *curr, *tmp;

	HASH_ITER(hh, self->vars, curr, tmp) {
		HASH_DEL(self->vars, curr);
		RemoteVar_free(curr);
	}

	realloc(self->report, 0);

	free(self);
}

void RemoteObj_addVar(RemoteObj* self, RemoteVar* var)
{
	if(nullptr == self)
		return;

	HASH_ADD_INT(self->vars, id, var);
}

RemoteVar* RemoteObj_findVar(RemoteObj* self, const char* name)
{
	RemoteVar* var = nullptr;
	XprHashCode id;
	if(nullptr == self)
		return nullptr;

	id = XPR_HASH(name);
	HASH_FIND_INT(self->vars, &id, var);

	return var;
}

void RemoteObj_getJson(RemoteObj* self)
{
	struct RemoteVar *curr, *tmp;
	char buf[64];

	if(nullptr == self)
		return;

	if(nullptr == self->report) {
		self->report = realloc(self->report, 1024);
	}
	sprintf(self->report, "\"object\":{");

	HASH_ITER(hh, self->vars, curr, tmp) {		
		sprintf(buf,
			"\"%s\":{\"v\":%f, \"u\":%f, \"l\":%f},",
			curr->name, *curr->value, curr->upperBound, curr->lowerBound
			);
		strcat(self->report, buf);
	}

	strcat(self->report, "}");
}


httpd* _httpd = nullptr;
RemoteObj* _remoteObj = nullptr;
float bgR = 255;
float bgG = 127;
float bgB = 0;

void PezUpdate(unsigned int elapsedMilliseconds)
{
	struct timeval to;
	to.tv_sec = 0;
	to.tv_usec = 100;
	
	if (httpdGetConnection(_httpd, &to) <= 0)
		return;

	if(httpdReadRequest(_httpd) < 0) {
		httpdEndRequest(_httpd);
		return;
	}

	httpdProcessRequest(_httpd);

	httpdEndRequest(_httpd);
}

void PezHandleMouse(int x, int y, int action)
{
}

void PezRender()
{
	glClearDepth(1);
	glClearColor(bgR / 255, bgG / 255, bgB / 255, 1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	{ // check for any OpenGL errors
	GLenum glerr = glGetError();

	if(glerr != GL_NO_ERROR)
		PezDebugString("GL has error %4x!", glerr);
	}
}

void PezConfig()
{
	PEZ_VIEWPORT_WIDTH = 640;
	PEZ_VIEWPORT_HEIGHT = 480;
	PEZ_ENABLE_MULTISAMPLING = 0;
	PEZ_VERTICAL_SYNC = 0;
}

void PezExit(void)
{
	RemoteObj_free(_remoteObj);
	httpdDestroy(_httpd);
}

void htmlIndex(httpd* server)
{
	struct RemoteVar *curr, *tmp;
	//RemoteObj_getJson(_remoteObj);
	//httpdPrintf(server, _remoteObj->report);

	httpdOutput(server, "\
<!DOCTYPE html>\
<html>\
<head>\
  <link href='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/themes/base/jquery-ui.css' rel='stylesheet' type='text/css'/>\
  <script src='http://ajax.googleapis.com/ajax/libs/jquery/1.4/jquery.min.js'></script>\
  <script src='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/jquery-ui.min.js'></script>\
    <style type='text/css'>\
    #slider { margin: 10px; }\
  </style>\n");

	HASH_ITER(hh, _remoteObj->vars, curr, tmp) {		
		httpdPrintf(server, "\
<script>\
  $(document).ready(function() {\
	var name = '%s';\
	var elem = $('#'+name);\
	elem.slider({\
      min:%f, max:%f, value:%f, stop:function(event, ui) {\
	    var data = {}; data[name] = elem.slider('value');\
	    jQuery.get('setter', data);\
	  }\
	});\
  });\
</script>\n",
curr->name, curr->lowerBound, curr->upperBound, *curr->value);
	}
	
	httpdOutput(server, "\
</head>\
<body style='font-size:62.5%;'><table>\n");

	HASH_ITER(hh, _remoteObj->vars, curr, tmp) {		
		httpdPrintf(server, "\
<tr><td>%s&nbsp</td><td width='200px'><div id='%s'></div></td></tr>\n",
		curr->name, curr->name);
	}

	httpdOutput(server, "\
</table>\
</body>\
</html>\
");
}

void htmlSetter(httpd* server)
{
	RemoteVar* rvar;
	httpVar* hvar;

	if(nullptr == (hvar = httpdGetVariableByPrefix(server, ""))) {
		httpdPrintf(server, "false");
		return;
	}

	if(nullptr == (rvar = RemoteObj_findVar(_remoteObj, hvar->name))) {
		httpdPrintf(server, "false");
		return;
	}

	if(0 == sscanf(hvar->value, "%f", rvar->value)) {
		httpdPrintf(server, "false");
		return;
	}
	httpdPrintf(server, "true");
}

const char* PezInitialize(int width, int height)
{
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);

	_httpd = httpdCreate(nullptr, HTTP_PORT);
	httpdAddCContent(_httpd, "/", "index.html", HTTP_TRUE, nullptr, htmlIndex);
	httpdAddCContent(_httpd, "/", "setter", HTTP_FALSE, nullptr, htmlSetter);

	_remoteObj = RemoteObj_alloc();
	{	RemoteVar* var = RemoteVar_alloc();
		RemoteVar_init(var, "bgR", &bgR, 0, 255);
		RemoteObj_addVar(_remoteObj, var);
	}

	{	RemoteVar* var = RemoteVar_alloc();
		RemoteVar_init(var, "bgG", &bgG, 0, 255);
		RemoteObj_addVar(_remoteObj, var);
	}

	{	RemoteVar* var = RemoteVar_alloc();
		RemoteVar_init(var, "bgB", &bgB, 0, 255);
		RemoteObj_addVar(_remoteObj, var);
	}
	
	atexit(PezExit);

	return "Embedded Httpd";
}

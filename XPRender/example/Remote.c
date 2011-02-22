#include "Remote.h"

#include "../lib/xprender/StrHash.h"
#include "../lib/xprender/uthash/uthash.h"
#include "../lib/httpd/httpd.h"

#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

typedef struct RemoteVar
{
	RemoteVarDesc desc;
	XprHashCode id;
	UT_hash_handle hh;
} RemoteVar;

RemoteVar* remoteVarAlloc()
{
	RemoteVar* self;
	self = malloc(sizeof(RemoteVar));
	memset(self, 0, sizeof(RemoteVar));
	return self;
}

void remoteVarFree(RemoteVar* self)
{
	free(self->desc.name);
	free(self);
}

void remoteVarInit(RemoteVar* self, RemoteVarDesc desc)
{
	self->id = XprHash(desc.name);
	self->desc.name = strdup(desc.name);
	self->desc.value = desc.value;
	self->desc.upperBound = desc.upperBound;
	self->desc.lowerBound = desc.lowerBound;
}

typedef struct RemoteConfigImpl
{
	RemoteVar* vars;
	httpd* http;

	// thread
	DWORD threadId;
	HANDLE threadHandle;
	XprBool requestExit;
	CRITICAL_SECTION criticalSection;

} RemoteConfigImpl;

RemoteConfig* remoteConfigAlloc()
{
	RemoteConfig* self;
	XprAllocWithImpl(self, RemoteConfig, RemoteConfigImpl);
	return self;
}

void remoteConfigFree(RemoteConfig* self)
{
	struct RemoteVar *curr, *tmp;

	if(nullptr == self)
		return;

	if(nullptr != self->impl->threadHandle) {
		// notify the thread to exit
		self->impl->requestExit = XprTrue;

		// cleanup thread related resources
		WaitForMultipleObjects(1, &self->impl->threadHandle, TRUE, INFINITE);
		CloseHandle(self->impl->threadHandle);
		DeleteCriticalSection(&self->impl->criticalSection);
	}

	HASH_ITER(hh, self->impl->vars, curr, tmp) {
		HASH_DEL(self->impl->vars, curr);
		remoteVarFree(curr);
	}

	free(self);
}

void remoteConfigLock(RemoteConfig* self)
{
	if(nullptr == self)
		return;

	if(nullptr == self->impl->threadHandle)
		return;

	EnterCriticalSection(&self->impl->criticalSection);
}

void remoteConfigUnlock(RemoteConfig* self)
{
	if(nullptr == self)
		return;

	if(nullptr == self->impl->threadHandle)
		return;

	LeaveCriticalSection(&self->impl->criticalSection);
}

DWORD WINAPI RemoteConfig_thread(void* param)
{
	RemoteConfig* self = (RemoteConfig*)param;

	while(!self->impl->requestExit) {
		struct timeval to = {0, 1000};

		if (httpdGetConnection(self->impl->http, &to) <= 0)
			continue;

		if(httpdReadRequest(self->impl->http) < 0) {
			httpdEndRequest(self->impl->http);
			continue;
		}

		remoteConfigLock(self);

		httpdProcessRequest(self->impl->http);

		remoteConfigUnlock(self);

		httpdEndRequest(self->impl->http);
	}

	return 0;
}

RemoteVar* remoteConfigFindVar(RemoteConfig* self, const char* name)
{
	RemoteVar* var = nullptr;
	XprHashCode id;
	if(nullptr == self)
		return nullptr;

	id = XprHash(name);
	HASH_FIND_INT(self->impl->vars, &id, var);

	return var;
}

void remoteConfigAddVar(RemoteConfig* self, RemoteVarDesc desc)
{
	RemoteVar* var;
	if(nullptr == self)
		return;

	var = remoteVarAlloc();
	remoteVarInit(var, desc);
	HASH_ADD_INT(self->impl->vars, id, var);
}

void remoteConfigIndexhtml(httpd* server)
{
	struct RemoteVar *curr, *tmp;
	RemoteConfig* config = (RemoteConfig*)server->userData;

	// html head
	{
		char* html = "\
<!DOCTYPE html>\n\
<html>\n\
<head>\n\
  <link href='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/themes/base/jquery-ui.css' rel='stylesheet' type='text/css'/>\n\
  <script src='http://ajax.googleapis.com/ajax/libs/jquery/1.4/jquery.min.js'></script>\n\
  <script src='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/jquery-ui.min.js'></script>\n\
  <style type='text/css'>\n\
  #slider { margin: 10px; }\n\
  body {\n\
    font-size:75%;\n\
    font:.75em/1.5 sans-serif;\n\
    background-color:LightBlue;\n\
  }\n\
  table.var {\n\
    background-color: white;\n\
  }\n\
  table.var th {\n\
    padding: 5px;\n\
    text-align:left;\n\
  }\n\
  table.var td {\n\
    padding: 8px;\n\
  }\n\
  </style>\n\
";
		httpdOutput(server, html);
	}

	// javascripts
	HASH_ITER(hh, config->impl->vars, curr, tmp) {

		char* html = "\
  <script>\n\
  $(document).ready(function() {\n\
    var name = '%s';\n\
    var elem = $('#'+name);\n\
    var elemVal = $('#'+name+'_val');\n\
    elem.slider({ min:%f, max:%f, value:%f,\n\
      stop:function(event, ui) {\n\
        var data = {}; data[name] = elem.slider('value');\n\
        jQuery.get('setter', data);\n\
        elemVal.html(data[name]);\n\
      }\n\
    });\n\
    elemVal.html(elem.slider('value'));\n\
  });\n\
  </script>\n\
";
		httpdPrintf(server, html, curr->desc.name, curr->desc.lowerBound, curr->desc.upperBound, *(curr->desc.value));
	}
	
	// html body
	{
		char* html = "\
</head>\n\
<body><table class='var'>\n\
<tr><th>Name</th><th style='width:200px;'>Value</th><th></th></tr>\n\
";
		httpdOutput(server, html);
	}

	// <div> tags
	HASH_ITER(hh, config->impl->vars, curr, tmp) {
		char* html = "<tr><td>%s</td><td><div id='%s'></div></td><td><div id='%s_val'></div></td></tr>\n";
		httpdPrintf(server, html,curr->desc.name, curr->desc.name, curr->desc.name);
	}

	// footer
	{
		char* html = "</table></body>\n</html>";
		httpdOutput(server, html);
	}
}

void remoteConfigSetterhtml(httpd* server)
{
	RemoteVar* rvar;
	httpVar* hvar;
	RemoteConfig* config = (RemoteConfig*)server->userData;

	if(nullptr == (hvar = httpdGetVariableByPrefix(server, ""))) {
		httpdPrintf(server, "false");
		return;
	}

	if(nullptr == (rvar = remoteConfigFindVar(config, hvar->name))) {
		httpdPrintf(server, "false");
		return;
	}

	if(0 == sscanf(hvar->value, "%f", rvar->desc.value)) {
		httpdPrintf(server, "false");
		return;
	}
	httpdPrintf(server, "true");
}

void remoteConfigInit(RemoteConfig* self, int port, XprBool useThread)
{
	self->impl->http = httpdCreate(nullptr, port);
	self->impl->http->userData = self;
	httpdAddCContent(self->impl->http, "/", "index.html", HTTP_TRUE, nullptr, remoteConfigIndexhtml);
	httpdAddCContent(self->impl->http, "/", "setter", HTTP_FALSE, nullptr, remoteConfigSetterhtml);

	if(useThread) {
		InitializeCriticalSection(&self->impl->criticalSection);
		self->impl->threadHandle = CreateThread(nullptr, 0, RemoteConfig_thread, self, 0, &self->impl->threadId);
	}
}

void remoteConfigAddVars(RemoteConfig* self, RemoteVarDesc* descs)
{
	RemoteVarDesc* curr = descs;

	while(nullptr != curr->name) {
		remoteConfigAddVar(self, *curr);
		++curr;
	}
}

void remoteConfigProcessRequest(RemoteConfig* self)
{
	struct timeval to;
	to.tv_sec = 0;
	to.tv_usec = 100;
	
	if (httpdGetConnection(self->impl->http, &to) <= 0)
		return;

	if(httpdReadRequest(self->impl->http) < 0) {
		httpdEndRequest(self->impl->http);
		return;
	}

	httpdProcessRequest(self->impl->http);

	httpdEndRequest(self->impl->http);
}

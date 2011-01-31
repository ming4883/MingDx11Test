#include "Remote.h"

#include "../lib/xprender/StrHash.h"
#include "../lib/xprender/uthash/uthash.h"
#include "../lib/httpd/httpd.h"

#include <stdio.h>
#include <winsock2.h>

typedef struct RemoteVar
{
	RemoteVarDesc desc;
	XprHashCode id;
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
	free(self->desc.name);
	free(self);
}

void RemoteVar_init(RemoteVar* self, RemoteVarDesc desc)
{
	self->id = XPR_HASH(desc.name);
	self->desc.name = strdup(desc.name);
	self->desc.value = desc.value;
	self->desc.upperBound = desc.upperBound;
	self->desc.lowerBound = desc.lowerBound;
}

typedef struct RemoteConfigImpl
{
	RemoteVar* vars;
	httpd* http;
} RemoteConfigImpl;

RemoteConfig* RemoteConfig_alloc()
{
	RemoteConfig* self;
	XprAllocWithImpl(self, RemoteConfig, RemoteConfigImpl);
	return self;
}

void RemoteConfig_free(RemoteConfig* self)
{
	struct RemoteVar *curr, *tmp;

	HASH_ITER(hh, self->impl->vars, curr, tmp) {
		HASH_DEL(self->impl->vars, curr);
		RemoteVar_free(curr);
	}

	free(self);
}


RemoteVar* RemoteConfig_findVar(RemoteConfig* self, const char* name)
{
	RemoteVar* var = nullptr;
	XprHashCode id;
	if(nullptr == self)
		return nullptr;

	id = XPR_HASH(name);
	HASH_FIND_INT(self->impl->vars, &id, var);

	return var;
}

void RemoteConfig_indexhtml(httpd* server)
{
	struct RemoteVar *curr, *tmp;
	RemoteConfig* config = (RemoteConfig*)server->userData;

	httpdOutput(server, "\
<!DOCTYPE html>\n\
<html>\n\
<head>\n\
  <link href='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/themes/base/jquery-ui.css' rel='stylesheet' type='text/css'/>\n\
  <script src='http://ajax.googleapis.com/ajax/libs/jquery/1.4/jquery.min.js'></script>\n\
  <script src='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/jquery-ui.min.js'></script>\n\
  <style type='text/css'>#slider { margin: 10px; }</style>\n");

	HASH_ITER(hh, config->impl->vars, curr, tmp) {	
		char* content = "\
  <script>\n\
  $(document).ready(function() {\n\
	var name = '%s';\n\
	var elem = $('#'+name);\n\
	elem.slider({ min:%f, max:%f, value:%f,\n\
	  stop:function(event, ui) {\n\
	    var data = {}; data[name] = elem.slider('value');\n\
	    jQuery.get('setter', data);\n\
	  }\n\
	});\n\
  });\n\
  </script>\n";
		httpdPrintf(server, content, curr->desc.name, curr->desc.lowerBound, curr->desc.upperBound, *(curr->desc.value));
	}
	
	httpdOutput(server, "</head>\n\
<body style='font-size:75%;'><table style='background-color:LightBlue'>\n\
  <tr><th>name&nbsp;</th><th style='width:200px'>value</th></tr>\n");

	HASH_ITER(hh, config->impl->vars, curr, tmp) {		
		httpdPrintf(server, "\
  <tr><td>%s&nbsp;</td><td><div id='%s'></div></td></tr>\n",
		curr->desc.name, curr->desc.name);
	}

	httpdOutput(server, "\
</table></body>\n\
</html>\
");
}

void RemoteConfig_setterhtml(httpd* server)
{
	RemoteVar* rvar;
	httpVar* hvar;
	RemoteConfig* config = (RemoteConfig*)server->userData;

	if(nullptr == (hvar = httpdGetVariableByPrefix(server, ""))) {
		httpdPrintf(server, "false");
		return;
	}

	if(nullptr == (rvar = RemoteConfig_findVar(config, hvar->name))) {
		httpdPrintf(server, "false");
		return;
	}

	if(0 == sscanf(hvar->value, "%f", rvar->desc.value)) {
		httpdPrintf(server, "false");
		return;
	}
	httpdPrintf(server, "true");
	/**/
}

void RemoteConfig_init(RemoteConfig* self, int port)
{
	self->impl->http = httpdCreate(nullptr, port);
	self->impl->http->userData = self;
	httpdAddCContent(self->impl->http, "/", "index.html", HTTP_TRUE, nullptr, RemoteConfig_indexhtml);
	httpdAddCContent(self->impl->http, "/", "setter", HTTP_FALSE, nullptr, RemoteConfig_setterhtml);
}

void RemoteConfig_addVar(RemoteConfig* self, RemoteVarDesc desc)
{
	RemoteVar* var;
	if(nullptr == self)
		return;

	var = RemoteVar_alloc();
	RemoteVar_init(var, desc);
	HASH_ADD_INT(self->impl->vars, id, var);
}

void RemoteConfig_addVars(RemoteConfig* self, RemoteVarDesc* descs)
{
	RemoteVarDesc* curr = descs;

	while(nullptr != curr->name) {
		RemoteConfig_addVar(self, *curr);
		++curr;
	}
}

void RemoteConfig_processRequest(RemoteConfig* self)
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

#ifndef __EXAMPLE_REMOTE_H__
#define __EXAMPLE_REMOTE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RemoteVarDesc
{
	char* name;
	float* value;
	float lowerBound, upperBound;
} RemoteVarDesc;

typedef struct RemoteConfigImpl;

typedef struct RemoteConfig
{
	struct RemoteConfigImpl* impl;
} RemoteConfig;

RemoteConfig* RemoteConfig_alloc();

void RemoteConfig_free(RemoteConfig* self);

void RemoteConfig_init(RemoteConfig* self, int port);

void RemoteConfig_addVar(RemoteConfig* self, RemoteVarDesc desc);

void RemoteConfig_addVars(RemoteConfig* self, RemoteVarDesc* descs);

void RemoteConfig_processRequest(RemoteConfig* self);

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_REMOTE_H__
#ifndef __EXAMPLE_REMOTE_H__
#define __EXAMPLE_REMOTE_H__

#include "../lib/xprender/Platform.h"

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

void RemoteConfig_init(RemoteConfig* self, int port, XprBool useThread);

void RemoteConfig_addVars(RemoteConfig* self, RemoteVarDesc* descs);

void RemoteConfig_processRequest(RemoteConfig* self);

void RemoteConfig_lock(RemoteConfig* self);

void RemoteConfig_unlock(RemoteConfig* self);

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_REMOTE_H__
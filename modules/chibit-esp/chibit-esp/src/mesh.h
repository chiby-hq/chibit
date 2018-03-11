#ifndef _MESH_H

#define _MESH_H

#include <Arduino.h>
#include "painlessMesh.h"

#define NODE_ID_T uint32_t

void onMeshMessage( NODE_ID_T from, String &msg );

void onNewMeshConnection(NODE_ID_T nodeId);


void meshInit();


NODE_ID_T meshGetNodeId();

void meshUpdate();

void meshSendSingle(NODE_ID_T nodeId, String& msg);
void meshSendBroadcast(String& msg);

void meshDeclareElectionTimeout(int timeoutInMillis, std::function<void()> concludeElectionCallback);
void meshStartElectionTimeout();
#endif

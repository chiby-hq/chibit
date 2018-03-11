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

bool meshSendSingle(NODE_ID_T nodeId, String& msg);
void meshSendBroadcast(String& msg);

void meshStartElectionConclusionTimeout(int timeoutInMillis, std::function<void()> electionConclusionCallback);

void meshStartElectionCalloutTimer(int timeoutInMillis, std::function<void()> electionCalloutCallback);
void meshCancelElectionCalloutTimer();

#endif

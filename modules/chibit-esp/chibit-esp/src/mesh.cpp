#ifndef _MESH_H_MOCK
#include "mesh.h"

#include "secret_mesh.h"

painlessMesh m_mesh;
Task m_electionConclusionTask;

void meshInit(){

  // Initialise a m_mesh with 4 connection maximum per default
  m_mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  m_mesh.onReceive(&onMeshMessage);
  m_mesh.onNewConnection(&onNewMeshConnection);

}


void meshUpdate(){
  m_mesh.update();
}

#ifndef UNIT_TEST
void meshSendSingle(NODE_ID_T nodeId, String& msg){
  m_mesh.sendSingle(nodeId, msg);
}


void meshSendBroadcast(String& msg){
  m_mesh.sendBroadcast(msg);
}
#endif

NODE_ID_T meshGetNodeId(){
  return m_mesh.getNodeId();
}

void meshDeclareElectionTimeout(int timeoutInMillis, std::function<void()> concludeElectionCallback){
  m_electionConclusionTask.set(timeoutInMillis, 1, concludeElectionCallback);
  // Delete any existing pre-declared tasks....
  m_mesh.scheduler.deleteTask( m_electionConclusionTask );
  // ... and add them again to the scheduler
  m_mesh.scheduler.addTask( m_electionConclusionTask );

}
void meshStartElectionTimeout(){
  if( ! m_electionConclusionTask.isEnabled()){
    m_electionConclusionTask.enable();
  }
}
#endif

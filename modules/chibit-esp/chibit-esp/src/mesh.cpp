#ifndef _MESH_H_MOCK
#include "mesh.h"

#include "secret_mesh.h"

painlessMesh m_mesh;
Task m_electionConclusionTask;
bool m_electionConclusionTaskDeclared = false;

Task m_electionCalloutTask;
bool m_electionCalloutTaskDeclared = false;


void meshInit(){

  // Initialise a m_mesh with 4 connection maximum per default
  m_mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  m_mesh.onReceive(&onMeshMessage);
  m_mesh.onNewConnection(&onNewMeshConnection);

}


void meshUpdate(){
  m_mesh.update();
}

#ifndef MOCK_MESH
  bool meshSendSingle(NODE_ID_T nodeId, String& msg){
    return m_mesh.sendSingle(nodeId, msg);
  }


  void meshSendBroadcast(String& msg){
    m_mesh.sendBroadcast(msg);
  }
#endif

NODE_ID_T meshGetNodeId(){
  return m_mesh.getNodeId();
}

void meshStartElectionConclusionTimeout(int timeoutInMillis, std::function<void()> electionConclusionCallback){
  m_electionConclusionTask.set(timeoutInMillis, 1, electionConclusionCallback);
  if(!m_electionConclusionTaskDeclared){
    m_mesh.scheduler.addTask( m_electionConclusionTask );
    m_electionConclusionTaskDeclared = true;
  }
  m_electionConclusionTask.enable();
}

void meshStartElectionCalloutTimer(int timeoutInMillis, std::function<void()> electionCalloutCallback){
  m_electionCalloutTask.set(timeoutInMillis, 1, electionCalloutCallback);
  if(!m_electionCalloutTaskDeclared){
    m_mesh.scheduler.addTask( m_electionCalloutTask );
    m_electionCalloutTaskDeclared = true;
  }
  m_electionCalloutTask.enable();
}

void meshCancelElectionCalloutTimer(){
  m_electionCalloutTask.disable();
}
#endif

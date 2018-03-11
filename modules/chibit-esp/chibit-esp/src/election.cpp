#include "election.h"

#ifndef MESH_MOCK
#include "mesh.h"
#endif

#include <iterator>
#include <map>


const char* ELECTION_JSON_OPERATION_MEMBER = "ope";
const char* ELECTION_JSON_COLOR_MEMBER = "color";
const char* ELECTION_JSON_VOTE_MEMBER = "vote";

const int ELECTION_NO_COLOR = 0x76448A;

const int ELECTION_COLOR_LEADER=0x4682B4;

const int ELECTION_COLOR_RED=0x8B0000;
const int ELECTION_COLOR_GREEN=0x228B22;
const int ELECTION_COLOR_YELLOW=0xFFFF00;
const int ELECTION_COLOR_PLAINRED=0xFF0000;
const int ELECTION_COLOR_DEEPPINK=0xFF1493;
const int ELECTION_COLOR_DARKKHAKI=0xBDB76B;
const int ELECTION_COLOR_TOMATO=0xFF6347;
const int ELECTION_COLOR_DARKCYAN=0x008B8B;
const int ELECTION_COLOR_PEACHPUFF=0xFFDAB9;
const int ELECTION_COLOR_SKYBLUE=0x87CEEB;
const int ELECTION_COLOR_CHOCOLATE=0xD2691E;
const int ELECTION_COLOR_SLATEGRAY=0x708090;

const int COLORS[] = {ELECTION_COLOR_RED, ELECTION_COLOR_GREEN,
                ELECTION_COLOR_YELLOW, ELECTION_COLOR_PLAINRED, ELECTION_COLOR_DEEPPINK,
                ELECTION_COLOR_DARKKHAKI, ELECTION_COLOR_TOMATO, ELECTION_COLOR_DARKCYAN,
                ELECTION_COLOR_PEACHPUFF, ELECTION_COLOR_SKYBLUE, ELECTION_COLOR_CHOCOLATE,
                ELECTION_COLOR_SLATEGRAY};


int nextColorIndex = 0;

int m_currentColor = ELECTION_NO_COLOR;
NODE_ID_T m_meshLeaderNodeId;
NODE_ID_T m_myNodeId;


int myVote = -1;


// forward declaration
void _concludeElection();


std::map<NODE_ID_T, int> m_mapNodeToColor;

std::map<NODE_ID_T, int> m_mapNodeToVote;



NODE_ID_T getMyNodeId(){
  return m_myNodeId;
}

NODE_ID_T getMyLeaderId(){  return m_meshLeaderNodeId;}

void setMyLeaderId(NODE_ID_T newLeaderId){
  m_meshLeaderNodeId = newLeaderId;
}



int getMyColor() { return m_currentColor;}


int _getColorForNode(NODE_ID_T nodeId){
  std::map<NODE_ID_T, int>::iterator it = m_mapNodeToColor.find(nodeId);
  if(it != m_mapNodeToColor.end()){
    return it->second;
  }else{
    int newColor = COLORS[nextColorIndex++];
    if (nextColorIndex > ((sizeof(COLORS)/sizeof(int)))){
      nextColorIndex = 0;
    }
    m_mapNodeToColor[nodeId] =  newColor;
    return newColor;
  }
}

/* ***********************************
 *  public Election methods
 * ***********************************/
void electionInit(bool resetState){
  if(resetState){
    m_mapNodeToColor.clear();
    nextColorIndex = 0;
  }
  // initialise random seed for mesh leader election
  randomSeed(analogRead(0));
  m_meshLeaderNodeId = ELECTION_NO_LEADER;
  myVote = random(1024);
  meshInit();
  m_myNodeId = meshGetNodeId();

  meshDeclareElectionTimeout(3000, &_concludeElection);
}

void electionUpdate(){
  meshUpdate();
}

/* ***********************************
 *  private mesh messaging methods
 * ***********************************/

void _sendMeetYourLeaderMessage(bool broadcast, NODE_ID_T nodeId = 0 ){

  StaticJsonBuffer<80> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root[ELECTION_JSON_OPERATION_MEMBER] = ELECTION_SUBPROTOCOL_LEADER_ANNOUNCE;

  // If not a broadcast, inform the given node
  if(!broadcast){
    String msg;
    root[ELECTION_JSON_COLOR_MEMBER] = _getColorForNode(nodeId);
    root.printTo(msg);
    meshSendSingle(nodeId, msg);
  }else{
    // If broadcast, inform all voters individually of their new color
    // Allocate or retrieve color for this node
    for (std::map<NODE_ID_T, int>::iterator it=m_mapNodeToVote.begin(); it!=m_mapNodeToVote.end(); ++it){
      if(it->first != m_myNodeId){  // don't send the message to yourself
        String msg;
        root[ELECTION_JSON_COLOR_MEMBER] = _getColorForNode(it->first);
        root.printTo(msg);
        meshSendSingle(it->first, msg);
      }
    }
  }

}

void _concludeElection(){
   // Look for the highest vote in the list and keep it as a leader
   int maxVote = -1;
   NODE_ID_T candidateLeader = ELECTION_NO_LEADER;
   for (std::map<NODE_ID_T, int>::iterator it=m_mapNodeToVote.begin(); it!=m_mapNodeToVote.end(); ++it){
     if(it->second > maxVote){
       candidateLeader = it->first;
       maxVote = it->second;
     }
   }
   if(candidateLeader != ELECTION_NO_LEADER){
    m_meshLeaderNodeId = candidateLeader;
    m_currentColor = ELECTION_COLOR_LEADER;
    if(candidateLeader == m_myNodeId){
      _sendMeetYourLeaderMessage(true);
    }
  }else{
    // No leader was elected, wait to hold another election
    m_meshLeaderNodeId = ELECTION_NO_LEADER;
  }

}

/* **************************************************
 *  Mesh callbacks (note that the headers are defined in mesh.h)
 * **************************************************/
void onMeshMessage(NODE_ID_T from, String &msg ) {
  StaticJsonBuffer<80> incomingBuffer;
  JsonObject& msgRoot = incomingBuffer.parseObject(msg);

  StaticJsonBuffer<80> outgoingBuffer;
  JsonObject& outgoingRoot = outgoingBuffer.createObject();

  int subprotocol = msgRoot[ELECTION_JSON_OPERATION_MEMBER];
  switch(subprotocol){
     case ELECTION_SUBPROTOCOL_LEADER_ANNOUNCE:
       // Greet your new leader and accept your color !
       m_meshLeaderNodeId = from;
       m_currentColor = msgRoot[ELECTION_JSON_COLOR_MEMBER];
     break;
     case ELECTION_SUBPROTOCOL_VOTE_ANNOUNCE:
       m_mapNodeToVote.clear();
       // Citizens, cast your votes !
       outgoingRoot[ELECTION_JSON_OPERATION_MEMBER] = ELECTION_SUBPROTOCOL_VOTE_VOTE;
       outgoingRoot[ELECTION_JSON_VOTE_MEMBER] = myVote;
       outgoingRoot.printTo(msg);
       meshSendBroadcast(msg);
     break;
     case ELECTION_SUBPROTOCOL_VOTE_VOTE:
       // Keep the received vote in memory
       m_mapNodeToVote[from] = msgRoot[ELECTION_JSON_VOTE_MEMBER];
       // Start a timer - upon expiry, if the current node is the winner
       // of the election, send a "Meet Your Leader message" to each
       // mesh member
       meshStartElectionTimeout();
     break;
  }
}

void onNewMeshConnection(NODE_ID_T nodeId) {
  // The node is the current leader, he assigns a color to the newcomer
  if(m_meshLeaderNodeId != ELECTION_NO_LEADER && m_myNodeId == m_meshLeaderNodeId){
    _sendMeetYourLeaderMessage(false, nodeId);
  }else{
    // There is no known leader, all nodes broadcast a vote within 3 seconds
    // and a leader is chosen
    String msg;
    StaticJsonBuffer<40> buf;
    JsonObject& root = buf.createObject();
    root[ELECTION_JSON_OPERATION_MEMBER] = ELECTION_SUBPROTOCOL_VOTE_ANNOUNCE;
    root.printTo(msg);
    meshSendBroadcast(msg);
  }
}

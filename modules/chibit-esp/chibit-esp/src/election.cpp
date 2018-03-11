#include "election.h"

#ifndef MESH_MOCK
#include "mesh.h"
#endif

#include <ArduinoLog.h>

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
  Log.notice(CR "Starting Election support ..");
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

  Log.notice("Started Election support" CR);

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
    Log.notice("Informing mesh participant of his color %d" CR, root[ELECTION_JSON_COLOR_MEMBER]);
    meshSendSingle(nodeId, msg);
  }else{
    Log.notice("Broadcasting election results to all mesh participants..." CR);

    // If broadcast, inform all voters individually of their new color
    // Allocate or retrieve color for this node
    for (std::map<NODE_ID_T, int>::iterator it=m_mapNodeToVote.begin(); it!=m_mapNodeToVote.end(); ++it){
      if(it->first != m_myNodeId){  // don't send the message to yourself
        String msg;
        root[ELECTION_JSON_COLOR_MEMBER] = _getColorForNode(it->first);
        Log.notice("  - Mesh participant '%X' receives color '%d'" CR, it->first ,root[ELECTION_JSON_COLOR_MEMBER]);
        root.printTo(msg);
        bool success = meshSendSingle(it->first, msg);
        if(!success){
          Log.warning("Could not send leader notification to %X" CR, it->first);
        }
      }
    }
    Log.notice("Completed broadcast election results to all mesh participants." CR);
  }

}

void _concludeElection(){
   Log.notice("Concluding leader election..." CR);
   // Look for the highest vote in the list and keep it as a leader
   int maxVote = -1;
   NODE_ID_T candidateLeader = ELECTION_NO_LEADER;
   for (std::map<NODE_ID_T, int>::iterator it=m_mapNodeToVote.begin(); it!=m_mapNodeToVote.end(); ++it){
     Log.trace("%X voted %d .", it->first, it->second);
     if(it->second > maxVote){
       candidateLeader = it->first;
       maxVote = it->second;
     }
   }
   if(candidateLeader != ELECTION_NO_LEADER){
    Log.notice(CR "Found new leader : %X" CR, candidateLeader);
    m_meshLeaderNodeId = candidateLeader;
    m_currentColor = ELECTION_COLOR_LEADER;
    if(candidateLeader == m_myNodeId){
      Log.notice("I am the new leader, informing all participants..." CR);
      _sendMeetYourLeaderMessage(true);
    }
  }else{
    // No leader was elected, wait to hold another election
    m_meshLeaderNodeId = ELECTION_NO_LEADER;
    Log.warning("Election failed, no leader found !" CR);
  }

}

void _calloutElection(){
  Log.notice("Now calling out an election" CR);
  String msg;
  StaticJsonBuffer<40> buf;
  JsonObject& root = buf.createObject();
  root[ELECTION_JSON_OPERATION_MEMBER] = ELECTION_SUBPROTOCOL_VOTE_ANNOUNCE;
  root.printTo(msg);
  meshSendBroadcast(msg);
  Log.notice("Called out an election" CR);
}


/* **************************************************
 *  Mesh callbacks (note that the headers are defined in mesh.h)
 * **************************************************/
void onMeshMessage(NODE_ID_T from, String &msg ) {
  StaticJsonBuffer<80> incomingBuffer;
  JsonObject& msgRoot = incomingBuffer.parseObject(msg);

  StaticJsonBuffer<80> outgoingBuffer;
  JsonObject& outgoingRoot = outgoingBuffer.createObject();
  String outgoingMsg;

  int subprotocol = msgRoot[ELECTION_JSON_OPERATION_MEMBER];
  switch(subprotocol){
     case ELECTION_SUBPROTOCOL_LEADER_ANNOUNCE:
       // cancelling the pending election
       meshCancelElectionCalloutTimer();
       // Greet your new leader and accept your color !
       m_meshLeaderNodeId = from;
       m_currentColor = msgRoot[ELECTION_JSON_COLOR_MEMBER];
       Log.notice("Mesh msg : Informed of my new leader %X, received color %d" CR, m_meshLeaderNodeId,m_currentColor);
     break;
     case ELECTION_SUBPROTOCOL_VOTE_ANNOUNCE:
       m_mapNodeToVote.clear();
       // Citizens, cast your votes !
       outgoingRoot[ELECTION_JSON_OPERATION_MEMBER] = ELECTION_SUBPROTOCOL_VOTE_VOTE;
       outgoingRoot[ELECTION_JSON_VOTE_MEMBER] = myVote;
       Log.notice("Mesh msg : Vote announcement, casting my vote '%d'" CR, myVote);
       outgoingRoot.printTo(outgoingMsg);
       m_mapNodeToVote[m_myNodeId] = myVote;
       meshSendBroadcast(outgoingMsg);
     break;
     case ELECTION_SUBPROTOCOL_VOTE_VOTE:
       // Keep the received vote in memory
       m_mapNodeToVote[from] = msgRoot[ELECTION_JSON_VOTE_MEMBER];
       Log.notice("Mesh msg : Received vote from '%X' (%d)" CR,from,m_mapNodeToVote[from]);
       // Start a timer - upon expiry, if the current node is the winner
       // of the election, send a "Meet Your Leader message" to each
       // mesh member
       meshStartElectionConclusionTimeout(3000, &_concludeElection);
     break;
  }
}

void onNewMeshConnection(NODE_ID_T from) {
  Log.notice("New Mesh connection notification from '%X' ( I am '%X' )" CR,from, m_myNodeId);
  // The node is the current leader, he assigns a color to the newcomer
  if(m_meshLeaderNodeId != ELECTION_NO_LEADER && m_myNodeId == m_meshLeaderNodeId){
    Log.notice("Mesh connection : LEADER says '%X' just joined, telling them I am the current leader" CR,from);
    _sendMeetYourLeaderMessage(false, from);
  }else{
    Log.notice("Mesh connection : no leader - '%X' just joined, requesting an election if no one answers" CR,from);
    // Start a timer and request an election if no leader answered within 2 seconds
    meshStartElectionCalloutTimer(2000, &_calloutElection);
  }
}

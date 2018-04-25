#include "main.h"
#ifdef UNIT_TEST


#include <unity.h>


/****
 * Forward declarations for mock testing
 **********************/

#include <election.h>

void onMeshMessage( uint32_t from, String &msg );

void onNewMeshConnection(uint32_t nodeId);

/*********************/

int singleMessageCount =0;
uint32_t singleMessageLastRecipient = 0;
String singleLastMessage;

int broadcastMessageCount = 0;
String broadcastLastMessage;

bool meshSendSingle(uint32_t recipient, String& msg){
  singleMessageCount++;
  singleMessageLastRecipient = recipient;
  singleLastMessage = msg;
  return true;
}


void meshSendBroadcast(String& msg){
  broadcastMessageCount++;
  broadcastLastMessage = msg;
}

void resetTestCounters(){
  broadcastMessageCount=0;
  singleMessageCount=0;
  singleMessageLastRecipient = 0;
}

void test_existing_leader(){
  resetTestCounters();
  electionInit(true);
  // force preset the leader to itself
  TEST_ASSERT_TRUE(getMyNodeId() != ELECTION_NO_LEADER);
  setMyLeaderId(getMyNodeId());
  TEST_ASSERT_TRUE(getMyLeaderId() != ELECTION_NO_LEADER);
  TEST_ASSERT_EQUAL_INT(getMyNodeId(), getMyLeaderId());
  // simulate a new node with ID 14 joining
  onNewMeshConnection(14);
  // we should receive a message indicating who is the leader....
  TEST_ASSERT_EQUAL_INT(1, singleMessageCount);
  TEST_ASSERT_EQUAL_INT(14, singleMessageLastRecipient);
  // ...and our newly assigned color
  {
    String expected = "";
    StaticJsonBuffer<80> buf;
    JsonObject& msg = buf.parseObject(singleLastMessage);
    TEST_ASSERT_EQUAL_INT(ELECTION_SUBPROTOCOL_LEADER_ANNOUNCE, msg[ELECTION_JSON_OPERATION_MEMBER]);
    TEST_ASSERT_EQUAL_INT(COLORS[0], msg[ELECTION_JSON_COLOR_MEMBER]);
  }
}

void test_color_assignment(){
  int firstColor = _getColorForNode(10);
  int secondColor = _getColorForNode(12);
  TEST_ASSERT_TRUE(firstColor != secondColor);
  TEST_ASSERT_EQUAL_INT(COLORS[0], firstColor);
  TEST_ASSERT_EQUAL_INT(COLORS[1], secondColor);
  // Test that colors are "sticky"
  TEST_ASSERT_EQUAL_INT(COLORS[0], _getColorForNode(10));

  // Test that colors loop around
  for(int i = 20 ; i < 40 ; i++){
    _getColorForNode(i);
  }

}

void test_simple_election(){
  resetTestCounters();
  electionInit(true);
  TEST_ASSERT_TRUE(getMyLeaderId()==ELECTION_NO_LEADER);

  onNewMeshConnection(1);
  TEST_ASSERT_TRUE(getMyLeaderId()==ELECTION_NO_LEADER);

  TEST_ASSERT_EQUAL_INT(1, broadcastMessageCount);
  {
    StaticJsonBuffer<80> buf;
    JsonObject& msg = buf.parseObject(broadcastLastMessage);
    TEST_ASSERT_EQUAL_INT(ELECTION_SUBPROTOCOL_VOTE_ANNOUNCE, msg[ELECTION_JSON_OPERATION_MEMBER]);
  }
  {
    StaticJsonBuffer<80> buf;
    JsonObject& jsonMsg = buf.createObject();
    jsonMsg[ELECTION_JSON_OPERATION_MEMBER] = ELECTION_SUBPROTOCOL_VOTE_VOTE;
    jsonMsg[ELECTION_JSON_VOTE_MEMBER] = 100;

    String msgStr;
    jsonMsg.printTo(msgStr);
    onMeshMessage(1, msgStr);
  }

}

void setup(){

  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_color_assignment);
  RUN_TEST(test_existing_leader);
  RUN_TEST(test_simple_election);
  UNITY_END();

}


void loop() {
    electionUpdate();

    delay(5000);
}
#endif

/* *********************************************
 * Mesh leader election protocol implementation
 */
#ifndef _ELECTION_H
#define _ELECTION_H

#include <ArduinoJson.h>
#define ELECTION_NO_LEADER 0

extern const int ELECTION_NO_COLOR;

extern const int ELECTION_COLOR_LEADER;

#define ELECTION_SUBPROTOCOL_LEADER_ANNOUNCE 1
#define ELECTION_SUBPROTOCOL_VOTE_ANNOUNCE 2
#define ELECTION_SUBPROTOCOL_VOTE_VOTE 3

extern const int ELECTION_COLOR_RED;
extern const int ELECTION_COLOR_GREEN;
extern const int ELECTION_COLOR_YELLOW;
extern const int ELECTION_COLOR_PLAINRED;
extern const int ELECTION_COLOR_DEEPPINK;
extern const int ELECTION_COLOR_DARKKHAKI;
extern const int ELECTION_COLOR_TOMATO;
extern const int ELECTION_COLOR_DARKCYAN;
extern const int ELECTION_COLOR_PEACHPUFF;
extern const int ELECTION_COLOR_SKYBLUE;
extern const int ELECTION_COLOR_CHOCOLATE;
extern const int ELECTION_COLOR_SLATEGRAY;


extern const char* ELECTION_JSON_OPERATION_MEMBER;
extern const char* ELECTION_JSON_COLOR_MEMBER;
extern const char* ELECTION_JSON_VOTE_MEMBER;

extern const int COLORS[];


void electionInit(bool resetState=false);

void electionUpdate();

int  getMyColor();

#ifdef UNIT_TEST
// We expose various methods to ease unit testing
// Note that the type actually depends on your mesh implementation
#include <Arduino.h>
void setMyLeaderId(uint32_t newLeaderId);
uint32_t getMyLeaderId();
uint32_t getMyNodeId();
int _getColorForNode(uint32_t nodeId);

#endif

#endif

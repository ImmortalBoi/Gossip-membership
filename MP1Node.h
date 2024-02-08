/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Header file of MP1Node class.
 **********************************/

#ifndef _MP1NODE_H_
#define _MP1NODE_H_

#include "stdincludes.h"
#include "Log.h"
#include "Params.h"
#include "Member.h"
#include "EmulNet.h"
#include "Queue.h"

/**
 * Macros
 */
#define TREMOVE 20
#define TFAIL 5
#define MEMBERLISTPOS 0
#define GROUPMAX 5
#define TIMEOUT 5
#define TCLEANUP 5
#define TTL 2

/*
 * Note: You can change/add any functions in MP1Node.{h,cpp}
 */

/**
 * Message Types
 */
enum MsgTypes{
    JOINREQ,
    JOINREP,
    GOSSIP,
	FAIL,
	JOINACK
};

/**
 * STRUCT NAME: MessageHdr
 *
 * DESCRIPTION: Header and content of a message
 */
typedef struct MessageHdr {
	enum MsgTypes msgType;
}MessageHdr;

/**
 * CLASS NAME: MP1Node
 *
 * DESCRIPTION: Class implementing Membership protocol functionalities for failure detection
 */
class MP1Node {
private:
	EmulNet *emulNet;
	Log *log;
	Params *par;
	Member *memberNode;
	short failedNodes[GROUPMAX + 1];

public:
	MP1Node(Member *, Params *, EmulNet *, Log *, Address *);
	Member * getMemberNode() {
		return memberNode;
	}
	int recvLoop();
	static int enqueueWrapper(void *env, char *buff, int size);
	void nodeStart(char *servaddrstr, short serverport);
	int initThisNode(Address *joinaddr);
	int introduceSelfToGroup(Address *joinAddress);
	int finishUpThisNode();
	void nodeLoop();
	void checkMessages();
	bool recvCallBack(void *env, char *data, int size);
	void nodeLoopOps();
	Address getJoinAddress();
	void initMemberListTable(Member *memberNode);
	void addMember(Address addr);
	vector<MemberListEntry> removeDuplicateMembers(vector<MemberListEntry> vec);
    void sendMemberTable(MsgTypes type, vector<MemberListEntry> vec, Address *target, Address *sender);
    void sendFailedAddress(MsgTypes type, Address *failedAddr, Address *target, Address *sender, long ttl);
    vector<MemberListEntry> selectRandomMembers(vector<MemberListEntry> vec);
    string Enum2Str(int num);
	void printSelfMembershipTable();
    vector<MemberListEntry> cleanupMembershipTable(vector<MemberListEntry> vec);
    void compareAdjustMembershipTable(vector<MemberListEntry> vec);
    void printMembershipTable(Address addr, vector<MemberListEntry> vec);
    void printAddress(Address *addr);
	void sendJoinAcknowledgement(MsgTypes type, long ttl);
	
    virtual ~MP1Node();
};

#endif /* _MP1NODE_H_ */

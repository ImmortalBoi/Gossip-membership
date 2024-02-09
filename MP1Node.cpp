/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of MP1Node class functions.
 **********************************/

#include "MP1Node.h"
#include "Member.h"
#include <sstream>

/*
 * Note: You can change/add any functions in MP1Node.{h,cpp}
 */

/**
 * Overloaded Constructor of the MP1Node class
 * You can add new members to the class if you think it
 * is necessary for your logic to work
 */
MP1Node::MP1Node(Member *member, Params *params, EmulNet *emul, Log *log, Address *address)
{
    // for (int i = 0; i < GROUPMAX + 1; i++)
    // {
    //     failedNodes[i] = 0;
    // }
    this->memberNode = member;
    this->emulNet = emul;
    this->log = log;
    this->par = params;
    this->memberNode->addr = *address;
}

/**
 * Destructor of the MP1Node class
 */
MP1Node::~MP1Node() {}

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: This function receives message from the network and pushes into the queue
 * 				This function is called by a node to receive messages currently waiting for it
 */
int MP1Node::recvLoop()
{
    if (memberNode->bFailed)
    {
        return false;
    }
    else
    {
        return emulNet->ENrecv(&(memberNode->addr), enqueueWrapper, NULL, 1, &(memberNode->mp1q));
    }
}

/**
 * FUNCTION NAME: enqueueWrapper
 *
 * DESCRIPTION: Enqueue the message from Emulnet into the queue
 */
int MP1Node::enqueueWrapper(void *env, char *buff, int size)
{
    Queue q;
    return q.enqueue((queue<q_elt> *)env, (void *)buff, size);
}

/**
 * FUNCTION NAME: nodeStart
 *
 * DESCRIPTION: This function bootstraps the node
 * 				All initializations routines for a member.
 * 				Called by the application layer.
 */
void MP1Node::nodeStart(char *servaddrstr, short servport)
{
    Address joinaddr;
    joinaddr = getJoinAddress();

    // Self booting routines
    if (initThisNode(&joinaddr) == -1)
    {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "init_thisnode failed. Exit.");
#endif
        exit(1);
    }

    if (!introduceSelfToGroup(&joinaddr))
    {
        finishUpThisNode();
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Unable to join self to group. Exiting.");
#endif
        exit(1);
    }

    return;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int MP1Node::initThisNode(Address *joinaddr)
{
    /*
     * This function is partially implemented and may require changes
     */
    // int id = *(int *)(&memberNode->addr.addr);
    // int port = *(short *)(&memberNode->addr.addr[4]);
    // Add self to the membership list
    memberNode->bFailed = false;
    memberNode->inited = true;
    memberNode->inGroup = false;
    // node is up!
    memberNode->nnb = 0;
    memberNode->heartbeat = 0;
    memberNode->pingCounter = TFAIL;
    memberNode->timeOutCounter = -1;
    initMemberListTable(memberNode);

    return 0;
}

/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int MP1Node::introduceSelfToGroup(Address *joinaddr)
{
    MessageHdr *msg;
#ifdef DEBUGLOG
    static char s[1024];
#endif

    if (0 == memcmp((char *)&(memberNode->addr.addr), (char *)&(joinaddr->addr), sizeof(memberNode->addr.addr)))
    {
        // I am the group booter (first process to join the group). Boot up the group
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Starting up group...");
#endif
        memberNode->inGroup = true;
    }
    else
    {
        size_t msgsize = sizeof(MessageHdr) + sizeof(joinaddr->addr) + sizeof(long) + 1;
        msg = (MessageHdr *)malloc(msgsize * sizeof(char));

        // create JOINREQ message: format of data is {struct Address myaddr}
        msg->msgType = JOINREQ;
        memcpy((char *)(msg + 1), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy((char *)(msg + 1) + 1 + sizeof(memberNode->addr.addr), &memberNode->heartbeat, sizeof(long));

#ifdef DEBUGLOG
        sprintf(s, "Attempting to Join");
        log->LOG(&memberNode->addr, s);
#endif

        // send JOINREQ message to introducer member
        emulNet->ENsend(&memberNode->addr, joinaddr, (char *)msg, msgsize);

        free(msg);
    }

    return 1;
}

/**
 * FUNCTION NAME: finishUpThisNode
 *
 * DESCRIPTION: Wind up this node and clean up state
 */
int MP1Node::finishUpThisNode()
{
    /*
     * Your code goes here
     */
    return 0;
}

/**
 * FUNCTION NAME: nodeLoop
 *
 * DESCRIPTION: Executed periodically at each member
 * 				Check your messages in queue and perform membership protocol duties
 */
void MP1Node::nodeLoop()
{
    if (memberNode->bFailed)
    {
        return;
    }

    // Check my messages
    checkMessages();

    // Wait until you're in the group...
    if (!memberNode->inGroup)
    {
        return;
    }

    // ...then jump in and share your responsibilites!
    nodeLoopOps();

    return;
}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: Check messages in the queue and call the respective message handler
 */
void MP1Node::checkMessages()
{
    void *ptr;
    int size;

    // Pop waiting messages from memberNode's mp1q
    while (!memberNode->mp1q.empty())
    {
        ptr = memberNode->mp1q.front().elt;
        size = memberNode->mp1q.front().size;
        memberNode->mp1q.pop();
        recvCallBack((void *)memberNode, (char *)ptr, size);
    }
    return;
}

/**
 * FUNCTION NAME: recvCallBack
 *
 * DESCRIPTION: Message handler for different message types
 */
bool MP1Node::recvCallBack(void *env, char *data, int size)
{
#ifdef DEBUGLOG
    static char s[1024];
#endif

    // Extracting msgType
    MessageHdr *msg = (MessageHdr *)data;
    int msgType = msg->msgType;

    // Create new MemberListEntry to add to the membership list

    // TODO Compare the received membership list with the one you have, if one with the same ip is found, compare heartbeat and timestamp then take
    // the higher one, if a new ip is found add it to the membership list
    // case 0 JOINREQ: receive request, add to the current member list and send membership table
    // case 1 JOINREP: receive request, dissect it and add membership list
    // case 2 GOSSIP: receive gossip, compare to current gossip table, update heartbeat/delete node
    // case 3 FAIL: if first time, Delete member from your table & forward the message to your people, add the member to a list of removed people, if second time ignore it
    if (msgType == JOINREQ)
    {
        // Extracting address
        Address sender;
        memcpy(&sender, (char *)(msg + 1), sizeof(sender));

        // Extracting heartbeat
        long receivedHeartbeat;
        memcpy(&receivedHeartbeat, (char *)(msg + 1) + 1 + sizeof(sender), sizeof(receivedHeartbeat));

        // #ifdef DEBUGLOG
        //         sprintf(s, "Received message type: JOINREQ, from address: %s, with heartbeat: %ld", sender.getAddress().c_str(), receivedHeartbeat);
        //         log->LOG(&memberNode->addr, s);
        // #endif

        addMember(sender);
        if (memberNode->memberList.size() < GROUPMAX)
        {
            // Send the entire membership table
            // printMembershipTable(memberNode->addr, memberNode->memberList);
            sendMemberTable(JOINREP, memberNode->memberList, &sender, &memberNode->addr);
        }
        else
        {
            // Select 6 randomly from it and send it
            vector<MemberListEntry> members = selectRandomMembers(memberNode->memberList);
            // printMembershipTable(memberNode->addr, members);
            sendMemberTable(JOINREP, members, &sender, &memberNode->addr);
        }
    }
    else if (msgType == JOINREP)
    {
        // Extracting address
        Address sender;
        memcpy(&sender, (char *)(msg + 1), sizeof(sender));

        long vecSize;
        // Assume 'netVecSize' is received from the network
        memcpy(&vecSize, (char *)(msg + 1) + sizeof(sender), sizeof(vecSize));
        vecSize >>= 16;

        vector<MemberListEntry> vec(vecSize);
        memcpy(vec.data(), (char *)(msg + 1) + sizeof(sender) + sizeof(vecSize), sizeof(MemberListEntry) * vecSize);
        // #ifdef DEBUGLOG
        //         sprintf(s, "Received message type: JOINREP, with Membership table of memory usage: %ld, from Address %s", sizeof(MemberListEntry) * vec.size(), sender.getAddress().c_str());
        //         log->LOG(&memberNode->addr, s);
        // #endif

        vec = cleanupMembershipTable(vec);
        // vec.shrink_to_fit();
        for (MemberListEntry member : vec)
        {
            memberNode->memberList.push_back(member);
        }
        memberNode->memberList = removeDuplicateMembers(memberNode->memberList);
        cout << "JOINREP Message Received\n";
        // printMembershipTable(memberNode->addr,vec);
        printSelfMembershipTable();

        memberNode->inGroup = true;
        sendJoinAcknowledgement(JOINACK, TTLAWK);

        // #ifdef DEBUGLOG
        //         sprintf(s, "Node %s joined at time %d", memberNode->addr.getAddress().c_str(), par->getcurrtime());
        //         log->LOG(&memberNode->addr, s);
        // #endif
    }
    else if (msgType == JOINACK)
    {
        // Dissect message into sender, heartbeat, ttl
        // If sender is in membership table then just skip, send it to membership table with ttl-1
        // If not and the size is smaller than the GROUPMAX then add it to the table with given heartbeat then send with lowered ttl
        // Extracting address
        Address sender;
        memcpy(&sender, (char *)(msg + 1), sizeof(sender));

        // Extracting heartbeat
        long receivedHeartbeat;
        memcpy(&receivedHeartbeat, (char *)(msg + 1) + 1 + sizeof(sender), sizeof(receivedHeartbeat));

        long ttl;
        memcpy(&ttl, (char *)(msg + 1) + 1 + sizeof(sender) + sizeof(receivedHeartbeat), sizeof(ttl));

        // #ifdef DEBUGLOG
        //         sprintf(s, "Received message type: JOINACK, from address: %s, with heartbeat: %ld, TTL: %ld", sender.getAddress().c_str(), receivedHeartbeat, ttl);
        //         log->LOG(&memberNode->addr, s);
        // #endif

#ifdef DEBUGLOG
        sprintf(s, "Node %s joined at time %d", printAddress(sender).c_str(), par->getcurrtime());
        log->LOG(&memberNode->addr, s);
#endif
        if (ttl == 0)
        {
            return true;
        }

        for (size_t i = 0; i < memberNode->memberList.size(); i++)
        {
            if (i == MEMBERLISTPOS)
            {
                continue;
            }
            string addr = to_string(memberNode->memberList[i].getid()) + ":" + to_string(memberNode->memberList[i].getport());
            Address member(addr);
            if (member == sender)
            {
                sendJoinAcknowledgement(JOINACK, ttl - 1);
                return true;
            }
        }
        if (memberNode->memberList.size() < GROUPMAX)
        {
            addMember(sender);
            sendJoinAcknowledgement(JOINACK, ttl - 1);
        }
    }
    else if (msgType == GOSSIP)
    {
        // Extracting address
        Address sender;
        memcpy(&sender, (char *)(msg + 1), sizeof(sender));

        long vecSize;

        memcpy(&vecSize, (char *)(msg + 1) + sizeof(sender), sizeof(vecSize));
        vecSize >>= 16;

        vector<MemberListEntry> vec(vecSize);
        memcpy(vec.data(), (char *)(msg + 1) + sizeof(sender) + sizeof(vecSize), sizeof(MemberListEntry) * vecSize);

        // #ifdef DEBUGLOG
        //         sprintf(s, "Received message type: GOSSIP, with Membership table of memory usage: %ld, from Address %s", sizeof(MemberListEntry) * vec.size(), sender.getAddress().c_str());
        //         log->LOG(&memberNode->addr, s);
        // #endif
        vec = cleanupMembershipTable(vec);
        vec.shrink_to_fit();
        // printMembershipTable(memberNode->addr,vec);
        compareAdjustMembershipTable(vec);
        cout << "GOSSIP Message Received\n";
        printSelfMembershipTable();
    }
    else if (msgType == FAIL)
    {
        Address sender;
        memcpy(&sender, (char *)(msg + 1), sizeof(sender));
        Address failedAddr;
        memcpy(&failedAddr, (char *)(msg + 1) + sizeof(sender), sizeof(failedAddr));
        long ttl;
        memcpy(&ttl, (char *)(msg + 1) + sizeof(sender) + sizeof(failedAddr) + 1, sizeof(ttl));

        string tmp = failedAddr.getAddress();
        size_t pos = tmp.find(":");
        int id = stoi(tmp.substr(0, pos));
        short port = (short)stoi(tmp.substr(pos + 1, tmp.size() - pos - 1));
        id >>= 16;
        port >>= 16;
        ttl >>= 32;
        memcpy(&failedAddr.addr[0], &id, sizeof(int));
        memcpy(&failedAddr.addr[4], &port, sizeof(short));

        // #ifdef DEBUGLOG
        //         sprintf(s, "Received message type: FAIL, informing that Address: %s has failed, from Address %s with ttl: %ld", failedAddr.getAddress().c_str(), sender.getAddress().c_str(), ttl);
        //         log->LOG(&memberNode->addr, s);
        // #endif

        if (ttl == 0)
        {
            return true;
        }
        // #ifdef DEBUGLOG
        //         // sprintf(s, "Node %s removed at time %d", failedAddr.getAddress().c_str(), par->getcurrtime());
        //         sprintf(s, "Node %s has failed ", printAddress(failedAddr).c_str());

        //         log->LOG(&memberNode->addr, s);
        // #endif
        for (size_t i = 0; i < memberNode->memberList.size(); i++)
        {
            if (i == MEMBERLISTPOS)
            {
                continue;
            }
            if (memberNode->memberList[i].getid() == id && memberNode->memberList[i].getport() == port)
            {
                // memberNode->memberList.erase(memberNode->memberList.begin() + i);
                failedNodes[i] = 1;
// #ifdef DEBUGLOG
//                 sprintf(s, "Node %s removed Node %s", printAddress(memberNode->addr).c_str(), printAddress(failedAddr).c_str());
//                 log->LOG(&memberNode->addr, s);
// #endif
            }
            string addr = to_string(memberNode->memberList[i].getid()) + ":" + to_string(memberNode->memberList[i].getport());
            Address target(addr);
            sendFailedAddress(FAIL, &failedAddr, &target, &memberNode->addr, ttl - 1);
        }
    }
    // Return true to indicate successful processing
    return true;
}

/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
 * 				the nodes
 * 				Propagate your membership list
 */
void MP1Node::nodeLoopOps()
{
#ifdef DEBUGLOG
    static char s[1024];
#endif
    // Loop over own membership table and check if any of the heartbeats exceed the timeout, if so remove them from the table
    // Send gossip over network
    // memberNode->memberList.shrink_to_fit();
    // memberNode->memberList = cleanupMembershipTable(memberNode->memberList);
    cout << "UPDATE my heartbeat and time stamp: " << memberNode->addr.getAddress() << "\n";
    memberNode->memberList[MEMBERLISTPOS].setheartbeat(memberNode->memberList[MEMBERLISTPOS].getheartbeat() + 1);
    // memberNode->heartbeat = memberNode->memberList[MEMBERLISTPOS].getheartbeat();
    memberNode->memberList[MEMBERLISTPOS].settimestamp(par->getcurrtime());

    // TODO check me's membership list of timed out individuals and if anybody did fail then send a FAIL

    // Loop over each element in the vector
    for (size_t i = 0; i < memberNode->memberList.size(); ++i)
    {
        if (failedNodes[i] == 1)
        {
            continue;
        }

        if (memberNode->memberList[MEMBERLISTPOS].gettimestamp() - memberNode->memberList[i].gettimestamp() > TIMEOUT)
        {
            cout << "FAILURE DETECTED\n";
            // printSelfMembershipTable();
            // cout << "Nodeloopops of: "<<memberNode->addr.getAddress().c_str()<<"\n";
            // cout<<"Time difference: "<<me.gettimestamp() - memberNode->memberList[i].gettimestamp()<<"\n";
            failedNodes[i] = 1;
            int id = memberNode->memberList[i].getid();
            short port = memberNode->memberList[i].getport();
            string addr = to_string(id) + ":" + to_string(port);
            Address failedAddr(addr);
            for (size_t k = 0; k < memberNode->memberList.size(); k++)
            {
                if (k == MEMBERLISTPOS)
                {
                    continue;
                }

                // if (memberNode->memberList[k].getid() == id && memberNode->memberList[k].getport() == port)
                // {
                //     memberNode->memberList.erase(memberNode->memberList.begin() + i);
                // }
                string addr = to_string(memberNode->memberList[k].getid()) + ":" + to_string(memberNode->memberList[k].getport());
                Address target(addr);
                sendFailedAddress(FAIL, &failedAddr, &target, &memberNode->addr, TTL);
            }

            memberNode->memberList[i].settimestamp(par->getcurrtime());
            // memberNode->memberList.erase(memberNode->memberList.begin() + i);
        }
    }
    // int TcleanUp = std::log10(MAX_NODES);
    // Check the Failed nodes to delete, get no. of nodes in the list, get log(n) of it, then set that as the time for failed nodes
    for (size_t i = 0,j = 0; j < memberNode->memberList.size(); i++,j++)
    {
        if (failedNodes[i] == 0)
        {
            continue;
        }
        else if (memberNode->memberList[MEMBERLISTPOS].gettimestamp() - memberNode->memberList[i].gettimestamp() >= TCLEANUP)
        {
            failedNodes[i] = 0;
#ifdef DEBUGLOG
            string addr = to_string(memberNode->memberList[j].getid()) + ":" + to_string(memberNode->memberList[j].getport());
            Address failedAddr(addr);
            sprintf(s, "Node %s removed Node %s", printAddress(memberNode->addr).c_str(), printAddress(failedAddr).c_str());
            log->LOG(&memberNode->addr, s);
#endif
            memberNode->memberList.erase(memberNode->memberList.begin() + j);
            j--;
        }
    }
    // TODO Send gossip with my new info in it
    // printSelfMembershipTable();
    short gossipSize = memberNode->memberList.size();
    for (size_t i = 0; i < gossipSize; i++)
    {
        if (i == MEMBERLISTPOS)
        {
            continue;
        }
        string addr = to_string(memberNode->memberList[i].getid()) + ":" + to_string(memberNode->memberList[i].getport());
        Address target(addr);
        sendMemberTable(GOSSIP, memberNode->memberList, &target, &memberNode->addr);
    }

    return;
}

/**
 * FUNCTION NAME: getJoinAddress
 *
 * DESCRIPTION: Returns the Address of the coordinator
 */
Address MP1Node::getJoinAddress()
{
    Address joinaddr;

    memset(&joinaddr, 0, sizeof(Address));
    *(int *)(&joinaddr.addr) = 1;
    *(short *)(&joinaddr.addr[4]) = 0;

    return joinaddr;
}

/**
 * FUNCTION NAME: initMemberListTable
 *
 * DESCRIPTION: Initialize the membership list
 */
void MP1Node::initMemberListTable(Member *memberNode)
{
    memberNode->memberList.clear();
    addMember(memberNode->addr);
}

/**
 * FUNCTION NAME: printAddress
 *
 * DESCRIPTION: Print the Address
 */
string MP1Node::printAddress(Address addr)
{
    std::stringstream ss;
    ss << static_cast<int>(addr.addr[0]) << "."
       << static_cast<int>(addr.addr[1]) << "."
       << static_cast<int>(addr.addr[2]) << "."
       << static_cast<int>(addr.addr[3]) << ":"
       << *reinterpret_cast<short *>(&addr.addr[4]);
    return ss.str();
}

/**
 * FUNCTION NAME: addMember
 *
 *  DESCRIPTION: Create a new member given an Address
 */
void MP1Node::addMember(Address addr)
{
    // Get string form of address and turn it into id form
    string addrString = addr.getAddress().c_str();
    size_t pos = addrString.find(":");
    int id = stoi(addrString.substr(0, pos));
    short port = (short)stoi(addrString.substr(pos + 1, addrString.size() - pos - 1));

    // Add the member to the membership list and remove duplicates
    MemberListEntry *member = new MemberListEntry(id, port, 0, 0);
    memberNode->memberList.push_back(*member);
    memberNode->memberList = removeDuplicateMembers(memberNode->memberList);
}

bool operator==(const MemberListEntry lhs, const MemberListEntry rhs)
{
    return (lhs.id == rhs.id && lhs.port == rhs.port);
}
/**
 * FUNCTION NAME: removeDuplicateMembers
 *
 *  DESCRIPTION: Remove duplicate members in a membership list
 */
vector<MemberListEntry> MP1Node::removeDuplicateMembers(vector<MemberListEntry> vec)
{
    // Loop over each element in the vector
    for (size_t i = 0; i < vec.size(); ++i)
    {
        // Compare the current element with all other elements
        for (size_t j = i + 1; j < vec.size(); ++j)
        {
            // If a match is found, erase the matching element
            if (vec[i] == vec[j])
            {
                vec.erase(vec.begin() + j); // Erase the matching element
                --j;                        // Adjust the index because the vector size has changed
            }
        }
    }

    return vec;
}

/**
 * FUNCTION NAME: sendMemberTable
 *
 *  DESCRIPTION: Send the membership table with appropriate message type
 */
void MP1Node::sendMemberTable(MsgTypes type, vector<MemberListEntry> vec, Address *target, Address *sender)
{
    MessageHdr *msg;

    // vec = cleanupMembershipTable(vec);
    vec.shrink_to_fit();
    // Calculate the size of the vector
    long vecSize = vec.size();

    // Adjust the total message size to account for the size of the vector
    size_t msgsize = sizeof(MessageHdr) + sizeof(sender) + sizeof(vecSize) + sizeof(MemberListEntry) * vecSize + 1;
    msg = (MessageHdr *)malloc(msgsize * sizeof(char));

    // create message: format of data is {MessageType Sender VectorSize VectorData}
    msg->msgType = type;
    memcpy((char *)(msg + 1), sender, sizeof(sender));
    memcpy((char *)(msg + 1) + sizeof(sender), &vecSize, sizeof(vecSize));
    memcpy((char *)(msg + 1) + sizeof(sender) + sizeof(vecSize), vec.data(), sizeof(MemberListEntry) * vecSize);

    // #ifdef DEBUGLOG
    //     static char s[1024];
    //     sprintf(s, "Sent membership table of memory size:%ld, with message type: %s to target: %s", sizeof(MemberListEntry) * vecSize, Enum2Str(msg->msgType).c_str(), target->getAddress().c_str());
    //     log->LOG(&memberNode->addr, s);
    // #endif

    emulNet->ENsend(&memberNode->addr, target, (char *)msg, msgsize);
    free(msg);
}

/**
 * FUNCTION NAME: sendJoinAcknowledgement
 *
 *  DESCRIPTION: Send to member's of the membership table that you have joined
 */
void MP1Node::sendJoinAcknowledgement(MsgTypes type, long ttl)
{
    short gossipSize = memberNode->memberList.size();
    for (size_t i = 0; i < gossipSize; i++)
    {
        if (i == MEMBERLISTPOS)
        {
            continue;
        }
        string addr = to_string(memberNode->memberList[i].getid()) + ":" + to_string(memberNode->memberList[i].getport());
        Address target(addr);

        MessageHdr *msg;
        size_t msgsize = sizeof(MessageHdr) + sizeof(&memberNode->addr.addr) + sizeof(long) + sizeof(long) + 1;
        msg = (MessageHdr *)malloc(msgsize * sizeof(char));

        // create JOINREQ message: format of data is {Address Heartbeat TTL}
        msg->msgType = type;
        memcpy((char *)(msg + 1), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy((char *)(msg + 1) + 1 + sizeof(memberNode->addr.addr), &memberNode->heartbeat, sizeof(long));
        memcpy((char *)(msg + 1) + 1 + sizeof(memberNode->addr.addr) + sizeof(memberNode->heartbeat), &ttl, sizeof(long));

        // #ifdef DEBUGLOG
        //         static char s[1024];
        //         sprintf(s, "Sent my details, with message type: JOINACK to target: %s", target.getAddress().c_str());
        //         log->LOG(&memberNode->addr, s);
        // #endif
        emulNet->ENsend(&memberNode->addr, &target, (char *)msg, msgsize);
        free(msg);
    }

    //     emulNet->ENsend(&memberNode->addr, target, (char *)msg, msgsize);
    //     free(msg);
}

/**
 * FUNCTION NAME: sendFailedAddress
 *
 *  DESCRIPTION: Send the address that failed
 */
void MP1Node::sendFailedAddress(MsgTypes type, Address *failedAddr, Address *target, Address *sender, long ttl)
{
    MessageHdr *msg;

    size_t msgsize = sizeof(MessageHdr) + sizeof(sender) + sizeof(failedAddr) + 1 + sizeof(ttl) + 1;
    msg = (MessageHdr *)malloc(msgsize * sizeof(char));

    // create message: format of data is {MessageType Sender FailedAddress}
    msg->msgType = type;
    memcpy((char *)(msg + 1), sender, sizeof(sender));
    memcpy((char *)(msg + 1) + sizeof(sender), failedAddr, sizeof(failedAddr));
    memcpy((char *)(msg + 1) + sizeof(sender) + sizeof(failedAddr) + 1, &ttl, sizeof(ttl));

    // #ifdef DEBUGLOG
    //     static char s[1024];
    //     sprintf(s, "Sent message type: %s to target: %s", Enum2Str(msg->msgType).c_str(), target->getAddress().c_str());
    //     log->LOG(&memberNode->addr, s);
    // #endif

    emulNet->ENsend(&memberNode->addr, target, (char *)msg, msgsize);
    free(msg);
}

/**
 * FUNCTION NAME: selectRandomMembers
 *
 *  DESCRIPTION: select 6 members from the membership table and sends them
 */
vector<MemberListEntry> MP1Node::selectRandomMembers(vector<MemberListEntry> vec)
{
    vector<MemberListEntry> res;
    vec.shrink_to_fit();
    res.push_back(memberNode->memberList[0]);
    while (res.size() != GROUPMAX - 1)
    {
        int idx = rand() % vec.size() + 1;
        res.push_back(vec[idx]);
        removeDuplicateMembers(res);
    }

    return res;
}

/**
 * FUNCTION NAME: Enum2Str
 *
 *  DESCRIPTION: Turn Enum to string value
 */
string MP1Node::Enum2Str(int num)
{
    switch (num)
    {
    case 0:
        return "JOINREQ";
        break;

    case 1:
        return "JOINREP";
        break;

    case 2:
        return "GOSSIP";
        break;

    case 3:
        return "FAIL";
        break;
    case 4:
        return "JOINACK";
        break;
    default:
        return "Unknown request";
        break;
    }
}

/**
 * FUNCTION NAME: printSelfMembershipTable
 *
 *  DESCRIPTION: Print the current node's membership table
 */
void MP1Node::printSelfMembershipTable()
{
    std::cout << "---------------------------------------------\n";
    std::cout << "Address " << memberNode->addr.getAddress().c_str() << "'s Table:\n";
    for (size_t i = 0; i < memberNode->memberList.size(); i++)
    {
        string addr = to_string(memberNode->memberList[i].getid()) + ":" + to_string(memberNode->memberList[i].getport());
        Address member(addr);
        std::cout << i << "- Address: " << member.getAddress() << " - Heartbeat: " << memberNode->memberList[i].getheartbeat() << " - Timestamp: " << memberNode->memberList[i].gettimestamp() << "\n";
    }

    std::cout << "---------------------------------------------\n";
}

/**
 * FUNCTION NAME: Enum2Str
 *
 *  DESCRIPTION: Print a given membership table of a node
 */
void MP1Node::printMembershipTable(Address addr, vector<MemberListEntry> vec)
{
    std::cout << "---------------------------------------------\n";
    std::cout << "Address " << addr.getAddress().c_str() << "'s Table:\n";
    for (size_t i = 0; i < vec.size(); i++)
    {
        string addr = to_string(vec[i].getid()) + ":" + to_string(vec[i].getport());
        Address member(addr);
        std::cout << i << "- Address: " << member.getAddress() << " - Heartbeat: " << vec[i].getheartbeat() << " - Timestamp: " << vec[i].gettimestamp() << "\n";
    }

    std::cout << "---------------------------------------------\n";
}

/**
 * FUNCTION NAME: cleanupMembershipTable
 *
 *  DESCRIPTION: Clean membership table after sending errors
 */
vector<MemberListEntry> MP1Node::cleanupMembershipTable(vector<MemberListEntry> vec)
{
    for (size_t i = 0; i < vec.size(); i++)
    {
        vec[i].setid(vec[i].getid() >> 16);
        vec[i].setport(vec[i].getport() >> 16);
        vec[i].setheartbeat(vec[i].getheartbeat() >> 16);
        vec[i].settimestamp(vec[i].gettimestamp() >> 16);

        if (vec[i].gettimestamp() > par->getcurrtime() || vec[i].gettimestamp() < 0 || vec[i].getid() <= 0 || vec[i].getheartbeat() < 0)
        {
            // cout<<"About to erase\n";
            vec.erase(vec.begin() + i);
            i--;
        }

        // memberNode->memberList.push_back(member);
    }
    return vec;
}

/**
 * FUNCTION NAME: compareAdjustMembershipTable
 *
 *  DESCRIPTION: See the values in the current membership table, compare to see if they are the same, do gossip comparison and if less than normal then adjust
 */
void MP1Node::compareAdjustMembershipTable(vector<MemberListEntry> vec)
{
    for (size_t i = 0; i < memberNode->memberList.size(); i++)
    {
        int id = memberNode->memberList[i].getid();
        int port = memberNode->memberList[i].getport();
        for (size_t j = 0; j < vec.size(); j++)
        {
            if (par->getcurrtime() - vec[j].gettimestamp() > 2 || vec[j].gettimestamp() < 0 || vec[j].getid() <= 0 || vec[j].getheartbeat() < 0 )
            {
                // cout<<"About to erase\n";
                vec.erase(vec.begin() + j);
                j--;
                continue;
            }
            // cout << "Equal Addresses found\n";
            if (vec[j].getid() == id && vec[j].getport() == port)
            {
                if (vec[j].gettimestamp() > memberNode->memberList[i].gettimestamp() && vec[j].getheartbeat() > memberNode->memberList[i].getheartbeat())
                {
                    // cout<<"Time stamp & hearbeat update\n";
                    memberNode->memberList[i].setheartbeat(vec[j].getheartbeat());
                    memberNode->memberList[i].settimestamp(vec[j].gettimestamp());
                }
            }
            else if (memberNode->memberList.size() < GROUPMAX)
            {
                memberNode->memberList.push_back(vec[j]);
                memberNode->memberList = removeDuplicateMembers(memberNode->memberList);
            }
        }
    }
}
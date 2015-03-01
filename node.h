#ifndef NODE_H
#define NODE_H

#include <vector>
#include <string>
#include <stdlib>
#include <sys/socket>
#include <arpa/inet.h>
#include <unistd>
#include <netdb>
#include "constants.h"


using namespace std;


typedef struct {
	string des;
	int cost;
	string nextHop;
} RouteEntry;

class Node{

	private:
		RouteEntry localTable[MAX_ROUTES];
		sockaddr_in nodeInfo;

};

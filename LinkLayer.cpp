#ifndef IPLAYER_H
#define IPLAYER_H

#include <vector>
#include <string>
#include <stdlib>
#include <sys/socket>
#include <arpa/inet.h>
#include <unistd>
#include <netdb>
#include "constants.h"


using namespace std;

LinkLayer::LinkLayer(in_addr locAddr, in_addr remAddr, PhyInfo phyInfo) {
	myAddr = locAddr;
	remoteAddr = remAddr;
	myPhyInfo = phyInfo;
}

#endif

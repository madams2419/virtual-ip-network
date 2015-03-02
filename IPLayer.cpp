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

IPLayer::IPLayer(PhyInfo pi, LinkLayer[] interface) {
	phyInfo = pi;
	myInterfaces = interfaces;
}

#endif

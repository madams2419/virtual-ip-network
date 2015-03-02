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

class LinkLayer {

	private:
		in_addr myAddr;
		in_addr remoteAddr;
		PhyInfo myPhyInfo;
		queue packetQueue; // not sure if queue is an c++ implementation

		void start();

	public:
		LinkLayer(in_addr locAddr, in_addr remAddr, PhyInfo phyInfo);
		bool hasData();
		byte[] getData();
		bool sendData();

};

#endif

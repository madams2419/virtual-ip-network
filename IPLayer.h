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

class IPLayer {

	private:
		Map routingTable;
		Map forwardingTable;
		LinkLayer[] myInterfaces;
		PhyInfo phyInfo;

		void start();

	public:
		IPLayer(PhyInfo pi, LinkLayer[] interfaces);
		bool hasData();
		byte[] getData();
		boolean sendData(byte[] data);
		string printInterfaces();
		string printRoutes();
};

#endif

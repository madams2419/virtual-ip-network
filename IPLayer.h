#ifndef IPLAYER_H
#define IPLAYER_H

#include <map>
#include <vector>
#include <queue>

#include "constants.h"
#include "LinkLayer.h"

#include "ip.h"
//#include <netinet/ip.h>

class IPLayer {

	private:
		map<u_int32_t, route_entry> routingTable;
		map<u_int32_t, int> fwdTable;
		vector<char*> myAddreses;
		queue<string> rcvQueue;
		LinkLayer* linkLayer;

		int getFwdInterface(u_int32_t daddr);
		void handleNewPacket(char* packet, int len);
		struct iphdr* parseHeader(char* packet);
		void decrementTTL(char* packet);
		void deliverLocal(char* packet);
		struct iphdr* genHeader(int dataLen, u_int32_t saddr, u_int32_t daddr);
		static void runThread(ipl_thread_pkg* pkg);

	public:
		IPLayer(LinkLayer* linkLayer);
		int send(char* data, int dataLen, char* destIP);
		int receive(char* buf, int bufLen);
		bool hasData();
		string getData();
		void runForwarding();
		void runRouting();
};

#endif

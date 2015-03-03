#ifndef LINKLAYER_H
#define LINKLAYER_H

#include <vector>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include "constants.h"

#define MAX_MSG_LENGTH (512)
#define MAX_BACK_LOG (5)
#define SEND_LENGTH (MAX_MSG_LENGTH*3)

using namespace std;

class LinkLayer {

	private:
		phy_info localPhy;
		vector<itf_info> itfs;
		struct addrinfo *localAI;

		int rcvSocket;
		vector<int> sendSockets;

		vector< vector<char> > packetQueue; // not sure if queue is an c++ implementation

		void start();
		int createSocket(phy_info phyInfo, struct addrinfo *ai, bool bindSock);

	public:
		LinkLayer(phy_info localPhy, vector<itf_info> itfs);
		int createListener(phy_info phyInfo);
		bool hasData();
		vector<char> getData();
		bool sendData(char* data, int dataLen, int itfNum);

};

LinkLayer::LinkLayer(phy_info localPhy, vector<itf_info> itfs) {
	this->localPhy = localPhy;
	this->itfs = itfs;

	rcvSocket = createSocket(localPhy, localAI, true);
}

/**
 * Sends dataLen bytes of data over the interface specified by itfNum
 */
bool LinkLayer::sendData(char* data, int dataLen, int itfNum) {
	int sendSocket, bytesSent;
	struct addrinfo *dstAI;

	sendSocket = createSocket(itfs[itfNum].rmtPhy, dstAI, false);

	if ((bytesSent = sendto(sendSocket, data, dataLen, 0, dstAI->ai_addr, dstAI->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

	freeaddrinfo(dstAI);

	printf("Sent %d bytes over interface %d\n", bytesSent, itfNum);
	close(sendSocket);

}

/**
 * Creates socket and populates &aiRet with socket address info
 */
int LinkLayer::createSocket(phy_info phyInfo, struct addrinfo *aiRet, bool bindSock) {
	int sockfd;
	struct addrinfo hints, *ai, *servinfo;
	int rv;

	// zero out address info hints structure
	memset(&hints, 0, sizeof hints);

	// populate address info hints
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_DGRAM; // UDP

	if ((rv = getaddrinfo(phyInfo.ipStr, phyInfo.portStr, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return -1;
	}

	// loop through getaddrinfo() results. connect (and bind if flagged) to first one possible
	for(ai = servinfo; ai != NULL; ai = ai->ai_next) {
		if ((sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bindSock) {
			if (bind(sockfd, ai->ai_addr, ai->ai_addrlen) == -1) {
				close(sockfd);
				perror("listener: bind");
				continue;
			}
		}

		break;
	}

	if (ai == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return -1;
	}

	// copy address info data
	memset(&aiRet, 0, sizeof aiRet);
	*aiRet = *ai;

	freeaddrinfo(servinfo);

	return sockfd;
}

/* This main just for testing */
int main(int argc, char ** argv) {
	phy_info locPhy, rmtPhy;
	itf_info itf;
	vector<itf_info> itfs;

	locPhy.ipAddr = inet_addr("127.0.0.1");
	rmtPhy.ipAddr = inet_addr("127.0.0.1");

	locPhy.port = atoi(argv[1]);
	locPhy.portStr = argv[1];
	rmtPhy.port = atoi(argv[2]);
	rmtPhy.portStr = argv[2];

	itf.locAddr = inet_addr("192.168.1.1");
	itf.rmtAddr = inet_addr("192.168.1.1");
	itf.rmtPhy = rmtPhy;

	itfs.push_back(itf);

	LinkLayer* l = new LinkLayer(locPhy, itfs);
}

#endif

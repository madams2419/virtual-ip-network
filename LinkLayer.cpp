#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include "LinkLayer.h"
#include "constants.h"
#include "LinkLayer.h"

using namespace std;

LinkLayer::LinkLayer(phy_info localPhy, vector<itf_info> itfs) {
	this->localPhy = localPhy;
	this->itfs = itfs;
	localAI = new struct addrinfo;
	rcvSocket = createSocket(localPhy, localAI, true);

	cout << "LinkLayer Config" << endl;
	cout << "==============================" << endl;
	cout << "Local phy address : " << localPhy.ipAddr << endl;
	cout << "Local phy port    : " << localPhy.port << endl;
	for(vector<itf_info>::size_type i = 0; i != itfs.size(); i++){
		cout << "---------------------------" << endl;
		cout << "Interface " << i << endl;
		cout << "---------------------------" << endl;
		cout << "Remote phy address : " << itfs[i].rmtPhy.ipAddr << endl;
		cout << "Remote phy port    : " << itfs[i].rmtPhy.port << endl;
		cout << "Local vip          : " << itfs[i].locAddr << endl;
		cout << "Remote vip         : " << itfs[i].rmtAddr << endl;
		cout << "MTU                : " << itfs[i].mtu << endl;
	}
	cout << "==============================" << endl;
}

/**
 * Returns the local IP address associated with the specified interface
 */
char* LinkLayer::getInterfaceAddr(int itf) {
	return itfs[itf].locAddr;
}

/**
 * Returns true if the specified address matches a local interface IP address
 */
bool LinkLayer::isLocalAddr(u_int32_t addr) {
	for(int i = 0; i < itfs.size(); i++) {
		u_int32_t locAddr = inet_addr(getInterfaceAddr(i));
		if (addr == locAddr) return true;
	}
	return false;
}

/**
 * Sends dataLen bytes of data over the interface specified by itfNum
 */
int LinkLayer::send(char* data, int dataLen, int itfNum) {
	int sendSocket, bytesSent;
	struct addrinfo *aiDest = new addrinfo;
	sendSocket = createSocket(itfs[itfNum].rmtPhy, aiDest, false);

	if ((bytesSent = sendto(sendSocket, data, dataLen, 0, aiDest->ai_addr, aiDest->ai_addrlen)) == -1) {
		perror("Send error:");
		return -1;
	}

	freeaddrinfo(aiDest);

	printf("Sent %d bytes over interface %d\n", bytesSent, itfNum);
	close(sendSocket);

	return bytesSent;
}

/**
 * Listen on UDP socket and copy data received to specified buffer
 * Return number of bytes received
 */
int LinkLayer::listen(char* buf, int bufLen) {
	int bytesRcvd;
	if ((bytesRcvd = recvfrom(rcvSocket, buf, bufLen, 0, NULL, NULL)) == -1) {
		perror("Receive error:");
		return -1;
	}
	printf("Recieved %d bytes", bytesRcvd);
	return bytesRcvd;
}

/**
 * Creates UDP socket and populates &aiRet with socket address info
 */
int LinkLayer::createSocket(phy_info phyInfo, struct addrinfo* aiRet, bool bindSock) {
	int sockfd, gai_ret;
	struct addrinfo aiHints, *aiList, *ai;

	// zero out address info hints structure
	memset(&aiHints, 0, sizeof aiHints);

	// populate address info hints
	aiHints.ai_family = AF_INET; // IPv4
	aiHints.ai_socktype = SOCK_DGRAM; // UDP

	if ((gai_ret = getaddrinfo(phyInfo.ipAddr, phyInfo.port, &aiHints, &aiList)) != 0) {
		perror("Get address info error:");
		return -1;
	}

	// loop through getaddrinfo() results. connect (and bind if flagged) to first one possible
	for(ai = aiList; ai != NULL; ai = ai->ai_next) {
		if ((sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) {
			perror("Socket creation error:");
			continue;
		}

		if (bindSock) {
			if (bind(sockfd, ai->ai_addr, ai->ai_addrlen) == -1) {
				close(sockfd);
				perror("Socking binding error:");
				continue;
			}
		}

		break;
	}

	if (ai == NULL) {
		printf("No valid address info structure found.\n");
		return -1;
	}

	// copy address info data
	*aiRet = *ai;

	// free address info list
	freeaddrinfo(aiList);

	return sockfd;
}

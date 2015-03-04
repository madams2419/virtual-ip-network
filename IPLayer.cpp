#include <iostream>
#include <vector>
#include <queue>
#include <map>
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
#include "ipsum.h"
#include "IPLayer.h"

#include "ip.h"
//#include <netinet/ip.h>

using namespace std;

typedef struct {
	IPLayer* ipl;
	string toRun;
} thread_pkg;

IPLayer::IPLayer(LinkLayer* link) {
	linkLayer = link;
	defaultItf = 0;

	//DEBUG
	popFwdTable();

	// create thread to handle forwarding tasks
	pthread_t* fwdWorker;
	thread_pkg* pkg = new thread_pkg;
	pkg->ipl = this;
	pkg->toRun = "forwarding";
	int err = pthread_create(fwdWorker, NULL, &runThread, (void*) pkg);
	if(err != 0) {
			perror("Threading error:");
	}

}

void *IPLayer::runThread(void* pkg) {
	thread_pkg* tp = (thread_pkg*) pkg;
	IPLayer* ipl = tp->ipl;
	string toRun = tp->toRun;
	ipl->runForwarding();
}

void IPLayer::runForwarding() {
	int rcvLen;
	char buf[MAX_MSG_LEN];

	while(1) {
		// get packet
		cout << "Worker thread listening for data." << endl;
		rcvLen = linkLayer->listen(buf, MAX_MSG_LEN);
		if (rcvLen < 0) {
			printf("IP Layer receive error.");
			continue;
		} else {
			printf("IP packet received.");
		}

		//TODO spawn new thread here
		handleNewPacket(buf, rcvLen);

		memset(buf, 0, sizeof(buf));
	}
}

void IPLayer::runRouting() {
}

void IPLayer::handleNewPacket(char* packet, int len) {
	int fwdItf;
	int checksum;
	struct iphdr* hdr;

	// parse header
	hdr = (struct iphdr*) packet; //TODO might want to deal with network ordering issues

	//DEBUG
	cout << "Header length: " << hdr->tot_len << endl;
	cout << "Received Length: " << len << endl;
	cout << "Message: " << packet << endl;
	cout << "Send message (hex): " << endl;
	for (int i = 0; i < len; i++) {
  		cout << hex << packet[i];
	}

	// check to make sure receive length equals header total length
	if (hdr->tot_len != len) {
		printf("Partial packet received, discarding.");
		return;
	}

	// verfiy checksum
	u_int16_t rcvCheck = hdr->check;
	hdr->check = 0;
	if (hdr->check != ip_sum(packet, sizeof(struct iphdr))) {
		printf("Invalid checksum, discarding packet.");
		return;
	}

	// decrement ttl or return if zero
	if (hdr->ttl == 0) {
		printf("TTL = 0, discarding packet.");
		return;
	} else { // decrement ttl and recalculate checksum
		hdr->ttl--;
		hdr->check = ip_sum(packet, sizeof(struct iphdr));
	}

	// forward or deliver locally
	if ((fwdItf = getFwdInterface(hdr->daddr)) == -1) {
		deliverLocal(packet);
	} else {
		linkLayer->send(packet, hdr->tot_len, fwdItf);
	}

}

struct iphdr* IPLayer::parseHeader(char* packet) {
	struct iphdr* hdr = (struct iphdr*) &packet;
	return hdr;
}

/**
 * Returns the next string in the receive buffer
 */
string IPLayer::getData() {
	string data = rcvQueue.front();
	rcvQueue.pop();
	return data;
}

/**
 * Return true if the IP layer has buffered data
 */
bool IPLayer::hasData() {
	return (rcvQueue.size() > 0);
}

/**
 * Copies packet data into receive queue and makes it available for retreival by the application layer
 */
void IPLayer::deliverLocal(char* packet) {
	struct iphdr* hdr = (struct iphdr*) &packet;

	// computer data length
	int dataLen = hdr->tot_len - (hdr->ihl * 4);

	// copy data into string
	string data (packet, HDR_SIZE, dataLen);

	//DEBUG
	cout << "Got data: " << data << endl;

	// add data buffer to data vector
	rcvQueue.push(data);
}

/**
 * Encapsulates data in IP header and sends via link layer
 */
int IPLayer::send(char* data, int dataLen, char* destIP) {
	int bytesSent, itfNum;
	u_int32_t daddr, saddr;
	struct iphdr* hdr;

	// initialize buffer to store new packet
	int packetLen = dataLen + sizeof(struct iphdr);
	char packet[packetLen];

	// convert destination ip in dots-and-number form to network order int form
	daddr = inet_addr(destIP);

	// get forwarding interface
	if ((itfNum = getFwdInterface(daddr)) < 0) {
		printf("Source address belongs to host. Aborting send.");
		return -1;
	}

	// get local IP address associated with interface in network order int form
	saddr = inet_addr(linkLayer->getInterfaceAddr(itfNum));

	// generate new IP header
	hdr = genHeader(dataLen, saddr, daddr);

	// copy header to packet buffer
	memcpy(&packet[0], hdr, sizeof(struct iphdr));

	// copy data to packet buffer
	memcpy(&packet[sizeof(struct iphdr)], data, dataLen);

	struct iphdr* test = (struct iphdr*) packet;
	cout << "Test header length: " << test->tot_len << endl;

	//DEBUG
	cout << "Send message: " << packet << endl;
	cout << "Send message (hex): " << endl;
	for (int i = 0; i < packetLen; i++) {
  		cout << hex << packet[i];
	}
	cout << endl;

	// send packet via link layer
	if((bytesSent = linkLayer->send(packet, packetLen, itfNum)) < 0) {
		printf("Sending error.");
		return -1;
	} else {
		printf("IP packet length = %d\n Sent %d bytes", packetLen, bytesSent);
	}

	return bytesSent;
}

/**
 * Returns pointer to newly populated IP header
 */
struct iphdr* IPLayer::genHeader(int dataLen, u_int32_t saddr, u_int32_t daddr) {
	struct iphdr* hdr = new struct iphdr;

	// pack header
	hdr->version = 4; // IP version 4
	hdr->ihl = 5; // no options
	hdr->tos = 0; // no TOS protocol
	hdr->tot_len = (hdr->ihl * 4) + dataLen; // header length (in bytes) + data length
	hdr->id = 0; // fragmentation not supported
	hdr->frag_off = 0; // fragmentation not supported
	hdr->ttl = MAX_TTL; // maximum TTL
	hdr->protocol = 143; // custom protocol (raw string data)
	hdr->check = 0; // checksum is zero for calculation
	hdr->saddr = saddr; // source address in network byte order
	hdr->daddr = daddr; // destination address in network byte order

	// calculate checksum
	hdr->check = ip_sum((char*) hdr, sizeof(struct iphdr));

	return hdr;
}

/**
 * Gets the interface number to use for forwarding the given IP address.
 * Returns -1 if the address matches one of the interface addresses.
 */
int IPLayer::getFwdInterface(u_int32_t daddr) {
	// check if network number is equal to the destination of any of the interfaces

	if(fwdTable.count(daddr) == 1) { // daddr is in fwd table; return itf value
		printf("Fwd table entry found. Forwarding on itf: %d", fwdTable[daddr]);
		return fwdTable[daddr];
	} else { // daddr is not in fwd table; return default itf
		return defaultItf;
	}

}

void IPLayer::popFwdTable() {
	fwdTable[inet_addr("10.116.89.157")] = 0;
	fwdTable[inet_addr("14.230.5.36")] = 1;
}

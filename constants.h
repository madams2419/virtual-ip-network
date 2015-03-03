#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MAX_ROUTES 128
#define MAX_TTL 120

#include <netinet/in.h>

typedef struct {
	in_addr_t ipAddr;
	char* ipStr;
	uint16_t port;
	char* portStr;
} phy_info;

typedef struct {
	in_addr_t locAddr;
	in_addr_t rmtAddr;
	phy_info rmtPhy;
} itf_info;


typedef struct {
	in_addr_t dest;
	in_addr_t nextHop;
	int cost;
	int TTL;
} route_entry;


#endif

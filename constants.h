#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MAX_ROUTES 128
#define MAX_TTL 120

#include <netinet/in.h>
#include <string>
#include "IPLayer.h"

typedef struct {
	char* ipAddr;
	char* port;
} phy_info;

typedef struct {
	char* locAddr;
	char* rmtAddr;
	phy_info rmtPhy;
} itf_info;

typedef struct {
	char* dest;
	char* nextHop;
	int cost;
	int TTL;
} route_entry;

typedef struct {
	IPLayer* ipl;
	string toRun;
} ipl_thread_pkg;

#endif

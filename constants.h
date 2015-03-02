#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MAX_ROUTES 128
#define MAX_TTL 120

typedef struct {
	in_addr ip;
	int port;
} PhyInfo;

typedef struct {
	in_addr dest;
	in_addr nextHop;
	int cost;
	int TTL;
} RouteEntry;


#endif

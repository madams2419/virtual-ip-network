struct phySocket {
	sockaddr_in ip;
	int port;
};

struct routerPort {
	phySocket ps;
	sockaddr_in vipLoc;
	sockaddr_in vipRem;
};

phySocket parsePhySocket(string config);

sockaddr_in stringToIP(string sip);

#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "AppLayer.h"
#include "LinkLayer.h"
#include "IPLayer.h"

using namespace std;

const string DEFAULT_IP = "127.0.0.1";

int main (int argc, char** argv){

	ifstream myReader;
	char* fileName = argv[1];
	myReader.open(fileName);
	cout << "The file name is " <<  fileName << endl;
	string line = "";
	string fileInfo[MAX_HOST];
	int count = 0;
	while(getline(myReader,line)) {
		char *pch;
		char *linecopy = new char[line.length() + 1];
		strcpy(linecopy,line.c_str());
		pch = strtok(linecopy, ": ");
		while(pch != NULL) {
			string str = pch;
			fileInfo[count] = str;
//			cout << pch << endl;
			pch = strtok(NULL, " : ");
//			cout << "The string info " << count << " is " << fileInfo[count] << endl; 
			count++;
		}
	}

	vector<itf_info> nodeItfs;
	for(int i = 2; i < count;){
		itf_info newItf;
		phy_info newPhy;
		newPhy.ipAddr = const_cast<char* >(fileInfo[i].c_str());
		i++;
		newPhy.port = const_cast<char* >(fileInfo[i].c_str());
		i++;
		newItf.rmtPhy = newPhy;
		newItf.locAddr = const_cast<char* >(fileInfo[i].c_str());
		i++;
		newItf.rmtAddr = const_cast<char* >(fileInfo[i].c_str());
		i++;
		newItf.mtu = DEFAULT_MTU;
		nodeItfs.push_back(newItf);
	}

	phy_info myPhyInfo;
	myPhyInfo.ipAddr = const_cast<char* >(fileInfo[0].c_str());
	myPhyInfo.port = const_cast<char* >(fileInfo[1].c_str());

	LinkLayer* linkLayer = new LinkLayer(myPhyInfo, nodeItfs);
	IPLayer* ipLayer = new IPLayer(linkLayer);
	AppLayer* myApp = new AppLayer(ipLayer);

	string input = "";
	while(1){
		getline(cin, input);
		myApp->runningApp(input);
	}

	return 0;
}

#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "AppLayer.h"


using namespace std;

const string DEFAULT_IP = "127.0.0.1";

int main (int argc, char** argv){

	ifstream myReader;
	char* fileName = argv[1];
	myReader.open(fileName);
	cout << "The file name is " <<  fileName << endl;
	string line = "";
	string fileInfo[128];
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
			cout << "The string info " << count << " is " << fileInfo[count] << endl; 
			count++;
		}
	}

	if(fileInfo[0].compare("localhost") == 0){
		fileInfo[0] = DEFAULT_IP;
	}

	for(int i = 2; i < count; i = i+4){
		if(fileInfo[i].compare("localhost") == 0){
			fileInfo[i] = DEFAULT_IP;
		}
	}

	phy_info myPhyInfo;
	myPhyInfo.ipAddr = const_cast<char* >(fileInfo[0].c_str());
	myPhyInfo.port = const_cast<char* >(fileInfo[1].c_str());
	string input = "";
	AppLayer myApp;
	while(1){
		cin >> input;
		myApp.runningApp(input);
	}

	return 0;
}
/*
IPLayer createIPLayer(string config) {
	char *pch;
	char *linecopy = new char[config.length() + 1];
	strcpy(linecopy, config.c_str());
	pch = strtok(linecopy, ": ");

	while(pch != NULL) {
		newPS.ip = inet_addr(pch);
		pch = strtok(NULL, " : ");
	}

	return 0;
}
*/

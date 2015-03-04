#include <vector>
#include <cstring>
#include <iostream>
#include "constants.h"
#include "AppLayer.h"

using namespace std;

AppLayer::AppLayer() {
	this->availDes = new string[MAX_HOST];
	this->desCount = 0;
}

string* AppLayer::getAvailDes(){
	return this->availDes;
}

void AppLayer::addDes(const string& des){
	this->availDes[desCount] = des;
}

int AppLayer::getDesCount(){
	return this->desCount;
}

void AppLayer::increDesCount(){
	this->desCount++;
}

void AppLayer::sendData(string instruction[]){
	cout << "The available destinations are ";
	for(int i = 0; i < desCount; i++){
		cout << availDes[i] << " ";
	}
	cout << endl;
	cout << "The destination is " << instruction[1] << " and the message is " << instruction[2] <<endl;
}


void AppLayer::runningApp(const string& command){
	cout << "Running AppLayer" << endl;
	string toDo[128];
	int count = 0;
	char* parse;
	char* line = new char[command.length()+1];
	strcpy(line, command.c_str());
	parse = strtok(line, "  ");
	cout << "The received message is: ";

	while(parse != NULL){
		string str = parse;
		toDo[count] = str;
		count++;
		cout << str << " ";
		parse = strtok(NULL, "  ");
	}

	string commandType = toDo[0];

	cout << endl;
	if(commandType.compare("send") == 0){
		cout << "The command is " << commandType << endl;
		sendData(toDo);
	} else if (commandType.compare("ipconfig") == 0) {
		cout << "The command is " << commandType << endl;
	} else if (commandType.compare("routes") == 0) {
		cout << "The command is " << commandType << endl;
	} else if (commandType.compare("up") == 0) {
		cout << "The command is " << commandType << endl;
	} else if (commandType.compare("down") == 0) {
		cout << "The command is " << commandType << endl;
	} else {
		cout << "The command is " << commandType << " which cannot be recognized. Please re-enter" << endl;
	}
}

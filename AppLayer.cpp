#include <vector>
#include <string>
#include <iostream>
#include "constants.h"
#include "AppLayer.h"

using namespace std;

AppLayer::AppLayer() {

}


void AppLayer::runningApp(const string& command){
	if(command.compare("send") == 0){
		cout << "The command is " << command << endl;
	} else if (command.compare("ipconfig") == 0) {
		cout << "The command is " << command << endl;
	} else if (command.compare("routes") == 0) {
		cout << "The command is " << command << endl;
	} else if (command.compare("up") == 0) {
		cout << "The command is " << command << endl;
	} else if (command.compare("down") == 0) {
		cout << "The command is " << command << endl;
	} else {
		cout << "The command cannot be recognized. Please re-enter" << endl;
	}
}

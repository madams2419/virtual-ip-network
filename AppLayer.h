#ifndef APPLAYER_H
#define APPLAYER_H

#include <vector>
#include <string>
#include "constants.h"

using namespace std;

class AppLayer {

	private:
		void start();

	public:
		AppLayer();
		void runningApp(const string& command);
};

#endif

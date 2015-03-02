#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;


int main (int argc, char** argv){
	ifstream myReader;
	char* fileName = argv[1];
	myReader.open(fileName);
	cout << fileName << endl;
	string line = "";
	while(getline(myReader,line)){
		cout << line << endl;

	}

	return 0;






}

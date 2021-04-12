#include <iostream>
#include <fstream>
#include <queue>

using namespace std;

int main(int argc, char const *argv[]) {

string inputFile = argv[1];                                             
string outputFile = argv[2]; 
ifstream in;
ofstream out;
in.open(inputFile);
out.open(outputFile);

string line;

while (in.peek() != EOF) {
	getline(in, line);
	

}
return 0; }
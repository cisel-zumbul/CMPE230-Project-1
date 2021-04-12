#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <cctype>
#include <algorithm>
#include <unordered_set>
using namespace std;


string findFirstWord(string line){
	string str = "";
	bool didStart = false;
	for(int i= 0; i < line.length(); i++){
		if((line[i] >= 48 && line[i] <= 57) || (line[i] >=65 && line[i] <=90) || (line[i]>=97 && line[i]<=122)){
			didStart = true;
			str = str + line[i];
		}
		else if(!didStart){
			continue;
		}
		else{
			break;
		}
	}
	return str;
}

int main(int argc, char const *argv[]) {

string inputFile = argv[1];                                             
string outputFile = argv[2]; 
ifstream in;
ofstream out;
in.open(inputFile);
out.open(outputFile);
unordered_set<string> keyWords = {"while", "if" , "choose", "print"};
string line;

while (in.peek() != EOF) { //Bu javadaki hasNextLine() fonksiyonunun yaptığını yapıyo
	getline(in, line);
	string first = findFirstWord(line);
	if( first.length() == 0 || (first[0] >= 48 && first[0] <= 57)){ //ilk kelimenin ilk karakteri sayıyla başlıyosa
		continue;
	}
	if(keyWords.find(first) == keyWords.end()){ // Kelime keyword değilse ve sayıyla başlamıyosa buraya, Assignment olcak
		char ch[line.length()+1];
		strcpy(ch, line.c_str());
		const char* lim = "=";
		string token = strtok(ch, lim); 
		token.erase(remove_if(token.begin(), token.end(), ::isspace), token.end()); //sondaki whitescapeleri silme kodu 
		if(first == token) {
			cout << token << endl ;
			//Check RHS
		}
		else{
			//LHS error
		}
	}
	else{
		//while if choose kısmı
	}
}
return 0; 
}
#include <iostream>
#include <fstream>
#include <queue>

using namespace std;


string findFirstWord(string line){
	string str = "";
	for(int i= 0; i < line.length(); i++){
		if((line[i] >= 48 && line[i] <= 57) || (line[i] >=65 && line[i] <=90) || (line[i]>=97 && line[i]<=122)){
			str = str + line[i];
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
nordered_set<string> keyWords = {"while", "if" , "choose", "print"};
string line;

while (in.peek() != EOF) { //Bu javadaki hasNextLine() fonksiyonunun yaptığını yapıyo
	getline(in, line);
	string first = findFirstWord(line);
	if( line.length() == 0 || (first[0] >= 48 && first[0] <= 57)){ //ilk kelimenin ilk karakteri sayıyla başlıyosa
		continue;
	}
	if(keyWords.find(first) == keyWords.end()){

	}
	else{
		//while if choose kısmı
	}

}
return 0; }
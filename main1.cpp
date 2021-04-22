#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <stack>
#include <cctype>
#include <algorithm>
#include <unordered_set>
using namespace std;

void createVar(string name, ofstream& out)
{
	out << "\t%" << name << " = alloca i32\n";
	return;
}

void store(string thing, string var, ofstream& out)
{
	out << "\tstore i32 " << thing << ", i32* %" << var << "\n";
	return;
}

void condition(string last, string before_last, ofstream& out){
	out << "\t" << last << " = icmp ne i32 " << before_last <<  ", 0" << endl;
}

void goBody(string where, string target, ofstream& out){
	out << "\tbr i1 " << target << ", label %" << where << "body, label %" << where << "end" << endl;
}
string getTemp()
{
	static int i = 0;
	return "%t" + to_string(i++);
}

string getTemp(string var, ofstream& out)
{
	string temp = getTemp();
	out << "\t" << temp << " = load i32* %" << var << "\n"; 
	return temp;
}

bool isAlphaNumeric(char a)
{
	return ('a'<=a and a<='z')or('A'<=a and a<='Z')or('0'<=a and a<='9');
}

bool isValidVar(string var)
{
	unordered_set<string> keyWords = {"while", "if" , "choose", "print"};
	if(keyWords.find(var) == keyWords.end())
	{
		if(('a' < var[0] and var[0] < 'z') or ('A' < var[0] and var[0] < 'Z'))
		{
			return true;
		} else
		{
			cout << "Syntax error: Variables start with letters";
			return false;
		}
	} else
	{
		cout << "Syntax error: Unexpected keyword";
		return false;
	}
}

bool isNumber(string var){
	for( int i = 0; i < var.length(); i++){
		if(var[i] >= '0' && var[i] <= '9'){
			continue;
		}
		else
			return false;
	}
	return true;
}

struct lineReader{
	const char white_space[2] = {' ','	'};
	string line_text;
	int line_length = 0;
	int cursor = 0;
	int cursor_size = 0;

	lineReader(string line){
		this->line_text = line;
		this->line_length = line.size();
		findNext();
	}

	string peek()
	{
		return line_text.substr(cursor,cursor_size);
	}
	string get()
	{
		string out = line_text.substr(cursor,cursor_size);
		findNext();
		return out;
	}
	bool has()
	{
		return cursor != line_length;
	}


private:
	void findNext()
	{
		cursor += cursor_size;
		cursor_size = 1;
		for(; cursor < line_length; cursor++)
		{
			if(!isWhiteSpace(line_text[cursor]))
			{
				break;
			}
		}		
		if(isAlphaNumeric(line_text[cursor]))
		{
			for(int i = cursor+1; i < line_length; i++)
			{
				if(isWhiteSpace(line_text[i]) or !isAlphaNumeric(line_text[i]))
				{
					break;
				}
				cursor_size++;
			}
		}
	}
	bool isWhiteSpace(char a)
	{
		return find(begin(white_space),end(white_space),a) != end(white_space);
	}


};

string operatinator(string operat, string op1, string op2, ofstream& out)
{
	string temp = getTemp();
	out << "\t" << temp << " = "<< operat <<" i32 " << op1 << ", " << op2 << "\n";
	return temp;
}

string expressionParser(queue<string> expr, ofstream& out)
{	
	const char operators[6] = {'+','-','*','/'};
	lineReader reader(expr);
	queue<string> out_queue;
	stack<char> op_stack;
	while(reader.has())
	{
		string token = reader.get();
		if(isAlphaNumeric(token[0]))
		{			
			out_queue.push(token);

		} else
		{
			if(find(begin(operators),end(operators),token[0]) != end(operators))
			{
				if(token[0] == '*' or token[0] == '/')
				{
					while(!op_stack.empty() and (op_stack.top() == '*' or op_stack.top() == '/'))
					{
						out_queue.push(string(1,op_stack.top()));						
						op_stack.pop();
					}
				}
				if(token[0] == '+' or token[0] == '-')
				{
					while(!op_stack.empty() and op_stack.top() != '(')
					{
						out_queue.push(string(1,op_stack.top()));						
						op_stack.pop();
					}
				}
				op_stack.push(token[0]);
			}else if(token[0] == '(')
			{				op_stack.push(token[0]);
			}else if(token[0] == ')')
			{
				while(op_stack.top() != '(')
				{
					out_queue.push(string(1,op_stack.top()));						
					op_stack.pop();
					if(op_stack.empty())
					{
						cout << "Syntax error: missing paranthesis '('";
						return "ERROR";
					}
				}
				op_stack.pop();
			}
			else
			{
				cout << "Syntax error: nonsensical character";
				return "ERROR";
			}
		}
	}
	while(!op_stack.empty())
	{
		out_queue.push(string(1,op_stack.top()));
		op_stack.pop();
	}


	stack<string> operand_stack;
	while(!out_queue.empty())
	{
		string subj = out_queue.front();
		out_queue.pop();	
			
		if(isAlphaNumeric(subj[0]) or subj[0] == '%')
		{
			if(subj[0] != '%' and !isNumber(subj))
			{
				operand_stack.push(getTemp(subj, out));
			} else
			{
				operand_stack.push(subj);
			}
		} else if (find(begin(operators),end(operators),subj[0]) != end(operators))
		{
			string op1;
			string op2;
			string* ops[2] = {&op1, &op2};
			for( int i = 0; i < 2; i++)
			{
				if(operand_stack.empty())
				{
					cout << "Syntax error: not enough operands";
					return "ERROR";
				} else
				{
					string curop = operand_stack.top();
					operand_stack.pop();					
					*ops[i] = curop;					
				}				
			}
			if(subj[0] == '+')
			{
				operand_stack.push(operatinator("add",op1,op2,out));
			} else if(subj[0] == '-')
			{
				operand_stack.push(operatinator("sub",op2,op1,out));
			}
			 else if(subj[0] == '*')
			{
				operand_stack.push(operatinator("mul",op1,op2,out));
			} else if(subj[0] == '/')
			{
				operand_stack.push(operatinator("udiv",op2,op1,out));
			}
		}	
	}
	return operand_stack.top();
}

int main(int argc, char const *argv[]) {
	const string boilerplate = "; ModuleID = 'mylang2ir'\ndeclare i32 @printf(i8*, ...)\n@print.str = constant [4 x i8] c\"%d\\0A\\00\"\n\ndefine i32 @main() {\n";

	string inputFile = argv[1];                                             
	string outputFile = argv[2]; 
	ifstream in;
	ofstream out;
	in.open(inputFile);
	out.open(outputFile);
	unordered_set<string> keyWords = {"while", "if" , "choose", "print"};
	string line;

	out << boilerplate;

	while (in.peek() != EOF) { //Bu javadaki hasNextLine() fonksiyonunun yaptığını yapıyo	
		getline(in, line);
		lineReader reader(line);
		string first_word = reader.get();
		if(first_word.length() == 0 || (first_word[0] >= 48 && first_word[0] <= 57)){ //ilk kelimenin ilk karakteri sayıyla başlıyosa
			// Syntax error
			return 0;
		}
		if(first_word == "}"){
			if(!conditionQ.empty()){
				out << "\tbr label" << conditionQ.top().substr(0,2) << "cond";
			}
		}
		if(keyWords.find(first_word) == keyWords.end()){ // Kelime keyword değilse ve sayıyla başlamıyosa buraya, Assignment olcak			
			if(reader.peek() == "=") {
				reader.get();
				createVar(first_word, out);
				//Shunting-Yard	y
				store(expressionParser(line, out), first_word, out);
				out << "\n";
			}
		}
		else{
			if(first_word == "while"){
				cout << "hello" << endl;
				queue<string> strQ;
				if(reader.peek() == "("){
					while(reader.peek() != "{") {
						token = reader.get();
						strQ.push(token);
						if(!reader.has()) {
							//Error
							return 0;
						}
					}
					if(reader.has()) {
						//Error
						return 0;
					}
					else {
						out << "br label %whcond\n\n";
						out << "whcond:\n" ;
						string before_last = expressionParser(strQ, out);
						string last = getTemp();
						condition(last, before_last, out);
						goBody("wh", last, out);
						out << "whbody:\n" ;
					}
					else{
						//error
						return 0;
					}	
				}
				else {
					//error
					return 0 ;
				}
			}
			else if (first_word == "if"){
				queue<string> strQ;
				if(reader.peek() == "("){
					while(reader.peek() != "{") {
						token = reader.get();
						strQ.push(token);
						if(!reader.has()) {
							//Error
							return 0;
						}
					}
					if(reader.has()) {
						//Error
						return 0;
					}
					else {
						out <<"br label %ifcond\n\n";
						out <<"ifcond:\n";
						string before_last = expressionParser(strQ, out);
						string last = getTemp();
						condition(last, before_last, out);
						goBody("if", last, out);
						out << "ifbody:\n";
					}
			}
			else if ( first_word == "print"){

			}
			else if(first_word == "choose"){

			}
			else {
					//error
			}
		}

	}
}
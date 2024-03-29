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


void store(string thing, string var, ofstream &out) { //This prints the "store" template.
    out << "\tstore i32 " << thing << ", i32* %" << var << "\n";
    return;
}

void condition(string last, string before_last, ofstream &out) { //This creates the line with comparison.
    out << "\t" << last << " = icmp ne i32 " << before_last <<  ", 0" << endl;
}

void goBody(string where, string target, ofstream &out) { //This prints branching inside the condition body.
    out << "\tbr i1 " << target << ", label %" << where << "body, label %" << where << "end\n\n";
}

string getTemp() { //Our temporary variable creater function, directly returns temporary variable.
    static int i = 0;
    return "%temp_var_" + to_string(i++);
}

string getTemp(string var, ofstream &out) { //This function gets variable and loads some value to it.
    string temp = getTemp();
    out << "\t" << temp << " = load i32* %" << var << "\n";
    return temp;
}

bool isAlphaNumeric(char a) { //Checks if a variable is alphanumeric or not.
    return ('a' <= a and a <= 'z') or ('A' <= a and a <= 'Z') or ('0' <= a and a <= '9');
}

bool isValidVar(string var) { //Checks if a variable is valid or not.
    unordered_set<string> keyWords = {"while", "if", "choose", "print"}; //Checks if the word is keyword
    if(keyWords.find(var) == keyWords.end()) {
        if(('a' <= var[0] and var[0] <= 'z') or ('A' <= var[0] and var[0] <= 'Z')) { //Checks if the first char of the word is a number or not.
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool isNumber(string var) { //Checks if a string is a number or not.
    for( int i = 0; i < var.length(); i++) {
        if(var[i] >= '0' && var[i] <= '9') {
            continue;
        } else
            return false;
    }
    return true;
}

struct lineReader {//Line reader is a tokenizer that has a stream like interface
    const char white_space[2] = {' ', '	'};
    string line_text;
    int line_length = 0;
    int cursor = 0;
    int cursor_size = 0;

    lineReader(string line) {// Constructer that takes the string to be read as the parameter
        this->line_text = line;
        this->line_length = line.size();
        findNext();
    }

    string peek() {//Returns what the cursor is pointing to
        return line_text.substr(cursor, cursor_size);
    }
    string get() {//Returns what the cursor is pointing to, then adances the cursor
        if(has()) {
            string out = line_text.substr(cursor, cursor_size);
            findNext();
            return out;
        } else {
            return "";
        }
    }
    bool has() {//Checks if the cursor is at the end of a string
        return cursor != line_length;
    }


private:
    void findNext() {//	Private funtion to advance the cursor, it groups alphanumeric characters 
    				//	into words and also stops reading upon encountering the comment character.
        cursor += cursor_size;
        cursor_size = 0;
        for(; cursor < line_length; cursor++) {
            if(!isWhiteSpace(line_text[cursor])) {
                break;
            }
        }
        if(isAlphaNumeric(line_text[cursor])) {
            cursor_size++;
            for(int i = cursor + 1; i < line_length; i++) {
                if(isWhiteSpace(line_text[i]) or !isAlphaNumeric(line_text[i])) {
                    break;
                }
                cursor_size++;
            }
        } else {
            cursor_size = 1;
        }
        if(line_text[cursor] == '#') {
            cursor = line_length;
        }
    }
    bool isWhiteSpace(char a) {// Wrapper for the isspace function, in case we need to change it
        return isspace(a);
    }


};

struct variableHandler {// A structure that deals with declaring, checking and storing variables
    static vector<string> variables;

    static string initialize(string id) {// Checks if the variiable is valid, then stores the id in a vector
        if(exists(id)) {
           
            return "ERROR";
        } else {
            if(isValidVar(id)) {
                variables.push_back(id);
                return "success";
            } else {
                return "ERROR";
            }
        }
    }

    static bool exists(string id) {// For checking if a variable is already declared
        return find(variables.begin(), variables.end(), id) != variables.end();
    }

    static string getInits() {// Returns all the allocations in llvm code
        string out;
        for(string id : variables) {
            out += "\t%" + id + " = alloca i32\n";
        }
        out += "\n";
        for(string id : variables) {
            out += "\tstore i32 0, i32* %" + id + "\n";
        }
        return out;
    }
};

vector<string> variableHandler::variables; // Initialization of the static vector

string expressionParser(queue<string> &expr, ofstream &out); //Forward declaration due to cross referencing functions

string choose (queue<string> expQ, ofstream &out) { //This function implements the choose function in mylang, taking expression as a string queue and outputs the necessary result.
    if(!expQ.empty()){ //Checks if there is something in the expression queue or not.
        string exp1, exp2, exp3, exp4;
        if(expQ.front() == "(") { //Checks the format of the choose function
        expQ.pop();
        int prs = 0; //This counts the parantheses to find the pair of first "(" which is the outermost parenthese.
        queue<string> strQ1;
        if(!expQ.empty()){ //This part is for extracting 1st expression
            while(expQ.front() != "," || prs != 0) {  //This while condition stops when it sees "," and this "," is not inside the another choose function. This is a way to determine boundaries of 1st expression.
                if(expQ.front() == "(") {
                    prs++;
                } else if(expQ.front() == ")") {
                    prs--;
                }
                strQ1.push(expQ.front());
                expQ.pop();
                if(expQ.empty()) {
                    return "ERROR";
                }
            }
            expQ.pop();
            exp1 = expressionParser(strQ1, out); //Sends 1st expression to the parser function and stores result in exp1.
            if(exp1 == "ERROR")
                return exp1;
        }    
        else{
            return "ERROR";
        }
            
        prs = 0; //This counts the parantheses to find the pair of first "(" which is the outermost parenthese.
        queue<string> strQ2;
        if(!expQ.empty()){ //This part is for extracting 2nd expression
            while(expQ.front() != "," || prs != 0) {  //This while condition stops when it sees "," and this "," is not inside the another choose function. This is a way to determine boundaries of 2nd expression.
                if(expQ.front() == "(") {
                    prs++;
                } else if(expQ.front() == ")") {
                    prs--;
                }
                strQ2.push(expQ.front());
                expQ.pop();
                if(expQ.empty()) {
                    return "ERROR";
                }
            }
            expQ.pop();
            exp2 = expressionParser(strQ2, out); //Sends 2nd expression to the parser function. Stores its result in exp2.
            if(exp2 == "ERROR")
                return exp2;
        }
        else{
            return "ERROR";
        }
            
        prs = 0; //This counts the parantheses to find the pair of first "(" which is the outermost parenthese.
        queue<string> strQ3;
        if(!expQ.empty()){ //This part is for extracting 3rd expression
            while(expQ.front() != "," || prs != 0) { //This while condition stops when it sees "," and this "," is not inside the another choose function. This is a way to determine boundaries of 3rd expression.
            if(expQ.front() == "(") {
                prs++;
            } else if(expQ.front() == ")") {
                prs--;
            }
            strQ3.push(expQ.front());
            expQ.pop();
            if(expQ.empty()) {
                return "ERROR";
            }
            }
            expQ.pop();
            exp3 = expressionParser(strQ3, out); //Sends 3rd expression to the parser function. Stores its result in exp3.
            if(exp3 == "ERROR")
                return exp3;
        }
        else{
            return "ERROR";
        }

        prs = 0;   //This counts the parantheses to find the pair of first "(" which is the outermost parenthese.
        queue<string> strQ4;
        if(!expQ.empty()){   //This part is for extracting 4th expression
            while(expQ.front() != ")" || prs != 0) { //This while stops when it sees ")" and this ")" is the last ")" in the exprQ.
            if(expQ.front() == "(") {
                prs++;
            } else if(expQ.front() == ")") {
                prs--;
            }
            strQ4.push(expQ.front());
            expQ.pop();
            if(expQ.empty()) {
                return "ERROR";
            }
            }
            expQ.pop();
            exp4 = expressionParser(strQ4, out); //Sends 4th exression to the parser function and stores its result on exp4.
            if(exp4 == "ERROR")
                return exp4;
        }
         else{
            return "ERROR";
        }
       
        if(!expQ.empty()){ //After extracting all necessary expressions, checks if there is left or not.
            return "ERROR";
        }
      
        //If function can make it to here, this means everything is fine , syntax is true, no error.
        string first = getTemp(); 
        string second = getTemp();
        string third = getTemp();
        string last = getTemp();
        out << "\t" << first << " = icmp sgt i32 " << exp1 << ", 0\n"; //Checks whether the first expression is greater than 0 or not
        out << "\t" << second << " = icmp eq i32 " << exp1 << ", 0\n"; //Checks whether the first expression is equal to 0 or not
        out << "\t" << third << " = select i1 " << first << ", i32 " << exp3 << ", i32 " << exp4 << "\n"; //If first is true then third will be 3rd exp, if not it will ve 4th exp
        out << "\t" << last << " = select i1 " << second << ", i32 " << exp2 << ", i32 " << third << "\n"; //If last is true then last will be 2nd exp, if not it will be the value of the third.
        return last;
    } else {
        return "ERROR";
        }
    }
    else{
        return "ERROR";
    }
    
}

string expressionParser(queue<string> &expr, ofstream &out) {// Function for dealing with expressions, it implements the Shunting-Yard algorithm
    const char operators[6] = {'+', '-', '*', '/'};
    queue<string> out_queue;
    stack<char> op_stack;
    bool operatortime = false;// This variable is for adding infix syntax checking to the algorithm
    while(!expr.empty()) {// This part reads the tokenized expression and turns it into a postfix queue
        string token = expr.front();
        expr.pop();
        if(isNumber(token) or isValidVar(token)) { 
            if(operatortime) {
                return "ERROR";
            }
            operatortime = true;
            out_queue.push(token);
        } else {
            if(find(begin(operators), end(operators), token[0]) != end(operators)) {
                if(!operatortime) {
                    return "ERROR";
                }
                operatortime = false;
                if(token[0] == '*' or token[0] == '/') {
                    while(!op_stack.empty() and (op_stack.top() == '*' or op_stack.top() == '/')) {
                        out_queue.push(string(1, op_stack.top()));
                        op_stack.pop();
                    }
                }
                if(token[0] == '+' or token[0] == '-') {
                    while(!op_stack.empty() and op_stack.top() != '(') {
                        out_queue.push(string(1, op_stack.top()));
                        op_stack.pop();
                    }
                }
                op_stack.push(token[0]);
            } else if(token[0] == '(') {
                if(operatortime) {
                    return "ERROR";
                }
                op_stack.push(token[0]);
            } else if(token[0] == ')') {
                if(!operatortime) {
                    return "ERROR";
                }
                operatortime = true;
                if(op_stack.empty()) {
                    return "ERROR";
                }
                while(op_stack.top() != '(') {

                    out_queue.push(string(1, op_stack.top()));
                    op_stack.pop();
                    if(op_stack.empty()) {
                        return "ERROR";
                    }

                }
                op_stack.pop();
            } else if(token == "choose") { //The choose functionality is implemented here

                if(operatortime) {
                    return "ERROR";
                }
                operatortime = true;
                int para = 0;
                queue<string> choose_q;

                while(!expr.empty()) { // the expression is extracted
                    string top = expr.front();

                    expr.pop();
                    choose_q.push(top);
                    if(top[0] == '(') {
                        para++;
                    } else if(top[0] == ')') {
                        para--;
                        if(para == 0) {
                            break;
                        }
                    }
                }
                if(para != 0) {
                    return "ERROR";
                }
                string choose_ret = choose(choose_q, out);
                if(choose_ret == "ERROR")
                    return "ERROR";
                out_queue.push(choose_ret);

            } else {
                return "ERROR";
            }
        }
    }
    while(!op_stack.empty()) {
        out_queue.push(string(1, op_stack.top()));
        op_stack.pop();
    }

    //This part turns the postfix queue into the llvm code
    stack<string> operand_stack;
    while(!out_queue.empty()) {
        string subj = out_queue.front();
        out_queue.pop();

        if(isAlphaNumeric(subj[0]) or subj[0] == '%') {
            if(subj[0] != '%' and !isNumber(subj)) { // Variables firsgt mentioned in the expresion are initialized here
                if(!variableHandler::exists(subj)) { 
                    if(variableHandler::initialize(subj) == "ERROR") {
                        return "ERROR";
                    }
                }
                operand_stack.push(getTemp(subj, out));
            } else {
                operand_stack.push(subj);
            }
        } else if (find(begin(operators), end(operators), subj[0]) != end(operators)) {
            string op1;
            string op2;
            string *ops[2] = {&op2, &op1};
            for( int i = 0; i < 2; i++) {
                if(operand_stack.empty()) {
                    return "ERROR";
                } else {
                    string curop = operand_stack.top();
                    operand_stack.pop();
                    *ops[i] = curop;
                }
            }
            string operat;
            string temp = getTemp();
            if(subj[0] == '+') {
                operat = "add";
            } else if(subj[0] == '-') {
                operat = "sub";
            } else if(subj[0] == '*') {
                operat = "mul";
            } else if(subj[0] == '/') {
                operat = "sdiv";
            }

            out << "\t" << temp << " = " << operat << " i32 " << op1 << ", " << op2 << "\n"; // the llvm code is constructed
            operand_stack.push(temp);
        } else if(subj[0] == '(') {
            return "ERROR";
        } else {
            cout << "this shouldn't happen :/\n"; // It shouldn't
            return "ERROR";
        }
    }

    return operand_stack.top(); //Finally returns the temp variable that the result is stored in
}

string expressionParser(lineReader &expr, ofstream &out) { // Alternative call to the expresion parser
    queue<string> strQ;
    while(expr.has()) {
        strQ.push(expr.get());
    }
    return expressionParser(strQ, out);
}



int main(int argc, char const *argv[]) {

    const string boilerplate = "; ModuleID = 'mylang2ir'\ndeclare i32 @printf(i8*, ...)\n@print.str = constant [4 x i8] c\"%d\\0A\\00\"\n\ndefine i32 @main() {\n";

    string inputFile = argv[1];
    string outputFile = inputFile.substr(0, inputFile.find_last_of('.')) + ".ll"; // The name of the output file is constructed
    ifstream in;
    ofstream out_final;
    ofstream out;
    in.open(inputFile);

    out_final.open(outputFile);
    out.open(".intermediate");// An hidden intermediate file is opened
    bool hasError = false;

    unordered_set<string> keyWords = {"while", "if", "choose", "print"};
    string line;
    stack<string> conditionStc;
    int count = -1;
    int cdc = 0 ;
    while (!hasError and in.peek() != EOF) { //	This loop reads the file line by line, as long as it 
    										//	isn't empty or an error has been encountered
        count++;
        getline(in, line);
        lineReader reader(line); // The line is used to create a line reader.
        string first_word = reader.get();// Then the first word is stored here to determine the purpose of the line.
        if(first_word.length() == 0 || first_word == "#") //If it is empty line or comment line it continue.
            continue;
        if(first_word == "}") { //If the first word is "}", it has to be closing parenthese of condition block to be syntactically correct.
            if(!conditionStc.empty()) { //Checks if there is condition block to close.
                string cond = conditionStc.top();
                //Creates end blocks 
                if(cond.substr(0, 2) == "wh") {
                    out << "\tbr label %" << cond << "cond\n";
                } else {
                    out << "\tbr label %" << cond << "end\n";
                }
                out << "\n" << conditionStc.top() << "end:\n";
                conditionStc.pop();
                if(reader.has()) { //Checks if there is something after "}".
                    hasError = true;
                    break;
                }
            } else {
                hasError = true;
                break;
            }
        }
        else if(first_word[0] >= 48 && first_word[0] <= 57) { //Checks if the first character is a number, which is always a syntax error.
            hasError = true;
            break;
        }
        else if(keyWords.find(first_word) == keyWords.end()) { // if the first word is not a keyword, it is assumed to be an assignment statement
            if(reader.peek() == "=") {// Checks for '=' statement
                reader.get();
                if(!variableHandler::exists(first_word)) { // if variable does not exist, we initialzie it.
                    variableHandler::initialize(first_word);
                }

                // Then the right hand side is passed to the expression parser
                string exp = expressionParser(reader, out);
                if(exp == "ERROR") {
                    hasError = true;
                }

                // Then the returned value is stored in the variable
                store(exp, first_word, out);
                out << "\n";
            } else {
                hasError = true;
            }
        } else { 
            //This part is used if the line starts with one of the special words. 
            if(first_word == "while") { //Implements while condition. 
                queue<string> strQ;
                string token;
                if(reader.peek() == "(") { // Pushes the tokens of the condition of "while" into a queue to sends parser function if expression has correct syntax.
                    while(reader.peek() != "{") {
                        token = reader.get();
                        strQ.push(token);
                        if(!reader.has()) {
                            hasError = true;
                            break;
                        }
                    }
                    reader.get();
                    if(reader.has()) {
                        hasError = true;
                    }
                    if(hasError) {
                        break;
                    }
                    string name = "wh" + to_string(cdc); //Creates the condition and comparison parts 
                    cdc++;
                    out << "\tbr label %" << name << "cond\n\n";
                    out << name << "cond:\n" ;
                    string before_last = expressionParser(strQ, out);
                    if(before_last == "ERROR" ) {
                        hasError = true;
                        break;
                    }
                    if(conditionStc.empty()) { //Checks if there is another condition on the stack that is not closed. It is to prevent nested loops.
                        string last = getTemp();
                        condition(last, before_last, out);  //Writes the condition part and body label
                        goBody(name, last, out);
                        out << name << "body:\n";
                        conditionStc.push(name); //After condition is created it is pushed to the stack
                    } else {
                        hasError = true;
                        break;
                    }
                } else {
                    hasError = true;
                    break;
                }
            } else if (first_word == "if") {  // Pushes the tokens of the condition of "if" into a queue to sends parser function if expression has correct syntax.
                queue<string> strQ;
                string token;
                if(reader.peek() == "(") {
                    while(reader.peek() != "{") {
                        token = reader.get();
                        strQ.push(token);
                        if(!reader.has()) {
                            hasError = true;
                            break;
                        }
                    }
                    reader.get();
                    if(reader.has()) {
                        hasError = true;
                    }
                    if(hasError) {

                        break;
                    }
                    string name = "if" + to_string(cdc); //Creates the condition and comparison parts
                    cdc++;
                    out << "\tbr label %" << name << "cond\n\n";
                    out << name << "cond:\n";
                    string before_last = expressionParser(strQ, out);
                    if(before_last == "ERROR" ) {
                        hasError = true;
                        break;
                    }
                    if(conditionStc.empty()) { //Checks if there is another condition on the stack that is not closed. It is to prevent nested loops.
                        string last = getTemp();
                        condition(last, before_last, out);    
                        goBody(name, last, out);        //Writes the condition part and body label
                        out << name << "body:\n"; 
                        conditionStc.push(name);    //After condition is created it is pushed to the stack
                    } else {
                        hasError = true;
                    }
                } else {

                    hasError = true;
                    break;
                }
            } else if (first_word == "print") { //If the first word is "print", then firstly sends expression to the parser function and then writes the return value of the function to the print template.
                queue<string> strQ;
                string token;
                if(reader.peek() == "(") {
                    string temp = expressionParser(reader, out);
                    if(temp == "ERROR" ) {
                        hasError = true;
                        break;
                    }
                    out << "\tcall i32 (i8*, ...)* @printf(i8* getelementptr ([4 x i8]* @print.str, i32 0, i32 0), i32 " << temp << ")\n";
                } else {
                    hasError = true;
                    break;
                }
            } else if(first_word == "choose") { //Choose cannot be first word in the line. Returns error in this case.
                hasError = true;
                break;
            } else {
                hasError = true;
                break;
            }

        }
    }

    if(!conditionStc.empty()) { //At the end, checks whether there is a condition block that is not closed by "}" or not.
        hasError = true;
    }

    out.close();
    in.close();
    if(hasError) {// ıf an error has been encountered, the intermediate file is removed and the code for printing the error is writte out instead

        remove(".intermediate");
        out_final << "; ModuleID = 'mylang2ir'\ndeclare i32 @printf(i8 *, ...)\n@print.error = constant [23 x i8] c\"Line %d: syntax error\\0A\\00\"\n\ndefine i32 @main() {\n\tcall i32 (i8 *, ...)* @printf(i8* getelementptr([23 x i8]* @print.error,i32 0, i32 0) , i32 " + to_string(count) + ")\n\tret i32 0\n}";
        out_final.close();
        return 0;
    }

    // At the end, the boilerplate code and the variable allocations are written to the output file and then the intermediate file is copied over to the output.
    out_final << boilerplate << "\n";
    out_final << variableHandler::getInits();
    out_final << "\n";

    ifstream copier;
    copier.open(".intermediate");
    while (copier.peek() != EOF) {
        getline(copier, line);
        out_final << line << "\n";
    }

    out_final << "\tret i32 0" << "\n" ;
    out_final << "}";

    // The files are closed and the intermediate file is removed
    copier.close();
    out_final.close();
    remove(".intermediate");

    return 0;
}

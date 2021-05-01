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


void store(string thing, string var, ofstream &out) {
    out << "\tstore i32 " << thing << ", i32* %" << var << "\n";
    return;
}

void condition(string last, string before_last, ofstream &out) {
    out << "\t" << last << " = icmp ne i32 " << before_last <<  ", 0" << endl;
}

void goBody(string where, string target, ofstream &out) {
    out << "\tbr i1 " << target << ", label %" << where << "body, label %" << where << "end\n\n";
}

string getTemp() {
    static int i = 0;
    return "%alperen_kalp_cisel" + to_string(i++); //Perfect couple variable name
}

string getTemp(string var, ofstream &out) {
    string temp = getTemp();
    out << "\t" << temp << " = load i32* %" << var << "\n";
    return temp;
}

bool isAlphaNumeric(char a) {
    return ('a' <= a and a <= 'z') or ('A' <= a and a <= 'Z') or ('0' <= a and a <= '9');
}

bool isValidVar(string var) {
    unordered_set<string> keyWords = {"while", "if", "choose", "print"};
    if(keyWords.find(var) == keyWords.end()) {
        if(('a' <= var[0] and var[0] <= 'z') or ('A' <= var[0] and var[0] <= 'Z')) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool isNumber(string var) {
    for( int i = 0; i < var.length(); i++) {
        if(var[i] >= '0' && var[i] <= '9') {
            continue;
        } else
            return false;
    }
    return true;
}

struct lineReader {
    const char white_space[2] = {' ', '	'};
    string line_text;
    int line_length = 0;
    int cursor = 0;
    int cursor_size = 0;

    lineReader(string line) {
        this->line_text = line;
        this->line_length = line.size();
        findNext();
    }

    string peek() {
        return line_text.substr(cursor, cursor_size);
    }
    string get() {
        if(has()) {
            string out = line_text.substr(cursor, cursor_size);
            findNext();
            return out;
        } else {
            return "";
        }
    }
    bool has() {
        return cursor != line_length;
    }


private:
    void findNext() {
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
    bool isWhiteSpace(char a) {
        return isspace(a);
    }


};

struct variableHandler {
    static vector<string> variables;

    static string initialize(string id) {
        if(exists(id)) {
            //cout << "Compile Error: double initialize variable";
            return "ERROR";
        } else {
            if(isValidVar(id)) {
                variables.push_back(id);
                return "success";
            } else {
                //cout << "Syntax error: invalid variable \n";
                return "ERROR";
            }
        }
    }

    static bool exists(string id) {
        return find(variables.begin(), variables.end(), id) != variables.end();
    }

    static string getInits() {
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

vector<string> variableHandler::variables;

string expressionParser(queue<string> &expr, ofstream &out);

string choose (queue<string> expQ, ofstream &out) {

    if(expQ.front() == "(") {
        expQ.pop();
        int prs = 0;
        queue<string> strQ1;
        while(expQ.front() != "," || prs != 0) {
            if(expQ.front() == "(") {
                prs++;
            } else if(expQ.front() == ")") {
                prs--;
            }
            strQ1.push(expQ.front());
            expQ.pop();
            if(expQ.empty()) {
                return "ERROR1";
            }
        }
        expQ.pop();
        string exp1 = expressionParser(strQ1, out);
        if(exp1 == "ERROR")
            return exp1;

        prs = 0;
        queue<string> strQ2;
        while(expQ.front() != "," || prs != 0) {
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
        string exp2 = expressionParser(strQ2, out);
        if(exp2 == "ERROR")
            return exp2;

        prs = 0;
        queue<string> strQ3;
        while(expQ.front() != "," || prs != 0) {
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
        string exp3 = expressionParser(strQ3, out);
        if(exp3 == "ERROR")
            return exp3;

        prs = 0;
        queue<string> strQ4;
        while(expQ.front() != ")" || prs != 0) {
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
        string exp4 = expressionParser(strQ4, out);
        if(exp4 == "ERROR")
            return exp4;

        string first = getTemp();
        string second = getTemp();
        string third = getTemp();
        string last = getTemp();
        out << "\t" << first << " = icmp sgt i32 " << exp1 << ", 0\n";
        out << "\t" << second << " = icmp eq i32 " << exp1 << ", 0\n";
        out << "\t" << third << " = select i1 " << first << ", i32 " << exp3 << ", i32 " << exp4 << "\n";
        out << "\t" << last << " = select i1 " << second << ", i32 " << exp2 << ", i32 " << third << "\n";
        return last;
    } else {
        return "ERROR";
    }
}

string expressionParser(queue<string> &expr, ofstream &out) {
    const char operators[6] = {'+', '-', '*', '/'};
    queue<string> out_queue;
    stack<char> op_stack;
    bool operatortime = false;
    while(!expr.empty()) {
        string token = expr.front();
        expr.pop();
        if(isNumber(token) or isValidVar(token)) {
            if(operatortime) {
                //cout << "Syntax error: non operator after operand1\n";
                return "ERROR";
            }
            operatortime = true;
            out_queue.push(token);
        } else {
            if(find(begin(operators), end(operators), token[0]) != end(operators)) {
                if(!operatortime) {
                    //cout << "Syntax error: operator after operand2\n";
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
                    //cout << "Syntax error: non operator after operand3\n";
                    return "ERROR";
                }
                op_stack.push(token[0]);
            } else if(token[0] == ')') {
                if(!operatortime) {
                    //cout << "Syntax error: wrong close paranthesis\n";
                    return "ERROR";
                }
                operatortime = true;
                if(op_stack.empty()) {
                    //cout << "Syntax error: missing paranthesis '('\n";
                    return "ERROR";
                }
                while(op_stack.top() != '(') {

                    out_queue.push(string(1, op_stack.top()));
                    op_stack.pop();
                    if(op_stack.empty()) {
                        //cout << "Syntax error: missing paranthesis '('\n";
                        return "ERROR";
                    }

                }
                op_stack.pop();
            } else if(token == "choose") {

                if(operatortime) {
                    //cout << "Syntax error: non operator after operand4\n";
                    return "ERROR";
                }
                operatortime = true;
                int para = 0;
                queue<string> choose_q;

                while(!expr.empty()) {
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
                    //cout << "Syntax error: missing choose )\n";
                    return "ERROR";
                }
                string choose_ret = choose(choose_q, out);
                if(choose_ret == "ERROR")
                    return "ERROR";
                out_queue.push(choose_ret);

            } else {
                //cout << "Syntax error: nonsensical character\n";
                return "ERROR";
            }
        }
    }
    while(!op_stack.empty()) {
        out_queue.push(string(1, op_stack.top()));
        op_stack.pop();
    }


    stack<string> operand_stack;
    while(!out_queue.empty()) {
        string subj = out_queue.front();
        out_queue.pop();

        if(isAlphaNumeric(subj[0]) or subj[0] == '%') {
            if(subj[0] != '%' and !isNumber(subj)) {
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
                    //cout << "Syntax error: not enough operands";
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

            out << "\t" << temp << " = " << operat << " i32 " << op1 << ", " << op2 << "\n";
            operand_stack.push(temp);
        } else if(subj[0] == '(') {
            //cout << "missing (\n" << endl;
            return "ERROR";
        } else {
            cout << "this shouldn't happen :/\n";
            return "ERROR";
        }
    }
    return operand_stack.top();
}

string expressionParser(lineReader &expr, ofstream &out) {
    queue<string> strQ;
    while(expr.has()) {
        strQ.push(expr.get());
    }
    return expressionParser(strQ, out);
}





int main(int argc, char const *argv[]) {

    const string boilerplate = "; ModuleID = 'mylang2ir'\ndeclare i32 @printf(i8*, ...)\n@print.str = constant [4 x i8] c\"%d\\0A\\00\"\n\ndefine i32 @main() {\n";

    string inputFile = argv[1];
    string outputFile = inputFile.substr(0, inputFile.find_last_of('.')) + ".ll";
    ifstream in;
    ofstream out_final;
    ofstream out;
    in.open(inputFile);

    out_final.open(outputFile);
    out.open(".intermediate");
    bool hasError = false;

    unordered_set<string> keyWords = {"while", "if", "choose", "print"};
    string line;
    stack<string> conditionStc;
    int count = -1;
    int cdc = 0 ;
    while (!hasError and in.peek() != EOF) { //Bu javadaki hasNextLine() fonksiyonunun yaptığını yapıyo
        count++;
        getline(in, line);
        lineReader reader(line);
        string first_word = reader.get();
        if(first_word.length() == 0 || first_word == "#")
            continue;
        if(first_word == "}") {
            if(!conditionStc.empty()) {
                string cond = conditionStc.top();
                if(cond.substr(0, 2) == "wh") {
                    out << "\tbr label %" << cond << "cond\n";
                } else {
                    out << "\tbr label %" << cond << "end\n";
                }
                out << "\n" << conditionStc.top() << "end:\n";
                conditionStc.pop();
                if(reader.has()) {
                    hasError = true;
                    break;
                }
            } else {
                hasError = true;
                break;
            }
        }
        else if(first_word[0] >= 48 && first_word[0] <= 57) { //ilk kelimenin ilk karakteri sayıyla başlıyosa

            hasError = true;
            break;
        }
        else if(keyWords.find(first_word) == keyWords.end()) { // Kelime keyword değilse ve sayıyla başlamıyosa buraya, Assignment olcak
            if(reader.peek() == "=") {
                reader.get();
                if(!variableHandler::exists(first_word)) { //Neyce yazıyo bu çocuk amk
                    variableHandler::initialize(first_word);
                }
                //Shunting-Yard
                string exp = expressionParser(reader, out);
                if(exp == "ERROR") {
                    hasError = true;
                }
                store(exp, first_word, out);
                out << "\n";
            } else {
                hasError = true;
            }
        } else {
            if(first_word == "while") {
                queue<string> strQ;
                string token;

                cout << "hello";
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
                    string name = "wh" + to_string(cdc);
                    cdc++;
                    out << "\tbr label %" << name << "cond\n\n";
                    out << name << "cond:\n" ;
                    string before_last = expressionParser(strQ, out);
                    if(before_last == "ERROR" ) {
                        hasError = true;
                        break;
                    }
                    if(conditionStc.empty()) {
                        string last = getTemp();
                        condition(last, before_last, out);
                        goBody(name, last, out);
                        out << name << "body:\n";
                        conditionStc.push(name);
                    } else {

                        hasError = true;
                        break;
                    }
                } else {

                    hasError = true;
                    break;
                }
            } else if (first_word == "if") {
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
                    string name = "if" + to_string(cdc);
                    cdc++;
                    out << "\tbr label %" << name << "cond\n\n";
                    out << name << "cond:\n";
                    string before_last = expressionParser(strQ, out);
                    if(before_last == "ERROR" ) {
                        hasError = true;
                        break;
                    }
                    if(conditionStc.empty()) {
                        string last = getTemp();
                        condition(last, before_last, out);
                        goBody(name, last, out);
                        out << name << "body:\n";
                        conditionStc.push(name);
                    } else {
                        hasError = true;
                    }
                } else {

                    hasError = true;
                    break;
                }
            } else if (first_word == "print") {
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
            } else if(first_word == "choose") {

                hasError = true;
                break;
            } else {

                hasError = true;
                break;
            }

        }
    }


    if(!conditionStc.empty()) {

        hasError = true;
    }

    out.close();
    in.close();
    if(hasError) {

        remove(".intermediate");
        out_final << "; ModuleID = 'mylang2ir'\ndeclare i32 @printf(i8 *, ...)\n@print.error = constant [23 x i8] c\"Line %d: syntax error\\0A\\00\"\n\ndefine i32 @main() {\n\tcall i32 (i8 *, ...)* @printf(i8* getelementptr([23 x i8]* @print.error,i32 0, i32 0) , i32 " + to_string(count) + ")\n\tret i32 0\n}";
        out_final.close();
        return 0;
    }

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
    out_final << "}" ;
    copier.close();
    out_final.close();
    remove(".intermediate");
}
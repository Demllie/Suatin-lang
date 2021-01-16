#include<iostream>
using namespace std;


typedef enum {
	NIL,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	NUM,
	LB,
	RB
}TokenType;



static FILE* output=NULL;


/*------------------------------------------------------*/
class Node {  //元节点
public:
	virtual void visit() = 0;
	virtual ~Node() {}
	virtual void destroy() = 0; //删除每一个节点
};
class TreeN : public Node { //树节点
public:
	Node* l;
	Node* r;
	TreeN(Node* _l, Node* _r) {
		l = _l;
		r = _r;
	}
	virtual ~TreeN() {
		destroy();
	}
	void destroy() {
		if (l != NULL)l->destroy();
		if (r != NULL)r->destroy();
		l = r = NULL;
	}
};



class AddN : public TreeN {
public:
	AddN(Node* _l, Node* _r) : TreeN(_l, _r) {	}
	void visit() {
		l->visit();
		r->visit();
		fprintf(output, "%s\n", "ADD");
	}
};
class SubN : public TreeN {
public:
	SubN(Node* _l, Node* _r) : TreeN(_l, _r) {	}
	void visit() {
		l->visit();
		r->visit();
		fprintf(output, "%s\n", "SUB");
	}
};
class MulN : public TreeN {
public:
	MulN(Node* _l, Node* _r) : TreeN(_l, _r) {	}
	void visit() {
		l->visit();
		r->visit();
		fprintf(output, "%s\n", "MUL");
	}
};
class DivN : public TreeN {
public:
	DivN(Node* _l, Node* _r) : TreeN(_l, _r) {	}
	void visit() {
		l->visit();
		r->visit();
		fprintf(output, "%s\n", "DIV");
	}
};
class ModN : public TreeN {
public:
	ModN(Node* _l, Node* _r) : TreeN(_l, _r) {	}
	void visit() {
		l->visit();
		r->visit();
		fprintf(output, "%s\n", "MOD");
	}
};
class ConstN : public Node {
private:
	double v;
public:
	ConstN(double _v) :v(_v) {	}
	void visit() {
		fprintf(output, "LOAD %lf\n",v);
	}
	void destroy() {
		//pass
	}
};
/*------------------------------------------------------*/









#define ENUM_TYPE(type)   (type==0?"NIL":  \
												  (type==1?"ADD": \
											      (type==2?"SUB": \
												  (type==3?"MUL": \
												  (type==4?"DIV":  \
												  (type==5?"MOD": \
												  (type==6?"NUM": \
												  (type==7?"LB":"RB"))))))))


typedef union {
	char c;
	double v;
}Var;


class Token {
private:
	static int counter;//类变量,不能再此处初始化
	int num;
	TokenType type = TokenType::NIL;
	Var value;
public:
	Token(char _value) {
		num = counter++;
		switch (_value) {
		case '+':
			type = TokenType::ADD;
			break;
		case '-':
			type = TokenType::SUB;
			break;
		case '*':
			type = TokenType::MUL;
			break;
		case '/':
			type = TokenType::DIV;
			break;
		case '%':
			type = TokenType::MOD;
			break;
		case '(':
			type = TokenType::LB;
			break;
		case ')':
			type = TokenType::RB;
			break;
		default:
			cout << "违法字符>" << _value << endl;
			exit(1);
			break;
		}

		value.c = _value;
	}
	Token(double _value) {
		num = counter++;
		type = TokenType::NUM;
		value.v = _value;
	}


	TokenType GetTokenType()const {
		return type;
	}
	double GetTokenValue()const {
		if (type == TokenType::NUM)return value.v;
		return 0;
	}
	int GetNum()const {
		return num;
	}
};

int Token::counter = 0;//类变量初始化








static Token* currentToken = NULL;
static int index = 0;
static char line[256] = { 0 };




void next();
Node* factor();
Node* term();
Node* expr();



int main(void) {

	FILE* file = NULL;
	int a = fopen_s(&file, "test.txt", "r");
	if (a != 0) {
		cout << "文件test.txt打不开!" << endl;
		return 1;
	}

	fgets(line, 256, file);
	cout << line << endl;
	next();//取得第一个token

	Node* root = expr();//获取AST抽象语法树

	//把字节码写入output.txt
	int b = fopen_s(&output, "output.txt", "w+"); //append
	if (b != 0) {
		cout << "文件output.txt打不开!" << endl;
		return 1;
	}
	root->visit();

	delete root;//删除抽象语法树


	fclose(file);
	fclose(output);
	return 0;
}



//获取下一个token
void next() {

	char c;
	double v = 0;

	while ((c = line[index++]) != '\0') {
		if (c == ' ')continue;
		else if (c <= '9' && c >= '0') {
			v = v * 10 + c - '0';
			char nextChar = line[index];
			if (!(nextChar <= '9' && nextChar >= '0')) {
				if (currentToken != NULL) {
					delete currentToken;
					currentToken = NULL;
				}
				currentToken = new Token((double)v);
				break;
			}
		}
		else if (c == '(' || c == ')') {
			if (currentToken != NULL)
				delete currentToken;
			currentToken = new Token((char)c);
			break;
		}
		else if (c == '+' || c == '-' || c == '*' || c == '%' || c == '/') {
			if (currentToken != NULL) {
				delete currentToken;
				currentToken = NULL;
			}
			currentToken = new Token((char)c);
			break;
		}
	}


}

//终结符，是最底层的元素
Node* factor() {

	double value;
	if (currentToken->GetTokenType() == TokenType::NUM) {
		value = currentToken->GetTokenValue();
		next();
		return  new ConstN(value);

	}
	else if (currentToken->GetTokenType() == TokenType::LB) {
		next();
		Node* ans = expr();//获取括号内的answer


		//错误处理
		if (currentToken->GetTokenType() != TokenType::RB) {
			cout << "语法分析出错，右括号不匹配！";
			if (currentToken->GetTokenType() == TokenType::NUM) {
				cout << "value > " << currentToken->GetTokenValue() << endl;
			}
			else {
				cout << "value > " << ENUM_TYPE(currentToken->GetTokenType()) << endl;
			}
		}
		next();
		return ans;
	}

	cout << "factor既不是数字也不是表达式！" << endl;
	return NULL;
}

//乘除取余表达式
Node* term() {

	Node* value;
	Node* left = factor();//乘除的上一级是单个元素或者表达
	Node* right;
	value = left;


	//暂时保存current token前两个token的类型和值,即前一个OP
	Token tempToken = *currentToken;




	while (tempToken.GetTokenType() == TokenType::MOD || tempToken.GetTokenType() == TokenType::MUL || tempToken.GetTokenType() == TokenType::DIV) {
		next();

		right = factor();//乘除的上一级是单个元素或者表达
		switch (tempToken.GetTokenType()) {
		case TokenType::MOD:
			value = new ModN(value, right);
			break;
		case TokenType::MUL:
			value = new MulN(value, right);
			break;
		case TokenType::DIV:
			if (right == 0) {
				cout << "除数不能为零" << endl;
				exit(1);
				return NULL;
			}
			value = new DivN(value, right);
			break;
		}
		tempToken = *currentToken;//移动token

	}

	return value;
}

//加减表达式
Node* expr() {

	Node* value;
	Node* left = term();//加减的上一级是乘除取余
	Node* right;
	value = left;


	//暂时保存current token前两个token的类型和值,即前一个OP
	Token tempToken = *currentToken;



	while (tempToken.GetTokenType() == TokenType::ADD || tempToken.GetTokenType() == TokenType::SUB) {
		next();

		right = term(); //加减的上一级是乘除取余
		switch (tempToken.GetTokenType()) {
		case TokenType::ADD:
			value = new AddN(value, right);
			break;
		case TokenType::SUB:
			value = new SubN(value, right);
			break;
		}
		tempToken = *currentToken;//移动token

	}

	return value;
}

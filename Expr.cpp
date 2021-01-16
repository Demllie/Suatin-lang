#include "Expr.h"


namespace sua {




	Expr::~Expr() { //虚析构

	}
	SuatinExprClassType Expr::GetClassType() { 
		return SuatinExprClassType_Expr;
	}
	double Expr::interpret() {
		return 0.; 
	}  
	std::string Expr::interpret_str() {
		return ""; 
	}
	bool Expr::interpret_bool() {
		return true; 
	} 




	VarExpr::VarExpr(std::string _name) {
		name = _name;
	}
	VarExpr::~VarExpr() { //虚析构

	}
	std::string VarExpr::GetName()const {
		return name;
	}
	SuatinExprClassType VarExpr::GetClassType() { 
		return SuatinExprClassType_VarExpr; 
	}



	SymbolExpr::SymbolExpr(Expr* _left, Expr* _right) {
		left = _left;
		right = _right;
	}
	SymbolExpr::~SymbolExpr() {
		if (left) {
			delete left;
			left = NULL;
		}
		if (right) {
			delete right;
			right = NULL;
		}
	}

	Expr* SymbolExpr::GetLeft()const {
		return left;
	}
	Expr* SymbolExpr::GetRight()const {
		return right;
	}
	void SymbolExpr::SetLeft(Expr* _left) {
		left = _left;
	}
	void SymbolExpr::SetRight(Expr* _right) {
		right = _right;
	}
	SuatinExprClassType SymbolExpr::GetClassType() {
		return SuatinExprClassType_SymbolExpr; 
	}




	OneOpExpr::OneOpExpr(Expr* _content) {
		content = _content;
	}
	OneOpExpr::~OneOpExpr() {
		if (content) {
			delete content;
			content = NULL;
		}
	}


	Expr* OneOpExpr::GetContent()const {
		return content;
	}
	void OneOpExpr::SetContent(Expr* _content) {
		content = _content;
	}
	SuatinExprClassType OneOpExpr::GetClassType() { 
		return SuatinExprClassType_OneOpExpr; 
	}





	LLeftExpr::LLeftExpr(Expr* _content) : OneOpExpr(_content) {
	
	}
	bool LLeftExpr::GetMatchedFlag()const { 
		return matched_flag; 
	}
	void LLeftExpr::SetMatchedFlag() { 
		matched_flag = true; 
	}
	SuatinExprClassType LLeftExpr::GetClassType() { 
		return SuatinExprClassType_LLeftExpr; 
	}



	double LLeftExpr::interpret() { 
		return content->interpret(); 
	}
	bool LLeftExpr::interpret_bool() {
		return content->interpret_bool();
	}



	InterfaceTypeExpr::InterfaceTypeExpr(Expr* _left, Expr* _right) : SymbolExpr(_left, _right) {
	
	}
	SuatinExprClassType  InterfaceTypeExpr::GetClassType() {
		return SuatinExprClassType_InterfaceTypeExpr; 
	}


	Unit_InterfaceType InterfaceTypeExpr::GetLeft_InterfaceType()const {
		return left_i_type;
	}
	Unit_InterfaceType InterfaceTypeExpr::GetRight_InterfaceType()const {
		return right_i_type;
	}
	void InterfaceTypeExpr::SetLeft_InterfaceType(Unit_InterfaceType _i_type) {
		left_i_type = _i_type;
	}
	void InterfaceTypeExpr::SetRight_InterfaceType(Unit_InterfaceType _i_type) {
		right_i_type = _i_type;
	}







	/*
	无法直接判断内容，因为
	字符串由两种形式：
					1.StrExpr字符串节点
					2.解释返回的字符串，可能是单个字符串，也可能是拼接之后的字符串
	数字有两种形式：
					 1.NumExpr数字节点
					 2.解释返回的数字
	另外，bool类型就只有一种形式：IDExpr变量的bool类型

	*/


	bool AndExpr::interpret_bool() {
		if (left_i_type == Unit_InterfaceType_bool) {
			if (right_i_type == Unit_InterfaceType_bool) {
				return left->interpret_bool() && right->interpret_bool();
			}
			else {
				return left->interpret_bool();//右边的为true，所以只用判断左边的
			}
		}
		else {
			if (right_i_type == Unit_InterfaceType_bool) {
				return right->interpret_bool();//左边的为true,所以只用判断右边的
			}
		}
		return true;
	}
	
	bool OrExpr::interpret_bool() {
		//除了bool和bool需要考虑外，其他分支的都有一方是真，这就够了
		if (left_i_type == Unit_InterfaceType_bool) {
			if (right_i_type == Unit_InterfaceType_bool) {
				return left->interpret_bool() || right->interpret_bool();
			}
		}
		return true;
	}

	bool NotExpr::interpret_bool() {
		if (i_type == Unit_InterfaceType_bool) {
			return !(content->interpret_bool());//not后面的内容返回真假
		}
		return false;//当not后面的内容返回字符串或数字时，其内容是true
		
	}



	NotExpr::NotExpr(Expr* _content) : OneOpExpr(_content) {
	
	}
	SuatinExprClassType NotExpr::GetClassType() { 
		return SuatinExprClassType_NotExpr; 
	}
	Unit_InterfaceType NotExpr::Get_InterfaceType()const {
		return i_type;
	}
	void NotExpr::Set_InterfaceType(Unit_InterfaceType _i_type) {
		i_type = _i_type;
	}







	AndExpr::AndExpr(Expr* _left, Expr* _right) : InterfaceTypeExpr(_left, _right) {
	
	}
	SuatinExprClassType AndExpr::GetClassType() {
		return SuatinExprClassType_AndExpr; 
	}





	OrExpr::OrExpr(Expr* _left, Expr* _right) : InterfaceTypeExpr(_left, _right) {
	
	}
	SuatinExprClassType OrExpr::GetClassType() { 
		return SuatinExprClassType_OrExpr; 
	}





	EqExpr::EqExpr(Expr* _left, Expr* _right) : SymbolExpr(_left, _right) {
	
	}
	SuatinExprClassType EqExpr::GetClassType() { 
		return SuatinExprClassType_EqExpr; 
	}


	//处理数字、数字类型的id--------------------
	double EqExpr::interpret() {
		double rightValue = right->interpret();//递归进去
		IDExpr* var = dynamic_cast<IDExpr*>(left);
		var->SetNumber(rightValue);
		return rightValue;
	}


	//处理字符串、字符串类型的id------------------
	std::string EqExpr::interpret_str() {
		std::string rightValue = right->interpret_str();//递归进去
		IDExpr* var = dynamic_cast<IDExpr*>(left);
		var->SetString(rightValue);
		return rightValue;
	}

	//bool类型的id--------------------
	bool EqExpr::interpret_bool() {
		bool rightValue = right->interpret_bool();//递归进去
		IDExpr* var = dynamic_cast<IDExpr*>(left);
		var->SetBool(rightValue);
		return rightValue;
	}











	GreExpr::GreExpr(Expr* _left, Expr* _right) : SymbolExpr(_left, _right) {
	
	}
	SuatinExprClassType GreExpr::GetClassType() { 
		return SuatinExprClassType_GreExpr; 
	}

	bool GreExpr::interpret_bool() {
		return left->interpret() > right->interpret();
	}

	GreEqExpr::GreEqExpr(Expr* _left, Expr* _right) : SymbolExpr(_left, _right) {
	
	}
	SuatinExprClassType  GreEqExpr::GetClassType() { 
		return SuatinExprClassType_GreEqExpr; 
	}

	bool GreEqExpr::interpret_bool() {
		return left->interpret() >= right->interpret();
	}

	LesExpr::LesExpr(Expr* _left, Expr* _right) : SymbolExpr(_left, _right) {
	
	}
	SuatinExprClassType LesExpr::GetClassType() { 
		return SuatinExprClassType_LesExpr; 
	}

	bool LesExpr::interpret_bool() {
		return left->interpret() < right->interpret();
	}

	LesEqExpr::LesEqExpr(Expr* _left, Expr* _right) : SymbolExpr(_left, _right) {
	
	}
	SuatinExprClassType LesEqExpr::GetClassType() {
		return SuatinExprClassType_LesEqExpr; 
	}

	bool LesEqExpr::interpret_bool() {
		return left->interpret() <=  right->interpret();
	}



	NeqExpr::NeqExpr(Expr* _left, Expr* _right) : InterfaceTypeExpr(_left, _right) {
	
	}
	SuatinExprClassType NeqExpr::GetClassType() { 
		return SuatinExprClassType_NeqExpr; 
	}

	//上面几个都是值比较，下面两个就不仅仅是值比较了！
	//暂时不做其他类型的比较，因为接口和表达式类型的复杂型，所以这个位置很难判断该如何比较！！！
	//暂时就只做数字的比较好了
	bool NeqExpr::interpret_bool() {
		if (left_i_type == Unit_InterfaceType_bool) {
			if (right_i_type == Unit_InterfaceType_bool) {
				return left->interpret_bool() != right->interpret_bool();
			}
			return !(left->interpret_bool());//右边是true,所以只用判断左边
		}
		else if (left_i_type == Unit_InterfaceType_number) {
			if (right_i_type == Unit_InterfaceType_bool) {
				return !(right->interpret_bool());//左边是true，所以只用判断右边
			}
			else if (right_i_type == Unit_InterfaceType_number) {
				return left->interpret() != right->interpret();
			}
		}
		else if(left_i_type==Unit_InterfaceType_string){
			if (right_i_type == Unit_InterfaceType_bool) {
				return !(right->interpret_bool());//左边是true，所以只用判断右边
			}
			else if (right_i_type == Unit_InterfaceType_string) {
				return left->interpret_str() != right->interpret_str();
			}
		}

		return true; //两边类型不一致
	}




	EqEqExpr::EqEqExpr(Expr* _left, Expr* _right) : InterfaceTypeExpr(_left, _right) {
	
	}
	SuatinExprClassType EqEqExpr::GetClassType() { 
		return SuatinExprClassType_EqEqExpr; 
	}

	bool EqEqExpr::interpret_bool() {
		if (left_i_type == Unit_InterfaceType_bool) {
			if (right_i_type == Unit_InterfaceType_bool) {
				return left->interpret_bool() == right->interpret_bool();
			}
			return left->interpret_bool();//右边是true,所以只用判断左边
		}
		else if (left_i_type == Unit_InterfaceType_number) {
			if (right_i_type == Unit_InterfaceType_bool) {
				return right->interpret_bool();//左边是true，所以只用判断右边
			}
			else if (right_i_type == Unit_InterfaceType_number) {
				return left->interpret() == right->interpret();
			}
		}
		else if (left_i_type==Unit_InterfaceType_string) {
			if (right_i_type == Unit_InterfaceType_bool) {
				return right->interpret_bool();//左边是true，所以只用判断右边
			}
			else if (right_i_type == Unit_InterfaceType_string) {
				return left->interpret_str() == right->interpret_str();
			}
		}

		return false; //两边类型不一致
	}








	AddExpr::AddExpr(Expr* _left, Expr* _right) :SymbolExpr(_left, _right) { 
	
	}
	SuatinExprClassType AddExpr::GetClassType() {
		return SuatinExprClassType_AddExpr; 
	}

	//加法运算
	double AddExpr::interpret() {
		return  left->interpret() + right->interpret();
	}

	//字符串拼接
	std::string AddExpr::interpret_str() {
		return left->interpret_str() + right->interpret_str();
	}


	SubExpr::SubExpr(Expr* _left, Expr* _right) :SymbolExpr(_left, _right) {
	
	}
	SuatinExprClassType SubExpr::GetClassType() { 
		return SuatinExprClassType_SubExpr; 
	}

	double SubExpr::interpret() {
		return  left->interpret() - right->interpret();
	}


	MulExpr::MulExpr(Expr* _left, Expr* _right) :SymbolExpr(_left, _right) {
	
	}
	SuatinExprClassType  MulExpr::GetClassType() { 
		return SuatinExprClassType_MulExpr; 
	}

	double MulExpr::interpret() {
		return  left->interpret() * right->interpret();
	}


	DivExpr::DivExpr(Expr* _left, Expr* _right) :SymbolExpr(_left, _right) {
	
	}
	SuatinExprClassType DivExpr::GetClassType() { 
		return SuatinExprClassType_DivExpr; 
	}

	double DivExpr::interpret() {
		double rightResult = right->interpret();
		if (rightResult == 0) {
			ThrowException(SuatinErrorType_ZeroDivisor, "divide number must be nonzero");
			return 0;
		}
		return  left->interpret() / rightResult;
	}

	PowExpr::PowExpr(Expr* _left, Expr* _right) :SymbolExpr(_left, _right) { 
	
	}
	SuatinExprClassType PowExpr::GetClassType() { 
		return SuatinExprClassType_PowExpr; 
	}

	double PowExpr::interpret() {
		return pow(left->interpret(), right->interpret());
	}







	//默认变量就是NIL类型的
	//如果已经存在就不会创建新的
	IDExpr::IDExpr(std::string _name) : VarExpr(_name){
		CreateID(_name);
	}
	SuatinExprClassType IDExpr::GetClassType() {
		return SuatinExprClassType_IDExpr; 
	}


	//IDExpr获取属性接口
	double IDExpr::GetNumber()const {
		return (double)SuatinEnv[name]->val.double_num;
	}
	int IDExpr::GetInt()const {
		return (int)SuatinEnv[name]-> val.int_num;
	}
	std::string IDExpr::GetString()const {
		return (std::string)(*SuatinEnv[name]->val.str);
	}
	SuatinIDType IDExpr::GetType()const {
		return SuatinEnv[name]->type;
	}

	bool IDExpr::GetBool()const {
		return (bool)SuatinEnv[name]->val.flag;
	}
	
	bool IDExpr::IsConst()const {
		return false;
	}

	//IDExpr设置属性接口
	void IDExpr::SetBool(bool _value){
		SetID(name, (bool)_value);
	}

	void IDExpr::SetNumber(double _value) {
		SetID(name,(double) _value);
	}
	void IDExpr::SetInt(int _value) {
		SetID(name, (int)_value);
	}
	void IDExpr::SetString(std::string _value) {
		SetID(name, (std::string)_value);
	}

	
	//IDExpr的解释接口
	double IDExpr::interpret() {
		if (GetType() == SuatinIDType_nil) {
			ThrowException(SuatinErrorType_Value, g_statement_start, g_statement_end, "nil type identifier cannot used");
		}
		return GetNumber();
	}

	std::string IDExpr::interpret_str() {
		if (GetType() == SuatinIDType_nil) {
			ThrowException(SuatinErrorType_Value,g_statement_start, g_statement_end, "nil type identifier cannot used");
		}
		return GetString();
	}
	bool IDExpr::interpret_bool() {
		if (GetType() == SuatinIDType_nil) {
			ThrowException(SuatinErrorType_Value,g_statement_start, g_statement_end, "nil type identifier cannot used");
		}
		return GetBool();
	}





	NumExpr::NumExpr(std::string _name) :VarExpr(_name) {
	
	}
	SuatinExprClassType NumExpr::GetClassType() { 
		return SuatinExprClassType_NumExpr; 
	}

	double NumExpr::interpret() {
		return atof(name.c_str());//字符串转浮点数
	}
	
	
	
	
	StrExpr::StrExpr(std::string _name) :VarExpr(_name) {
	
	}
	SuatinExprClassType StrExpr::GetClassType() {
		return SuatinExprClassType_StrExpr; 
	}

	std::string StrExpr::interpret_str() {
		return name.substr(1,name.size() - 2);//返回去掉左右双引号后的内容
	}





};


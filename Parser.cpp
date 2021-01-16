#include "Parser.h"





namespace sua {


	void Parser::DelTree(Expr* _expr) {
		if (_expr == NULL)return;


		//当前节点是一个数或变量，直接删除并返回
		if(_expr->GetClassType() == SuatinExprClassType_NumExpr || _expr->GetClassType() == SuatinExprClassType_IDExpr || _expr->GetClassType() == SuatinExprClassType_StrExpr ){
			delete _expr;
			_expr = NULL;
		}
		//当前节点的左括号节点，迭代进去，找到一个非左括号节点
		else if(_expr->GetClassType() == SuatinExprClassType_LLeftExpr){
			LLeftExpr* branch2 = dynamic_cast<LLeftExpr*>(_expr);
			//while (typeid(*(branch2->GetContent()->GetClassType())) == typeid(LLeftExpr)) { //内容还是个左括号节点
			while( branch2->GetContent()->GetClassType() == SuatinExprClassType_LLeftExpr ){ //内容还是个左括号节点
				LLeftExpr* branch3 = dynamic_cast<LLeftExpr*>(branch2->GetContent());
				branch2 = branch3;
			}

			DelTree(branch2->GetContent());

		}
		//当前节点是not节点，not只有一个，不需要迭代进去
		else if(_expr->GetClassType() == SuatinExprClassType_NotExpr){
			NotExpr* node_tmp = dynamic_cast<NotExpr*>(_expr);
			DelTree(node_tmp->GetContent());
		}
		else {
			SymbolExpr* branch4 = dynamic_cast<SymbolExpr*>(_expr);
			DelTree(branch4->GetLeft());
			DelTree(branch4->GetRight());

		}

	}


	Parser::Parser(int _start,int _end) {
		//初始化语句的范围
		g_statement_start = start = _start;
		g_statement_end = end = _end;


		//初始化处理Token的函数
		funcMap[SuatinTokenType_Num] = &Parser::DealToken_Num;
		funcMap[SuatinTokenType_Id] = &Parser::DealToken_Id;
		funcMap[SuatinTokenType_Str] = &Parser::DealToken_Str;
		funcMap[SuatinTokenType_Pow] = &Parser::DealToken_Pow;
		funcMap[SuatinTokenType_Mul] = &Parser::DealToken_Mul;
		funcMap[SuatinTokenType_Div] = &Parser::DealToken_Div;
		funcMap[SuatinTokenType_Add] = &Parser::DealToken_Add;
		funcMap[SuatinTokenType_Sub] = &Parser::DealToken_Sub;
		funcMap[SuatinTokenType_Gre] = &Parser::DealToken_Gre;
		funcMap[SuatinTokenType_GreEq] = &Parser::DealToken_GreEq;
		funcMap[SuatinTokenType_Les] = &Parser::DealToken_Les;
		funcMap[SuatinTokenType_LesEq] = &Parser::DealToken_LesEq;
		funcMap[SuatinTokenType_Neq] = &Parser::DealToken_Neq;
		funcMap[SuatinTokenType_EqEq] = &Parser::DealToken_EqEq;
		funcMap[SuatinTokenType_Eq] = &Parser::DealToken_Eq;
		funcMap[SuatinTokenType_Com] = &Parser::DealToken_Com;
		funcMap[SuatinTokenType_ML] = &Parser::DealToken_ML;
		funcMap[SuatinTokenType_MR] = &Parser::DealToken_MR;
		funcMap[SuatinTokenType_LL] = &Parser::DealToken_LL;
		funcMap[SuatinTokenType_LR] = &Parser::DealToken_LR;
		funcMap[SuatinTokenType_BL] = &Parser::DealToken_BL;
		funcMap[SuatinTokenType_BR] = &Parser::DealToken_BR;
		funcMap[SuatinTokenType_Dot] = &Parser::DealToken_Dot;
		funcMap[SuatinTokenType_Sem] = &Parser::DealToken_Sem;
		//funcMap[SuatinTokenType_Eol] = &Parser::DealToken_Eol;


		//初始化处理关键字的函数
		k_funcMap[SuatinKeyWordType_if] = &Parser::Deal_k_if;
		k_funcMap[SuatinKeyWordType_elif] = &Parser::Deal_k_elif;
		k_funcMap[SuatinKeyWordType_else] = &Parser::Deal_k_else;
		k_funcMap[SuatinKeyWordType_for] = &Parser::Deal_k_for;
		k_funcMap[SuatinKeyWordType_break] = &Parser::Deal_k_break;
		k_funcMap[SuatinKeyWordType_continue] = &Parser::Deal_k_continue;
		k_funcMap[SuatinKeyWordType_do] = &Parser::Deal_k_do;
		k_funcMap[SuatinKeyWordType_until] = &Parser::Deal_k_until;
		k_funcMap[SuatinKeyWordType_while] = &Parser::Deal_k_while;
		k_funcMap[SuatinKeyWordType_local] = &Parser::Deal_k_local;
		k_funcMap[SuatinKeyWordType_const] = &Parser::Deal_k_const;
		k_funcMap[SuatinKeyWordType_and] = &Parser::Deal_k_and;
		k_funcMap[SuatinKeyWordType_or] = &Parser::Deal_k_or;
		k_funcMap[SuatinKeyWordType_not] = &Parser::Deal_k_not;
		k_funcMap[SuatinKeyWordType_function] = &Parser::Deal_k_function;
		k_funcMap[SuatinKeyWordType_end] = &Parser::Deal_k_end;
		k_funcMap[SuatinKeyWordType_return] = &Parser::Deal_k_return;
		k_funcMap[SuatinKeyWordType_assert] = &Parser::Deal_k_assert;


	}
	Parser::~Parser() {
		//释放语法树
		DelTree(root);
		
		//释放容器工具
		while (v_NoMatchedLLeft.size() > 0) {
			v_NoMatchedLLeft.pop();
		}
		std::stack<Expr*>().swap(v_NoMatchedLLeft);
		funcMap.clear();
		std::map<SuatinTokenType, DealFuncPtr>().swap(funcMap);
		k_funcMap.clear();
		std::map<SuatinKeyWordType, DealFuncPtr>().swap(k_funcMap);


	}






	void Parser::CreateASTree() {
		//遍历全局中缀表达式
		try {
			for (int it = start; it <= end; ++it) {
				//处理Token和keyword
				(this->*funcMap[global_infix[it]->type])(it);

				//遇到分号跳出
				if (end_flag)break;
				else if (it == end) { //最后一个没有分号
					DealToken_Sem(it);
				}
			}

			if (root == NULL) ThrowException(SuatinErrorType_Value,start,end,"abstract syntax tree was none");
			/*
			judgeRoot <-> exprRoot
			logicRoot  <->  exprRoot
			exprRoot  <->  root 
			最后除了root外，其他指针都为空！
			*/

			//最后检查一下语法错误
			CheckASTreeSyntax();

		}
		catch (SuatinExcept& e) {
			PrintException(e.what());
		}
		catch (...) {
			PrintException("parser analysis error\n");
		}
	}

	void Parser::interpret() {


		try {

			//1.确定解释接口
			Confirm_ASTree_InterfaceType();

			//2.解释
			if (m_exprType == ExprType_Simple) { //简单表达式
				if (m_simpleExprType == SimpleExprType_NumCalc) { //五则计算式
					
					//先判断类型
					if(root->GetClassType() == SuatinExprClassType_IDExpr){ //表达式为单个ID
						IDExpr* node_tmp = dynamic_cast<IDExpr*>(root);
						if (node_tmp->GetType() == SuatinIDType_nil) {
#ifdef _SUATIN_DEBUG_
							std::cout << "["<<g_statement_index<<"][result]nil\n";
#endif
							g_statement_returnVal = false;//设置语句返回结果
							return;
						}
						else if (node_tmp->GetType() == SuatinIDType_bool) {
							g_statement_returnVal = node_tmp->GetBool();//设置语句返回结果
#ifdef _SUATIN_DEBUG_
							std::cout << "[" << g_statement_index << "][result]" << BOOL2STR(g_statement_returnVal) << std::endl;
#endif
							return;
						}
					}					

					double result = root == NULL ? 0 : root->interpret();
#ifdef _SUATIN_DEBUG_
					std::cout << "[" << g_statement_index << "][result]" << result << std::endl;
#endif
					g_statement_returnVal = true;//设置语句返回结果
				}
				else if (m_simpleExprType == SimpleExprType_StrLink) { //字符串拼接式
					std::string result = root == NULL ? "" : root->interpret_str();
					//std::cout << "[result]" << result << std::endl;
					g_statement_returnVal = true;//设置语句返回结果
				}
			}
			else { //判断式或逻辑式
				bool result = root == NULL ? true : root->interpret_bool();
#ifdef _SUATIN_DEBUG_
				std::cout << "[" << g_statement_index << "][result]" << BOOL2STR(result) << std::endl;
#endif
				g_statement_returnVal = result;//设置语句返回结果
			}

		}
		catch (SuatinExcept& e) {
			PrintException(e.what());
		}
		catch (...) {
			PrintException("interpret error\n");
		}
	}



	bool Parser::GetCompletedASTreeFlag()const { 
		return completedASTree_flag; 
	}








	/*----------------------------------------------------------------------------------------------------------------------------------------------------------show the  tree ----------------------------------------------------------------------------*/



	void Parser::ShowASTree() {

		if (exprRoot == NULL && root == NULL)return;
		//默认把flag设置true，当遍历遇到Expr时，再设置false
		completedASTree_flag = true;
		try {
			_fact_ShowASTree(root);
		}
		catch (SuatinExcept& e) {
			PrintException(e.what());
		}
	}




	//打印语法树时，遇到树枝节点的操作如下
#define DISPLAY_ASTREE_PROCESS(str,CLASS)   \
{ \
	std::cout << str;\
	CLASS* branch = dynamic_cast<CLASS*>(_node);\
	v_forDisplayASTree[_num] = 1;\
	_fact_ShowASTree(branch->GetLeft(), 1 + _num);\
	v_forDisplayASTree[_num] = 0;\
	_fact_ShowASTree(branch->GetRight(), 1 + _num);\
}



	void Parser::_fact_ShowASTree(Expr* _node, int _num) {
		if (_node == NULL)return;
		/*Linux风格的二叉树

			一行一行处理的，每一个节点都占据单独的一行！
			每次打印完当前节点的分支装饰+节点内容后，除了叶子节点外，下一个都要进入左子树！！！
			所以设置当前num的位置为1，迭代左子树时下一行的num加一！！！
					进入左子树后，当前num的位置一直是 |
			当左子树迭代完，当前num位置恢复为0，开始迭代右子树并且下一行num加一！！！
					进入右子树后，当前num的位置一直是空的

		*/
		//打印一些分支装饰
		if (_num > 0) {
			for (int i = 0; i < _num - 1; ++i) {
				//为1表示还在左子树中，为0表示左子树结束，处于右子树中
				std::cout << ((v_forDisplayASTree[i] == 1) ? "│   " : "    ");
			}			  
			//if (v_forDisplayASTree[_num - 1] == 2) std::cout << "└── "; //打印只有单个孩子的节点的分支
			//else 
			std::cout << ((v_forDisplayASTree[_num - 1] == 1) ? "├── " : "└── ");//打印两个孩子的分支。为1打印左节点分支，为0打印右节点分支          
		}



		//叶子节点
		if(_node->GetClassType() == SuatinExprClassType_IDExpr){
			IDExpr* branch = dynamic_cast<IDExpr*>(_node);
			std::cout << branch->GetName() << "\n";
			return;
		}
		else if(_node->GetClassType() == SuatinExprClassType_NumExpr){
			NumExpr* branch = dynamic_cast<NumExpr*>(_node);
			std::cout << branch->GetName() << "\n";
			return;
		}
		else if(_node->GetClassType() == SuatinExprClassType_StrExpr){
			StrExpr* branch = dynamic_cast<StrExpr*>(_node);
			std::cout << branch->GetName() << "\n";
			return;
		}

		//树枝节点
		if(_node->GetClassType() == SuatinExprClassType_AddExpr){
			DISPLAY_ASTREE_PROCESS("+\n", AddExpr);
		}
		else if(_node->GetClassType() == SuatinExprClassType_SubExpr){
			DISPLAY_ASTREE_PROCESS("-\n", SubExpr);
		}
		else if (_node->GetClassType() == SuatinExprClassType_MulExpr) {
			DISPLAY_ASTREE_PROCESS("*\n", MulExpr);
		}
		else if (_node->GetClassType() == SuatinExprClassType_DivExpr) {
			DISPLAY_ASTREE_PROCESS("/\n", DivExpr);
		}
		else if (_node->GetClassType() == SuatinExprClassType_PowExpr) {
			DISPLAY_ASTREE_PROCESS("^\n", PowExpr);
		}
		else if (_node->GetClassType() == SuatinExprClassType_LLeftExpr) {//左括号节点
			std::cout << "()\n";
			LLeftExpr* branch = dynamic_cast<LLeftExpr*>(_node);

			if (branch->GetMatchedFlag() == false) {
				completedASTree_flag = false;
				ThrowException(SuatinErrorType_Syntax , start,end,"the left didnot matched its right part");
				return;
			}

			v_forDisplayASTree[_num] = 0;
			_fact_ShowASTree(branch->GetContent(), 1 + _num);
			v_forDisplayASTree[_num] = 0;
		}
		else if (_node->GetClassType() == SuatinExprClassType_NotExpr) { //NOT节点
			std::cout << "not\n";
			NotExpr* branch = dynamic_cast<NotExpr*>(_node);

			v_forDisplayASTree[_num] = 0;
			_fact_ShowASTree(branch->GetContent(), 1 + _num);
			v_forDisplayASTree[_num] = 0;
		}
		else if (_node->GetClassType() == SuatinExprClassType_AndExpr) {
			DISPLAY_ASTREE_PROCESS("and\n", AndExpr);
		}
		else if (_node->GetClassType() == SuatinExprClassType_OrExpr) {
			DISPLAY_ASTREE_PROCESS("or\n", OrExpr);
		}
		else if (_node->GetClassType() == SuatinExprClassType_EqExpr) {
			DISPLAY_ASTREE_PROCESS("=\n", EqExpr);
		}
		else if (_node->GetClassType() == SuatinExprClassType_GreExpr) {
			DISPLAY_ASTREE_PROCESS(">\n", GreExpr);
		}
		else if (_node->GetClassType() == SuatinExprClassType_GreEqExpr) {
			DISPLAY_ASTREE_PROCESS(">=\n", GreEqExpr);
		}
		else if (_node->GetClassType() == SuatinExprClassType_LesExpr) {
			DISPLAY_ASTREE_PROCESS("<\n", LesExpr);
		}
		else if (_node->GetClassType() == SuatinExprClassType_LesEqExpr) {
			DISPLAY_ASTREE_PROCESS("<=\n", LesEqExpr);
		}
		else if (_node->GetClassType() == SuatinExprClassType_NeqExpr) {
			DISPLAY_ASTREE_PROCESS("~=\n", NeqExpr);
		}
		else if (_node->GetClassType() == SuatinExprClassType_EqEqExpr) {
			DISPLAY_ASTREE_PROCESS("==\n", EqEqExpr);
		}
		else { //还存在表达式没构造成树
			completedASTree_flag = false;
		}
	}

#undef DISPLAY_ASTREE_PROCESS   ///这个宏只在打印语法树时起作用








	/*-----------------------------------------------------------------------------------------------------------------VarExpr sign------------------------------------------------------------------------------------------------*/


	void Parser::DealToken_Num(int& _t) {
		//数字不能存在于【字符串拼接模式】的表达式中
		if (m_simpleExprType == SimpleExprType_StrLink)return;


		//存在待处理的节点-------------------
		if (expTmp) {
			if(expTmp->GetClassType() == SuatinExprClassType_LLeftExpr){//待处理节点是左括号节点
				/*
				  (    ->    (
							   |
							 Num
				*/
				LLeftExpr* node_tmp = dynamic_cast<LLeftExpr*>(expTmp);
				node_tmp->SetContent(new NumExpr(global_infix[_t]->name));
			}
			else {
				/*
						+            ->          +
					  /     \                    /     \
				Num     None      Num     Num
				*/
				SymbolExpr* node_tmp = dynamic_cast<SymbolExpr*>(expTmp);
				node_tmp->SetRight(new NumExpr(global_infix[_t]->name));
				expTmp = expTmpLeft;//上移待处理节点的指针
			}
			return;
		}

		//不存在待处理的节点-------------------
		exprRoot = new NumExpr(global_infix[_t]->name);

	}
	void Parser::DealToken_Id(int& _t) {
		//处理关键字
		if (global_infix[_t]->k_type != SuatinKeyWordType_NOT_KEYWORD) {
			(this->*k_funcMap[global_infix[_t]->k_type])(_t);
			return;
		}


		//字符串连接模式下
		if (m_simpleExprType == SimpleExprType_StrLink) {
			/*即出现了这种情况："xxx"+a
				其中a是字符串类型
				      +           ->          +
				    /    \                     /   \
			"xxx"      None     "xxx"    a
			*/
			if (expTmp == NULL) {//不存在待处理的节点
				ThrowException(SuatinErrorType_Syntax,start,end,"[Id] string connect expression wrong");
				return;
			}
			if ( SuatinEnv[global_infix[_t]->name]->type != SuatinIDType_string) { //字符串连接模式下，其他类型的Id不做处理
				return;
			}
			
			SymbolExpr* node_tmp = dynamic_cast<SymbolExpr*>(expTmp);
			node_tmp->SetRight(new IDExpr(global_infix[_t]->name));
			expTmp = NULL;
			return;
		}



		

		//存在待处理的节点------------------
		if (expTmp) {
			if (SuatinEnv[global_infix[_t]->name]->type == SuatinIDType_bool) { //五则计算模式下，他类型的Id不做处理
				ThrowException(SuatinErrorType_Syntax,start, end, "[Id] boolean identifier cannot used for calculate");
				return;
			}
			if (expTmp->GetClassType() == SuatinExprClassType_LLeftExpr) {//待处理节点是左括号节点
				LLeftExpr* node_tmp = dynamic_cast<LLeftExpr*>(expTmp);
				node_tmp->SetContent(new IDExpr(global_infix[_t]->name));
			}
			else {
				SymbolExpr* node_tmp = dynamic_cast<SymbolExpr*>(expTmp);
				node_tmp->SetRight(new IDExpr(global_infix[_t]->name));
				expTmp = expTmpLeft;//上移待处理节点的指针
			}
			return;
		}

		


		//不存在待处理的节点--------------
		exprRoot = new IDExpr(global_infix[_t]->name);
		
		//检查一下，表达式的开头第一个id的类型
		if (firstId_flag) {
			if (SuatinEnv[global_infix[_t]->name]->type == SuatinIDType_string) {
				m_simpleExprType = SimpleExprType_StrLink;//如果是字符串类型就转变表达式类型为字符串拼接模式
				firstId_flag = false;
			}
			else if (SuatinEnv[global_infix[_t]->name]->type == SuatinIDType_bool) {
				if (m_exprType == ExprType_Simple) {
					m_exprType = ExprType_Compare;//如果是布尔类型，表达式类型至少是判断式。这样就不会把当做简单表达式中的计算模式被interpret接口处理了
				}
				firstId_flag = false;
			}
		}
		

	}


	void Parser::DealToken_Str(int& _t) {

		if (exprRoot == NULL)//【第一个】Token是字符串，进入字符串拼接模式
			m_simpleExprType = SimpleExprType_StrLink;


		if (m_simpleExprType == SimpleExprType_StrLink) { //进入字符串拼接模式
			if (exprRoot == NULL) {
				exprRoot = new StrExpr(global_infix[_t]->name);
				return;
			}

			//存在待处理的节点-------------------
			//if (expTmp && typeid(*(expTmp->GetClassType())) == typeid(AddExpr)) {
			if(expTmp != NULL && expTmp->GetClassType() == SuatinExprClassType_AddExpr){
				StrExpr* tmp = new StrExpr(global_infix[_t]->name);
				AddExpr* exprRoot_tmp = dynamic_cast<AddExpr*>(exprRoot);
				exprRoot_tmp->SetRight(tmp);
				expTmp = NULL;
				return;
			}

			//不存在待处理节点或者待处理节点不是加号都要抛出异常
		}
		//计算模式中出现字符串要抛出异常
		ThrowException(SuatinErrorType_Syntax,start, end, "[str] a string in calculate expression");


	}



	/*-----------------------------------------------------------------------------------------------------------------SymbolExpr Calculate sign------------------------------------------------------------------------------------------------*/



	void Parser::DealToken_Pow(int& _t) {
		if (m_simpleExprType == SimpleExprType_StrLink) { //字符串连接模式
			ThrowException(SuatinErrorType_Syntax,start, end, "[pow] wrong expression in string connection process");
			return;
		}
		if (exprRoot == NULL) { //缺少左操作数
			ThrowException(SuatinErrorType_Syntax,start, end, "[pow] operate process have not left operator");
			return;
		}




		/*
				第一种情况
					 (              ->         (
					 |                          |
				 Num                       ^
											 /    \
										Num   None
				第二种情况
						 (           ->         (
						 |                        |
						 ^                       ^
					   /   \                    /    \
			   Num     Num      Num     ^
													 /   \
											Num      None
		*/

		auto leftRightConnect = [this](Expr** _node) {
			//【根节点】是Num、LLeft、Id
			if( (*_node)->GetClassType() == SuatinExprClassType_NumExpr || (*_node)->GetClassType() == SuatinExprClassType_IDExpr || (*_node)->GetClassType() == SuatinExprClassType_LLeftExpr ){

				//检查左边的ID是否是布尔，布尔是不能用来计算的
				if ((*_node)->GetClassType() == SuatinExprClassType_IDExpr) {
					IDExpr* node_tmp = dynamic_cast<IDExpr*>(*_node);
					if (node_tmp->GetType() == SuatinIDType_bool) {
						ThrowException(SuatinErrorType_Syntax,start,end,"[pow] boolean identifier cannot used for calculate");
						return;
					}
				}

				(*_node) = new PowExpr((*_node), NULL);
				expTmp = (*_node);
			}
			//【根节点】是加减、乘除、乘方				
			else {
				//找到最右的Num、Id、LLeft的父节点
				SymbolExpr* exprRootTmp = dynamic_cast<SymbolExpr*>((*_node));
				while( exprRootTmp->GetRight()->GetClassType() != SuatinExprClassType_NumExpr  && exprRootTmp->GetRight()->GetClassType() != SuatinExprClassType_IDExpr && exprRootTmp->GetRight()->GetClassType() != SuatinExprClassType_LLeftExpr){
					SymbolExpr* exprRootTmp2 = dynamic_cast<SymbolExpr*>(exprRootTmp->GetRight());
					exprRootTmp = exprRootTmp2;
				}
				
				Expr* tmp = new PowExpr(exprRootTmp->GetRight(), NULL);
				exprRootTmp->SetRight(tmp);
				expTmp = tmp;

			}

		};




		//待处理节点存在-------------------
		if (expTmp) {
			if(expTmp->GetClassType() == SuatinExprClassType_LLeftExpr){ //待处理节点是左括号节点				
				LLeftExpr* node_tmp = dynamic_cast<LLeftExpr*>(expTmp);
				Expr* node_tmp2 = node_tmp->GetContent();
				leftRightConnect(&node_tmp2);//把左括号节点中的内容当做是一个新树来处理				
				node_tmp->SetContent(node_tmp2);//装上新树

			}
			else 	leftRightConnect(&expTmp);
			return;
		}

		//待处理节点不存在-------------------
		leftRightConnect(&exprRoot);

	}




	void Parser::DealToken_Mul(int& _t) {
		if (m_simpleExprType == SimpleExprType_StrLink) { //字符串拼接模式
			ThrowException(SuatinErrorType_Syntax,start, end, "[mul] wrong expression in string connection process");
			return;
		}
		if (exprRoot == NULL) { //没有左操作数
			ThrowException(SuatinErrorType_Syntax,start, end, "[mul] multiply operate process have not left operator");
			return;
		}




		auto leftRightConnect = [this](Expr** _node) {

			//【根节点】是加减
			if((*_node)->GetClassType() == SuatinExprClassType_AddExpr || (*_node)->GetClassType() == SuatinExprClassType_SubExpr){
				SymbolExpr* exprRootTmp = dynamic_cast<SymbolExpr*>((*_node));
				Expr* tmp = new MulExpr(exprRootTmp->GetRight(), NULL);
				exprRootTmp->SetRight(tmp);
				expTmp = tmp;
			}
			//【根节点】是乘除、乘方、Num、ID
			else {

				//检查左边的ID是否是布尔，布尔是不能用来计算的
				if ((*_node)->GetClassType() == SuatinExprClassType_IDExpr) {
					IDExpr* node_tmp = dynamic_cast<IDExpr*>(*_node);
					if (node_tmp->GetType() == SuatinIDType_bool) {
						ThrowException(SuatinErrorType_Syntax,start, end, "[pow] boolean identifier cannot used for calculate");
						return;
					}
				}

				(*_node) = new MulExpr((*_node), NULL);
				expTmp = (*_node);
			}

		};


		//待处理节点存在-------------------
		if (expTmp) {
			if (expTmp->GetClassType() == SuatinExprClassType_LLeftExpr) { //待处理节点是左括号节点	
				LLeftExpr* node_tmp = dynamic_cast<LLeftExpr*>(expTmp);
				Expr* node_tmp2 = node_tmp->GetContent();
				leftRightConnect(&node_tmp2);//把左括号节点中的内容当做是一个新树来处理				
				node_tmp->SetContent(node_tmp2);//装上新树，如果不把新树装上去，那么原来的内容还在那！经过lambda传入双指针操作后，node_tmp2就已经不是node_tmp的内容了！！！

			}
			else 	leftRightConnect(&expTmp);
			return;
		}

		//待处理节点不存在-------------------
		leftRightConnect(&exprRoot);
	}




	void Parser::DealToken_Div(int& _t) {
		if (m_simpleExprType == SimpleExprType_StrLink) {//字符串拼接模式
			ThrowException(SuatinErrorType_Syntax,start, end, "[div] wrong expression in string connection process");
			return;
		}
		if (exprRoot == NULL) {//没有左操作数
			ThrowException(SuatinErrorType_Syntax,start, end, "[div] divide operate process have not left operator");
			return;
		}


		auto leftRightConnect = [this](Expr** _node) {

			//【根节点】是加减
			if ((*_node)->GetClassType() == SuatinExprClassType_AddExpr || (*_node)->GetClassType() == SuatinExprClassType_SubExpr) {
				SymbolExpr* exprRootTmp = dynamic_cast<SymbolExpr*>((*_node));
				Expr* tmp = new DivExpr(exprRootTmp->GetRight(), NULL);
				exprRootTmp->SetRight(tmp);
				expTmp = tmp;
			}
			//【根节点】是乘除、乘方、Num、ID
			else {

				//检查左边的ID是否是布尔，布尔是不能用来计算的
				if ((*_node)->GetClassType() == SuatinExprClassType_IDExpr) {
					IDExpr* node_tmp = dynamic_cast<IDExpr*>(*_node);
					if (node_tmp->GetType() == SuatinIDType_bool) {
						ThrowException(SuatinErrorType_Syntax,start, end, "[pow] boolean identifier cannot used for calculate");
						return;
					}
				}

				(*_node) = new DivExpr((*_node), NULL);
				expTmp = (*_node);
			}

		};


		//待处理节点存在-------------------
		if (expTmp) {
			if (expTmp->GetClassType() == SuatinExprClassType_LLeftExpr) { //待处理节点是左括号节点	
				LLeftExpr* node_tmp = dynamic_cast<LLeftExpr*>(expTmp);
				Expr* node_tmp2 = node_tmp->GetContent();
				leftRightConnect(&node_tmp2);//把左括号节点中的内容当做是一个新树来处理				
				node_tmp->SetContent(node_tmp2);//装上新树

			}
			else 	leftRightConnect(&expTmp);
			return;
		}


		//待处理节点不存在-------------------
		leftRightConnect(&exprRoot);

	}



	void Parser::DealToken_Add(int& _t) {
		if (m_simpleExprType == SimpleExprType_StrLink) {//字符串拼接模式
			exprRoot = new AddExpr(exprRoot, NULL);
			expTmp = exprRoot;
			return;
		}
		if (exprRoot == NULL) { //没有左操作数，前面补个零
			exprRoot = new AddExpr(new NumExpr("0"), NULL);
			expTmp = exprRoot;
			return;
		}



		auto leftConnect = [this](Expr** _node) {

			//检查左边的ID是否是布尔，布尔是不能用来计算的
			if ((*_node)->GetClassType() == SuatinExprClassType_IDExpr) {
				IDExpr* node_tmp = dynamic_cast<IDExpr*>(*_node);
				if (node_tmp->GetType() == SuatinIDType_bool) {
					ThrowException(SuatinErrorType_Syntax,start, end, "[pow] boolean identifier cannot used for calculate");
					return;
				}
			}

			(*_node) = new AddExpr((*_node), NULL);
			expTmp = (*_node);
		};


		//存在待处理节点-------------------
		if (expTmp) {
			if (expTmp->GetClassType() == SuatinExprClassType_LLeftExpr) { //待处理节点是左括号节点	
				LLeftExpr* node_tmp = dynamic_cast<LLeftExpr*>(expTmp);
				Expr* node_tmp2 = node_tmp->GetContent();
				leftConnect(&node_tmp2);//把左括号节点中的内容当做是一个新树来处理				
				node_tmp->SetContent(node_tmp2);//装上新树

			}
			else 	leftConnect(&expTmp);
			return;
		}

		//不存在待处理节点-------------------
		leftConnect(&exprRoot);
	}



	void Parser::DealToken_Sub(int& _t) {
		if (m_simpleExprType == SimpleExprType_StrLink) {//字符串拼接模式
			ThrowException(SuatinErrorType_Syntax,start, end, "[sub] wrong expression in string connection process");
			return;
		}
		if (exprRoot == NULL) { //没有左操作数,前面补个零
			exprRoot = new SubExpr(new NumExpr("0"), NULL);
			expTmp = exprRoot;
			return;
		}



		auto leftConnect = [this](Expr** _node) {

			//检查左边的ID是否是布尔，布尔是不能用来计算的
			if ((*_node)->GetClassType() == SuatinExprClassType_IDExpr) {
				IDExpr* node_tmp = dynamic_cast<IDExpr*>(*_node);
				if (node_tmp->GetType() == SuatinIDType_bool) {
					ThrowException(SuatinErrorType_Syntax,start, end, "[pow] boolean identifier cannot used for calculate");
					return;
				}
			}

			(*_node) = new SubExpr((*_node), NULL);
			expTmp = (*_node);
		};


		//存在待处理节点----------------
		if (expTmp) {
			if (expTmp->GetClassType() == SuatinExprClassType_LLeftExpr) { //待处理节点是左括号节点	
				LLeftExpr* node_tmp = dynamic_cast<LLeftExpr*>(expTmp);
				Expr* node_tmp2 = node_tmp->GetContent();
				leftConnect(&node_tmp2);//把左括号节点中的内容当做是一个新树来处理				
				node_tmp->SetContent(node_tmp2);//装上新树

			}
			else leftConnect(&expTmp);
			return;
		}

		//不存在待处理节点------------------
		leftConnect(&exprRoot);
	}




	/*-----------------------------------------------------------------------------------------------------------------  judge operate sign---------------------------------------------------------------------------------------------------------------------*/



	void Parser::DealToken_Gre(int& _t) {
		if (  m_simpleExprType == SimpleExprType_StrLink ) { //字符串拼接模式
			ThrowException(SuatinErrorType_Syntax,start, end, "[>] wrong expression");
			return;
		}		
		if (exprRoot == NULL) {//没有左操作数
			ThrowException(SuatinErrorType_Syntax,start, end, "[>]  have not left operator");
			return;
		}

		//表达式转为判断式
		if (m_exprType == ExprType_Simple) {
			m_exprType = ExprType_Compare;
		}			   
		//初始化表达式（简单表达式的模式、firstId）
		m_simpleExprType = SimpleExprType_NumCalc;
		firstId_flag = true;


		//存在待处理的节点-----------------
		if (expTmp) {
			ThrowException(SuatinErrorType_Syntax,start, end, "[>] wrong expression");
			expTmp = NULL;
			return;
		}

		//不存在待处理的节点------------------
		//检查左边的ID是否是布尔，布尔是不能用来计算的
		if(exprRoot->GetClassType() == SuatinExprClassType_IDExpr){
			IDExpr* node_tmp = dynamic_cast<IDExpr*>(exprRoot);
			if (node_tmp->GetType() == SuatinIDType_bool) {
				ThrowException(SuatinErrorType_Syntax,start, end, "[>] boolean identifier cannot used for compare");
				return;
			}
		}
		judgeRoot = new GreExpr(exprRoot, NULL);
		exprRoot = NULL;
	}



	void Parser::DealToken_GreEq(int& _t) {
		if (m_simpleExprType == SimpleExprType_StrLink) { //字符串拼接模式
			ThrowException(SuatinErrorType_Syntax,start, end, "[>=] wrong expression");
			return;
		}
		if (exprRoot == NULL) {//没有左操作数
			ThrowException(SuatinErrorType_Syntax,start, end, "[>=]  have not left operator");
			return;
		}

		//表达式转为判断式
		if (m_exprType == ExprType_Simple) {
			m_exprType = ExprType_Compare;
		}
		//初始化表达式（简单表达式的模式、firstId）
		m_simpleExprType = SimpleExprType_NumCalc;
		firstId_flag = true;


		//存在待处理的节点-----------------
		if (expTmp) {
			ThrowException(SuatinErrorType_Syntax,start, end, "[>=] wrong expression");
			expTmp = NULL;
			return;
		}

		//不存在待处理的节点------------------
		//检查左边的ID是否是布尔，布尔是不能用来计算的
		if (exprRoot->GetClassType() == SuatinExprClassType_IDExpr) {
			IDExpr* node_tmp = dynamic_cast<IDExpr*>(exprRoot);
			if (node_tmp->GetType() == SuatinIDType_bool) {
				ThrowException(SuatinErrorType_Syntax,start, end, "[>=] boolean identifier cannot used for compare");
				return;
			}
		}
		judgeRoot = new GreEqExpr(exprRoot, NULL);
		exprRoot = NULL;
	}




	void Parser::DealToken_Les(int& _t) {
		if (m_simpleExprType == SimpleExprType_StrLink) { //字符串拼接模式
			ThrowException(SuatinErrorType_Syntax,start, end, "[<] wrong expression");
			return;
		}
		if (exprRoot == NULL) {//没有左操作数
			ThrowException(SuatinErrorType_Syntax,start, end, "[<]  have not left operator");
			return;
		}

		//表达式转为判断式
		if (m_exprType == ExprType_Simple) {
			m_exprType = ExprType_Compare;
		}
		//初始化表达式（简单表达式的模式、firstId）
		m_simpleExprType = SimpleExprType_NumCalc;
		firstId_flag = true;


		//存在待处理的节点-----------------
		if (expTmp) {
			ThrowException(SuatinErrorType_Syntax,start, end, "[<] wrong expression");
			expTmp = NULL;
			return;
		}

		//不存在待处理的节点------------------
		//检查左边的ID是否是布尔，布尔是不能用来计算的
		if (exprRoot->GetClassType() == SuatinExprClassType_IDExpr) {
			IDExpr* node_tmp = dynamic_cast<IDExpr*>(exprRoot);
			if (node_tmp->GetType() == SuatinIDType_bool) {
				ThrowException(SuatinErrorType_Syntax,start, end, "[<] boolean identifier cannot used for compare");
				return;
			}
		}
		judgeRoot = new LesExpr(exprRoot, NULL);
		exprRoot = NULL;
	}



	void Parser::DealToken_LesEq(int& _t) {
		if (m_simpleExprType == SimpleExprType_StrLink) { //字符串拼接模式
			ThrowException(SuatinErrorType_Syntax,start, end, "[<=] wrong expression");
			return;
		}
		if (exprRoot == NULL) {//没有左操作数
			ThrowException(SuatinErrorType_Syntax,start, end, "[<=]  have not left operator");
			return;
		}

		//表达式转为判断式
		if (m_exprType == ExprType_Simple) {
			m_exprType = ExprType_Compare;
		}
		//初始化表达式（简单表达式的模式、firstId）
		m_simpleExprType = SimpleExprType_NumCalc;
		firstId_flag = true;


		//存在待处理的节点-----------------
		if (expTmp) {
			ThrowException(SuatinErrorType_Syntax,start, end, "[<=] wrong expression");
			expTmp = NULL;
			return;
		}

		//不存在待处理的节点------------------
		//检查左边的ID是否是布尔，布尔是不能用来计算的
		if (exprRoot->GetClassType() == SuatinExprClassType_IDExpr) {
			IDExpr* node_tmp = dynamic_cast<IDExpr*>(exprRoot);
			if (node_tmp->GetType() == SuatinIDType_bool) {
				ThrowException(SuatinErrorType_Syntax,start, end, "[<=] boolean identifier cannot used for compare");
				return;
			}
		}
		judgeRoot = new LesEqExpr(exprRoot, NULL);
		exprRoot = NULL;
	}



	void Parser::DealToken_Neq(int& _t) {
		if (exprRoot == NULL) {//没有左操作数
			ThrowException( SuatinErrorType_Syntax,start, end, "[~=]  have not left operator");
			return;
		}

		//表达式转为判断式
		if(m_exprType == ExprType_Simple){
			m_exprType = ExprType_Compare;
		}
		//初始化表达式（简单表达式的模式、firstId）
		m_simpleExprType = SimpleExprType_NumCalc;
		firstId_flag = true;


		//存在待处理的节点------------------
		if (expTmp) {
			if( expTmp->GetClassType() != SuatinExprClassType_AddExpr ){
				ThrowException( SuatinErrorType_Syntax,start, end, "[~=] wrong expression");
				expTmp = NULL;
				return;
			}
		}

		//不存在待处理的节点--------------------
		judgeRoot = new NeqExpr(exprRoot, NULL);
		exprRoot = NULL;
	
	}



	void Parser::DealToken_EqEq(int& _t) {
		if (exprRoot == NULL) {//没有左操作数
			ThrowException( SuatinErrorType_Syntax,start, end, "[==]  have not left operator");
			return;
		}

		//表达式转为判断式
		if (m_exprType == ExprType_Simple) {
			m_exprType = ExprType_Compare;
		}
		//初始化表达式（简单表达式的模式、firstId）
		m_simpleExprType = SimpleExprType_NumCalc;
		firstId_flag = true;


		//存在待处理的节点--------------------
		if (expTmp) {
			if (expTmp->GetClassType() != SuatinExprClassType_AddExpr) {
				ThrowException( SuatinErrorType_Syntax,start, end, "[==] wrong expression");
				expTmp = NULL;
				return;
			}
		}

		//不存在待处理的节点----------------------
		judgeRoot = new EqEqExpr(exprRoot, NULL);
		exprRoot = NULL;
		
	}










	void Parser::DealToken_Eq(int& _t) {
		if (exprRoot == NULL||expTmp) { //没有左操作数或存在待处理节点
			ThrowException( SuatinErrorType_Syntax, start, end, "[=] wrong expression");
			expTmp = NULL;
			return;
		}


		if( exprRoot->GetClassType() == SuatinExprClassType_IDExpr){
			//等号左边不能是常数
			IDExpr* check_node = dynamic_cast<IDExpr*>(exprRoot);
			/*if (SuatinEnv[check_node->GetName()]->isconst) { //isconst已经被删除了！！！
				ThrowException( SuatinErrorType_Syntax , start, end, "[=] left operator cannot be constant");
				return;
			}*/
		}
		else { //等号左边不是ID就抛出异常
			ThrowException( SuatinErrorType_Syntax, start, end, "[=] left operator isnot id");
			return;
		}


		/*
	    第一种情况：
		a      ->        =
		                  /   \
						a      None

		第二种情况：
		        =      +  b     ->           =
			   /  \                             /    \
			 a    None                  a      =
			                                         /   \
												   b    None
		*/
		
		//第一次遇到等号，并且左边就是一个标识符
		if (firstEq_flag==true) {
			root = new EqExpr(exprRoot, NULL);
			m_statType = StatType_Eval;//语句变为赋值式
			exprRoot = NULL;
			firstEq_flag = false;
			return;
		}


		//第N次遇到等号，找到最右的等号节点，取下，换上新小树
		//if (typeid(*(root->GetClassType())) == typeid(EqExpr)) {
		if(root->GetClassType() == SuatinExprClassType_EqExpr){
			SymbolExpr* eq = dynamic_cast<SymbolExpr*>(root);
			while(eq->GetRight() != NULL  && eq->GetRight()->GetClassType() == SuatinExprClassType_EqExpr){
				SymbolExpr* eq2 = dynamic_cast<SymbolExpr*>(eq->GetRight());
				eq = eq2;
			}
			//找到最右边的等于号节点eq
			EqExpr* tmp = new EqExpr(exprRoot, NULL);
			eq->SetRight(tmp);
			exprRoot = NULL;
			return;
		}
		

		
		ThrowException( SuatinErrorType_Syntax, start, end, "[=] wrong expression");

	}

	
	





	/*-----------------------------------------------------------------------------------------------------------------  ERROR  ------------------------------------------------------------------------------------------------*/

	void Parser::DealToken_Com(int& _t) {
		ThrowException(SuatinErrorType_Syntax,start, end, "[comma] wrong expression");
	}
	void Parser::DealToken_ML(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[ML] wrong expression");
	}
	void Parser::DealToken_MR(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[MR] wrong expression");
	}
	void Parser::DealToken_BL(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[BL] wrong expression");
	}
	void Parser::DealToken_BR(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[BR] wrong expression");
	}
	void Parser::DealToken_Dot(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[Dot] wrong expression");
	}
	void Parser::Deal_k_if(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[if] wrong expression");
	}
	void Parser::Deal_k_elif(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[elif] wrong expression");
	}
	void Parser::Deal_k_else(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[else] wrong expression");
	}
	void Parser::Deal_k_for(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[for] wrong expression");
	}

	void Parser::Deal_k_do(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[do] wrong expression");
	}
	void Parser::Deal_k_until(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[until] wrong expression");
	}
	void Parser::Deal_k_while(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[while] wrong expression");
	}
	void Parser::Deal_k_local(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[local] wrong expression");
	}
	void Parser::Deal_k_const(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[const] wrong expression");
	}
	void Parser::Deal_k_function(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[function] wrong expression"); 
	}
	void Parser::Deal_k_end(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[end] wrong expression");
	}
	void Parser::Deal_k_return(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[return] wrong expression");
	}
	
	void Parser::Deal_k_break(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[break] wrong expression");
	}
	void Parser::Deal_k_continue(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[continue] wrong expression");
	}
	void Parser::Deal_k_assert(int& _t) {
		ThrowException(SuatinErrorType_Syntax, start, end, "[assert] wrong expression");
	}




	/*-----------------------------------------------------------------------------------------------------------------   Zone operate sign  ------------------------------------------------------------------------------------------------*/

	void Parser::DealToken_LL(int& _t) {
		if ( m_simpleExprType == SimpleExprType_StrLink ) { //字符串拼接模式
			ThrowException(SuatinErrorType_Syntax,start, end, "[LL] wrong expression in string connection process");
			return;
		}


		/*每次遇到一个左括号，新建一个左括号节点，让expTmpLeft保留这个节点！
			在匹配到右括号前，都把余下的树装在这个节点下！除非遇到新的左括号，
			那时，expTmpLeft会下移到该节点上，在该左括号节点下装余下的子树！！！
		*/

		//第一次出现左括号
		if (exprRoot == NULL) {
			exprRoot = new LLeftExpr();
			expTmpLeft = exprRoot;
			expTmp = exprRoot;
			v_NoMatchedLLeft.push(expTmpLeft);//保存左括号节点，用于expTmpLeft遍历
			return;
		}



		//有待处理的节点---------------
		if (expTmp) {
			if(expTmp->GetClassType() == SuatinExprClassType_LLeftExpr){ //待处理节点是左括号节点
				/*
				  (   ->    (
							 |
							 (
				*/
				LLeftExpr* node_tmp = dynamic_cast<LLeftExpr*>(expTmp);
				expTmpLeft = new LLeftExpr();//最底层的左括号指针下移
				node_tmp->SetContent(expTmpLeft);

			}
			//待处理节点是加减乘除、乘方
			else {
				/*
						+             ->          +
					  /     \                      /   \
				Num    None        Num     (
				*/
				SymbolExpr* node_tmp = dynamic_cast<SymbolExpr*>(expTmp);
				expTmpLeft = new LLeftExpr();//最底层的左括号指针下移
				node_tmp->SetRight(expTmpLeft);
			}


			v_NoMatchedLLeft.push(expTmpLeft);//保存左括号节点，用于expTmpLeft遍历
			expTmp = expTmpLeft;//待处理节点指针下移
			return;
		}

		//没有待处理的节点---------------
		//说明语法树完整，左括号和数字间缺少运算符！！！
		ThrowException(SuatinErrorType_Syntax,start, end, "[LL] wrong expression");


	}


	void Parser::DealToken_LR(int& _t) {
		if (m_simpleExprType == SimpleExprType_StrLink) { //字符串拼接模式
			ThrowException(SuatinErrorType_Syntax,start, end, "[LR] wrong expression in string connection process");
			return;
		}
		if (exprRoot == NULL) { //括号内没有内容
			ThrowException(SuatinErrorType_Syntax,start, end, "[LR] wrong expression");
			expTmp = NULL;
			return;
		}


	


		//有待处理的节点--------------------
		if (expTmp) {
			if (expTmp->GetClassType() == SuatinExprClassType_LLeftExpr) { //待处理节点是左括号节点

				LLeftExpr* node_tmp = dynamic_cast<LLeftExpr*>(expTmpLeft);
				if (node_tmp->GetMatchedFlag()) {//如果该左括号节点已经匹配到了右括号
					ThrowException(SuatinErrorType_Syntax,start, end, "[LR] wrong expression");
					return;
				}
				node_tmp->SetMatchedFlag();//匹配到右括号


				//检查此左括号节点内是否有内容！！！
				if (node_tmp->GetContent() == NULL) {
					ThrowException(SuatinErrorType_Syntax,start,end,"[LR] none content in parenthesis");
					return;
				}


				//(最底层的没有匹配到右括号的)指针上移
				while (v_NoMatchedLLeft.size() > 0) {   //这个数组中存放的是一条链上的左括号，而链是变化的，所以这个数组也应该是变化的
					v_NoMatchedLLeft.pop();
					if (v_NoMatchedLLeft.size() == 0)break;

					expTmpLeft = v_NoMatchedLLeft.top();
					LLeftExpr* node_tmp = dynamic_cast<LLeftExpr*>(expTmpLeft);
					if (node_tmp->GetMatchedFlag() == false) {
						expTmp = expTmpLeft;
						return;//找到了
					}
				}
				expTmpLeft = expTmp = NULL;

				return;//没有找到没匹配的
			}
		}


		//没有待处理节点--------------------
		ThrowException(SuatinErrorType_Syntax,start, end, "[LR] wrong expression");

	}





	void Parser::DealToken_Sem(int& _t) {

		end_flag = true;//遇到了分号，可以结束了

		if (exprRoot == NULL) {
			ThrowException(SuatinErrorType_Syntax,start, end, "[sem] wrong expression");
			return;
		}



		auto judge_chunk = [this](Expr** _node,Expr** _exprRoot) {
			//>,<,>=,<=这几个节点的情况下，左右操作数不能是布尔
			if((*_node)->GetClassType() == SuatinExprClassType_GreExpr || (*_node)->GetClassType() == SuatinExprClassType_GreEqExpr || (*_node)->GetClassType() == SuatinExprClassType_LesExpr || (*_node)->GetClassType() == SuatinExprClassType_LesEqExpr  ){
			
				//检查右边的ID是否是布尔，布尔是不能用来计算的
				if((*_exprRoot)->GetClassType() == SuatinExprClassType_IDExpr ){ 
					IDExpr* node_tmp = dynamic_cast<IDExpr*>((*_exprRoot));
					if (node_tmp->GetType() == SuatinIDType_bool) {
						ThrowException(SuatinErrorType_Syntax,start, end, "[sem] boolean identifier cannot used for compare");
						return;
					}
				}
			}

			//替换judgeRoot
			SymbolExpr* node_tmp = dynamic_cast<SymbolExpr*>((*_node));
			node_tmp->SetRight((*_exprRoot));

			(*_exprRoot) = (*_node);//根节点上移
			(*_node) = NULL;

		};


		//1.判断模式下，将exprRoot下的语法树，接到judgeRoot右孩子上
		//再把exprRoot替换掉judgeRoot
		if (m_exprType == ExprType_Compare  && judgeRoot) {
			judge_chunk(&judgeRoot,&exprRoot);
		}




		
		//2.逻辑模式下，将exprRoot下的语法树，接到judgeRoot右孩子上
		//再把exprRoot替换掉judgeRoot
		//将exprRoot下的语法树，接到 logicRoot右孩子上
		//再把exprRoot替换掉 logicRoot
		if (m_exprType == ExprType_Logic && logicRoot) {
	
			//1.替换judgeRoot-----------
			if (judgeRoot) {
				judge_chunk(&judgeRoot, &exprRoot);
			}




			auto logic_chunk = [this](Expr** _node) {
				if((*_node)->GetClassType() == SuatinExprClassType_AndExpr){ //【根节点】是And

					//将exprRoot装上
					AndExpr* root_tmp = dynamic_cast<AndExpr*>(*_node);
					root_tmp->SetRight(exprRoot);					
				}
				else if((*_node)->GetClassType() == SuatinExprClassType_OrExpr){ //【根节点】是Or

					//确定【根节点】右边的解释接口类型
					OrExpr* root_tmp = dynamic_cast<OrExpr*>(*_node);
					if (root_tmp->GetRight() == NULL) {
						//将exprRoot装上
						root_tmp->SetRight(exprRoot);
					}
					else {
						AndExpr* root_tmp2 = dynamic_cast<AndExpr*>(root_tmp->GetRight());
						//将exprRoot装上
						root_tmp2->SetRight(exprRoot);
					}
				}
				else {
					ThrowException(SuatinErrorType_Syntax,start, end, "[sem] error in sem sign process");
				}
			};





			//2.替换logicRoot------------
			if(logicRoot->GetClassType() == SuatinExprClassType_NotExpr){ //根节点是Not
				
				NotExpr* node_tmp = dynamic_cast<NotExpr*>(logicRoot);
				if (node_tmp->GetContent() == NULL) {
					node_tmp->SetContent(exprRoot);//装上exprRoot
				}
				else {
					Expr* node_tmp2 = dynamic_cast<Expr*>(node_tmp->GetContent());
					logic_chunk(&node_tmp2);//更改not后的内容
					node_tmp->SetContent(node_tmp2);//重新把小树装上
				}
			}
			else if(logicRoot->GetClassType() == SuatinExprClassType_AndExpr || logicRoot->GetClassType() == SuatinExprClassType_OrExpr){ //根节点是And/Or
				logic_chunk(&logicRoot);
			}

			exprRoot = logicRoot;//根节点上移
			logicRoot = NULL;

		}




		//将exprRoot下的语法树，接到root最右等号节点的右孩子上
		if (root) { //是赋值语句
			//找到最右下面的等号节点
			SymbolExpr* eq = dynamic_cast<SymbolExpr*>(root);
			while(eq->GetRight() != NULL && eq->GetRight()->GetClassType() == SuatinExprClassType_EqExpr){
				SymbolExpr* eq2 = dynamic_cast<SymbolExpr*>(eq->GetRight());
				eq = eq2;
			}
			eq->SetRight(exprRoot);//在root的最右等号上装上小树
		}
		else {
			root = exprRoot;//用root替换exprRoot	
		}
		exprNoVal = exprRoot; //保留非赋值表达式！！！
		exprRoot = NULL;


		/*-----------------------------最后，除了root和非赋值表达式exprNoVal外，其他指针都为空了------------------------------*/
	

	}

	/*void Parser::DealToken_Eol(int& _t) {

	}*/
	




	

	/*-----------------------------------------------------------------------------------------------------------------   logic operate  ------------------------------------------------------------------------------------------------*/




	void Parser::Deal_k_and(int& _t) {
		//表达式转变为逻辑式
		if (m_exprType != ExprType_Logic) {
			m_exprType = ExprType_Logic;
		}
		//初始化表达式（简单表达式的模式、firstId）
		m_simpleExprType = SimpleExprType_NumCalc;
		firstId_flag = true;


		//存在待处理的节点-------------------------
		if (expTmp) {
			ThrowException(SuatinErrorType_Syntax,start, end, "[and] wrong expression");
			return;
		}



		
		//把exprRoot装上judgeRoot
		if (judgeRoot) {			
			//>,<,>=,<=这几个节点的情况下，左右操作数不能是布尔
			if (judgeRoot->GetClassType() == SuatinExprClassType_GreExpr || judgeRoot->GetClassType() == SuatinExprClassType_GreEqExpr || judgeRoot->GetClassType() == SuatinExprClassType_LesExpr || judgeRoot->GetClassType() == SuatinExprClassType_LesEqExpr) {
				
				//检查右边的ID是否是布尔，布尔是不能用来计算的
				if (exprRoot->GetClassType() == SuatinExprClassType_IDExpr) {  
					IDExpr* node_tmp = dynamic_cast<IDExpr*>(exprRoot);
					if (node_tmp->GetType() == SuatinIDType_bool) {
						ThrowException(SuatinErrorType_Syntax,start, end, "[and] boolean identifier cannot used for compare");
						return;
					}
				}
			}
			SymbolExpr* node_tmp = dynamic_cast<SymbolExpr*>(judgeRoot);
			node_tmp->SetRight(exprRoot);


			exprRoot = judgeRoot;//根节点上移
			judgeRoot = NULL;
			expTmp = NULL;
		}




		
		auto and_chunk = [this](Expr** _node) {//在logicRoot不为空的情况下
			//if (typeid(*((*_node)->GetClassType())) == typeid(AndExpr)) { //【根节点】是And
			if ((*_node)->GetClassType() == SuatinExprClassType_AndExpr) { //【根节点】是And

				//将exprRoot装上
				AndExpr* root_tmp = dynamic_cast<AndExpr*>(*_node);
				root_tmp->SetRight(exprRoot);
				//左结合
				AndExpr* root_tmp2 = new AndExpr((*_node), NULL);
			
				(*_node) = root_tmp2;
			}
			//else if (typeid(*((*_node)->GetClassType())) == typeid(OrExpr)) { //【根节点】是Or
			else if ((*_node)->GetClassType() == SuatinExprClassType_OrExpr) { //【根节点】是Or

				OrExpr* root_tmp = dynamic_cast<OrExpr*>(*_node);
				if (root_tmp->GetRight() == NULL) {
			
					AndExpr* root_tmp2 = new AndExpr(exprRoot, NULL);
					//更改【根节点】右孩子
					root_tmp->SetRight(root_tmp2);

					(*_node) = root_tmp;
				}
				else {
					AndExpr* tmp = dynamic_cast<AndExpr*>(root_tmp->GetRight());
					AndExpr* root_tmp2 = new AndExpr(tmp, NULL);
					//更改【根节点】右孩子
					root_tmp->SetRight(root_tmp2);

					(*_node) = root_tmp;
				}
			}
			else {
				ThrowException(SuatinErrorType_Syntax,start, end, "error in and sign process");
			}

			exprRoot = NULL;
		};




		//不存在待处理的节点-------------------------
		//之前没有逻辑运算符
		if (logicRoot == NULL) {//logicRoot内容为空--------------
			//将exprRoot装上
			logicRoot = new AndExpr(exprRoot, NULL);
			exprRoot = NULL;
		}
		else {//logicRoot内容不为空--------------
			if( logicRoot->GetClassType() == SuatinExprClassType_NotExpr){//一句话中not只能有一个，放在表达式开头

				NotExpr* node_tmp = dynamic_cast<NotExpr*>(logicRoot);
				if (node_tmp->GetContent() == NULL) { //not内容为空--------------
					//将exprRoot装上
					AndExpr* tmp_1 = new AndExpr(exprRoot,NULL);
					node_tmp->SetContent(tmp_1);
					exprRoot = NULL;
				}
				else {//not内容不为空--------------
					Expr* node_tmp2 = node_tmp->GetContent();
					and_chunk(&node_tmp2);//更改not后面的内容
					node_tmp->SetContent(node_tmp2);//重新把内容装上
				}
			}
			else if(logicRoot->GetClassType() == SuatinExprClassType_AndExpr || logicRoot->GetClassType() == SuatinExprClassType_OrExpr){
				and_chunk(&logicRoot);
			}
		}
		

	}





	void Parser::Deal_k_or(int& _t) {
		//表达式转变为逻辑式
		if (m_exprType != ExprType_Logic) {
			m_exprType = ExprType_Logic;
		}
		//初始化表达式（简单表达式的模式、firstId）
		m_simpleExprType = SimpleExprType_NumCalc;
		firstId_flag = true;


		//存在待处理的节点-------------------------
		if (expTmp) {
			ThrowException(SuatinErrorType_Syntax,start, end, "[and] wrong expression");
			return;
		}




		//把exprRoot装上judgeRoot
		if (judgeRoot) {
			//>,<,>=,<=这几个节点的情况下，左右操作数不能是布尔
			if (judgeRoot->GetClassType() == SuatinExprClassType_GreExpr || judgeRoot->GetClassType() == SuatinExprClassType_GreEqExpr || judgeRoot->GetClassType() == SuatinExprClassType_LesExpr || judgeRoot->GetClassType() == SuatinExprClassType_LesEqExpr) {
			
				//检查右边的ID是否是布尔，布尔是不能用来计算的
				if (exprRoot->GetClassType() == SuatinExprClassType_IDExpr) { 

					IDExpr* node_tmp = dynamic_cast<IDExpr*>(exprRoot);
					if (node_tmp->GetType() == SuatinIDType_bool) {
						ThrowException(SuatinErrorType_Syntax,start, end, "[or] boolean identifier cannot used for compare");
						return;
					}
				}
			}
			SymbolExpr* node_tmp = dynamic_cast<SymbolExpr*>(judgeRoot);
			node_tmp->SetRight(exprRoot);
			

			exprRoot = judgeRoot;//根节点上移
			judgeRoot = NULL;
			expTmp = NULL;
		}





		auto or_chunk = [this](Expr** _node) {//在logicRoot不为空的情况下
			if ((*_node)->GetClassType() == SuatinExprClassType_AndExpr) { //【根节点】是And

				//将exprRoot装上
				AndExpr* root_tmp = dynamic_cast<AndExpr*>(*_node);
				root_tmp->SetRight(exprRoot);
				//左结合
				OrExpr* root_tmp2 = new OrExpr((*_node), NULL);
			
				(*_node) = root_tmp2;
			}
			else if ((*_node)->GetClassType() == SuatinExprClassType_OrExpr) { //【根节点】是Or

				OrExpr* root_tmp = dynamic_cast<OrExpr*>(*_node);
				if (root_tmp->GetRight() == NULL) {
					//将exprRoot装上
					root_tmp->SetRight(exprRoot);
					//左结合
					OrExpr* root_tmp2 = new OrExpr((*_node), NULL);
				
					(*_node) = root_tmp2;
				}
				else {
					//将exprRoot装上
					AndExpr* tmp = dynamic_cast<AndExpr*>(root_tmp->GetRight());
					tmp->SetRight(exprRoot);
					//左结合
					OrExpr* root_tmp2 = new OrExpr((*_node), NULL);
			
					(*_node) = root_tmp2;
				}
			}
			else {
				ThrowException(SuatinErrorType_Syntax,start, end, "error in or sign process");
			}

			exprRoot = NULL;
		};






		//不存在待处理的节点-------------------------
		//之前没有逻辑运算符
		if (logicRoot == NULL) {//logicRoot内容为空--------------
			//将exprRoot装上
			logicRoot = new OrExpr(exprRoot, NULL);
			exprRoot = NULL;
		}
		//之前有逻辑运算符
		else {//logicRoot内容不为空--------------
			if (logicRoot->GetClassType() == SuatinExprClassType_NotExpr) {//一句话中not只能有一个，放在表达式开头

				NotExpr* node_tmp = dynamic_cast<NotExpr*>(logicRoot);
				if (node_tmp->GetContent() == NULL) {//not内容为空--------------
					//将exprRoot装上
					OrExpr* tmp_1 = new OrExpr(exprRoot, NULL);
					node_tmp->SetContent(tmp_1);
					exprRoot = NULL;
				}
				else {//not内容不为空--------------
					Expr* node_tmp2 = node_tmp->GetContent();
					or_chunk(&node_tmp2);//更改not后面的内容
					node_tmp->SetContent(node_tmp2);//重新把内容装上
				}
			}
			else if (logicRoot->GetClassType() == SuatinExprClassType_AndExpr || logicRoot->GetClassType() == SuatinExprClassType_OrExpr) {
				or_chunk(&logicRoot);
			}

		}
	}



	void Parser::Deal_k_not(int& _t) {
		//表达式转变为逻辑式
		if (m_exprType != ExprType_Logic) {
			m_exprType = ExprType_Logic;
		}
		//初始化表达式（简单表达式的模式、firstId）
		m_simpleExprType = SimpleExprType_NumCalc;
		firstId_flag = true;


		//存在待处理的节点-------------------------
		if (expTmp) {
			ThrowException(SuatinErrorType_Syntax,start, end, "[and] wrong expression");
			return;
		}

		//不存在待处理的节点-------------------------
		logicRoot = new NotExpr();
		exprRoot = NULL;
	}









	/*返回_node该调用的解释接口类型
	 一个Unit返回三种数据：
		1.字符串、字符串类型的id
		2.数字、数字类型的id
		3.布尔类型的id
	*/
	Unit_InterfaceType Parser::GetTree_InterfaceType(Expr* _node) {
		//字符串处理
		if(_node->GetClassType() == SuatinExprClassType_StrExpr){
			return Unit_InterfaceType_string;
		}

		//Num处理
		if (_node->GetClassType() == SuatinExprClassType_NumExpr) {
			return Unit_InterfaceType_number;
		}

		//id处理
		if(_node->GetClassType() == SuatinExprClassType_IDExpr){
			IDExpr* tmp = dynamic_cast<IDExpr*>(_node);
			if (tmp->GetType() == SuatinIDType_bool || tmp->GetType() == SuatinIDType_nil ) {
				return Unit_InterfaceType_bool;
			}
			else  if (tmp->GetType() == SuatinIDType_string) {
				return Unit_InterfaceType_string;
			}
			else if (tmp->GetType() == SuatinIDType_int || tmp->GetType() == SuatinIDType_number) {
				return Unit_InterfaceType_number;
			}
		}

	


		//~=/==/>/</>=/<=处理
		if( _node->GetClassType() == SuatinExprClassType_NeqExpr  || _node->GetClassType() == SuatinExprClassType_EqEqExpr || _node->GetClassType() == SuatinExprClassType_GreExpr || _node->GetClassType() == SuatinExprClassType_GreEqExpr || _node->GetClassType() == SuatinExprClassType_LesExpr || _node->GetClassType() == SuatinExprClassType_LesEqExpr ){
			return Unit_InterfaceType_bool;
		}



		//处理not,and,or
		if(_node->GetClassType() == SuatinExprClassType_NotExpr || _node->GetClassType() == SuatinExprClassType_AndExpr || _node->GetClassType() == SuatinExprClassType_OrExpr ){
			return Unit_InterfaceType_bool;
		}


		//返回计算结果或拼接结果
		if(_node->GetClassType() == SuatinExprClassType_AddExpr){
			SymbolExpr* node_tmp = dynamic_cast<SymbolExpr*>(_node);
			if (node_tmp->GetRight() == NULL) {
				ThrowException(SuatinErrorType_Syntax,start, end, "[GetTree_InterfacceType] null right value");
				return Unit_InterfaceType_number;
			}

			//字符串拼接处理
			if( node_tmp->GetLeft()->GetClassType() == SuatinExprClassType_StrExpr || node_tmp->GetRight()->GetClassType() == SuatinExprClassType_StrExpr){
				return Unit_InterfaceType_string;
			}
	
			//五则运算处理，基本就是不处理，因为默认就是使用五则运算接口
		}

		return Unit_InterfaceType_number;
	}







	/*在遇到分号之后，构造语法树操作结束，检查ast的语法*/
	void Parser::CheckASTreeSyntax() {
		if (expTmp) {//还有待处理的节点
			ThrowException(SuatinErrorType_Syntax,start,end,"still have signs which didn't handle");
		}

		if (v_NoMatchedLLeft.size() > 0) {//还有未匹配的左括号
			ThrowException(SuatinErrorType_Syntax,start, end, "the left didnot matched its right part");
		}
		
	}








	/*解释的第一步，确定Abstract Syntax Tree上==,~=,and,or,not的解释接口类型*/
	void	Parser::Confirm_ASTree_InterfaceType() {
		if (root == NULL || exprNoVal == NULL) {
			ThrowException(SuatinErrorType_Syntax,"root node was null");
			return;
		}

		_fact_Confirm_ASTree_InterfaceType(exprNoVal);//开始迭代

	}

	//实际的确定接口类型的函数
	void Parser::_fact_Confirm_ASTree_InterfaceType(Expr* _node) {
		if (_node == NULL)return;

		switch (_node->GetClassType()) {
		case SuatinExprClassType_NotExpr:
		{
			NotExpr* node_tmp = dynamic_cast<NotExpr*>(_node);
			//确定not的解释接口类型
			node_tmp->Set_InterfaceType(GetTree_InterfaceType(node_tmp));
			_fact_Confirm_ASTree_InterfaceType(node_tmp->GetContent());
		}
		return;
		case SuatinExprClassType_AndExpr:
		{
			//DLR遍历
			AndExpr* node_tmp = dynamic_cast<AndExpr*>(_node);
			//确定左边的解释接口类型
			node_tmp->SetLeft_InterfaceType(GetTree_InterfaceType(node_tmp->GetLeft()));
			_fact_Confirm_ASTree_InterfaceType(node_tmp->GetLeft());
			//确定右边的解释接口类型
			node_tmp->SetRight_InterfaceType(GetTree_InterfaceType(node_tmp->GetRight()));
			_fact_Confirm_ASTree_InterfaceType(node_tmp->GetRight());
		}
		return;
		case SuatinExprClassType_OrExpr:
		{
			OrExpr* node_tmp = dynamic_cast<OrExpr*>(_node);
			//确定左边的解释接口类型
			node_tmp->SetLeft_InterfaceType(GetTree_InterfaceType(node_tmp->GetLeft()));
			_fact_Confirm_ASTree_InterfaceType(node_tmp->GetLeft());
			//确定右边的解释接口类型
			node_tmp->SetRight_InterfaceType(GetTree_InterfaceType(node_tmp->GetRight()));
			_fact_Confirm_ASTree_InterfaceType(node_tmp->GetRight());
		}
		return;
		case SuatinExprClassType_EqEqExpr:
		{
			EqEqExpr* node_tmp = dynamic_cast<EqEqExpr*>(_node);
			//确定左边的解释接口类型
			node_tmp->SetLeft_InterfaceType(GetTree_InterfaceType(node_tmp->GetLeft()));
			_fact_Confirm_ASTree_InterfaceType(node_tmp->GetLeft());
			//确定右边的解释接口类型
			node_tmp->SetRight_InterfaceType(GetTree_InterfaceType(node_tmp->GetRight()));
			_fact_Confirm_ASTree_InterfaceType(node_tmp->GetRight());
		}
		return;
		case SuatinExprClassType_NeqExpr:
		{
			NeqExpr* node_tmp = dynamic_cast<NeqExpr*>(_node);
			//确定左边的解释接口类型
			node_tmp->SetLeft_InterfaceType(GetTree_InterfaceType(node_tmp->GetLeft()));
			_fact_Confirm_ASTree_InterfaceType(node_tmp->GetLeft());
			//确定右边的解释接口类型
			node_tmp->SetRight_InterfaceType(GetTree_InterfaceType(node_tmp->GetRight()));
			_fact_Confirm_ASTree_InterfaceType(node_tmp->GetRight());
		}
		return;
		case SuatinExprClassType_EqExpr:
			ThrowException(SuatinErrorType_Type,"Expr pointer was evaluate type");
			return;
		default:
			return;
		}
	}


};


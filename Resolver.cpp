#include "Resolver.h"

namespace sua {

	Resolver::Resolver()
	{

		//初始化容器
		v_exprs.clear();
		semantic_tree = new BlockCmd();


		//初始化信号与槽
		g_signal = new SuatinSignal<int>();
		Connect(*g_signal,  std::bind(&Resolver::_slot_interpret, this, std::placeholders::_1));




		/*
		为什么还写这么多？
		
		因为如果Resolver继承Parser的话，引用关系可能会产生死循环！！！

		所以，Resolver和Parser应该是分离的！！！！代码多了，但是架构清楚了！！！
		作用被分离了，Parser中只处理单条语句，而Resolver中负责处理语义！！！
		*/

		//初始化处理Token的函数
		funcMap[SuatinTokenType_Num] = &Resolver::DealToken_Num;
		funcMap[SuatinTokenType_Id] = &Resolver::DealToken_Id;
		funcMap[SuatinTokenType_Str] = &Resolver::DealToken_Str;
		funcMap[SuatinTokenType_Pow] = &Resolver::DealToken_Pow;
		funcMap[SuatinTokenType_Mul] = &Resolver::DealToken_Mul;
		funcMap[SuatinTokenType_Div] = &Resolver::DealToken_Div;
		funcMap[SuatinTokenType_Add] = &Resolver::DealToken_Add;
		funcMap[SuatinTokenType_Sub] = &Resolver::DealToken_Sub;
		funcMap[SuatinTokenType_Gre] = &Resolver::DealToken_Gre;
		funcMap[SuatinTokenType_GreEq] = &Resolver::DealToken_GreEq;
		funcMap[SuatinTokenType_Les] = &Resolver::DealToken_Les;
		funcMap[SuatinTokenType_LesEq] = &Resolver::DealToken_LesEq;
		funcMap[SuatinTokenType_Neq] = &Resolver::DealToken_Neq;
		funcMap[SuatinTokenType_EqEq] = &Resolver::DealToken_EqEq;
		funcMap[SuatinTokenType_Eq] = &Resolver::DealToken_Eq;
		funcMap[SuatinTokenType_Com] = &Resolver::DealToken_Com;
		funcMap[SuatinTokenType_ML] = &Resolver::DealToken_ML;
		funcMap[SuatinTokenType_MR] = &Resolver::DealToken_MR;
		funcMap[SuatinTokenType_LL] = &Resolver::DealToken_LL;
		funcMap[SuatinTokenType_LR] = &Resolver::DealToken_LR;
		funcMap[SuatinTokenType_BL] = &Resolver::DealToken_BL;
		funcMap[SuatinTokenType_BR] = &Resolver::DealToken_BR;
		funcMap[SuatinTokenType_Dot] = &Resolver::DealToken_Dot;
		funcMap[SuatinTokenType_Sem] = &Resolver::DealToken_Sem;
		//funcMap[SuatinTokenType_Eol] = &Resolver::DealToken_Eol;


		//初始化处理关键字的函数
		k_funcMap[SuatinKeyWordType_if] = &Resolver::Deal_k_if;
		k_funcMap[SuatinKeyWordType_elif] = &Resolver::Deal_k_elif;
		k_funcMap[SuatinKeyWordType_else] = &Resolver::Deal_k_else;
		k_funcMap[SuatinKeyWordType_for] = &Resolver::Deal_k_for;
		k_funcMap[SuatinKeyWordType_break] = &Resolver::Deal_k_break;
		k_funcMap[SuatinKeyWordType_continue] = &Resolver::Deal_k_continue;
		k_funcMap[SuatinKeyWordType_do] = &Resolver::Deal_k_do;
		k_funcMap[SuatinKeyWordType_until] = &Resolver::Deal_k_until;
		k_funcMap[SuatinKeyWordType_while] = &Resolver::Deal_k_while;
		k_funcMap[SuatinKeyWordType_local] = &Resolver::Deal_k_local;
		k_funcMap[SuatinKeyWordType_const] = &Resolver::Deal_k_const;
		k_funcMap[SuatinKeyWordType_and] = &Resolver::Deal_k_and;
		k_funcMap[SuatinKeyWordType_or] = &Resolver::Deal_k_or;
		k_funcMap[SuatinKeyWordType_not] = &Resolver::Deal_k_not;
		k_funcMap[SuatinKeyWordType_function] = &Resolver::Deal_k_function;
		k_funcMap[SuatinKeyWordType_end] = &Resolver::Deal_k_end;
		k_funcMap[SuatinKeyWordType_return] = &Resolver::Deal_k_return;
		k_funcMap[SuatinKeyWordType_assert] = &Resolver::Deal_k_assert;


	}


	Resolver::~Resolver()
	{

		//释放多行语句
		v_exprs.clear();
		std::vector<Parser*>().swap(v_exprs);


		//释放语义树
		if (semantic_tree != NULL) {
			delete semantic_tree;
			semantic_tree = NULL;
		}


		//释放信号
		if (g_signal) {
			delete g_signal;
			g_signal = NULL;
		}


		//释放容器
		funcMap.clear();
		std::map<SuatinTokenType, R_DealFuncPtr>().swap(funcMap);
		k_funcMap.clear();
		std::map<SuatinKeyWordType, R_DealFuncPtr>().swap(k_funcMap);

	}



	void Resolver::create() {
		
		bool judge_statement_flag = false;
		bool for_flag = false;

	

		auto print_chunk = [this]() { //打印语句
			for (int k = start; k <= end; ++k) {
				std::cout << global_infix[k]->name;
			}
			std::cout << "\n";
		};




		
		g_statement_index = 0;
		for (int it = 0; it < (int)global_infix.size(); ++it) {
			try {
				//处理Token和keyword
				(this->*funcMap[global_infix[it]->type])(it);
				//单个分号不算作语句
				if (start == end && global_infix[it]->type == SuatinTokenType_Sem) {
					++start;
					++end;
					continue;
				}
			}
			catch (SuatinExcept& e) {
				PrintException(e.what());
			}


			if (judge_statement_flag) { //if/elif/else/while/until括号内的语句---------------------------
				//遇到if/elif/else/while/until后面的右括号
				if (count_little == 0) {
					end = it - 1;//在右括号之前结束

					if (for_init_cond_add_flag != 0) {//for
#ifdef _SUATIN_DEBUG_
						std::cout << "[" << g_statement_index << "]for括号内语句>";
#endif
						for_flag = false;
					}
#ifdef _SUATIN_DEBUG_
					else std::cout << "[" << g_statement_index << "]括号内语句>";
					print_chunk();
#endif
					v_exprs.push_back(new Parser(start, end));
					
					try {
						Confirm_LR_Cond();//将括号中的语句装入if/elif/else/while/until
					}
					catch (SuatinExcept& e) {
						PrintException(e.what());
					}

					//之后的普通语句在右括号之后开始
					start = it + 1;
					end = it;
					++g_statement_index;
					judge_statement_flag = false;
				}
			}
			else{ //普通语句---------------------------
				if (global_infix[it]->type == SuatinTokenType_Sem) {
					if (for_init_cond_add_flag != 0) {//for
#ifdef _SUATIN_DEBUG_
						std::cout << "[" << g_statement_index << "]for括号内语句>";
#endif
						end = it ;
					}
#ifdef _SUATIN_DEBUG_
					else std::cout << "[" << g_statement_index << "]普通语句>";
					print_chunk();
#endif
					if (uniqueKeyWord_flag == true) { //特殊关键字，不构造语法树
						uniqueKeyWord_flag = false;
					}
					else v_exprs.push_back(new Parser(start, end));


					if (for_init_cond_add_flag != 0) {//for
						try {
							Confirm_LR_Cond();//将括号中的语句装入if/elif/else/while/until
						}
						catch (SuatinExcept& e) {
							PrintException(e.what());
						}
						if (for_init_cond_add_flag == 3) { //for_stat_add 语句在上面匹配
							judge_statement_flag = true;							
						}
					}

					//之后的普通语句在分号之后开始
					start = end + 1;
					++g_statement_index;
				}

				if (for_flag == true) { //出现for ( 之后!!!!

				}

				
				else if (for_flag == false &&  count_little == 1 ) {//遇到if/elif/else/while/until后面的左括号!!!!!
					start = end = it + 1;//在左括号之后开始
					if (for_init_cond_add_flag != 0) {
						for_flag = true;
					}
					else judge_statement_flag = true;
				}

			}
			
			++end;
			//每次循环开始和最后，end=it
		}
		


		try {
			//检查一下
			CheckSemanticTreeSyntax();
		}
		catch (SuatinExcept& e) {
			PrintException(e.what());
		}




		if (g_error_lex_flag)return;//词法期出错就没必要再构造语法树了！！！！！！！！！！！

		//根据Parser构造语法树
		g_run_time = SuatinRunTimeType_Parse;
		g_statement_index = 0;
		for (std::vector<Parser*>::iterator it = v_exprs.begin(); it != v_exprs.end(); ++it) {
			(*it)->CreateASTree();
			++g_statement_index;
		}
	

	}

	void Resolver::print_ast() {
		//打印语法树
		g_statement_index = 0;

		std::cout<<"suatin abstract syntax tree>"<<std::endl;
		for (std::vector<Parser*>::iterator it = v_exprs.begin(); it != v_exprs.end(); ++it) {
			std::cout << "[" << g_statement_index << "]\n";
			(*it)->ShowASTree();
			++g_statement_index;
		}
	}

	void Resolver::print_ast(int _index) {
		//打印某条语法树

		std::cout<<"suatin abstract syntax tree>"<<std::endl;
		if (CheckStatementIndex(_index) == false)return;
		g_statement_index = _index;
		std::cout << "[" << g_statement_index << "]\n";
		v_exprs[_index]->ShowASTree();
	}




	void Resolver::interpret() {
		if (g_error_parse_flag)return;//语法期出错就没必要再解释了！！！！！！！！！！！！

		g_run_time = SuatinRunTimeType_Interpret;
		g_statement_index = 0;
		try {
			if (semantic_tree == NULL) {
				ThrowException(SuatinErrorType_Other,"[resolver] none of statement");
				return;//解释期错误
			}
			//解释语义树
			semantic_tree->interpret();
		}
		catch (SuatinExcept& e) {
			PrintException(e.what());
		}
		catch (...) {
			PrintException("[resolver] interpret error");
		}
	}



	//接收从语义树中发射的信号
	void Resolver::_slot_interpret(int _index) {
		if (CheckStatementIndex(_index) == false)return;
		g_statement_index = _index;//更新语句索引
		 v_exprs[_index]->interpret();
		 
	}




	bool Resolver::CheckStatementIndex(int _index) {
		//检查索引的范围
		if (_index < 0 || _index >= (int)v_exprs.size()) {
			std::string s = "index = " + std::to_string(_index) + " was wrong";
			ThrowException(SuatinErrorType_OutRange,s);
			return false;
		}
		return true;
	}





	




	
	/*---------------------------------------------------------------------------------------------------------------------------------- none ------------------------------------------------------------------------------------------------*/

	void Resolver::DealToken_Id(int& _t) {
		//处理关键字
		try {
			if (global_infix[_t]->k_type != SuatinKeyWordType_NOT_KEYWORD) {
				(this->*k_funcMap[global_infix[_t]->k_type])(_t);
				return;
			}
		}
		catch (...) {
			PrintException("the keyword didnot initializated");
		}

		//不处理变量
	}
	void Resolver::DealToken_Num(int& _t) {}
	void Resolver::DealToken_Str(int& _t) {	}
	void Resolver::DealToken_Pow(int& _t) {}
	void Resolver::DealToken_Mul(int& _t) {	}
	void Resolver::DealToken_Div(int& _t) {	}
	void Resolver::DealToken_Add(int& _t) {}
	void Resolver::DealToken_Sub(int& _t) {}
	void Resolver::DealToken_Gre(int& _t) {	}
	void Resolver::DealToken_GreEq(int& _t) {}
	void Resolver::DealToken_Les(int& _t) {	}
	void Resolver::DealToken_LesEq(int& _t) {}
	void Resolver::DealToken_Neq(int& _t) {}
	void Resolver::DealToken_EqEq(int& _t) {}
	void Resolver::DealToken_Eq(int& _t) {}
	void Resolver::Deal_k_and(int& _t) {}
	void Resolver::Deal_k_or(int& _t) {}
	void Resolver::Deal_k_not(int& _t) {}


	/*---------------------------------------------------------------------------------------------------------------------------------- deal with token ------------------------------------------------------------------------------------------------*/
	

	void Resolver::DealToken_Com(int& _t) {

	}
	
	void Resolver::DealToken_LL(int& _t) {
		++count_little;
	}
	void Resolver::DealToken_LR(int& _t) {
		--count_little;
	}

	void Resolver::Confirm_LR_Cond() {
		if (uncompleted_tree == NULL) {//没有待处理的语义树-----------------
			ThrowException(SuatinErrorType_Syntax,"[LR] wrong syntax");
			return;
		}


		//有待处理的语义树------------------

		//检查栈大小
		if (CheckStack_UnCompletedBlock())return;

		Cmd* node_tmp = v_UnCompletedBlock[  v_UnCompletedBlock.size() -1 ]  ;    //拿到最近的没有结束的语句块
		if (node_tmp->GetClassType() == SuatinCmdClassType_IfCmd) {
			IfCmd* node_tmp2 = dynamic_cast<IfCmd*>(node_tmp);
			node_tmp2->SetCondition(new SingleCmd(g_statement_index)); //设置if的条件语句
			return;
		}
		else if (node_tmp->GetClassType() == SuatinCmdClassType_ElifCmd) {
			ElifCmd* node_tmp2 = dynamic_cast<ElifCmd*>(node_tmp);
			node_tmp2->SetCondition(new SingleCmd(g_statement_index)); //设置elif的条件语句
			return;
		}
		else  if (node_tmp->GetClassType() == SuatinCmdClassType_DoUntilCmd) {
			DoUntilCmd* node_tmp2 = dynamic_cast<DoUntilCmd*>(node_tmp);
			node_tmp2->SetCondition(new SingleCmd(g_statement_index)); //设置do-until的条件语句
			//抛出do-until
			v_UnCompletedBlock.pop_back();

			//第一级的do-until语义树终于结束了！！！！！！
			if ((int)v_UnCompletedBlock.size() == 0) {
				//待处理的语义树已经处理完了，压入semantic_tree中
				semantic_tree->Push(uncompleted_tree);
				uncompleted_tree = NULL;
			}
			return;
		}
		else  if (node_tmp->GetClassType() == SuatinCmdClassType_WhileCmd) {
			WhileCmd* node_tmp2 = dynamic_cast<WhileCmd*>(node_tmp);
			node_tmp2->SetCondition(new SingleCmd(g_statement_index)); //设置while的条件语句
			return;
		}
		else  if (node_tmp->GetClassType() == SuatinCmdClassType_ForCmd) {
			ForCmd* node_tmp2 = dynamic_cast<ForCmd*>(node_tmp);
			if (for_init_cond_add_flag == 1) {
				node_tmp2->SetInit(new SingleCmd(g_statement_index));				//设置for的初始化语句
				for_init_cond_add_flag = 2;
			}
			else if (for_init_cond_add_flag == 2) {
				node_tmp2->SetCondition(new SingleCmd(g_statement_index));		//设置for的条件语句
				for_init_cond_add_flag = 3;
			}
			else if (for_init_cond_add_flag == 3) {
				node_tmp2->SetAdd(new SingleCmd(g_statement_index));				//设置for的自增语句
				for_init_cond_add_flag = 0;		//重置for循环工具
			}
			return;
		}

		//最近的未结束语句块类型不符合要求

		ThrowException(SuatinErrorType_Syntax,"[LR] wrong syntax");

	}


	void Resolver::DealToken_ML(int& _t) {
		++count_middle;
	}
	void Resolver::DealToken_MR(int& _t) {
		--count_middle;
	}
	void Resolver::DealToken_BL(int& _t) {
		++count_big;
	}
	void Resolver::DealToken_BR(int& _t) {
		--count_big;
	}
	
	
	void Resolver::DealToken_Dot(int& _t) {

	}



	void Resolver::DealToken_Sem(int& _t) {
		//1.单个分号不算做语句
		if (start == end) {
			return;
		}

		//2.处理特殊关键字------------
		if (kFlag.assert_flag == true) {
			Deal_UniqueKeyWord<AssertCmd>();
			kFlag.assert_flag = false;
			return;
		}
		else if (kFlag.break_flag == true) {
			Deal_UniqueKeyWord<BreakCmd>();
			kFlag.break_flag = false;
			return;
		}
		else if (kFlag.continue_flag == true) {
			Deal_UniqueKeyWord<ContinueCmd>();
			kFlag.continue_flag = false;
			return;
		}
		else if (kFlag.return_flag == true) {

			kFlag.return_flag = false;
			return;
		}


		if (uncompleted_tree == NULL) {//没有待处理的语义树-----------------
			semantic_tree->Push(new SingleCmd(g_statement_index));//压入普通语句
			return;
		}


		//有待处理的语义树------------------

		//检查栈大小
		if (CheckStack_UnCompletedBlock())return;

		Cmd* node_tmp = v_UnCompletedBlock[v_UnCompletedBlock.size() - 1];    //拿到最近的没有结束的语句块
		//3.处理分支或循环------------
		if (node_tmp->GetClassType() == SuatinCmdClassType_IfCmd) {
			IfCmd* node_tmp2 = dynamic_cast<IfCmd*>(node_tmp);
			node_tmp2->Push(new SingleCmd(g_statement_index)); //将语句加入最近的未结束语句块
			return;
		}
		else if (node_tmp->GetClassType() == SuatinCmdClassType_ElifCmd) {
			ElifCmd* node_tmp2 = dynamic_cast<ElifCmd*>(node_tmp);
			node_tmp2->Push(new SingleCmd(g_statement_index)); //将语句加入最近的未结束语句块
			return;
		}
		else if (node_tmp->GetClassType() == SuatinCmdClassType_ElseCmd) {
			ElseCmd* node_tmp2 = dynamic_cast<ElseCmd*>(node_tmp);
			node_tmp2->Push(new SingleCmd(g_statement_index)); //将语句加入最近的未结束语句块
			return;
		}
		else  if (node_tmp->GetClassType() == SuatinCmdClassType_DoUntilCmd) {
			DoUntilCmd* node_tmp2 = dynamic_cast<DoUntilCmd*>(node_tmp);
			node_tmp2->Push(new SingleCmd(g_statement_index)); //将语句加入最近的未结束语句块
			return;
		}
		else  if (node_tmp->GetClassType() == SuatinCmdClassType_WhileCmd) {
			WhileCmd* node_tmp2 = dynamic_cast<WhileCmd*>(node_tmp);
			node_tmp2->Push(new SingleCmd(g_statement_index)); //将语句加入最近的未结束语句块
			return;
		}
		else  if (node_tmp->GetClassType() == SuatinCmdClassType_ForCmd) {
			if (for_init_cond_add_flag == 0) { //for括号内语句加载完后，再加载for_block
				ForCmd* node_tmp2 = dynamic_cast<ForCmd*>(node_tmp);
				node_tmp2->Push(new SingleCmd(g_statement_index)); //将语句加入最近的未结束语句块
			}
			return;
		}

		






		//最近的未结束语句块类型不符合要求

		ThrowException(SuatinErrorType_Syntax,"[sem] wrong syntax");

	}
	
	//void Resolver::DealToken_Eol(int& _t){
	//
	//}



	/*--------------------------------------------------------------------------------------------------------------------------------- deal with keyword ------------------------------------------------------------------------------------------------*/


	void Resolver::Deal_k_if(int& _t) {
		if (uncompleted_tree == NULL) { //没有待处理的语义树-----------------
			uncompleted_tree = new IfCmd();
			v_UnCompletedBlock.push_back(uncompleted_tree); //将未处理的if压入栈中
			return;
		}


		//有待处理的语义树------------------
		//检查栈大小
		if (CheckStack_UnCompletedBlock())return;

		BlockCmd*  node_tmp = dynamic_cast<BlockCmd*>(v_UnCompletedBlock[  v_UnCompletedBlock.size()   -1]  );//拿到最近的没有结束的语句块
		IfCmd* node_new = new IfCmd(); //创建新的if
		node_tmp->Push(node_new); //将if加入最近的未结束语句块
		v_UnCompletedBlock.push_back(node_new); //将新的if压入栈
	}




	void Resolver::Deal_k_elif(int& _t) {
		if (uncompleted_tree == NULL) {//没有待处理的语义树-----------------
			ThrowException(SuatinErrorType_Syntax,"[elif] wrong syntax");
			return;
		}


		//有待处理的语义树------------------
			/*	if节点加在哪种语句块中都可以，但是elif/else不行，这两种节点只能跟在if后面！！！	*/


			//检查栈大小
		if (CheckStack_UnCompletedBlock())return;

		Cmd* node_tmp = v_UnCompletedBlock[v_UnCompletedBlock.size() - 1];    //拿到最近的没有结束的语句块
		if (node_tmp->GetClassType() == SuatinCmdClassType_IfCmd) {
			//结束掉最近的if
			IfCmd* node_tmp2 = dynamic_cast<IfCmd*>(node_tmp);
			node_tmp2->SetEnd();
			//弹出该if
			v_UnCompletedBlock.pop_back();

			ElifCmd* node_new = new ElifCmd();//创建新的elif
			node_tmp2->last = node_new; //将新的elif装入最近的if链条上
			v_UnCompletedBlock.push_back(node_new); //将新的elif压入栈
			return;
		}
		else if (node_tmp->GetClassType() == SuatinCmdClassType_ElifCmd) {
			//结束掉最近的elif
			ElifCmd* node_tmp2 = dynamic_cast<ElifCmd*>(node_tmp);
			node_tmp2->SetEnd();
			//弹出该elif
			v_UnCompletedBlock.pop_back();

			ElifCmd* node_new = new ElifCmd();//创建新的elif
			node_tmp2->last = node_new; //将新的elif装入最近的if链条上
			v_UnCompletedBlock.push_back(node_new); //将新的elif压入栈
			return;
		}

		//最近的未结束语句块类型不符合要求

		ThrowException(SuatinErrorType_Syntax,"[elif] wrong syntax");

	}



	void Resolver::Deal_k_else(int& _t) {
		++start;//因为else关键字不算在语句内

		if (uncompleted_tree == NULL) {//没有待处理的语义树-----------------
			ThrowException(SuatinErrorType_Syntax,"[else] wrong syntax");
			return;
		}


		//有待处理的语义树------------------

		//检查栈大小
		if (CheckStack_UnCompletedBlock())return;

		Cmd* node_tmp = v_UnCompletedBlock[v_UnCompletedBlock.size() - 1];    //拿到最近的没有结束的语句块
		if (node_tmp->GetClassType() == SuatinCmdClassType_IfCmd) {
			//结束掉最近的if
			IfCmd* node_tmp2 = dynamic_cast<IfCmd*>(node_tmp);
			node_tmp2->SetEnd();
			//弹出该if
			v_UnCompletedBlock.pop_back();

			ElseCmd* node_new = new ElseCmd();//创建新的else
			node_tmp2->last = node_new;//将新的else装入最近的if链条上	
			v_UnCompletedBlock.push_back(node_new); //将新的else压入栈
			return;
		}
		else if (node_tmp->GetClassType() == SuatinCmdClassType_ElifCmd) {
			//结束掉最近的elif
			ElifCmd* node_tmp2 = dynamic_cast<ElifCmd*>(node_tmp);
			node_tmp2->SetEnd();
			//弹出该elif
			v_UnCompletedBlock.pop_back();

			ElseCmd* node_new = new ElseCmd();//创建新的else
			node_tmp2->last = node_new;//将新的else装入最近的if链条上		
			v_UnCompletedBlock.push_back(node_new); //将新的else压入栈
			return;
		}

		//最近的未结束语句块类型不符合要求

		ThrowException(SuatinErrorType_Syntax,"[else] wrong syntax");

	}






	
	void Resolver::Deal_k_for(int& _t) {
		for_init_cond_add_flag = 1;//设置for循环工具

		if (uncompleted_tree == NULL) {//没有待处理的语义树-----------------
			uncompleted_tree = new ForCmd();
			v_UnCompletedBlock.push_back(uncompleted_tree);//将新的for节点压入栈中
			return;
		}


		//有待处理的语义树------------------

		//检查栈大小
		if (CheckStack_UnCompletedBlock())return;

		BlockCmd*  node_tmp = dynamic_cast<BlockCmd*>(v_UnCompletedBlock[ v_UnCompletedBlock.size() -1 ]   );//拿到最近的没有结束的语句块
		ForCmd* node_new = new ForCmd(); //创建新的for
		node_tmp->Push(node_new); //将for加入最近的未结束语句块
		v_UnCompletedBlock.push_back(node_new); //将新的for压入栈

	}







	void Resolver::Deal_k_do(int& _t) {
		++start;//do关键字不属于语句

		if (uncompleted_tree == NULL) {//没有待处理的语义树-----------------
			uncompleted_tree = new DoUntilCmd();
			v_UnCompletedBlock.push_back(uncompleted_tree); //将新的do-until压入栈中
			return;
		}

		//有待处理的语义树------------------
		//检查栈大小
		if (CheckStack_UnCompletedBlock())return;

		BlockCmd*  node_tmp = dynamic_cast<BlockCmd*>(v_UnCompletedBlock[v_UnCompletedBlock.size() - 1]);//拿到最近的没有结束的语句块
		DoUntilCmd* node_new = new DoUntilCmd(); //创建新的do-until
		node_tmp->Push(node_new); //将do-until加入最近的未结束语句块
		v_UnCompletedBlock.push_back(node_new); //将新的do-until压入栈

	}
	void Resolver::Deal_k_until(int& _t) {
		if (uncompleted_tree == NULL) { //没有待处理的语义树--------------------------
			ThrowException(SuatinErrorType_Syntax,"[until] wrong syntax");
			return;
		}


		//有待处理的语义树-----------------------
		//检查栈大小
		if (CheckStack_UnCompletedBlock())return;

		if (v_UnCompletedBlock[v_UnCompletedBlock.size() - 1]->GetClassType() == SuatinCmdClassType_DoUntilCmd) {
			DoUntilCmd* node_tmp = dynamic_cast<DoUntilCmd*>(   v_UnCompletedBlock[   v_UnCompletedBlock.size() -1]    );//拿到do-until语句块
			node_tmp->SetEnd();//结束掉do-until语句块
			//不抛出do-until节点！！！！
			return;
		}

		ThrowException(SuatinErrorType_Syntax,"[until] wrong syntax");
	}





	void Resolver::Deal_k_while(int& _t) {
		if (uncompleted_tree == NULL) {//没有待处理的语义树-----------------
			uncompleted_tree = new WhileCmd();
			v_UnCompletedBlock.push_back(uncompleted_tree); //将新的while压入栈中
			return;
		}

		//有待处理的语义树-----------------
		//检查栈大小
		if (CheckStack_UnCompletedBlock())return;

		BlockCmd*  node_tmp = dynamic_cast<BlockCmd*>(v_UnCompletedBlock[  v_UnCompletedBlock.size() -1]   );//拿到最近的没有结束的语句块
		WhileCmd* node_new = new WhileCmd(); //创建新的while
		node_tmp->Push(node_new); //将while加入最近的未结束语句块
		v_UnCompletedBlock.push_back(node_new); //将新的while压入栈

	}
	void Resolver::Deal_k_local(int& _t) {

	}
	void Resolver::Deal_k_const(int& _t) {

	}	
	void Resolver::Deal_k_function(int& _t) {

	}
	
	void Resolver::Deal_k_return(int& _t) {

	}






	//遇到assert关键字后，将停止语义树的解释，只有回车才能继续  
	void Resolver::Deal_k_assert(int& _t) { //for debug
		kFlag.assert_flag = true;
	}


	void Resolver::Deal_k_break(int& _t) {
		kFlag.break_flag = true;
	}



	void Resolver::Deal_k_continue(int& _t) {
		kFlag.continue_flag = true;
	}






	//特殊关键字处理函数，将关键字语句压入最近的block中
	template<typename T>
	void Resolver::Deal_UniqueKeyWord() {
		//检查语句的长度
		if (start + 1 != end) {
			ThrowException(SuatinErrorType_Syntax,"[UniqueKeyWord] wrong expression");
			return;
		}

		uniqueKeyWord_flag = true;//准备跳过语法树的构造
		--g_statement_index;//减去当前语句，因为这不算语句

		if (uncompleted_tree == NULL) {//没有待处理的语义树-----------------
			semantic_tree->Push(new T()); //将xxx加入语义树
			return;
		}

		//有待处理的语义树-----------------
		//检查栈大小
		if (CheckStack_UnCompletedBlock())return;

		BlockCmd*  node_tmp = dynamic_cast<BlockCmd*>(v_UnCompletedBlock[v_UnCompletedBlock.size() - 1]);//拿到最近的没有结束的语句块
		T* node_new = new T(); //创建新的xxx
		node_tmp->Push(node_new); //将xxx加入最近的未结束语句块
	}










	void Resolver::Deal_k_end(int& _t) {
		++start;;//因为end关键字不算在语句内


		if (uncompleted_tree == NULL) {//没有待处理的语义树-----------------
			ThrowException(SuatinErrorType_Syntax,"[end] wrong syntax");
			return;
		}


		//有待处理的语义树------------------

			//检查栈大小
		if (CheckStack_UnCompletedBlock())return;

		Cmd* node_tmp = v_UnCompletedBlock[  v_UnCompletedBlock.size() -1]; //拿到最近的没有结束的语句块
		if (node_tmp->GetClassType() == SuatinCmdClassType_IfCmd) {
			//结束掉最近的if
			IfCmd* node_tmp2 = dynamic_cast<IfCmd*>(node_tmp);
			node_tmp2->SetEnd();
			//弹出该if
			v_UnCompletedBlock.pop_back();


			//第一级的if语义树终于结束了！！！！！！
			if ((int)v_UnCompletedBlock.size() == 0) {
				//待处理的语义树已经处理完了，压入semantic_tree中
				semantic_tree->Push(uncompleted_tree);
				uncompleted_tree = NULL;
			}
			return;
		}
		else if (node_tmp->GetClassType() == SuatinCmdClassType_ElifCmd) {
			//结束掉最近的elif
			ElifCmd* node_tmp2 = dynamic_cast<ElifCmd*>(node_tmp);
			node_tmp2->SetEnd();
			//弹出该elif
			v_UnCompletedBlock.pop_back();

			//第一级的if语义树终于结束了！！！！！！
			if ((int)v_UnCompletedBlock.size() == 0) {
				//待处理的语义树已经处理完了，压入semantic_tree中
				semantic_tree->Push(uncompleted_tree);
				uncompleted_tree = NULL;
			}
			return;
		}
		else if (node_tmp->GetClassType() == SuatinCmdClassType_ElseCmd) {
			//结束掉最近的else
			ElseCmd* node_tmp2 = dynamic_cast<ElseCmd*>(node_tmp);
			node_tmp2->SetEnd();
			//弹出该elif
			v_UnCompletedBlock.pop_back();

			//第一级的if语义树终于结束了！！！！！！
			if ((int)v_UnCompletedBlock.size() == 0) {
				//待处理的语义树已经处理完了，压入semantic_tree中
				semantic_tree->Push(uncompleted_tree);
				uncompleted_tree = NULL;
			}
			return;
		}
		else if (node_tmp->GetClassType() == SuatinCmdClassType_WhileCmd) {
			//结束掉最近的While
			WhileCmd* node_tmp2 = dynamic_cast<WhileCmd*>(node_tmp);
			node_tmp2->SetEnd();
			//弹出该While
			v_UnCompletedBlock.pop_back();

			//第一级的whilef语义树终于结束了！！！！！！
			if ((int)v_UnCompletedBlock.size() == 0) {
				//待处理的语义树已经处理完了，压入semantic_tree中
				semantic_tree->Push(uncompleted_tree);
				uncompleted_tree = NULL;
			}
			return;
		}
		else if (node_tmp->GetClassType() == SuatinCmdClassType_ForCmd) {
			//结束掉最近的for
			ForCmd* node_tmp2 = dynamic_cast<ForCmd*>(node_tmp);
			node_tmp2->SetEnd();
			//弹出该for
			v_UnCompletedBlock.pop_back();

			//第一级的for语义树终于结束了！！！！！！
			if ((int)v_UnCompletedBlock.size() == 0) {
				//待处理的语义树已经处理完了，压入semantic_tree中
				semantic_tree->Push(uncompleted_tree);
				uncompleted_tree = NULL;
			}
			return;
		}




		//最近的未结束语句块类型不符合要求

		ThrowException(SuatinErrorType_Syntax,"[end] wrong syntax");

	}









	//构造完后，最后检查一下错误
	void Resolver::CheckSemanticTreeSyntax() {
		if (semantic_tree == NULL) {
			ThrowException(SuatinErrorType_Syntax,"[last] semantic tree was null");
			return;
		}
		if (uncompleted_tree) {
			ThrowException(SuatinErrorType_Syntax,"[last] Cmd pointer uncomplated_tree wasnot null");
			return;
		}
		if ((int)v_UnCompletedBlock.size() > 0) {
			ThrowException(SuatinErrorType_Syntax,"[last] stack v_UnCompletedBlock wasnot null");
			return;
		}
	}

	//检查栈是否为空
	bool Resolver::CheckStack_UnCompletedBlock() {
		if (v_UnCompletedBlock.size() <= 0) {
			ThrowException(SuatinErrorType_Value,"[stack] v_UnCompletedBlock 's size is zero");
			return true;
		}
		return false;
	}




	/*-----------------------------------------------------------------------------------------------------------------print semantic tree-------------------------------------------------------------------------------------------------------------------------*/

	/*
	suatin->code
	>>>>>>
	sum=0;
	for(i=0;i<10;i=i+1)
		if(i==0 or i==2 or i==4 or i==6 or (i-1+1)==8)
			sum=sum+i;
		else
			sum=sum-1;
		end
		assert;
	end

	suatin->semantic tree
	>>>>>>
	block
	├── [0]
	└── for
			├── init
			│       └── [1]
			├── cond
			│       └── [2]
            ├── add
			│       └── [3]
			└── block
					├── if
					 │     ├── cond
					 │     │      └── [4]
					 │     ├── block
					 │     │      └── [5]
					 │     └── last
					 │             └── else
					 │                     └── block
					 │                             └── [6]
					└── assert
	
	*/
	void Resolver::print_st() {

		std::cout<<"suatin semantic tree>"<<std::endl;
		if (semantic_tree == NULL)return;
		try {
			_fact_print_st(semantic_tree);
		}
		catch (SuatinExcept& e) {
			PrintException(e.what());
		}
	}



	void Resolver::_fact_print_st(Cmd* _node, int _num) {
		//返回条件
		if (_node == NULL)return;


		//打印分支装饰
		auto print_decoration = [this](int _num) {
			if (_num > 0) {
				//1.打印每行前的空格和竖线
				for (int i = 0; i < _num - 1; ++i) {
					std::cout << ((v_forDisplaySemaTree[i] == 1) ? "│   " : "    ");//1:竖线   0:空格
				}
				//2.打印每行终点的分叉
				std::cout << ((v_forDisplaySemaTree[_num - 1] == 1) ? "├── " : "└── ");//1:T型线  0:τ型线
			}
		};


		//语句块的迭代
		auto block_iter = [this](BlockCmd* _node,int _num) {
			std::cout << "block\n";
			int i = 0;
			for(auto it  : _node->GetBlockVector()){
				if (_node->GetBlockVector().size() == 1 || i + 1 == _node->GetBlockVector().size()) {
					v_forDisplaySemaTree[_num] = 0; //"└── "
				}
				else {
					v_forDisplaySemaTree[_num] = 1;// "├── " 
				}
				_fact_print_st(it, 1 + _num);
				v_forDisplaySemaTree[_num] = 0;
				++i;
			}
		};


		print_decoration(_num);






		//打印节点
		if (_node->GetClassType() == SuatinCmdClassType_SingleCmd) {
			SingleCmd* tmp = dynamic_cast<SingleCmd*>(_node);
			std::cout << "["<<tmp->GetIndex()<<"]\n";
		}
		else if (_node->GetClassType() == SuatinCmdClassType_BlockCmd) {
			BlockCmd* tmp = dynamic_cast<BlockCmd*>(_node);
			block_iter(tmp,_num);
		}
		else if (_node->GetClassType() == SuatinCmdClassType_IfCmd) {
			IfCmd* tmp = dynamic_cast<IfCmd*>(_node);
			std::cout << "if\n";
			
			//if_cond
			v_forDisplaySemaTree[_num] = 1;// "├── "
			print_decoration(1 + _num);
			std::cout << "cond\n";
			v_forDisplaySemaTree[1 + _num] = 0;//"└── "
			_fact_print_st(tmp->GetCondition(), 2 + _num);//condition只有一句话

			
			if (tmp->last != NULL) {
				//if_block
				print_decoration(1 + _num);
				block_iter(tmp, _num + 1);

				//if_last
				v_forDisplaySemaTree[ _num] = 0; //"└── "
				print_decoration(1 + _num);
				std::cout << "last\n";
				v_forDisplaySemaTree[1 + _num] = 0; //"└── "
				_fact_print_st(tmp->last, 2 + _num);
				v_forDisplaySemaTree[1 + _num] = 0;
			}
			else {
				//if_block
				v_forDisplaySemaTree[_num] = 0; //"└── "
				print_decoration(1 + _num);
				block_iter(tmp, _num + 1);
			}

		}
		else if (_node->GetClassType() == SuatinCmdClassType_ElifCmd) {
			ElifCmd* tmp = dynamic_cast<ElifCmd*>(_node);
			std::cout << "elif\n";

			//elif_cond
			v_forDisplaySemaTree[_num] = 1;// "├── "
			print_decoration(1 + _num);
			std::cout << "cond\n";
			v_forDisplaySemaTree[1 + _num] = 0;//"└── "
			_fact_print_st(tmp->GetCondition(), 2 + _num);//condition只有一句话

			if (tmp->last != NULL) {
				//elif_block
				print_decoration(1 + _num);
				block_iter(tmp, _num + 1);

				//elif_last
				v_forDisplaySemaTree[_num] = 0; //"└── "
				print_decoration(1 + _num);
				std::cout << "last\n";
				v_forDisplaySemaTree[1 + _num] = 0; //"└── "
				_fact_print_st(tmp->last, 2 + _num);
				v_forDisplaySemaTree[1 + _num] = 0;
			}
			else {
				//elif_block
				v_forDisplaySemaTree[_num] = 0; //"└── "
				print_decoration(1 + _num);
				block_iter(tmp, _num + 1);
			}

		}
		else if (_node->GetClassType() == SuatinCmdClassType_ElseCmd) {
			ElseCmd* tmp = dynamic_cast<ElseCmd*>(_node);
			std::cout << "else\n";

			//else_block
			v_forDisplaySemaTree[_num] = 0; //"└── "
			print_decoration(1 + _num);
			block_iter(tmp, _num + 1);

		}
		else if (_node->GetClassType() == SuatinCmdClassType_WhileCmd) {
			WhileCmd* tmp = dynamic_cast<WhileCmd*>(_node);
			std::cout << "while\n";

			//while_cond
			v_forDisplaySemaTree[_num] = 1;// "├── "
			print_decoration(1 + _num );
			std::cout << "cond\n";
			v_forDisplaySemaTree[1 + _num] = 0;//"└── "
			_fact_print_st(tmp->GetCondition(), 2 + _num);//condition只有一句话


			//while_block
			v_forDisplaySemaTree[_num] = 0; //"└── "
			print_decoration(1 + _num);
			block_iter(tmp,  _num + 1);


		}
		else if (_node->GetClassType() == SuatinCmdClassType_DoUntilCmd) {
			DoUntilCmd* tmp = dynamic_cast<DoUntilCmd*>(_node);
			std::cout << "do-until\n";

			//do_until_block
			v_forDisplaySemaTree[_num] = 1;// "├── "
			print_decoration(1 + _num);
			block_iter(tmp, _num + 1);
			

			//do_until_cond
			v_forDisplaySemaTree[_num] = 0; //"└── "
			print_decoration(1 + _num);			
			std::cout << "cond\n";
			v_forDisplaySemaTree[1 + _num] = 0;//"└── "
			_fact_print_st(tmp->GetCondition(), 2 + _num);//condition只有一句话


		}
		else if (_node->GetClassType() == SuatinCmdClassType_ForCmd) {
			ForCmd* tmp = dynamic_cast<ForCmd*>(_node);
			std::cout << "for\n";

			//for_init
			v_forDisplaySemaTree[_num] = 1;// "├── "
			print_decoration(1 + _num);
			std::cout << "init\n";
			v_forDisplaySemaTree[1 + _num] = 0;//"└── "
			_fact_print_st(tmp->GetInit(), 2 + _num);//init只有一句话


			//for_cond
			print_decoration(1 + _num);
			std::cout << "cond\n";
			_fact_print_st(tmp->GetCondition(), 2 + _num);//condition只有一句话


			//for_add
			print_decoration(1 + _num);
			std::cout << "add\n";
			_fact_print_st(tmp->GetAdd(), 2 + _num);//add只有一句话



			//for_block
			v_forDisplaySemaTree[_num] = 0;//"└── "
			print_decoration(1 + _num);
			block_iter(tmp, _num+1);


		}
		else if (_node->GetClassType() == SuatinCmdClassType_AssertCmd) {
			AssertCmd* tmp = dynamic_cast<AssertCmd*>(_node);
			std::cout << "assert\n";
		}
		else if (_node->GetClassType() == SuatinCmdClassType_BreakCmd) {
			BreakCmd* tmp = dynamic_cast<BreakCmd*>(_node);
			std::cout << "break\n";
		}
		else if (_node->GetClassType() == SuatinCmdClassType_ContinueCmd) {
			ContinueCmd* tmp = dynamic_cast<ContinueCmd*>(_node);
			std::cout << "continue\n";
		}




	}





};

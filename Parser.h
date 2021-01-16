#pragma once
#ifndef _PARSER_H_
#define _PARSER_H_
#include"Expr.h"






namespace sua {



	//成员函数指针，如果不加上Parser::修饰，就只能把对应的成员函数都改成静态的！！！
	class Parser;
	typedef void(Parser::*DealFuncPtr)(int&);//单行语句解析器的函数指针



	//语法解析器——单条语句的解释
	class Parser {
	private:

		//处理Token的函数
		void DealToken_Num(int& _t);
		void DealToken_Id(int& _t);
		void DealToken_Str(int& _t);
		void DealToken_Pow(int& _t);
		void DealToken_Mul(int& _t);
		void DealToken_Div(int& _t);
		void DealToken_Add(int& _t);
		void DealToken_Sub(int& _t);
		void DealToken_Gre(int& _t);
		void DealToken_GreEq(int& _t);
		void DealToken_Les(int& _t);
		void DealToken_LesEq(int& _t);
		void DealToken_Neq(int& _t);
		void DealToken_EqEq(int& _t);
		void DealToken_Eq(int& _t);
		void DealToken_Com(int& _t);
		void DealToken_ML(int& _t);
		void DealToken_MR(int& _t);
		void DealToken_LL(int& _t);
		void DealToken_LR(int& _t);
		void DealToken_BL(int& _t);
		void DealToken_BR(int& _t);
		void DealToken_Dot(int& _t);
		void DealToken_Sem(int& _t);
		//void DealToken_Eol(int& _t);


		
		//处理关键字的函数
		void Deal_k_if(int& _t);
		void Deal_k_elif(int& _t);
		void Deal_k_else(int& _t);
		void Deal_k_for(int& _t);
		void Deal_k_break(int& _t);
		void Deal_k_continue(int& _t);
		void Deal_k_do(int& _t);
		void Deal_k_until(int& _t);
		void Deal_k_while(int& _t);
		void Deal_k_local(int& _t);
		void Deal_k_const(int& _t);
		void Deal_k_and(int& _t);
		void Deal_k_or(int& _t);
		void Deal_k_not(int& _t);
		void Deal_k_function(int& _t);
		void Deal_k_end(int& _t);
		void Deal_k_return(int& _t);
		void Deal_k_assert(int& _t);



		
		//判断小树的解释接口类型
		Unit_InterfaceType GetTree_InterfaceType(Expr* _node);

		//解释前，构造后，在构造的时候不应该确定解释接口类型，因为变量的类型是动态的。所以应该在解释前，构造后确定
		void	Confirm_ASTree_InterfaceType();//迭代函数的封装
		void _fact_Confirm_ASTree_InterfaceType(Expr* _node);//迭代函数

		
	
		void CheckASTreeSyntax();	//在创建完了最后，检查一下所有的语法问题
		void _fact_ShowASTree(Expr* _node, int _num = 0);////打印漂亮的二叉树。实际打印的函数
		void DelTree(Expr* _expr);//删除二叉树
		
	public:
		Parser(int _start=0,int _end=0);
		~Parser();


		void CreateASTree();
		void ShowASTree();//因为要迭代，所以实际打印函数有形参，但是给外部提供的打印函数不能有形参！
		void interpret();		





		//语法树是否构造完全，前提是打印完语句
		bool GetCompletedASTreeFlag()const;
	

	private:

		int start = 0;																									//当前语句在全局中缀表达式中的起始位置
		int end = 0;																									//当前语句在全局中缀表达式中的结束位置
		bool   completedASTree_flag = false;															//是否已经完成语法树的构造
		bool   end_flag = false;																				//结束标志，是否遇到了分号
		int  v_forDisplayASTree[100] = { 0 };															//打印语法树要用的工具
		std::map<SuatinTokenType, DealFuncPtr> funcMap;									//为了减少if-else的个数，使用查表的方法，根据不同的枚举来调用不同的函数
		std::map<SuatinKeyWordType, DealFuncPtr> k_funcMap;						//处理keyword的函数表
		bool	firstEq_flag = true;																				//是否第一次遇到等于号
		bool	firstId_flag = true; 																				//表达式的第一个Id是否是flag

		
		StatType    m_statType = StatType_NoEval;												//语句类型，默认非赋值式
		ExprType   m_exprType = ExprType_Simple;												//表达式类型，默认简单表达式
		SimpleExprType  m_simpleExprType = SimpleExprType_NumCalc;			//简单表达式类型，默认五则计算式



		Expr* exprNoVal = NULL;																			//非赋值表达式 a = exprNoVal ，或 exprNoVal




		/*root不为空时语句是赋值语句,exprRoot不为空时是表达式，root和exprRoot在构造完语法树后不能同时为空也不能同时不为空*/
		//与exprRoot进行交互
		Expr* root = NULL;																						//语法树根节点
		
		/*logicRoot不为空时语句和exprRoot不为空时一样，都是表达式！！！*/
		//与judgeRoot进行交互，最后用exprRoot去替换掉logicRoot
		Expr*  logicRoot = NULL;																			//逻辑式语法树根节点
		
		/*judgeRoot不为空时语句和exprRoot不为空时语句一样，都是表达式！！！	*/
		//judgeRoot与exprRoot进行交互，最后用exprRoot去替换掉judgeRoot
		Expr* judgeRoot = NULL;																			//判断式语法树根节点
											
		//普通表达式要构造语法树用得到的指针
		Expr* exprRoot = NULL;																				//表达式语法树根节点
		Expr* expTmp = NULL;																				//待处理的节点（为空）的父节点
		Expr* expTmpLeft = NULL;																			//最下层的没匹配到右括号的左括号节点
		std::stack<Expr*>  v_NoMatchedLLeft;														//expTmp，保存所有的未匹配到右括号的左括号节点
		
		
		
	};

};


#endif
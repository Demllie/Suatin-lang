#pragma once
#ifndef _RESOLVER_H_
#define _RESOLVER_H_
#include"Cmd.h"



namespace sua {


	class Resolver;
	typedef void(Resolver::*R_DealFuncPtr)(int&);//多行语句解析器的函数指针






	//语法解析器——多条语句的解释
	class Resolver
	{
	private:

		/*
		多行语句的解释并不会把单行语句作为语法树节点来创建更大的语法树，
		而是将所有的单行语句放入一个有序列表中，再根据一个【语义语法树】
		来决定这个列表中语句的解释顺序！！！		
		*/
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
		void Confirm_LR_Cond();	//确定小括号中的条件语句
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
		void Deal_k_break(int& _t);
		void Deal_k_continue(int& _t);

		
		//特殊关键字处理函数，将关键字语句压入最近的block中
		template<typename T>
		void Deal_UniqueKeyWord();


		void _fact_print_st(Cmd* _node, int _num = 0); //打印语义树，实际打印函数
		
		
		void CheckSemanticTreeSyntax();//语义树构造完了后再检查一下
		bool CheckStack_UnCompletedBlock();//检查栈是否为空
		bool CheckStatementIndex(int _index);//检查语句索引的范围

	private slots:
		void _slot_interpret(int _index); //接收从语义树中发射的信号


	public:
		Resolver();
		~Resolver();


		void create();
		void print_st();//打印语义树
		void print_ast();//打印语法树
		void print_ast(int _index);
		void interpret();





	private:

		//临时的变量，用来分割语句
		int		start = 0;
		int		end = 0;
	
		std::map<SuatinTokenType, R_DealFuncPtr> funcMap;						//为了减少if-else的个数，使用查表的方法，根据不同的枚举来调用不同的函数
		std::map<SuatinKeyWordType, R_DealFuncPtr> k_funcMap;				//处理keyword的函数表
		int count_little=0;																						//小括号计数器
		int count_middle = 0;																				//中括号计数器
		int count_big = 0;																						//大括号计数器
		std::vector<Parser*>  v_exprs;																//存放多条简单语句的有序列表
		int  v_forDisplaySemaTree[100] = { 0 };													//打印语义树要用的工具

		/*
		for循环的工具
			0 : 默认值
			1 : 遇到for后
			2 : 遇到for后面第一个语句stat_init后
			3 : 遇到for后面第二个语句stat_cond后
		*/
		int for_init_cond_add_flag = 0;																//for循环时判断三条语句的工具
		
		
		BlockCmd*	semantic_tree = NULL;														//语义树
		Cmd* uncompleted_tree = NULL;															//未完成的小语义树
		std::vector<Cmd*> v_UnCompletedBlock;												//存放未结束的语句块，结束了就抛掉
			



		//特殊关键字的布尔
		struct KeyWordFlag {
			bool assert_flag = false;
			bool break_flag = false;
			bool continue_flag = false;
			bool return_flag = false;
		}kFlag;
		//用于帮助特殊关键字跳过构造语法树的布尔
		bool	uniqueKeyWord_flag = false;

	};

};

#endif
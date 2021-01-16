#pragma once
#ifndef _CMD_H_
#define _CMD_H_
#include"Parser.h"






namespace sua {

	/*
	语义树控制语句的执行顺序，即执行哪条语句，至于语句是怎样的语法树是无所谓的！
	语义树只管拿到该语句的索引，然后调用该语句的解释接口就行
	
	语义树和语法树是交叉构造起来的，在Resolver解析器里，全局中缀表达式会被扫描一遍，分割一条条语句后，
	每条语句还会被Parser解析器扫描一遍！那么，在Resolver的构造里，总共会扫描两边，不能增加了，怕性能降低！

	
	*/



	////用来替代typeid，进行类类型判断
	enum SuatinCmdClassType{
		SuatinCmdClassType_Cmd,
		SuatinCmdClassType_SingleCmd,
		SuatinCmdClassType_BlockCmd,
		SuatinCmdClassType_CondCmd,
		SuatinCmdClassType_IfCmd,
		SuatinCmdClassType_ElifCmd,
		SuatinCmdClassType_ElseCmd,
		SuatinCmdClassType_WhileCmd,
		SuatinCmdClassType_DoUntilCmd,
		SuatinCmdClassType_ForCmd,
		SuatinCmdClassType_AssertCmd,
		SuatinCmdClassType_BreakCmd,
		SuatinCmdClassType_ContinueCmd

	};



	//跳转信息的枚举
	enum SuatinJumpSignalType {
		SuatinJumpSignalType_None,
		SuatinJumpSignalType_Break,
		SuatinJumpSignalType_Continue,
		SuatinJumpSignalType_Return
	};







	//抽象控制器
	class Cmd
	{
	public:
		virtual ~Cmd();
		virtual SuatinCmdClassType GetClassType();

	
		//控制语句解释的顺序
		virtual void interpret() = 0;

	};






	
	//单条语句控制器（终结符
	class SingleCmd : public Cmd {
	protected:
		int index = 0;//语句的顺序
	public:
		SingleCmd(int _index);
		virtual SuatinCmdClassType GetClassType();

		int GetIndex()const;
		void SetIndex(int _index);
		
	signals:
		virtual void interpret();
	
	};


	//语句块控制器（终极符
	class BlockCmd : public Cmd {		
	protected:
		std::vector<Cmd*> block;	//语句块
		bool end_flag = false;			//end结束符
		SuatinJumpSignalType jumpWhat = SuatinJumpSignalType_None;//默认无跳转信息
	public:
		~BlockCmd();
		virtual SuatinCmdClassType GetClassType();

		void Push(Cmd* _statement);
		std::vector<Cmd*> GetBlockVector() const;
		void SetEnd();
		bool IsEnd()const;
		void SetJumpSignal(SuatinJumpSignalType _jumpSignal);
		SuatinJumpSignalType GetJumpSignal()const;

		virtual void interpret();

	};


	

	//条件控制器（非终极符
	class CondCmd : public BlockCmd {
	protected:
		SingleCmd* condition = NULL;
	public:
		~CondCmd();
		void SetCondition(SingleCmd* _cond);
		SingleCmd* GetCondition()const;

		virtual SuatinCmdClassType GetClassType();

		bool IsTrue()const;
	};



	//if控制器（终结符
	class IfCmd : public CondCmd{
	public:		
		Cmd* last = NULL;//当条件不满足时，要解释下一个。if的下一个是elif或者else
	public:
		virtual SuatinCmdClassType GetClassType();

		virtual void interpret();
	};
	//elif控制器（终结符
	class ElifCmd : public IfCmd {//当条件不满足时，要解释下一个。elif的下一个是elif或者else
	public:
		virtual SuatinCmdClassType GetClassType();
	};
	//else控制器（终结符
	class ElseCmd : public BlockCmd {
	public:
		virtual SuatinCmdClassType GetClassType();
	};





	/*----------------------------------------------------------------------------------------------------------------------------loop controller---------------------------------------------------------------------------------------------------------------------*/



	//while控制器 （终结符
	class WhileCmd : public CondCmd {
	public:
		virtual SuatinCmdClassType GetClassType();

		virtual void interpret();
	};


	//do-until控制器 （终结符
	class DoUntilCmd : public CondCmd {
	public:
		virtual SuatinCmdClassType GetClassType();

		virtual void interpret();
	};


	//for控制器 （终结符
	class ForCmd : public CondCmd {
	protected:
		SingleCmd* stat_init;//初始语句
		SingleCmd* stat_add;//子增语句
	public:
		~ForCmd();
		void SetInit(SingleCmd* _stat_init);
		void SetAdd(SingleCmd* _stat_add);
		SingleCmd* GetInit()const;
		SingleCmd* GetAdd()const;
		virtual SuatinCmdClassType GetClassType();

		virtual void interpret();
	};




	/*----------------------------------------------------------------------------------------------------------------------------keyword controller---------------------------------------------------------------------------------------------------------------------*/



	//断言控制器（终结符
	class AssertCmd : public Cmd {
	public:
		virtual SuatinCmdClassType GetClassType();
		virtual void interpret();
	};





	//break控制器（终结符
	class BreakCmd : public Cmd {
	public:
		virtual SuatinCmdClassType GetClassType();
		virtual void interpret();
	};

	//continue控制器（终结符
	class ContinueCmd : public Cmd {
	public:
		virtual SuatinCmdClassType GetClassType();
		virtual void interpret();
	};









};


#endif
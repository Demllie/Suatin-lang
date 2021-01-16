#pragma once
#ifndef _EXPR_H_
#define _EXPR_H_
#include"Utils.h"





namespace sua {



	//该判断类或逻辑类中，单元（简单表达式）该使用的解释接口
	enum Unit_InterfaceType {
		Unit_InterfaceType_number,  //调用interpret接口
		Unit_InterfaceType_string,     //调用interpret_str接口
		Unit_InterfaceType_bool        //调用interpret_bool接口
	};




	//用来替代typeid，进行类类型判断
	enum SuatinExprClassType {
		SuatinExprClassType_Expr,
		SuatinExprClassType_VarExpr,
		SuatinExprClassType_SymbolExpr,
		SuatinExprClassType_OneOpExpr,

		SuatinExprClassType_LLeftExpr,
		SuatinExprClassType_NotExpr,

		SuatinExprClassType_InterfaceTypeExpr,
		SuatinExprClassType_AndExpr,
		SuatinExprClassType_OrExpr,
		SuatinExprClassType_EqEqExpr,
		SuatinExprClassType_NeqExpr,

		
		SuatinExprClassType_IDExpr ,
		SuatinExprClassType_NumExpr,
		SuatinExprClassType_StrExpr,

		SuatinExprClassType_GreExpr ,
		SuatinExprClassType_GreEqExpr,
		SuatinExprClassType_LesExpr,
		SuatinExprClassType_LesEqExpr,

		SuatinExprClassType_AddExpr,
		SuatinExprClassType_SubExpr,
		SuatinExprClassType_MulExpr,
		SuatinExprClassType_DivExpr,
		SuatinExprClassType_PowExpr,

		SuatinExprClassType_EqExpr


	};





	//抽象表达式
	class Expr
	{
	public:
		virtual ~Expr();  //虚析构让对象释放的时候不会调用到这个类的析构函数
		virtual SuatinExprClassType GetClassType();

		/*接口不能写成纯虚函数，因为我已经增加了三个接口，可能还要增加
		如果是纯虚函数的话，不管子类用不用得到都要实现该接口，这样的话增加一个接口就要修改所有的类！！！
		现在我只写成虚函数，子类只实现自己需要的接口就行！！！
		*/		
		virtual double interpret();					 //五则运算
		virtual std::string interpret_str();		 //字符串拼接
		virtual bool interpret_bool();             //布尔运算

	};



	//变量解析器（非终结符
	class VarExpr : public Expr {
	protected:
		std::string name = "";//常数、字符串、标识符的名称
	public:
		VarExpr(std::string _name);
		virtual ~VarExpr();
		std::string GetName()const;
		virtual SuatinExprClassType GetClassType();
	};



	//抽象符号解析器（非终结符），二元操作
	class SymbolExpr : public Expr {
	protected:
		Expr* left = NULL;
		Expr* right = NULL;
	public:
		SymbolExpr(Expr* _left, Expr* _right);
		~SymbolExpr();
		Expr* GetLeft()const;
		Expr* GetRight()const;
		void SetLeft(Expr* _left);
		void SetRight(Expr* _right);
		virtual SuatinExprClassType GetClassType();
	};



	//抽象符号解析器（非终结符），一元操作
	class OneOpExpr : public Expr {
	protected:
		Expr* content = NULL;
	public:
		OneOpExpr() = default;
		OneOpExpr(Expr* _content);
		~OneOpExpr();
		Expr* GetContent()const;
		void SetContent(Expr* _content);
		virtual SuatinExprClassType GetClassType();

	};

	/*--------------------------------------------------------------------------------------------OneOpExpr  operate sign------------------------------------------------------------------------------------------------*/



	//左括号解析器
	class LLeftExpr : public OneOpExpr {
	protected:
		bool  matched_flag = false;//是否匹配到右括号
	public:
		LLeftExpr() = default;
		LLeftExpr(Expr* _content);
		bool GetMatchedFlag()const;
		void SetMatchedFlag();
		virtual SuatinExprClassType GetClassType();


		virtual double interpret();
		virtual bool interpret_bool();

	};
	
	//Not解析器
	class NotExpr : public OneOpExpr {
	private:
		Unit_InterfaceType  i_type = Unit_InterfaceType_number;//默认调用interpret接口
	public:
		NotExpr() = default;
		NotExpr(Expr* _content);
		virtual SuatinExprClassType GetClassType();
		Unit_InterfaceType Get_InterfaceType()const;
		void Set_InterfaceType(Unit_InterfaceType _i_type);

		virtual bool interpret_bool();
	};


	/*--------------------------------------------------------------------------------------------SymbolExpr logic operate sign------------------------------------------------------------------------------------------------*/
	


	//接口类型传递解析器，保证解释语法树时，能调用到准确的接口
	class InterfaceTypeExpr : public SymbolExpr{
	protected:
		Unit_InterfaceType left_i_type = Unit_InterfaceType_number;//默认调用interpret接口
		Unit_InterfaceType right_i_type = Unit_InterfaceType_number;//默认调用interpret接口
	public:
		InterfaceTypeExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();

		
		Unit_InterfaceType GetLeft_InterfaceType()const;
		Unit_InterfaceType GetRight_InterfaceType()const;
		void SetLeft_InterfaceType(Unit_InterfaceType _i_type);
		void SetRight_InterfaceType(Unit_InterfaceType _i_type);

	};



	
	
	//and解析器
	class AndExpr : public InterfaceTypeExpr {
	public:
		AndExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();


		virtual bool interpret_bool();
	};


	//or解析器
	class OrExpr : public InterfaceTypeExpr {
	public:
		OrExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();


		virtual bool interpret_bool();
	};



	/*--------------------------------------------------------------------------------------------SymbolExpr evaluation operate sign------------------------------------------------------------------------------------------------*/

	//赋值符号解析器（非终极符
	class EqExpr : public SymbolExpr {
	public:
		EqExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();


		virtual double interpret();
		virtual std::string interpret_str();
		virtual bool interpret_bool();

	};





	/*--------------------------------------------------------------------------------------------SymbolExpr judge operate sign------------------------------------------------------------------------------------------------*/

	//大于号解析器   >
	class GreExpr : public SymbolExpr {
	public:
		GreExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();


		virtual bool interpret_bool();

	};
	//大于等于号解析器  >=
	class GreEqExpr : public SymbolExpr {
	public:
		GreEqExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();


		virtual bool interpret_bool();
	};
	//小于号解析器 <
	class LesExpr : public SymbolExpr {
	public:
		LesExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();


		virtual bool interpret_bool();
	};
	//小于等于解析器 <=
	class LesEqExpr : public SymbolExpr {
	public:
		LesEqExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();


		virtual bool interpret_bool();
	};

	/*
	and  not  or  Neq  EqEq  
	这几个拥有调用不同解释接口的能力！
	*/

	//不等于号解析器 ~=
	class NeqExpr : public InterfaceTypeExpr {
	public:
		NeqExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();


		virtual bool interpret_bool();
	};
	//等于等于号解析器 ==
	class EqEqExpr : public InterfaceTypeExpr {
	public:
		EqEqExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();


		virtual bool interpret_bool();
	};


	/*--------------------------------------------------------------------------------------------SymbolExpr Calculate sign------------------------------------------------------------------------------------------------*/


	//加法解析器
	class AddExpr : public SymbolExpr {
	public:
		AddExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();


		//加法运算
		virtual double interpret();

		//字符串拼接
		virtual std::string interpret_str();
	};

	//减法解析器
	class SubExpr : public SymbolExpr {
	public:
		SubExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();

		virtual double interpret();
		
	};

	//乘法解析器
	class MulExpr : public SymbolExpr {
	public:
		MulExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();

		virtual double interpret();
		
	};

	//除法解析器
	class DivExpr : public SymbolExpr {
	public:
		DivExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();

		virtual double interpret();
		
	};

	//乘方解析器
	class PowExpr : public SymbolExpr {
	public:
		PowExpr(Expr* _left, Expr* _right);
		virtual SuatinExprClassType GetClassType();

		virtual double interpret();
		
	};




	/*--------------------------------------------------------------------------------------------VarExpr sign------------------------------------------------------------------------------------------------*/

	//ID解析器
	class IDExpr : public VarExpr {
	public:
		IDExpr(std::string _name);
		virtual SuatinExprClassType GetClassType();


		//获取变量属性
		double GetNumber()const;
		int GetInt()const;
		SuatinIDType GetType()const;
		std::string GetString()const;
		bool GetBool()const;
		bool IsConst()const;

		//设置变量属性
		void SetNumber(double _value);
		void SetInt(int _value);
		void SetString(std::string _value);
		void SetBool(bool _value);		

		
		virtual double interpret();
		virtual std::string interpret_str();
		virtual bool interpret_bool();

	};

	//NUM解析器
	class NumExpr : public VarExpr {
	public:
		NumExpr(std::string _name);
		virtual SuatinExprClassType GetClassType();

		virtual double interpret();
	};


	//STR解析器
	class StrExpr : public VarExpr {
	public:
		StrExpr(std::string _name);
		virtual SuatinExprClassType GetClassType();

		virtual std::string interpret_str();

	};






};

#endif
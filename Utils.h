#pragma once
#ifndef _UTILS_H_
#define _UTILS_H_
#include<iostream>
#include<windows.h>
#include<string>
#include<regex>
#include<iomanip>
#include<functional>
#include<memory>
#include<algorithm>
#include<stack>
#include<ctime>
#include<fstream>
#include<sstream>
#include<list>
#include<vector>
#include<exception>
#include<map>
#include<thread>
#include<mutex>
#include<cmath>




//代码转字符串
#define CODE2STR(x)   #x


//计算函数运行时间
#define SHOWFUNCTIME(str,func) \
{\
time_t start = clock();\
func();\
std::cout <<str<< " time consumed " << clock() - start << " ms \n";\
}


//标识符最多只能有200个
#define  MAX_ID_NUM  200

//布尔转字符串
#define BOOL2STR(x)  (x?"true":"false")


//变量类型的枚举转字符串
#define IDTYPE2STR(x)  (x==0?"nil":( x==1?"int":(x==2?"bool":(x==3?"number":(x==4?"string":(x==5?"function":(x==6?"array":"other"))))) ))



//Suatin的slot/signals机制，最简易的信号槽了
#define emit			
#define slots
#define signals public
//非成员信号 -> 非成员函数
//非成员信号 -> 成员函数
#define Connect(signal,slot)   ((signal).Bind(slot))   /*绑定信号和执行函数*/



//正在测试
//#define _SUATIN_DEBUG_






namespace sua {




	/*--------------------------------------------------------------------------------------------------------------------------------------Suatin code reader ---------------------------------------------------------------------------------------------------------*/
	//解释器运行的三个阶段
	enum SuatinRunTimeType {
		SuatinRunTimeType_Lex,
		SuatinRunTimeType_Parse,
		SuatinRunTimeType_Interpret
	};


	//词法期起作用
	extern std::string g_file_dir;									//文件路径
	//词法期、语法期、解释期起作用
	extern int g_statement_index;								//语句索引，当前语句索引，用来构造语法树、打印语句、打印语法树
	//该语句的开始，在Parser类中初始化，因为Expr中出异常时，没有start,end，所以需要用全局变量传递过去
	//解释时候起作用
	extern int g_statement_start;								
	extern int g_statement_end;								
	//阶段之间起作用
	extern bool g_error_lex_flag;								//词法期错误
	extern bool g_error_parse_flag;							//语法期错误
	extern bool g_error_interpret_flag;						//解释期错误
	extern SuatinRunTimeType g_run_time;				//运行阶段
	//解释时起作用
	extern bool g_statement_returnVal;						//语句返回是否为真
	




	//词法分析
	void Lexer();





	/*--------------------------------------------------------------------------------------------------------------------------SuatinToken & SuatinKeyWord -------------------------------------------------------------------------------------------*/
	//cannot change the sequence 
	enum SuatinTokenType {
		SuatinTokenType_Note = 1,
		SuatinTokenType_Num = 7,
		SuatinTokenType_Id = 10,
		SuatinTokenType_Str,
		SuatinTokenType_Pow,
		SuatinTokenType_Mul,
		SuatinTokenType_Div,
		SuatinTokenType_Add,
		SuatinTokenType_Sub,
		SuatinTokenType_GreEq,
		SuatinTokenType_LesEq,
		SuatinTokenType_Gre,
		SuatinTokenType_Les,
		SuatinTokenType_Neq,
		SuatinTokenType_EqEq,
		SuatinTokenType_Eq,
		SuatinTokenType_Com,  //comma ,
		SuatinTokenType_ML,
		SuatinTokenType_MR,
		SuatinTokenType_LL,
		SuatinTokenType_LR,
		SuatinTokenType_BL,
		SuatinTokenType_BR,
		SuatinTokenType_Dot,
		SuatinTokenType_Sem //semicolon ;
		//SuatinTokenType_Eol    //end of line
		

	};



	//Suatin语言中，NIL,TRUE,FALSE不是关键字，是特殊变量
	enum SuatinKeyWordType {
		SuatinKeyWordType_NOT_KEYWORD,  //非关键字，默认token中的标识符都是这个
		SuatinKeyWordType_if,
		SuatinKeyWordType_elif,
		SuatinKeyWordType_else,
		SuatinKeyWordType_for,
		SuatinKeyWordType_break,
		SuatinKeyWordType_continue,
		SuatinKeyWordType_do,
		SuatinKeyWordType_until,
		SuatinKeyWordType_while,
		SuatinKeyWordType_local,
		SuatinKeyWordType_const,
		SuatinKeyWordType_and,  //biop
		SuatinKeyWordType_or,     //biop
		SuatinKeyWordType_not,   //aop		
		SuatinKeyWordType_function,
		SuatinKeyWordType_end,
		SuatinKeyWordType_return,
		SuatinKeyWordType_assert  //for debug

	};





	typedef struct _SuatinToken {
		std::string name = "";																					//符号名
		SuatinTokenType type = SuatinTokenType_Note;										//Token种类,默认是注释，注释是要被忽略的
		int pos = 0;																									//此Token在中缀表达式的的位置
		SuatinKeyWordType k_type = SuatinKeyWordType_NOT_KEYWORD;	//默认是非关键字

		_SuatinToken(std::string _name, int _pos, SuatinTokenType _type);

	}SuatinToken;



	//全局中缀表达式
	extern std::vector<SuatinToken*> global_infix;

	

	//显示全局中缀表达式
	void ShowGlobalInfix();

	//关键字判断和分类
	void CheckKeyWordType(SuatinToken*  _token);

	

	

	/*----------------------------------------------------------------------------------------------------------------------------------Suatin project inner exception------------------------------------------------------------------------------------------------*/
	//错误类型的枚举
	enum SuatinErrorType {
		SuatinErrorType_None,								//缺失这个变量
		SuatinErrorType_Name,								//命名错误，拿没赋值的NIL变量来操作引起的错误
		SuatinErrorType_ZeroDivisor,						//零是除数
		SuatinErrorType_Syntax,							//语法错误，比如拼写错误，右括号没匹配，语法树构造完了还有未处理的节点
		SuatinErrorType_IO,									//文件读写、在终端中输出输入……都是IO错误
		SuatinErrorType_Value,								//值错误
		SuatinErrorType_Type,								//类型错误
		SuatinErrorType_OutRange,						//超出范围
		SuatinErrorType_Other								//未知错误，这个用不到，毕竟所有的抛出异常函数都是自己写的，没理由写一个未知错误类型
	};


	//自定义异常类
	class SuatinExcept : public std::exception
	{
	public:
		SuatinExcept(std::string error){
			msg=error;
		}
		const char* what() const throw () {
        	return msg.c_str();
		}
	private:
		std::string msg;
	};


	/*
	Suatin这个项目都异常真是写炸了！！！
	几千行的代码中，异常的抛出修改了好多次，每次多要改几百个位置！！！
	下次设计异常抛出要好好设计，注意下面几点
		1.准确且丰富的错误类型
		2.准确的错误位置
		3.能把所有的问题都找出来

	*/

	
	//与语句无关的异常信息
	void ThrowException(SuatinErrorType _errorType, std::string _description);

	//语句中出现的异常信息
	void ThrowException(SuatinErrorType _errorType, int _start, int _end, std::string _description);

	//实际的打印异常信息的函数
	void _fact_ThrowException(SuatinErrorType _errorType, std::string _errorString, std::string _description);

	//打印红色的异常信息
	void PrintException(std::string  _info);


	/*----------------------------------------------------------------------------------------------------------------------Suatin  Id and its Environment---------------------------------------------------------------------------------------------------*/
	
	//变量类型的枚举
	enum SuatinIDType {
		SuatinIDType_nil,
		SuatinIDType_int,
		SuatinIDType_bool,
		SuatinIDType_number,
		SuatinIDType_string,
		SuatinIDType_function,
		SuatinIDType_array

	};


	//变量实际的值
	typedef union _SuatinIDValue {
		explicit _SuatinIDValue(){}
		~_SuatinIDValue(){}
		bool flag;
		int int_num;
		double double_num;
		std::string* str;
	}SuatinIDValue;

	//变量
	typedef struct _SuatinID {
		SuatinIDType type;
		SuatinIDValue val;
		explicit _SuatinID();
		explicit _SuatinID(bool _flag);
		explicit _SuatinID(int _int_num);
		explicit _SuatinID(double _double_num);
		explicit _SuatinID(std::string _str);
		~_SuatinID();
	}SuatinID;



	//typedef std::map<std::string, SuatinID*> _VarZone_Unit;
	//extern std::vector<_VarZone_Unit*> SuatinEnv;

	//语言环境
	extern std::map<std::string, SuatinID*> SuatinEnv;







	//初始化环境，提供一些必要的ID
	void InitEnv();

	//打印环境变量的信息
	void PrintEnvInfo();

	//检查变量个数、并且判断是否存在这个变量，存在就不创建了
	bool CheckIDExisted(std::string _name);
	
	
	//创建变量
	void CreateID(std::string _name);
	template<typename T>
	void CreateID(std::string _name, T _value) {
		if (CheckIDExisted(_name))return;
		SuatinID* id = new SuatinID(_value);
		SuatinEnv[_name] = id;
	}


	//改变变量的值
	void SetID(std::string _name, bool  _flag);
	void SetID(std::string _name, int  _int_num);
	void SetID(std::string _name, double  _double_num);
	void SetID(std::string _name, std::string  _str);


	//拷贝变量的值
	void CopyID(std::string _dst, std::string _src);
	

	//释放环境内存
	void FreeEnv();



	



	/*----------------------------------------------------------------------------------------------------------------   classfiy expression type and statement type ----------------------------------------------------------------------------------------------*/


	//语句类型
	//如果是if、while后面的判断语句，一定要是非赋值式，但是其表达式类型不要求
	//如果是语句块中的语句，一定要是赋值式
	enum StatType{
		StatType_NoEval,						//非赋值式
		StatType_Eval							    //赋值式
	};

	
	//表达式类型，和语句类型有交叉。如果是非赋值式的话，那么语句肯定是三种表达式之一
	//默认表达式为简单表达式，遇到判断符后转为判断式，遇到逻辑符后转为逻辑式
	//在一个Parser实例里，表达式类型的转换只有一个方向
	enum ExprType{
		ExprType_Simple,						//简单表达式
		ExprType_Compare,					//判断式
		ExprType_Logic							//逻辑式
	};

	//简单表达式类型，简单表达式是表达式的单元，也是语句的单元
	//默认是五则计算，遇到字符串转变为字符串拼接
	//字符串拼接模式下，如果又遇到了数字，就报错！
	//在一个Parser实例里，简单表达式类型的转换只有一个方向
	enum SimpleExprType{
		SimpleExprType_NumCalc,		//五则计算式，式中只能有数字、数字类型的id
		SimpleExprType_StrLink             //字符串拼接式，式子只能有字符串、字符串类型的id
	};









	/*----------------------------------------------------------------------------------------------------------------  Suatin singnals and slots mechanism  ---------------------------------------------------------------------------------------------------------*/

	
	//slot
	template<typename ... Args>
	class SuatinSlot {
	public:
		using FuncPtr = std::function<void(Args ...)>; //重命名函数容器
	private:
		FuncPtr funcPtr = NULL;
	public:
		SuatinSlot(const FuncPtr& _funcPtr) {
			funcPtr = _funcPtr;//传入槽
		}
		//执行绑定的槽
		void Exec(Args ... args) {
			funcPtr(std::forward<Args>(args)...);  //执行绑定的方法
		}
	};


	//signal
	template<typename ... Args>
	class SuatinSignal { 
	public:
		using SlotPtr = std::shared_ptr<SuatinSlot<Args...>>;//重命名Slot的智能指针
		using FuncPtr = std::function<void(Args ...)>; //重命名函数容器
	private:
		std::vector<SlotPtr> v_slots;
	public:
		//绑定信号和槽
		void Bind(const FuncPtr& _funcPtr) {
			SuatinSlot<Args...>* p = new SuatinSlot<Args...>(_funcPtr);  //生成一个slot实例
			v_slots.push_back(SlotPtr(p));   //放入智能指针，并把智能指针放入slot容器中
		}
		//发射信号
		void operator()(Args...args) {
			//void Emit(Args ... args){
			for (auto& it : v_slots) {
				it->Exec(std::forward<Args>(args)...);//执行所有绑定的方法，因为一个信号能触发多个动作
			}
		}
	};


	//语义树发给Resolver的信号
	extern SuatinSignal<int>* g_signal;



};

#endif
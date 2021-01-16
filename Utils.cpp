#include "Utils.h"


namespace sua {

	//初始化几乎所有的全局变量
	std::string g_file_dir = "";					
	int g_statement_index = 0;	
	int g_statement_start=0;						
	int g_statement_end=0;			
	bool g_error_lex_flag = false;
	bool g_error_parse_flag = false;			
	bool g_error_interpret_flag = false;	
	SuatinRunTimeType g_run_time = SuatinRunTimeType_Lex;
	bool g_statement_returnVal = true;
	std::vector<SuatinToken*> global_infix;
	std::map<std::string, SuatinKeyWordType> string_keyword_map = {};
	std::map<std::string, SuatinID*> SuatinEnv = {};
	SuatinSignal<int>* g_signal = NULL;
	

	
	








	void InitEnv() {

		CreateID("NIL");
		CreateID("TRUE", true);
		CreateID("FALSE", false);

	}



	void ShowGlobalInfix() {
		std::cout << "local infix expression>";
		std::cout << "\n\t\tname\t pos\ttype\tkeyword" << std::endl;
		//for (std::vector<SuatinToken*>::iterator& it = global_infix.begin(); it != global_infix.end(); ++it) {
		for(auto it : global_infix){
			std::cout << std::setw(20);
			if (it->name == "\n")std::cout << "\\n";
			else std::cout << it->name;
			std::cout << std::setw(8) << it->pos
				<< std::setw(8) << it->type
				<< std::setw(8) << it->k_type
				<< std::endl;

		}
	}


	void PrintEnvInfo() {

		//map根据属性type进行排序
		auto compare = [](const std::pair<std::string, SuatinID*>& _arr1, const std::pair<std::string, SuatinID*>& _arr2) {
			return _arr1.second->type <  _arr2.second->type;
		};
		//map是不能排序的，内容是无序的，所以只能用别的容器来排序！！！
		std::vector< std::pair<std::string, SuatinID*>> v_forSortMap;
		//for (std::map<std::string, SuatinID*>::iterator it = SuatinEnv.begin(); it != SuatinEnv.end(); ++it) {
		//	v_forSortMap.push_back(std::make_pair((*it).first, (*it).second));
		//}
		for (auto it : SuatinEnv) {
			v_forSortMap.push_back(std::make_pair(it.first,it.second));
		}
		sort(v_forSortMap.begin(), v_forSortMap.end(),compare);//vector排序


		

		//打印信息
		std::cout << "suatin environment>\n";
		std::cout << "------------------------------------------------------------------------------\n";
		std::cout <<std::setw(21)<< "name" << std::setw(9) <<"type"<< std::setw(9) <<"value" << std::endl;;
		for (auto it : v_forSortMap) {
			std::cout << std::right<<std::setw(21)<< it.first << std::setw(9)<< IDTYPE2STR(it.second->type);
			std::cout << "    " <<std::setw(12)<< std::left;
			switch (it.second->type) {
			case SuatinIDType_nil:
				std::cout << "false";
				break;
			case SuatinIDType_int:
				std::cout<< it.second->val.int_num;
				break;
			case SuatinIDType_number:
				std::cout  << it.second->val.double_num;
				break;
			case SuatinIDType_bool:
				std::cout  << BOOL2STR(it.second->val.flag);
				break;
			case SuatinIDType_string:
				std::cout  << (*it.second->val.str);
				break;
			}
			std::cout << std::endl;
		}
		std::cout << "------------------------------------------------------------------------------\n";
		//std::cout << "\n                name   isconst    type      funcPtr     flag             num   str" << std::endl;
		//for (std::map<std::string, SuatinID*>::iterator it = SuatinEnv.begin();it!=SuatinEnv.end(); ++it) {
		//for(std::vector< std::pair<std::string, SuatinID*>>::iterator it=v_forSortMap.begin();it!=v_forSortMap.end();++it){
		//	std::cout<<std::setw(20) 
		//		<<std::right
		//		<< (*it).first
		//		<< std::setw(8) << BOOL2STR( (*it).second->isconst)
		//		<< std::setw(10) << IDTYPE2STR(  (*it).second->type)
		//		<< std::setw(13) << (*it).second->value.funcPtr
		//		<< std::setw(9) << BOOL2STR((*it).second->value.flag)
		//		<< std::setw(16) << (*it).second->value.num
		//		<<"   " <<std::setw(30) 
		//		<<std::left
		//		<< (*it).second->value.str     //除了string左对齐外，其他都右对齐
		//		<< std::endl;
		//
		//}


	}









	void Lexer() {

		std::ifstream file(g_file_dir);

		try {
			if (file.is_open()) {

				/*
				注释一定要放在最前面，至少放在/前面
				数字要放到标识符前面，一定要放在点前面
				>=,<=一定要放在>,<前面
				==，~=，>=,<=一定要放在=前面
				
				多行注释失效了。。。

				这里的括号不能动，因为括号数和括号的位置与token&keyword的枚举有关
				*/
				std::string regStr = "((//.*((\n)$)?)|(/\\*(.|\n)*\\*/))|"\
					"(\\d+(\\.\\d+)?(e[\\+\\-]?\\d+)?)|"\
					"([a-zA-Z_]\\w*)|"\
					"(\"[^\"]*\")|"\
					"(\\^)|"\
					"(\\*)|"\
					"(\\/)|"\
					"(\\+)|"\
					"(\\-)|"\
					"(>=)|"\
					"(<=)|"\
					"(>)|"\
					"(<)|"\
					"(~=)|"\
					"(==)|"\
					"(=)|"\
					"(,)|"\
					"(\\[)|"\
					"(\\])|"\
					"(\\()|"\
					"(\\))|"\
					"(\\{)|"\
					"(\\})|"\
					"(\\.)|"\
					"(;)";
				//\n不处理！！！


				std::string file_content;
				//一行一行的处理
				while (std::getline(file, file_content)) {
					
					std::regex reg(regStr);
					for (std::sregex_iterator it(file_content.begin(), file_content.end(), reg), it_end; it != it_end; ++it) {
						int index = 0;
						for (int i = 1; i < 34; ++i) {
							/*std::cout << i << ">";
							if ((*it).str(i) == "\n")std::cout << "\\n ";
							else std::cout << (*it).str(i) << " ";*/
							if ((*it).str(i) != "") {
								index = i;
								break;//找到了就不继续遍历了
							}
						}
						//std::cout << ">index=" << std::setw(4) << index << std::endl;
						if (index == 0) {
							ThrowException(SuatinErrorType_Value,"wrong index");
						}
						else if (index > 1) { //去掉注释
							SuatinToken* t = new SuatinToken((*it).str(), global_infix.size(), (SuatinTokenType)(index));
							CheckKeyWordType(t);
							global_infix.push_back(t);
						}

					}
				}
			}
			else {
				ThrowException(SuatinErrorType_IO, "file cannot open");
			}

		}
		catch (SuatinExcept& e) {
			PrintException(e.what());
		}
		catch (...) {
			PrintException("lexer analysis error\n");
		}
	}



	   





	void CheckKeyWordType(SuatinToken*  _token){
		//不是标识符要退出
		if (_token->type != SuatinTokenType_Id)return;

		/*
		为什么这么多if却不用hash容器？？？

		因为用了 string->int 的map容器后，发现速度变慢了！！！！
		不知道什么原因，反正用map容器没有用if 快！！！
		*/
		std::string name = _token->name;
		if (name == "if") {
			_token->k_type = SuatinKeyWordType_if;
		}
		else if (name == "elif") {
			_token->k_type = SuatinKeyWordType_elif;
		}
		else if (name == "else") {
			_token->k_type = SuatinKeyWordType_else;
		}
		else if (name == "for") {
			_token->k_type = SuatinKeyWordType_for;
		}
		else if (name == "break") {
			_token->k_type = SuatinKeyWordType_break;
		}
		else if (name == "continue") {
			_token->k_type = SuatinKeyWordType_continue;
		}
		else if (name == "do") {
			_token->k_type = SuatinKeyWordType_do;
		}
		else if (name == "until") {
			_token->k_type = SuatinKeyWordType_until;
		}
		else if (name == "while") {
			_token->k_type = SuatinKeyWordType_while;
		}
		else if (name == "local") {
			_token->k_type = SuatinKeyWordType_local;
		}
		else if (name == "const") {
			_token->k_type = SuatinKeyWordType_const;
		}
		else if (name == "and") {
			_token->k_type = SuatinKeyWordType_and;
		}
		else if (name == "or") {
			_token->k_type = SuatinKeyWordType_or;
		}
		else if (name == "not") {
			_token->k_type = SuatinKeyWordType_not;
		}
		else if (name == "function") {
			_token->k_type = SuatinKeyWordType_function;
		}
		else if (name == "end") {
			_token->k_type = SuatinKeyWordType_end;
		}
		else if (name == "return") {
			_token->k_type = SuatinKeyWordType_return;
		}
		else if (name == "assert") {
			_token->k_type = SuatinKeyWordType_assert;
		}


	}




	_SuatinToken::_SuatinToken(std::string _name, int _pos, SuatinTokenType _type) {
		name = _name;
		type = _type;
		pos = _pos;

	}





	

	bool CheckIDExisted(std::string _name){
		try {
			if (SuatinEnv.size() >= MAX_ID_NUM) {
				ThrowException(SuatinErrorType_OutRange,"id number more than MAX_ID_NUM");
			}
		
			//该变量是否有一个了，有的话返回真，不再创建此变量
			if (SuatinEnv.count(_name) == 1)return true;
				
		}
		catch (SuatinExcept& e) {
			std::cout << e.what();
		}
		return false;
	}




	void PrintException(std::string  _info){
		//设置异常信息的字体颜色
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED); //高亮红色
		std::cout << _info;
		//恢复字体颜色
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),  FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);//白色
	}








	void _fact_ThrowException(SuatinErrorType _errorType, std::string _errorString, std::string _description) {

		//打印错误类型和错误信息
		switch (_errorType) {
		case SuatinErrorType_Name:
			_errorString += "SuatinErrorType_Name : " + _description;
			break;
		case SuatinErrorType_ZeroDivisor:
			_errorString += "SuatinErrorType_ZeroDivisor : " + _description;
			break;
		case SuatinErrorType_Syntax:
			_errorString += "SuatinErrorType_Syntax : " + _description;
			break;
		case SuatinErrorType_IO:
			_errorString += "SuatinErrorType_IO : " + _description;
			break;
		case SuatinErrorType_Value:
			_errorString += "SuatinErrorType_Value : " + _description;
			break;
		case SuatinErrorType_Type:
			_errorString += "SuatinErrorType_Type : " + _description;
			break;
		case SuatinErrorType_OutRange:
			_errorString += "SuatinErrorType_OutRange : " + _description;
			break;
		case SuatinErrorType_Other:
			_errorString += "SuatinErrorType_Other : " + _description;
			break;
		}


		//是否是语法期/解释期错误
		switch (g_run_time) {
		case SuatinRunTimeType_Lex:
			_errorString += " in lex period\n";
			g_error_lex_flag = true;
			break;
		case SuatinRunTimeType_Parse:
			_errorString += " in parse period\n";
			g_error_parse_flag = true;
			break;
		case SuatinRunTimeType_Interpret:
			_errorString += " in interpret period\n";
			g_error_interpret_flag = true;
			break;
		}

		throw SuatinExcept(_errorString);

	}


	//与语句无关的异常信息
	void ThrowException(SuatinErrorType _errorType, std::string _description) {
		//打印文件路径和语句索引
		std::string errorString = "Traceback at : File \"" + g_file_dir + "\"\n";
		_fact_ThrowException(_errorType,errorString, _description);
	}

	//语句中出现的异常信息
	void ThrowException(SuatinErrorType _errorType, int _start, int _end, std::string _description) {
		//打印文件路径和语句索引
		std::string errorString = "Traceback at : File \"" + g_file_dir + "\",no. " + std::to_string(g_statement_index) + "\n\t>";

		//打印该语句的内容
		for (int i = _start; i <= _end; ++i) {
			errorString += global_infix[i]->name;
		}
		errorString += "\n";

		_fact_ThrowException(_errorType,errorString, _description);
	}





	_SuatinID::_SuatinID() {
		type = SuatinIDType_nil;
	}
	_SuatinID::_SuatinID(bool _flag) {
		type = SuatinIDType_bool;
		val.flag = _flag;
	}
	_SuatinID::_SuatinID(int _int_num) {
		type = SuatinIDType_int;
		val.int_num = _int_num;
	}
	_SuatinID::_SuatinID(double _double_num) {
		type = SuatinIDType_number;
		val.double_num = _double_num;
	}
	_SuatinID::_SuatinID(std::string _str) {
		type = SuatinIDType_string;
		val.str = new std::string(_str);
	}
	_SuatinID::~_SuatinID() {
		if (type == SuatinIDType_string) {
			if (val.str != NULL) {
				delete val.str;
				val.str = NULL;
			}
		}
	}



	void CreateID(std::string _name) {
		if (CheckIDExisted(_name))return;
		SuatinID* id = new SuatinID();
		SuatinEnv[_name] = id;
	}

	void SetID(std::string _name, bool  _flag) {
		if (CheckIDExisted(_name) == false) {
			ThrowException(SuatinErrorType_None, "[bool] environment donot have the identifier");
			return;//没有这个变量
		}
		SuatinEnv[_name]->type = SuatinIDType_bool;
		SuatinEnv[_name]->val.flag = _flag;
	}

	void SetID(std::string _name, int  _int_num) {
		if (CheckIDExisted(_name) == false) {
			ThrowException(SuatinErrorType_None, "[int] environment donot have the identifier");
			return;//没有这个变量
		}
		SuatinEnv[_name]->type = SuatinIDType_int;
		SuatinEnv[_name]->val.int_num = _int_num;
	}

	void SetID(std::string _name, double  _double_num) {
		if (CheckIDExisted(_name) == false) {
			ThrowException(SuatinErrorType_None, "[number] environment donot have the identifier");
			return;//没有这个变量
		}
		SuatinEnv[_name]->type = SuatinIDType_number;
		SuatinEnv[_name]->val.double_num = _double_num;
	}

	void SetID(std::string _name, std::string  _str) {
		if (CheckIDExisted(_name) == false) {
			ThrowException(SuatinErrorType_None, "[string] environment donot have the identifier");
			return;//没有这个变量
		}
		SuatinEnv[_name]->type = SuatinIDType_string;
	/*	std::string* s_tmp = SuatinEnv[_name]->val.str;
		if (s_tmp != NULL) {
			delete s_tmp;
			s_tmp = NULL;
		}*/
		SuatinEnv[_name]->val.str = new std::string(_str);
	}

	void CopyID(std::string _dst, std::string _src) {
		SuatinEnv[_dst]->type = SuatinEnv[_src]->type;
		SuatinEnv[_dst]->val = SuatinEnv[_src]->val;
	}




	void FreeEnv() {
		SuatinEnv.clear();
		std::map<std::string, SuatinID*>().swap(SuatinEnv);
	}






};
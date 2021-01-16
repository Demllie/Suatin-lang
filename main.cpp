#include"Resolver.h"


int main(int argc, char *argv[]){

//int main(void){

	//int argc = 2;

	if (argc < 2) {//参数不够
		std::cout << "created executed file!"<<std::endl;
		return 1;
	}

	//std::string s = "main.suatin"; 
	std::string s= argv[1];
	std::string suffixStr =s.substr(s.find_last_of('.') + 1);//获取文件后缀
	if (suffixStr != "suatin") {
		std::cout << "文件不是suatin类型文件" << std::endl;
		//system("pause");
		return 1;
	}
	sua::g_file_dir = argv[1];//获取文件路径
	//sua::g_file_dir = "main.suatin";

	time_t start = clock();

	////初始化环境
	sua::InitEnv();


	//词法分析
	sua::Lexer();
	////显示全局中缀表达式
	////SHOWFUNCTIME("全局中缀表达式", sua::ShowGlobalInfix);
	sua::ShowGlobalInfix();

	////语法分析
	sua::Resolver  p;
	p.create();
	p.print_ast();
	p.print_st();
	p.interpret();

	//显示环境信息
	sua::PrintEnvInfo();
	
	//释放环境
	sua::FreeEnv();

	
	std::cout << "program time consumed " << clock() - start <<" ms" <<std::endl;


	//system("pause");
	return 0;
}

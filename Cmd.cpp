#include "Cmd.h"



namespace sua {


	Cmd::~Cmd() { //虚析构

	}
	SuatinCmdClassType Cmd::GetClassType() {
		return SuatinCmdClassType_Cmd;
	}


	SingleCmd::SingleCmd(int _index) {
		index = _index;
	}

	SuatinCmdClassType SingleCmd::GetClassType() {
		return SuatinCmdClassType_SingleCmd;
	}


	int SingleCmd::GetIndex()const {
		return index;
	}
	void SingleCmd::SetIndex(int _index) {
		index = _index;
	}


	void SingleCmd::interpret() {
		emit(*g_signal)(index); //发射信号到Resolver中的解释方法
	}



	BlockCmd::~BlockCmd() {
		//释放容器
		block.clear();
		std::vector<Cmd*>().swap(block);
	}
	void BlockCmd::Push(Cmd* _statement) {
		block.push_back(_statement);
	}
	SuatinCmdClassType BlockCmd::GetClassType() {
		return SuatinCmdClassType_BlockCmd;
	}

	std::vector<Cmd*> BlockCmd::GetBlockVector() const {
		return block;
	}
	void BlockCmd::SetEnd() {
		end_flag = true;
	}
	bool BlockCmd::IsEnd()const {
		return end_flag;
	}
	void BlockCmd::SetJumpSignal(SuatinJumpSignalType _jumpSignal) {
		jumpWhat = _jumpSignal;
	}
	SuatinJumpSignalType BlockCmd::GetJumpSignal()const {
		return jumpWhat;
	}
	void BlockCmd::interpret() {
		for (std::vector<Cmd*>::iterator it = block.begin(); it != block.end(); ++it) {
			//1.先解释
			(*it)->interpret();
			//2.语句块中出现break/continue节点
			if ((*it)->GetClassType() == SuatinCmdClassType_BreakCmd) {
				jumpWhat = SuatinJumpSignalType_Break;
				return;
			}
			else if ((*it)->GetClassType() == SuatinCmdClassType_ContinueCmd) {
				jumpWhat = SuatinJumpSignalType_Continue;
				return;
			}
			//3.语句块中的if-elif-else链条中出现break/continue信号
			if ((*it)->GetClassType() == SuatinCmdClassType_IfCmd) {
				IfCmd* tmp = dynamic_cast<IfCmd*>(*it);
				if (tmp->GetJumpSignal() == SuatinJumpSignalType_Break) {
					jumpWhat = SuatinJumpSignalType_Break;
					return;
				}
				else if (tmp->GetJumpSignal() == SuatinJumpSignalType_Continue) {
					jumpWhat = SuatinJumpSignalType_Continue;
					return;
				}
			}		
			
		}
	}






	CondCmd::~CondCmd() {
		if (condition) {
			delete condition;
			condition = NULL;
		}
	}
	void CondCmd::SetCondition(SingleCmd* _cond) {
		condition = _cond;
	}
	SingleCmd* CondCmd::GetCondition()const {
		return condition;
	}

	SuatinCmdClassType CondCmd::GetClassType() {
		return SuatinCmdClassType_CondCmd;
	}


	bool CondCmd::IsTrue()const {
		//先解释
		condition->interpret();
		//再得到语句的返回结果
		return g_statement_returnVal; //当前解释完后，真假结果放在这个全局变量中
	}




	SuatinCmdClassType IfCmd::GetClassType() {
		return SuatinCmdClassType_IfCmd;
	}

	void IfCmd::interpret() {
		//if
		if (IsTrue()) {
			BlockCmd::interpret();
			if (jumpWhat == SuatinJumpSignalType_Break || jumpWhat == SuatinJumpSignalType_Continue)return;
		}
		else {
			//elif or else
			if (last) {
				last->interpret();
			}
		}

		//将if-elif-else链条下的跳转信息传递上来
		if (last != NULL) {
			BlockCmd* tmp = dynamic_cast<BlockCmd*>(last);
			jumpWhat = tmp->GetJumpSignal();
		}
	}

	SuatinCmdClassType ElifCmd::GetClassType() {
		return SuatinCmdClassType_ElifCmd;
	}

	SuatinCmdClassType ElseCmd::GetClassType() {
		return SuatinCmdClassType_ElseCmd;
	}



	SuatinCmdClassType WhileCmd::GetClassType() {
		return SuatinCmdClassType_WhileCmd;
	}
	void WhileCmd::interpret() {
		while (IsTrue()) {
			BlockCmd::interpret();
			if (jumpWhat == SuatinJumpSignalType_Break) {
				jumpWhat = SuatinJumpSignalType_None;
				return;
			}
		}
	}
	

	SuatinCmdClassType DoUntilCmd::GetClassType() { 
		return SuatinCmdClassType_DoUntilCmd; 
	}
	void DoUntilCmd::interpret() {
		do {
			BlockCmd::interpret();
			if (jumpWhat == SuatinJumpSignalType_Break) {
				jumpWhat = SuatinJumpSignalType_None;
				return;
			}
		} while (IsTrue() == false); //do-until 满足条件就退出

	}




	void ForCmd::SetInit(SingleCmd* _stat_init) {
		stat_init = _stat_init;
	}
	void ForCmd::SetAdd(SingleCmd* _stat_add) { 
		stat_add = _stat_add; 
	}
	SingleCmd* ForCmd::GetInit()const {
		return stat_init;
	}
	SingleCmd* ForCmd::GetAdd()const {
		return stat_add;
	}
	SuatinCmdClassType ForCmd::GetClassType() { 
		return SuatinCmdClassType_ForCmd; 
	}

	void ForCmd::interpret() {
		stat_init->interpret();//初始化
		while (IsTrue()) { //判断			
			BlockCmd::interpret(); //语句块
			if (jumpWhat == SuatinJumpSignalType_Break) {
				jumpWhat = SuatinJumpSignalType_None;
				return;
			}
			stat_add->interpret();  //自增
		}
	}



	ForCmd::~ForCmd() {
		if (stat_init) {
			delete stat_init;
			stat_init = NULL;
		}
		if (stat_add) {
			delete stat_add;
			stat_add = NULL;
		}
	}




	SuatinCmdClassType AssertCmd::GetClassType() {
		return SuatinCmdClassType_AssertCmd; 
	}
	void AssertCmd::interpret() {  //卡住解释
		std::string s;
		do {
			std::cout << ">";
			s = "";
			std::cin >> s;
			if (s == "exit") { //输入exit退出解释器
				exit(0);
				return;
			}
		} while (s.length() == 0); //随便输入点什么都可以往下继续执行
	}


	SuatinCmdClassType BreakCmd::GetClassType() {
		return SuatinCmdClassType_BreakCmd;
	}
	void BreakCmd::interpret() {

	}

	SuatinCmdClassType ContinueCmd::GetClassType() {
		return SuatinCmdClassType_ContinueCmd;
	}
	void ContinueCmd::interpret(){
	
	}



};
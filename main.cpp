#include "Logger.h"
<<<<<<< HEAD
#include <iostream>
=======
>>>>>>> 1dc835062323b68006a9a21a17ca53464f5d8044


//һ��������д���̣߳�
int main() {
	try {
		Logger logger("testlog.txt");
		logger.log("Log starting application");
		int user_id = 27;
		std::string  user_name = "yzj";
		float weight = 2.75;
		std::string hello_obj = "world";
		logger.log("Userid: {},User name: {}",user_id,user_name);
		logger.log("Hello {},weight is {}", hello_obj, weight);
		logger.log("--------Log end---------");

		//ģ���ӳٵȴ�ȷ��д�����
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	catch (const std::exception& ex){
		std::cerr << "��־ϵͳ��ʼ��ʧ��: " << ex.what() << std::endl;
	}
	return 0;
}
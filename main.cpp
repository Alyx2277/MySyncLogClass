#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <fstream>
#include <atomic>
#include <sstream>
#include <vector>
#include <stdexcept>

//����������ʵ�ֽ���������ת��Ϊ�ַ���
template <typename T>
std::string to_string_helper(T&& arg) {
	//T&&�ǿ���ƥ���κ����͵Ĳ���(��ֵ��ֵ)
	//����һ���ַ������������ oss�����ڽ����ݸ�ʽ��Ϊ�ַ�����
	std::ostringstream oss;
	//ʹ������ת��std::forward<T>(arg); ��������ʲô���ͣ���ֵ��ֵ����ȥ�����Ǹ�ֵ
	oss << std::forward<T>(arg);
	return oss.str();
}
int main() {
	return 0;
}
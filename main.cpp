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

//辅助函数，实现将单个参数转换为字符串
template <typename T>
std::string to_string_helper(T&& arg) {
	//T&&是可以匹配任何类型的参数(左值右值)
	//创建一个字符串输出流对象 oss，用于将数据格式化为字符串。
	std::ostringstream oss;
	//使用完美转发std::forward<T>(arg); 传进来是什么类型，左值右值，出去还是那个值
	oss << std::forward<T>(arg);
	return oss.str();
}
int main() {
	return 0;
}
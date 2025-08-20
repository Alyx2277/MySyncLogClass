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

//需要一个线程安全的日志队列作为生产者
class LogQueue {
public:
	void push(std::string& msg) {
		//RAII思想，一旦成功构造就自动加锁，一旦析构就自动解锁
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push(msg);
		//如果队列中数据为空，数据为1
		//if (queue_.size() == 1) {
		//	//告诉消费者线程，如果是挂起状态，现在可以消费了。
		//	cond_var_.notify_one();
		//}
		//可以直接通知，在消费者中处理就行了
		cond_var_.notify_one();
	}
	bool pop(std::string& msg) {
		std::unique_lock<std::mutex> lock(mutex_);
		if (queue_.empty()) {
			cond_var_.wait(lock, [this]() {
				return !queue_.empty() || is_shutdown_;
			});
			//要注意操作系统虚假唤醒行为
		}
	}
	void shutDown() {
	}
private:
	std::queue<std::string> queue_;
	std::mutex mutex_;
	std::condition_variable cond_var_;//条件变量
	bool is_shutdown_ = false;
};

//一个独立的写入线程，
int main() {
	return 0;
}
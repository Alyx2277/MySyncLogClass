#pragma once
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
	void push(std::string&& msg) {
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
				//返回true，退出等待，返回false，继续等待
				});
			//要注意操作系统虚假唤醒行为
			if (is_shutdown_ && queue_.empty()) {
				return false;
			}
		}
		msg = queue_.front();
		queue_.pop();
		return true;
	}
	void shutDown() {
		std::lock_guard<std::mutex> lock(mutex_);
		is_shutdown_ = true;
		cond_var_.notify_all();//通知所有消费者退出
	}
private:
	std::queue<std::string> queue_;
	std::mutex mutex_;
	std::condition_variable cond_var_;//条件变量
	bool is_shutdown_ = false;
};

class Logger
{
public:
	Logger(const std::string& filename) :log_file_(filename, std::ios::out | std::ios::app), exit_flag_(false)
	{
		//打开文件，创建写入线程
		if (!log_file_.is_open()) {
			throw std::runtime_error("Fail to open file");
		}

		work_thread_ = std::thread([this]() {
			std::string msg;
			while (log_queue_.pop(msg)) {
				log_file_ << msg << std::endl;
			}
		});
	}
	//退出队列，标记设为true
	~Logger() {
		exit_flag_ = true;
		log_queue_.shutDown();
		if (work_thread_.joinable()) {
			work_thread_.join();
		}

		if (log_file_.is_open()) {
			log_file_.close();
		}
	}
	template<typename... Args>
	void log(const std::string& format, Args&&... args) {
		log_queue_.push(formatMessage(format, std::forward<Args>(args)...));
	}
private:
	LogQueue log_queue_;
	std::thread work_thread_;
	std::ofstream log_file_;
	std::atomic<bool> exit_flag_;

	template<typename... Args>
	std::string formatMessage(const std::string& format, Args&&... args) {
		std::vector<std::string> arg_strings = { to_string_helper(std::forward<Args>(args))... };
		std::ostringstream oss; // 输出流
		size_t arg_index = 0; // 参数索引
		size_t pos = 0; // 当前在格式字符串中的位置
		size_t placeholder = format.find("{}", pos); // 查找第一个占位符

		// std::string::npos是未找到或者无效位置的特殊值
		while (placeholder != std::string::npos) {
			oss << format.substr(pos, placeholder - pos); // 保存占位符前的文字
			if (arg_index < arg_strings.size()) {
				oss << arg_strings[arg_index++]; // 如果有合适的参数，把参数替换进去
			}
			else {
				// 没有足够的参数，保留 "{}"
				oss << "{}";
			}
			pos = placeholder + 2; // 跳过 "{}"
			placeholder = format.find("{}", pos); // 查找下一个占位符
		}

		// 添加剩余的字符串
		oss << format.substr(pos);

		// 如果还有剩余的参数，按原方式拼接
		while (arg_index < arg_strings.size()) {
			oss << arg_strings[arg_index++];
		}

		return oss.str();
	}
};
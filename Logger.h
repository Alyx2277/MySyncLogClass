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

//��Ҫһ���̰߳�ȫ����־������Ϊ������
class LogQueue {
public:
	void push(std::string&& msg) {
		//RAII˼�룬һ���ɹ�������Զ�������һ���������Զ�����
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push(msg);
		//�������������Ϊ�գ�����Ϊ1
		//if (queue_.size() == 1) {
		//	//�����������̣߳�����ǹ���״̬�����ڿ��������ˡ�
		//	cond_var_.notify_one();
		//}
		//����ֱ��֪ͨ�����������д��������
		cond_var_.notify_one();
	}
	bool pop(std::string& msg) {
		std::unique_lock<std::mutex> lock(mutex_);
		if (queue_.empty()) {
			cond_var_.wait(lock, [this]() {
				return !queue_.empty() || is_shutdown_;
				//����true���˳��ȴ�������false�������ȴ�
				});
			//Ҫע�����ϵͳ��ٻ�����Ϊ
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
		cond_var_.notify_all();//֪ͨ�����������˳�
	}
private:
	std::queue<std::string> queue_;
	std::mutex mutex_;
	std::condition_variable cond_var_;//��������
	bool is_shutdown_ = false;
};

class Logger
{
public:
	Logger(const std::string& filename) :log_file_(filename, std::ios::out | std::ios::app), exit_flag_(false)
	{
		//���ļ�������д���߳�
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
	//�˳����У������Ϊtrue
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
		std::ostringstream oss; // �����
		size_t arg_index = 0; // ��������
		size_t pos = 0; // ��ǰ�ڸ�ʽ�ַ����е�λ��
		size_t placeholder = format.find("{}", pos); // ���ҵ�һ��ռλ��

		// std::string::npos��δ�ҵ�������Чλ�õ�����ֵ
		while (placeholder != std::string::npos) {
			oss << format.substr(pos, placeholder - pos); // ����ռλ��ǰ������
			if (arg_index < arg_strings.size()) {
				oss << arg_strings[arg_index++]; // ����к��ʵĲ������Ѳ����滻��ȥ
			}
			else {
				// û���㹻�Ĳ��������� "{}"
				oss << "{}";
			}
			pos = placeholder + 2; // ���� "{}"
			placeholder = format.find("{}", pos); // ������һ��ռλ��
		}

		// ���ʣ����ַ���
		oss << format.substr(pos);

		// �������ʣ��Ĳ�������ԭ��ʽƴ��
		while (arg_index < arg_strings.size()) {
			oss << arg_strings[arg_index++];
		}

		return oss.str();
	}
};
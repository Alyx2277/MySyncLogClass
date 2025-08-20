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
	void push(std::string& msg) {
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
			});
			//Ҫע�����ϵͳ��ٻ�����Ϊ
		}
	}
	void shutDown() {
	}
private:
	std::queue<std::string> queue_;
	std::mutex mutex_;
	std::condition_variable cond_var_;//��������
	bool is_shutdown_ = false;
};

//һ��������д���̣߳�
int main() {
	return 0;
}
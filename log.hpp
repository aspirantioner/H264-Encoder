#include "json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

class Logger {
public:
    // 构造函数，默认输出到 std::cout，并设定 logger 名称
    Logger(const std::string& json_str){}

    // 设置日志输出到某个 std::ostream 对象
    void SetOutput(std::ostream& os) {
        log_output_ptr_ = &os;
    }

    // 重载 << 运算符，支持链式调用
    template <typename T>
    Logger& operator<<(const T& value) {
        if (log_output_ptr_) {
            printPrefix();
            *log_output_ptr_ << value;
        }
        return *this; // 返回当前对象的引用，使得链式调用成为可能
    }

    // 特别处理 std::endl 和其他类似的操纵符（因为它们实际上是函数）
    typedef std::ostream& (*StreamManipulator)(std::ostream&);
    Logger& operator<<(StreamManipulator manip) {
        if (log_output_ptr_) {
            // 如果 manip 是 std::endl，那么就不输出前缀
            if (manip == static_cast<StreamManipulator>(std::flush) ||
                manip == static_cast<StreamManipulator>(std::endl)) {
                is_start_ofline_ = true;
            }
            manip(*log_output_ptr_);
        }
        return *this;
    }

private:
    std::ostream* log_output_ptr_; // 指向当前输出流的指针
    std::string logger_name_;     // Logger 的名称
    bool is_start_ofline_ = true;  // 是否是新的一行，如果是，我们将输出前缀

    // 输出前缀（只有在新行开始时）
    void PrintPrefix() {
        if (is_start_ofline_) {
            *log_output_ptr_ << "[" << logger_name_ << "] ";
            is_start_ofline_ = false;
        }
    }
};

int main() {
    Logger logger("MyLogger");

    // 使用 Logger 类像使用 std::cout 那样输出信息
    logger << "Logging message 1: ";
    logger << 123 << ", ";
    logger << 4.56 << ", ";
    logger << std::boolalpha << true << std::endl;

    // 创建文件输出流，并将日志输出重定向到文件
    std::ofstream file("log.txt");
    if (file) {
        logger.SetOutput(file);
        logger << "Logging to file." << std::endl;
    }

    // 清理资源
    if (file.is_open()) {
        file.close();
    }

    return 0;
}

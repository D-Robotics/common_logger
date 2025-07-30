#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <rcpputils/asserts.hpp>

namespace common_logger
{
    class Logger
    {
    public:
        static Logger &getInstance();

        // 禁用拷贝和移动
        Logger(const Logger &) = delete;
        Logger &operator=(const Logger &) = delete;
        Logger(Logger &&) = delete;
        Logger &operator=(Logger &&) = delete;

        std::shared_ptr<spdlog::logger> getLogger(
            const std::string &package_name,
            const std::string &logger_name = "");

        void setLogLevel(const std::string &package_name, spdlog::level::level_enum level);
        void setGlobalLogLevel(spdlog::level::level_enum level);
        void setPattern(const std::string &pattern);

        // 添加/移除日志接收器
        void addSink(spdlog::sink_ptr sink);
        void removeSink(spdlog::sink_ptr sink);
        void clearSinks();

    private:
        Logger();
        ~Logger() = default;

        std::string createLoggerKey(const std::string &package_name, const std::string &logger_name);
        // 存放创建的spdlog::logger对象
        std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers_;
        // 添加控制台对象
        std::vector<spdlog::sink_ptr> sinks_;
        std::mutex mutex_;
        // 全局日志级别
        spdlog::level::level_enum global_level_ = spdlog::level::info;
    };

    bool setPackageLogLevel(const std::string &target, const std::string &level_str);

} // namespace common_logger
#include <iostream>
#include <map>
#include <spdlog/sinks/basic_file_sink.h>

#include "common_logger/logger.h"

namespace common_logger
{

    // 类丰富, 创建当前对象的实例
    Logger &Logger::getInstance()
    {
        static Logger instance;
        return instance;
    }

    Logger::Logger()
    {
        // sink:日志的输出口, 可以是终端, 可以是文件
        // spdlog::sinks::stdout_color_sink_mt:彩色终端输出口
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sinks_.push_back(console_sink);
        // 设置日志模式
        setPattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] [%s:%#] [%!] %v");
    }

    // 设置终端模式
    void Logger::setPattern(const std::string &pattern)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        for (auto &sink : sinks_)
        {
            // 为单个sink设置日志输出格式
            sink->set_pattern(pattern);
        }
    }

    std::string Logger::createLoggerKey(
        const std::string &package_name,
        const std::string &logger_name)
    {
        return logger_name.empty() ? package_name : package_name + "::" + logger_name;
    }

    // 创建spdlog::logger对象
    std::shared_ptr<spdlog::logger> Logger::getLogger(
        const std::string &package_name,
        const std::string &logger_name)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        // logger名
        const auto key = createLoggerKey(package_name, logger_name);
        // 判断是否已经存在logger对象
        // 预防再次创建对象
        auto it = loggers_.find(key);
        if (it != loggers_.end())
        {
            return it->second;
        }

        // 创建新的logger
        auto logger = std::make_shared<spdlog::logger>(key, sinks_.begin(), sinks_.end());
        // 设置日志等级
        logger->set_level(global_level_);
        // 当某条日志的级别 ≥ 指定级别时，立即把缓冲区内容刷到磁盘/终端，防止丢日志
        // eg:出现 error 及以上立即 flush
        logger->flush_on(spdlog::level::err);

        loggers_[key] = logger;
        return logger;
    }

    // 设置指定包的日志等级
    void Logger::setLogLevel(
        const std::string &package_name,
        spdlog::level::level_enum level)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        bool found = false;

        for (auto &[key, logger] : loggers_)
        {
            // 设置指定包的日志等级
            if (key == package_name || key.find(package_name) == 0)
            {
                logger->set_level(level);
                found = true;
            }
        }

        if (!found)
        {
            // 为尚未创建的logger设置默认级别
            auto logger = std::make_shared<spdlog::logger>(package_name, sinks_.begin(), sinks_.end());
            logger->set_level(level);
            loggers_[package_name] = logger;
        }
    }

    // 设置所有包的日志等级
    void Logger::setGlobalLogLevel(spdlog::level::level_enum level)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        global_level_ = level;
        for (auto &[_, logger] : loggers_)
        {
            logger->set_level(level);
        }
    }

    // 添加sink
    void Logger::addSink(spdlog::sink_ptr sink)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        sinks_.push_back(sink);

        // 给每个日志对象添加sink
        for (auto &[_, logger] : loggers_)
        {
            logger->sinks().push_back(sink);
        }
    }

    // 删除指定类型的sink
    void Logger::removeSink(spdlog::sink_ptr sink)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), sink), sinks_.end());
        // 删除各个对象指定类型的sink
        for (auto &[_, logger] : loggers_)
        {
            // 获取当前对象所有的sink
            auto &logger_sinks = logger->sinks();
            logger_sinks.erase(std::remove(logger_sinks.begin(), logger_sinks.end(), sink), logger_sinks.end());
        }
    }

    // 清除各个日志对象的所有的sink
    void Logger::clearSinks()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sinks_.clear();
        for (auto &[_, logger] : loggers_)
        {
            logger->sinks().clear();
        }
    }

    bool setPackageLogLevel(const std::string &target, const std::string &level_str)
    {
        static const std::map<std::string, spdlog::level::level_enum> level_map{
            {"trace", spdlog::level::trace},
            {"debug", spdlog::level::debug},
            {"info", spdlog::level::info},
            {"warn", spdlog::level::warn},
            {"error", spdlog::level::err},
            {"critical", spdlog::level::critical},
            {"off", spdlog::level::off}};

        auto it = level_map.find(level_str);
        if (it == level_map.end())
        {
            return false;
        }

        if (target == "global")
        {
            // 设置所有包的日志级别
            common_logger::Logger::getInstance().setGlobalLogLevel(it->second);
        }
        else
        {
            // 设置指定包的日志级别
            common_logger::Logger::getInstance().setLogLevel(target, it->second);
        }
        return true;
    }
} // namespace common_logger
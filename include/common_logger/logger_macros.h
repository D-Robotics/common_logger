#pragma once

#include <spdlog/fmt/fmt.h>
#include "common_logger/logger.h"

// 获取文件名（不带路径）
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// 主日志宏
#define SPDLOG_MANAGER_LOG(package, level, ...)                                \
    do                                                                         \
    {                                                                          \
        auto logger = common_logger::Logger::getInstance().getLogger(package); \
        if (logger->should_log(level))                                         \
        {                                                                      \
            logger->log(                                                       \
                spdlog::source_loc{__FILENAME__, __LINE__, SPDLOG_FUNCTION},   \
                level,                                                         \
                __VA_ARGS__);                                                  \
        }                                                                      \
    } while (0)

// 自定义日志对象
#define LOG_TRACE(package, ...) SPDLOG_MANAGER_LOG(package, spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(package, ...) SPDLOG_MANAGER_LOG(package, spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(package, ...) SPDLOG_MANAGER_LOG(package, spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(package, ...) SPDLOG_MANAGER_LOG(package, spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(package, ...) SPDLOG_MANAGER_LOG(package, spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(package, ...) SPDLOG_MANAGER_LOG(package, spdlog::level::critical, __VA_ARGS__)

// 带检查的断言宏, 条件不成立打印日志
#define LOG_ASSERT(package, condition, ...)                            \
    do                                                                 \
    {                                                                  \
        if (!(condition))                                              \
        {                                                              \
            LOG_CRITICAL(package, "Assertion failed: {}", #condition); \
            LOG_CRITICAL(package, __VA_ARGS__);                        \
            rcpputils::require_true(condition);                        \
        }                                                              \
    } while (0)

// 设置包的日志级别
#define SET_PACKAGE_LOG_LEVEL(target, level) common_logger::setPackageLogLevel(target, level)
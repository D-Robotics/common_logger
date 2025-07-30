#include <map>
#include <vector>
#include <rclcpp/rclcpp.hpp>

#include "common_logger/logger_macros.h"

class LogParamServer : public rclcpp::Node
{
public:
    LogParamServer() : Node("log_param_server")
    {
        // 声明参数并给定默认值
        declare_parameter("global_level", "info");
        rcl_interfaces::msg::ParameterDescriptor global_desc;
        global_desc.description = "Global log level (trace, debug, info, warn, error, critical, off)";

        declare_parameter("package_levels", std::map<std::string, std::string>{});
        rcl_interfaces::msg::ParameterDescriptor packages_desc;
        packages_desc.description = "Per-package log levels as {package_name: level}";

        param_callback_ = add_on_set_parameters_callback(
            [this](const std::vector<rclcpp::Parameter> &params)
            {
                return onParameterChange(params);
            });

        // 初始参数应用
        applyInitialParameters();
    }

private:
    void applyInitialParameters()
    {
        // 获取全局日志等级
        auto global_level = get_parameter("global_level").as_string();
        // 获取包的日志等级
        auto package_levels = get_parameter("package_levels").as_string_map();

        SET_PACKAGE_LOG_LEVEL("global", global_level);

        for (const auto &[pkg, level] : package_levels)
        {
            // 设置各个包的日志等级
            SET_PACKAGE_LOG_LEVEL(pkg, level);
        }
    }

    rcl_interfaces::msg::SetParametersResult onParameterChange(
        const std::vector<rclcpp::Parameter> &params)
    {
        auto result = rcl_interfaces::msg::SetParametersResult();
        result.successful = true;

        // 处理每个参数
        for (const auto &param : params)
        {
            // 全局日志级别
            if (param.get_name() == "global_level")
            {
                if (!SET_PACKAGE_LOG_LEVEL("global", param.as_string()))
                {
                    result.successful = false;
                    result.reason = "Invalid global log level: " + param.as_string();
                }
            }
            else if (param.get_name() == "package_levels")
            {
                // 判断参数类型是否正确
                if (param.get_type() != rclcpp::ParameterType::PARAMETER_STRING_MAP)
                {
                    result.successful = false;
                    result.reason = "package_levels must be a string map";
                    continue;
                }

                auto package_levels = param.as_string_map();
                for (const auto &[pkg, level] : package_levels)
                {
                    if (!SET_PACKAGE_LOG_LEVEL(pkg, level))
                    {
                        result.successful = false;
                        result.reason = "Invalid log level for package '" + pkg + "': " + level;
                    }
                }
            }
        }
        return result;
    }

    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_callback_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<LogParamServer>();

    // 打印启动信息
    std::string package_log_name = "spdlog_manager";
    SET_PACKAGE_LOG_LEVEL(package_log_name, "info")

    size_t num = 100000000;
    for(size_t i = 0, i < num, i++)
    {
        LOG_DEBUG(package_log_name, "TEST DEBUG");
        LOG_INFO(package_log_name, "TEST INFO");
        LOG_WARN(package_log_name, "TEST WARN");
    }
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
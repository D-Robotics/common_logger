cmake_minimum_required(VERSION 3.16)
project(common_logger)

# 编译为release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# 设置编译选项
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 主要用于寻找普通的包, ros2包:ament_target_dependencies 会解决
find_package(ament_cmake REQUIRED)
find_package(spdlog REQUIRED)
find_package(rcpputils REQUIRED)
# find_package(rclcpp REQUIRED)

# 创建库
add_library(${PROJECT_NAME} SHARED
  src/logger.cpp
)

# 当前所有编译对象在编译过程头文件搜索路径
# ${rclcpp_INCLUDE_DIRS} : /opt/ros/humble/include
include_directories(
        include        
        /usr/include   
        # ${rclcpp_INCLUDE_DIRS} 
)

# 链接依赖的普通库
target_link_libraries(${PROJECT_NAME}
  spdlog::spdlog
  rcpputils::rcpputils
)

# 链接依赖的ros2库
ament_target_dependencies(${PROJECT_NAME} spdlog rcpputils)


# 告诉编译器当前编译 {PROJECT_NAME}以及未来使用 {PROJECT_NAME} 头文件的的搜索路径
target_include_directories(${PROJECT_NAME} PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<INSTALL_INTERFACE:include>)

# 导出当前功能包的目标信息，让其他功能包可以通过find_package()引用这些目标
# HAS_LIBRARY_TARGET：标记该导出包含库目标（非必需，但推荐）
ament_export_targets(${PROJECT_NAME}_export HAS_LIBRARY_TARGET)

# 声明当前功能包的依赖需要被传递给依赖它的其他功能包
ament_export_dependencies(spdlog rcpputils)


# EXPORT ${PROJECT_NAME}_export:生成一个 CMake导出文件
# 其他项目通过 find_package() 或 include() 导入此库时，CMake 会读取该文件
install(TARGETS
      ${PROJECT_NAME}
      EXPORT ${PROJECT_NAME}_export
      ARCHIVE DESTINATION lib
      LIBRARY DESTINATION lib
      RUNTIME DESTINATION bin
      )


#  DESTINATION: 实际为:${CMAKE_INSTALL_PREFIX}/include
install(DIRECTORY include/
  DESTINATION include
)

# 声明 ament 包
ament_package()

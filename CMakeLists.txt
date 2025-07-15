cmake_minimum_required(VERSION 3.14)

if (WIN32)
    project(QRcode_Generate_Recongnise LANGUAGES CXX)
elseif(UNIX)
    project(QRcode_Generate_Recongnise)
endif()

set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo;Debug" CACHE STRING "" FORCE)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#======================= INCLUSION OF Qt =======================#
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_PREFIX_PATH $ENV{QTDIR})
find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
find_package(Qt6 REQUIRED COMPONENTS Network)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)
find_package(Qt6 REQUIRED COMPONENTS MultimediaWidgets)

# 添加头文件目录
include_directories(include)

# 修改 ZXing 配置，禁用 stb 下载
set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples")
set(BUILD_BLACKBOX_TESTS OFF CACHE BOOL "Build blackbox tests")

# 直接添加 ZXing 核心库而不依赖 stb
add_subdirectory(3rd/zxing/core)

# 源文件
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
)

# 头文件
set(HEADERS
    include/mainwindow.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 链接库
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Core5Compat
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::Network
    ZXing
)

# 设置目标属性
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# 复制 Qt 运行时库（Windows）
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}>
            COMMENT "Deploying Qt libraries")
    endif()
endif()

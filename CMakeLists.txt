cmake_minimum_required(VERSION 3.14)

if (WIN32)
    project(QRcode_Generator_Recongniser LANGUAGES CXX)
elseif(UNIX)
    project(QRcode_Generator_Recongniser)
endif()

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 为MinGW设置UTF-8编码支持
if(MINGW)
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
    message(STATUS "MinGW UTF-8 support enabled")
endif()

# 为MSVC设置UTF-8编码支持
if(MSVC)
    add_compile_options(/utf-8)
    message(STATUS "MSVC UTF-8 support enabled")
endif()

#set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets MultimediaWidgets Multimedia Core5Compat Svg Network)

#=================== INCLUSION OF Project Files ====================#
set(FORMS_DIR "${CMAKE_SOURCE_DIR}/forms")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

# 修改 ZXing 配置 - 重要：确保启用写入功能
set(BUILD_EXAMPLES OFF)
set(BUILD_BLACKBOX_TESTS OFF)
set(BUILD_UNIT_TESTS OFF)
set(BUILD_DEPENDENCIES OFF)
set(BUILD_WRITERS ON CACHE BOOL "Build with writer support" FORCE)
set(BUILD_READERS ON CACHE BOOL "Build with reader support" FORCE)

# 检查ZXing是否支持实验性API
set(ZXING_EXPERIMENTAL_API OFF)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rd/zxing/core/src/WriteBarcode.h")
    message(STATUS "Found WriteBarcode.h - enabling experimental API")
    set(ZXING_EXPERIMENTAL_API ON)
    add_definitions(-DZXING_EXPERIMENTAL_API)
else()
    message(STATUS "WriteBarcode.h not found - using legacy API")
endif()

# 检查关键的ZXing头文件
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rd/zxing/core/src/MultiFormatWriter.h")
    message(STATUS "Found MultiFormatWriter.h")
else()
    message(WARNING "MultiFormatWriter.h not found - this will cause linking issues")
endif()

# 在添加ZXing之前设置选项
set(BUILD_SHARED_LIBS OFF)

# 直接添加 ZXing 核心库
add_subdirectory(3rd/zxing/core)

# 显示ZXing目标信息
get_target_property(ZXING_SOURCES ZXing SOURCES)
message(STATUS "ZXing sources: ${ZXING_SOURCES}")

include_directories(${FORMS_DIR})
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})
include_directories(3rd/zxing/core/src)

# 更精确地收集源文件
file(GLOB_RECURSE UI_FILES "${FORMS_DIR}/*.ui")
file(GLOB_RECURSE QRC_FILES "${FORMS_DIR}/*.qrc")
file(GLOB_RECURSE HEADER_FILES "${INCLUDE_DIR}/*.h")
file(GLOB_RECURSE SOURCE_FILES "${SOURCE_DIR}/*.cpp")

set(SOURCES ${UI_FILES} ${QRC_FILES} ${HEADER_FILES} ${SOURCE_FILES})

# 显示收集到的源文件
message(STATUS "UI files: ${UI_FILES}")
message(STATUS "QRC files: ${QRC_FILES}")
message(STATUS "Header files: ${HEADER_FILES}")
message(STATUS "Source files: ${SOURCE_FILES}")

#=================== SETUP EXECTUABLE ====================#
# Enable debug logging on RELWITHDEBINFO configuration
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)

# Add the forms directory to the AUTOUIC search paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${FORMS_DIR})

# Add the executable
if (WIN32) 
    add_executable(QRcode_Generator_Recongniser WIN32 ${SOURCES})
elseif(UNIX)
    add_executable(QRcode_Generator_Recongniser ${SOURCES})
endif()

# 建议使用set_target_properties来代替全局set
set_target_properties(QRcode_Generator_Recongniser PROPERTIES
    AUTOMOC ON    # 处理Q_OBJECT宏
    AUTOUIC ON    # 处理.ui文件
    AUTORCC ON    # 处理.qrc文件
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Add the target includes for MY_PROJECT 
target_include_directories(QRcode_Generator_Recongniser PRIVATE 
    ${FORMS_DIR}
    ${INCLUDE_DIR}
    ${SOURCE_DIR}
    3rd/zxing/core/src
)

# 为MinGW添加特定的编译选项
if(MINGW)
    target_compile_options(QRcode_Generator_Recongniser PRIVATE 
        -finput-charset=UTF-8 
        -fexec-charset=UTF-8
    )
endif()

#===================== LINKING LIBRARIES =======================#
target_link_libraries(QRcode_Generator_Recongniser 
    Qt6::Widgets 
    Qt6::Multimedia 
    Qt6::MultimediaWidgets 
    Qt6::Core5Compat 
    Qt6::Svg
    Qt6::Network
    ZXing
)

# 显示链接的库信息
get_target_property(ZXING_TYPE ZXing TYPE)
message(STATUS "ZXing target type: ${ZXING_TYPE}")
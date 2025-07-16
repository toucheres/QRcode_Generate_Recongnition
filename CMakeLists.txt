cmake_minimum_required(VERSION 3.14)

if (WIN32)
    project(QRcode_Generator_Recongniser LANGUAGES CXX)
elseif(UNIX)
    project(QRcode_Generator_Recongniser)
endif()

#set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)

# #======================= INCLUSION OF Qt =======================#
# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_PREFIX_PATH $ENV{QTDIR})

find_package(Qt6 REQUIRED COMPONENTS Core Widgets MultimediaWidgets Multimedia Core5Compat)
#=================== INCLUSION OF Project Files ====================#
set(FORMS_DIR "${CMAKE_SOURCE_DIR}/forms")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
# 修改 ZXing 配置
set(BUILD_EXAMPLES OFF)
set(BUILD_BLACKBOX_TESTS OFF)
set(BUILD_UNIT_TESTS OFF)
set(BUILD_DEPENDENCIES OFF)

# 检查ZXing是否支持实验性API
set(ZXING_EXPERIMENTAL_API OFF)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rd/zxing/core/src/WriteBarcode.h")
    message(STATUS "Found WriteBarcode.h - enabling experimental API")
    set(ZXING_EXPERIMENTAL_API ON)
    add_definitions(-DZXING_EXPERIMENTAL_API)
else()
    message(STATUS "WriteBarcode.h not found - using legacy API")
endif()

# 直接添加 ZXing 核心库
add_subdirectory(3rd/zxing/core)
include_directories(${FORMS_DIR})
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})
include_directories(3rd/zxing/core/src)
file(GLOB_RECURSE SOURCES
    "${FORMS_DIR}/*.ui"
    "${FORMS_DIR}/*.qrc"
    "${INCLUDE_DIR}/*.h"
    "${SOURCE_DIR}/*.cpp"
)

#=================== SETUP EXECTUABLE ====================#
# Enable debug logging on RELWITHDEBINFO configuration
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)

#${CMAKE_AUTOUIC_SEARCH_PATHS}可能默认为cmakelists同级目录
message("--------------------------")
message(STATUS ${CMAKE_AUTOUIC_SEARCH_PATHS})
message("--------------------------")

#!!!必须将包含.ui .qrc的目录写入
# Add the forms directory to the AUTOUIC search paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${FORMS_DIR})

message("--------------------------")
message(STATUS ${CMAKE_AUTOUIC_SEARCH_PATHS})
message("--------------------------")

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
)
# Add the target includes for MY_PROJECT 
target_include_directories(QRcode_Generator_Recongniser PRIVATE ${FORMS_DIR})
target_include_directories(QRcode_Generator_Recongniser PRIVATE ${INCLUDE_DIR})
target_include_directories(QRcode_Generator_Recongniser PRIVATE ${SOURCE_DIR})

#===================== LINKING LIBRARIES =======================#
#===================== LINKING LIBRARIES =======================#

target_link_libraries(QRcode_Generator_Recongniser Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets Qt6::Core5Compat ZXing)
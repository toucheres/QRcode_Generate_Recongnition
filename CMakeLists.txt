cmake_minimum_required(VERSION 3.14)

if (WIN32)
    project(QRcode_Generate_Recongnise LANGUAGES CXX)
elseif(UNIX)
    project(QRcode_Generate_Recongnise)
endif()

set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)

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

#=================== INCLUSION OF Project Files ====================#
set(FORMS_DIR "${CMAKE_SOURCE_DIR}/forms")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

# 添加ZXing库路径
set(ZXING_DIR "${CMAKE_SOURCE_DIR}/3rd/zxing")
add_subdirectory(${ZXING_DIR})

include_directories(${FORMS_DIR})
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})

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

# Add the forms directory to the AUTOUIC search paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${FORMS_DIR})

# Add the executable
if (WIN32) 
    add_executable(QRcode_Generate_Recongnise WIN32 ${SOURCES})
elseif(UNIX)
    add_executable(QRcode_Generate_Recongnise ${SOURCES})
endif()

# Add the target includes for QRcode_Generate_Recongnise 
target_include_directories(QRcode_Generate_Recongnise PRIVATE ${FORMS_DIR})
target_include_directories(QRcode_Generate_Recongnise PRIVATE ${INCLUDE_DIR})
target_include_directories(QRcode_Generate_Recongnise PRIVATE ${SOURCE_DIR})

#===================== LINKING LIBRARIES =======================#
target_link_libraries(QRcode_Generate_Recongnise Qt6::Core Qt6::Widgets Qt6::Core5Compat Qt6::Network ZXing::ZXing)

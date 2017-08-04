# MDM 2017/08/04 3.8 is on ubuntu 17.10, 3.7 is on gentoo
cmake_minimum_required(VERSION 3.7)

# MDM silently append relative paths to absolute paths without warnings
cmake_policy(SET CMP0015 NEW)   

# MDM Set the executable name!
project(portable)

# -------------------------------
# MDM Set up relative paths to external folders
# -------------------------------
set(Reusable "../../")
# set(ThirdParty ".../")
set(BOOST_ROOT "../../../$ENV{boost_version}/")   # NOTE: boost_version is set in .bashrc

# MDM We want c++11 (or later)
set(CMAKE_CXX_STANDARD 11)
# This is the old way for CMake <3.1
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# MDM external include dirs
INCLUDE_DIRECTORIES(${Reusable})
INCLUDE_DIRECTORIES(${Reusable}sqlite/)
INCLUDE_DIRECTORIES(${ThirdParty})

# MDM CMake handles boost like this...
set(BOOST_INCLUDE_DIR ${BOOST_ROOT})
IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    message("Building with boost debug libraries...")
    set(BOOST_LIBRARYDIR ${BoostRoot}lib/debug)
ELSE()
    set(BOOST_LIBRARYDIR ${BoostRoot}lib)
ENDIF()
find_package(Boost COMPONENTS system thread date_time filesystem regex REQUIRED)
include_directories(${BOOST_INCLUDE_DIR})
link_directories(${BOOST_LIBRARYDIR})

set(SOURCE_FILES
        scrap.cpp
        scrap.hpp)

add_executable(portable ${SOURCE_FILES})


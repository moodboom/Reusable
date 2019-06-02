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
set(ThirdParty "../../../../Libraries/c++/")
set(ThirdPartyC "../../../../Libraries/c/")
set(BOOST_ROOT "${ThirdParty}$ENV{boost_version}/")   # NOTE: boost_version is set in .bashrc

# MDM pqxx uses c++17 so here we go - why tf was this currently commented out above?
set (CMAKE_CXX_STANDARD 17)

# We want -g to always sideload enough debug info for stacktraces
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# MDM Strangely, it seems Clion needs this or it will not index include files...?
INCLUDE_DIRECTORIES(".")

# MDM external include dirs
INCLUDE_DIRECTORIES(${Reusable})
INCLUDE_DIRECTORIES(${ThirdParty})
INCLUDE_DIRECTORIES(${ThirdPartyC})
INCLUDE_DIRECTORIES(${ThirdPartyC}sqlite/)
INCLUDE_DIRECTORIES(${ThirdParty}json/single_include/nlohmann/)
INCLUDE_DIRECTORIES(${ThirdParty}SQLiteCpp/include/)

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

# -------------------------------
# MDM Now add external modules
# -------------------------------
# orig added by clion: add_executable(portable ${SOURCE_FILES})
add_executable(
    ${PROJECT_NAME}
    ${SOURCE_FILES}
    ${Reusable}utilities.cpp
    ${Reusable}RandomHelpers.cpp
    ${Reusable}base64.cpp
    ${Reusable}crypto/sha256/sha256.cpp
    ${Reusable}miniz.c
    ${Reusable}oauth/HMAC_SHA1.cpp
    ${Reusable}oauth/oauthlib.cpp
    ${Reusable}oauth/SHA1.cpp
    ${Reusable}oauth/urlencode.cpp
    ${ThirdPartyC}sqlite/sqlite3.c
    ${ThirdParty}SQLiteCpp/src/Backup.cpp
    ${ThirdParty}SQLiteCpp/src/Column.cpp
    ${ThirdParty}SQLiteCpp/src/Database.cpp
    ${ThirdParty}SQLiteCpp/src/Exception.cpp
    ${ThirdParty}SQLiteCpp/src/Statement.cpp
    ${ThirdParty}SQLiteCpp/src/Transaction.cpp
)

# dl is needed by sqlite
TARGET_LINK_LIBRARIES(${PROJECT_NAME} pthread ssl crypto dl ${Boost_LIBRARIES})

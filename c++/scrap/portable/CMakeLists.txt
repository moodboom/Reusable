# MDM 2017/08/04 3.8 is on ubuntu 17.10, 3.7 is on gentoo
cmake_minimum_required(VERSION 3.7)

# MDM silently append relative paths to absolute paths without warnings
cmake_policy(SET CMP0015 NEW)   

# MDM Set the executable name!
project(scrap)

# -------------------------------
# MDM Set up relative paths to external folders
# sync with .vscode/c_cpp_properties.json
# -------------------------------
set(Reusable "../../") # just step back to Reusable/c++
set(ThirdParty "../../../../Libraries/c++/")
set(ThirdPartyC "../../../../Libraries/c/")
set(BOOST_ROOT "${ThirdParty}boost/")

# MDM pqxx uses c++17 so here we go - why tf was this currently commented out above?
# push to c++20
# sync with .vscode/c_cpp_properties.json
set (CMAKE_CXX_STANDARD 23)

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
find_package(Boost COMPONENTS system thread date_time filesystem regex stacktrace_addr2line REQUIRED)
include_directories(${BOOST_INCLUDE_DIR})
link_directories(${BOOST_LIBRARYDIR})

# -------------------------------
# MDM Build *everything* in .
# -------------------------------
# WARNING: [make] won't pick up new files until you rerun [cmake].  
# Change this to be explicit if that becomes a problem.
aux_source_directory(. SRC_LIST)

# -------------------------------
# MDM Now add external modules
# -------------------------------
# orig added by clion: add_executable(portable ${SOURCE_FILES})
add_executable(
    ${PROJECT_NAME}
    ${SRC_LIST}
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

# MDM This creates compile_commands.json, which can be imported by vscode to set include paths from here, w00t DRY
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# dl is needed by sqlite
TARGET_LINK_LIBRARIES(${PROJECT_NAME} pthread ssl crypto dl ${Boost_LIBRARIES})

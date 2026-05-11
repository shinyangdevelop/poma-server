cmake_minimum_required(VERSION 3.20)
project(GameServer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Tell Boost we are targeting Windows 10/11
add_definitions(-D_WIN32_WINNT=0x0A00)

# 2. Find Boost
find_package(Boost CONFIG REQUIRED COMPONENTS system)
find_package(libpqxx CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

add_executable(GameServer src/main.cpp
        src/handlers/UserHandler.cpp
        src/core/UserManager.cpp
        src/database/DatabaseManager.cpp
        src/handlers/PacketProcess.cpp
)

add_executable(Test test/test_client.cpp)

include_directories(include)

target_include_directories(GameServer PRIVATE ${PQXX_INCLUDE_DIRS})

# 3. Link Boost AND the required Windows networking libraries
if (WIN32)
    target_link_libraries(GameServer PRIVATE
            Boost::system
            ws2_32      # Windows Sockets 2
            wsock32     # Windows Sockets
            ${PQXX_LIBRARIES}
    )
    target_link_libraries(Test PRIVATE
            Boost::system
            ws2_32
            wsock32
    )
else ()
    # Just in case you ever move this code to Linux/macOS
    target_link_libraries(GameServer PRIVATE Boost::system ${PQXX_LIBRARIES})
    target_link_libraries(Test PRIVATE Boost::system)
endif ()
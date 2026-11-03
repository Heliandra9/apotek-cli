cmake_minimum_required(VERSION 3.16)
project(myapp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Daftar semua file .cpp
add_executable(myapp
    src/main.cpp
    # src/fitur.cpp
)

# Kalau punya folder header
target_include_directories(myapp PRIVATE include)

# Warning compiler
if(MSVC)
  target_compile_options(myapp PRIVATE /W4)
else()
  target_compile_options(myapp PRIVATE -Wall -Wextra -Wpedantic)
endif()
find_package(SQLite3 REQUIRED)

if(WIN32)
  # Pakai libsqlite3.a (statis), bukan import library DLL
  find_library(SQLITE3_STATIC NAMES libsqlite3.a sqlite3.a REQUIRED)
  target_include_directories(myapp PRIVATE ${SQLite3_INCLUDE_DIRS})
  target_link_libraries(myapp PRIVATE ${SQLITE3_STATIC})
  target_link_options(myapp PRIVATE -static)
else()
  target_link_libraries(myapp PRIVATE SQLite3::SQLite3)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)
target_include_directories(myapp PRIVATE ${SODIUM_INCLUDE_DIRS})
target_link_libraries(myapp PRIVATE ${SODIUM_LIBRARIES})
if(WIN32)
  target_link_options(myapp PRIVATE -static)
  target_link_directories(myapp PRIVATE ${SODIUM_STATIC_LIBRARY_DIRS})
  target_link_libraries(myapp PRIVATE ${SODIUM_STATIC_LIBRARIES})
endif()

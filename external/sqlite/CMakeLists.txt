cmake_minimum_required(VERSION 3.15)
project(sqlite3 C)

add_library(sqlite3 STATIC
    sqlite3.c
)

target_include_directories(sqlite3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Enable threading support
if(UNIX)
    target_link_libraries(sqlite3 PRIVATE pthread dl)
endif()

# SQLite compile options
target_compile_definitions(sqlite3 PRIVATE
    SQLITE_THREADSAFE=1
    SQLITE_ENABLE_FTS5
    SQLITE_ENABLE_JSON1
    SQLITE_ENABLE_RTREE
)

# Disable warnings for third-party code
if(MSVC)
    target_compile_options(sqlite3 PRIVATE /W0)
else()
    target_compile_options(sqlite3 PRIVATE -w)
endif()

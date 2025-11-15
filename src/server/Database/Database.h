#pragma once

#include <sqlite3.h>

namespace TDS {

class Database {
public:
    Database();
    ~Database();
    bool initialize(const char* path);

private:
    sqlite3* db;
};

} // namespace TDS

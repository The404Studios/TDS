#pragma once
namespace TDS {
class Database {
public:
    Database();
    ~Database();
    bool initialize(const char* path);
};
}

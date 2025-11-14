#include "Database.h"
#include <iostream>

namespace TDS {

Database::Database() : db(nullptr) {}

Database::~Database() {
    if (db) {
        // TODO: Close SQLite connection
        // sqlite3_close(db);
    }
}

bool Database::initialize(const char* path) {
    std::cout << "[Database] Initializing database: " << path << std::endl;

    // TODO: Actual SQLite initialization
    // For now, just simulate success
    
    std::cout << "[Database] Database initialized (stub)" << std::endl;
    return true;
}

} // namespace TDS

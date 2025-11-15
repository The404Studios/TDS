#include "Database.h"
#include <iostream>

namespace TDS {

Database::Database() : db(nullptr) {}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
        std::cout << "[Database] Database connection closed" << std::endl;
    }
}

bool Database::initialize(const char* path) {
    std::cout << "[Database] Initializing database: " << path << std::endl;

    int rc = sqlite3_open(path, &db);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database] Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Create users table
    const char* createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db, createUsersTable, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database] Failed to create users table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // Create player_data table
    const char* createPlayerDataTable = R"(
        CREATE TABLE IF NOT EXISTS player_data (
            user_id INTEGER PRIMARY KEY,
            level INTEGER DEFAULT 1,
            experience INTEGER DEFAULT 0,
            money INTEGER DEFAULT 0,
            inventory TEXT,
            stash TEXT,
            FOREIGN KEY(user_id) REFERENCES users(id)
        );
    )";

    rc = sqlite3_exec(db, createPlayerDataTable, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database] Failed to create player_data table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    std::cout << "[Database] Database initialized successfully" << std::endl;
    return true;
}

} // namespace TDS

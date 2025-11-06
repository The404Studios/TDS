#pragma once
#include "../Common/NetworkProtocol.h"
#include "../Common/DataStructures.h"
#include <map>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>

// Simple hash function (in production, use proper crypto library like OpenSSL)
inline std::string simpleHash(const std::string& input) {
    uint64_t hash = 0;
    for (char c : input) {
        hash = hash * 31 + static_cast<uint8_t>(c);
    }

    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << hash;
    return ss.str();
}

// Generate random session token
inline uint64_t generateSessionToken() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;
    return dis(gen);
}

// Authentication Manager - handles login, registration, and session management
class AuthManager {
public:
    AuthManager() : nextAccountId(1) {
        loadAccounts();
    }

    ~AuthManager() {
        saveAccounts();
    }

    // Register new account
    bool registerAccount(const std::string& username, const std::string& passwordHash,
                        const std::string& email, uint64_t& outAccountId, std::string& errorMsg) {
        // Validate username
        if (username.length() < 3 || username.length() > 16) {
            errorMsg = "Username must be 3-16 characters";
            return false;
        }

        // Check if username already exists
        for (const auto& pair : accounts) {
            if (pair.second.username == username) {
                errorMsg = "Username already taken";
                return false;
            }
        }

        // Create account
        Account account;
        account.accountId = nextAccountId++;
        account.username = username;
        account.passwordHash = passwordHash;
        account.email = email;
        account.created = getCurrentTimestamp();
        account.lastLogin = 0;

        accounts[account.accountId] = account;
        accountsByUsername[username] = account.accountId;

        outAccountId = account.accountId;

        std::cout << "[AuthManager] Registered new account: " << username
                  << " (ID: " << account.accountId << ")" << std::endl;

        // Auto-save
        saveAccounts();

        return true;
    }

    // Login to account
    bool login(const std::string& username, const std::string& passwordHash,
               uint64_t clientId, uint64_t& outAccountId, uint64_t& outSessionToken,
               std::string& errorMsg) {
        // Find account by username
        auto it = accountsByUsername.find(username);
        if (it == accountsByUsername.end()) {
            errorMsg = "Invalid username or password";
            return false;
        }

        uint64_t accountId = it->second;
        Account& account = accounts[accountId];

        // Verify password
        if (account.passwordHash != passwordHash) {
            errorMsg = "Invalid username or password";
            return false;
        }

        // Check if already logged in
        for (const auto& pair : sessions) {
            if (pair.second.accountId == accountId && pair.second.valid) {
                errorMsg = "Account already logged in";
                return false;
            }
        }

        // Create session
        Session session;
        session.sessionToken = generateSessionToken();
        session.accountId = accountId;
        session.created = getCurrentTimestamp();
        session.lastActivity = session.created;
        session.valid = true;

        sessions[session.sessionToken] = session;
        sessionsByClient[clientId] = session.sessionToken;
        clientsByAccount[accountId] = clientId;

        // Update last login
        account.lastLogin = session.created;

        outAccountId = accountId;
        outSessionToken = session.sessionToken;

        std::cout << "[AuthManager] User logged in: " << username
                  << " (Session: " << session.sessionToken << ")" << std::endl;

        return true;
    }

    // Logout
    void logout(uint64_t sessionToken) {
        auto it = sessions.find(sessionToken);
        if (it != sessions.end()) {
            it->second.valid = false;

            // Remove client mapping
            uint64_t accountId = it->second.accountId;
            clientsByAccount.erase(accountId);

            std::cout << "[AuthManager] Session logged out: " << sessionToken << std::endl;
        }
    }

    // Validate session
    bool validateSession(uint64_t sessionToken, uint64_t& outAccountId) {
        auto it = sessions.find(sessionToken);
        if (it == sessions.end() || !it->second.valid) {
            return false;
        }

        // Check for timeout (1 hour)
        uint64_t currentTime = getCurrentTimestamp();
        if (it->second.isExpired(currentTime, 3600)) {
            it->second.valid = false;
            return false;
        }

        // Update last activity
        it->second.lastActivity = currentTime;
        outAccountId = it->second.accountId;
        return true;
    }

    // Get account by ID
    Account* getAccount(uint64_t accountId) {
        auto it = accounts.find(accountId);
        if (it != accounts.end()) {
            return &it->second;
        }
        return nullptr;
    }

    // Get account by username
    Account* getAccountByUsername(const std::string& username) {
        auto it = accountsByUsername.find(username);
        if (it != accountsByUsername.end()) {
            return getAccount(it->second);
        }
        return nullptr;
    }

    // Get client ID for account
    bool getClientForAccount(uint64_t accountId, uint64_t& outClientId) {
        auto it = clientsByAccount.find(accountId);
        if (it != clientsByAccount.end()) {
            outClientId = it->second;
            return true;
        }
        return false;
    }

    // Get session token for client
    bool getSessionForClient(uint64_t clientId, uint64_t& outSessionToken) {
        auto it = sessionsByClient.find(clientId);
        if (it != sessionsByClient.end()) {
            outSessionToken = it->second;
            return true;
        }
        return false;
    }

    // Handle client disconnect
    void handleClientDisconnect(uint64_t clientId) {
        // Find and invalidate session
        auto it = sessionsByClient.find(clientId);
        if (it != sessionsByClient.end()) {
            uint64_t sessionToken = it->second;
            logout(sessionToken);
            sessionsByClient.erase(it);
        }
    }

    // Save accounts to file
    void saveAccounts() {
        std::ofstream file("Server/accounts.dat");
        if (!file.is_open()) {
            std::cout << "[AuthManager] Failed to save accounts" << std::endl;
            return;
        }

        file << "ACCOUNTS_V1\n";
        file << nextAccountId << "\n";
        file << accounts.size() << "\n";

        for (const auto& pair : accounts) {
            const Account& acc = pair.second;
            file << acc.accountId << "\n";
            file << acc.username << "\n";
            file << acc.passwordHash << "\n";
            file << acc.email << "\n";
            file << acc.created << "\n";
            file << acc.lastLogin << "\n";
        }

        file.close();
        std::cout << "[AuthManager] Saved " << accounts.size() << " accounts" << std::endl;
    }

    // Load accounts from file
    void loadAccounts() {
        std::ifstream file("Server/accounts.dat");
        if (!file.is_open()) {
            std::cout << "[AuthManager] No accounts file found, starting fresh" << std::endl;
            return;
        }

        std::string line;
        std::getline(file, line);  // Version
        if (line != "ACCOUNTS_V1") {
            std::cout << "[AuthManager] Invalid accounts file version" << std::endl;
            return;
        }

        file >> nextAccountId;
        int accountCount;
        file >> accountCount;
        file.ignore();

        for (int i = 0; i < accountCount; i++) {
            Account acc;
            file >> acc.accountId;
            file.ignore();
            std::getline(file, acc.username);
            std::getline(file, acc.passwordHash);
            std::getline(file, acc.email);
            file >> acc.created >> acc.lastLogin;
            file.ignore();

            accounts[acc.accountId] = acc;
            accountsByUsername[acc.username] = acc.accountId;
        }

        file.close();
        std::cout << "[AuthManager] Loaded " << accounts.size() << " accounts" << std::endl;
    }

private:
    std::map<uint64_t, Account> accounts;
    std::map<std::string, uint64_t> accountsByUsername;
    std::map<uint64_t, Session> sessions;
    std::map<uint64_t, uint64_t> sessionsByClient;   // clientId -> sessionToken
    std::map<uint64_t, uint64_t> clientsByAccount;   // accountId -> clientId
    uint64_t nextAccountId;
};

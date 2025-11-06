#pragma once
#include "../../common/NetworkProtocol.h"
#include "../../common/DataStructures.h"
#include "../../common/Utils.h"
#include <map>
#include <vector>
#include <string>

// Authentication Manager - handles login, registration, and session management
class AuthManager {
public:
    AuthManager();
    ~AuthManager();

    // Register new account
    bool registerAccount(const std::string& username, const std::string& passwordHash,
                        const std::string& email, uint64_t& outAccountId, std::string& errorMsg);

    // Login to account
    bool login(const std::string& username, const std::string& passwordHash,
               uint64_t clientId, uint64_t& outAccountId, uint64_t& outSessionToken,
               std::string& errorMsg);

    // Logout
    void logout(uint64_t sessionToken);

    // Validate session
    bool validateSession(uint64_t sessionToken, uint64_t& outAccountId);

    // Get account by ID
    Account* getAccount(uint64_t accountId);

    // Get account by username
    Account* getAccountByUsername(const std::string& username);

    // Get client ID for account
    bool getClientForAccount(uint64_t accountId, uint64_t& outClientId);

    // Get session token for client
    bool getSessionForClient(uint64_t clientId, uint64_t& outSessionToken);

    // Handle client disconnect
    void handleClientDisconnect(uint64_t clientId);

    // Save accounts to file
    void saveAccounts();

    // Load accounts from file
    void loadAccounts();

private:
    std::map<uint64_t, Account> accounts;
    std::map<std::string, uint64_t> accountsByUsername;
    std::map<uint64_t, Session> sessions;
    std::map<uint64_t, uint64_t> sessionsByClient;   // clientId -> sessionToken
    std::map<uint64_t, uint64_t> clientsByAccount;   // accountId -> clientId
    uint64_t nextAccountId;
};

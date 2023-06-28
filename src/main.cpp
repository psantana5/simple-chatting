#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <openssl/sha.h>

// Simulated database storing user credentials
struct User {
    int id;
    std::string username;
    std::string password;
};

// Simulated database of registered users
std::vector<User> registeredUsers;

// SQLite database connection
sqlite3* db;

// SQLite callback function to retrieve registered users from the database
int RetrieveUsersCallback(void* data, int argc, char** argv, char** colNames)
{
    std::vector<User>& users = *reinterpret_cast<std::vector<User>*>(data);
    users.push_back({ std::stoi(argv[0]), argv[1], argv[2] });
    return 0;
}

// Initialize the SQLite database and create the user table
bool InitializeDatabase()
{
    int rc = sqlite3_open("users.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::string createTableQuery = "CREATE TABLE IF NOT EXISTS users ("
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                   "username TEXT NOT NULL,"
                                   "password TEXT NOT NULL);";

    char* errMsg;
    rc = sqlite3_exec(db, createTableQuery.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to create table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

// Simulates user registration and stores the username and password in the database
bool RegisterUser(const std::string& username, const std::string& password)
{
    // Check if the user is already registered
    std::string selectQuery = "SELECT id FROM users WHERE username = '" + username + "';";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to execute select query: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        std::cout << "User already exists." << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    // Hash the password using OpenSSL's SHA256
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.length(), hash);

    // Store the username and password hash in the database
    std::string insertQuery = "INSERT INTO users (username, password) VALUES ('" + username + "', '";
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        insertQuery += std::to_string(hash[i]);
    }
    insertQuery += "');";

    char* errMsg;
    int rcInsert = sqlite3_exec(db, insertQuery.c_str(), nullptr, nullptr, &errMsg);
    if (rcInsert != SQLITE_OK) {
        std::cerr << "Failed to execute insert query: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    std::cout << "User registered successfully." << std::endl;
    return true;
}

// Authenticate the user based on the provided username and password
bool AuthenticateUser(const std::string& username, const std::string& password)
{
    // Find the user in the database
    std::string selectQuery = "SELECT id, username, password FROM users WHERE username = '" + username + "';";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to execute select query: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        std::cout << "User not found." << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    int userId = sqlite3_column_int(stmt, 0);
    std::string storedUsername = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    std::string storedPassword = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    sqlite3_finalize(stmt);

    // Hash the provided password and compare with the stored password
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.length(), hash);

    std::string inputPasswordHash;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        inputPasswordHash += std::to_string(hash[i]);
    }

    if (storedPassword == inputPasswordHash) {
        std::cout << "User authenticated successfully. User ID: " << userId << std::endl;
        return true;
    } else {
        std::cout << "Authentication failed. Invalid password." << std::endl;
        return false;
    }
}

// Cleanup and close the SQLite database connection
void Cleanup()
{
    sqlite3_close(db);
}

int main()
{
    // Initialize the database
    if (!InitializeDatabase()) {
        std::cerr << "Failed to initialize the database." << std::endl;
        return 1;
    }

    // Interactive menu
    int option;
    std::string username, password;
    std::cout << "Secure Chat Application" << std::endl;
    std::cout << "1. Register" << std::endl;
    std::cout << "2. Login" << std::endl;
    std::cout << "Select an option: ";
    std::cin >> option;

    switch (option) {
        case 1:
            std::cout << "Enter username: ";
            std::cin >> username;
            std::cout << "Enter password: ";
            std::cin >> password;
            RegisterUser(username, password);
            break;
        case 2:
            std::cout << "Enter username: ";
            std::cin >> username;
            std::cout << "Enter password: ";
            std::cin >> password;
            AuthenticateUser(username, password);
            break;
        default:
            std::cout << "Invalid option." << std::endl;
            break;
    }

    // Cleanup and close the database connection
    Cleanup();

    return 0;
}

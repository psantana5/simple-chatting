#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sqlite3.h>
#include <libwebsockets.h>


bool AuthenticateUser(const std::string& username, const std::string& password)
{
    sqlite3* db;
    int rc = sqlite3_open("users.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::string selectQuery = "SELECT * FROM users WHERE username = '" + username + "' AND password = '" + password + "';";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to execute select query." << std::endl;
        sqlite3_close(db);
        return false;
    }

    int result = sqlite3_step(stmt);
    bool authenticated = (result == SQLITE_ROW);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return authenticated;
}

static int callback_echo(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{
    std::string message;

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            std::cout << "Client connected" << std::endl;
            break;

        case LWS_CALLBACK_RECEIVE:
            std::cout << "Received message: " << static_cast<char*>(in) << std::endl;

            message = static_cast<char*>(in);

            if (message.find('|') != std::string::npos) {
                std::string username = message.substr(0, message.find('|'));
                std::string password = message.substr(message.find('|') + 1);

                bool authenticated = AuthenticateUser(username, password);
                std::string response = authenticated ? "success" : "failure";

                lws_write(wsi, reinterpret_cast<unsigned char*>(const_cast<char*>(response.c_str())), response.size(), LWS_WRITE_TEXT);

                if (authenticated) {
                    std::cout << "Authentication successful for user: " << username << std::endl;

                    while (true) {
                        // Receive message from client
                        if (lws_recv(wsi, reinterpret_cast<unsigned char*>(buffer), sizeof(buffer), LWS_WRITE_TEXT) <= 0) {
                            std::cerr << "Error receiving message." << std::endl;
                            break;
                        }

                        std::string clientMessage = buffer;
                        if (clientMessage == "exit") {
                            std::cout << "User: " << username << " has left the messaging app." << std::endl;
                            break;
                        }

                        // Process the received message (e.g., perform any required operations)
                        // ...

                        // Send response to client
                        std::string responseMessage = "Server received message: " + clientMessage;
                        lws_write(wsi, reinterpret_cast<unsigned char*>(const_cast<char*>(responseMessage.c_str())), responseMessage.size(), LWS_WRITE_TEXT);
                    }
                } else {
                    std::cout << "Authentication failed for user: " << username << std::endl;
                }
            }
            break;

        default:
            break;
    }

    return 0;
}

int main()
{
    struct lws_context_creation_info info;
    struct lws_context *context;
    const char *interface = NULL;
    int port = 8080;

    memset(&info, 0, sizeof info);
    info.port = port;
    info.iface = interface;
    info.protocols = nullptr;
    info.extensions = nullptr;
    info.gid = -1;
    info.uid = -1;

    info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    context = lws_create_context(&info);
    if (!context) {
        std::cerr << "Failed to create libwebsocket context" << std::endl;
        return 1;
    }

    std::cout << "Server started. Waiting for connections..." << std::endl;

    while (true) {
        lws_service(context, 0);
    }

    lws_context_destroy(context);

    return 0;
}

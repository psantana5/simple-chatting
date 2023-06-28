#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr{};
    socklen_t addr_size;

    // Create the client socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    // Configure server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to the server." << std::endl;
        return 1;
    }

    char buffer[1024];

    // Prompt for login
    std::string username, password;
    std::cout << "Login" << std::endl;
    std::cout << "Username: ";
    std::getline(std::cin, username);
    std::cout << "Password: ";
    std::getline(std::cin, password);

    // Send login details to the server
    std::string loginMessage = username + "|" + password;
    if (send(clientSocket, loginMessage.c_str(), loginMessage.size(), 0) == -1) {
        std::cerr << "Error sending login details." << std::endl;
        close(clientSocket);
        return 1;
    }

    // Receive login response from the server
    memset(buffer, 0, sizeof(buffer));
    if (recv(clientSocket, buffer, sizeof(buffer), 0) == -1) {
        std::cerr << "Error receiving login response." << std::endl;
        close(clientSocket);
        return 1;
    }

    std::string response = buffer;

    if (response == "success") {
        // Successful login
        std::cout << "Login successful. Welcome, " << username << "!" << std::endl;

        // Messaging app logic
        while (true) {
            // Send message to server
            std::cout << "Client: ";
            std::string message;
            std::getline(std::cin, message);
            if (message == "exit") {
                std::cout << "Exiting messaging app." << std::endl;
                break;
            }

            if (send(clientSocket, message.c_str(), message.size(), 0) == -1) {
                std::cerr << "Error sending message." << std::endl;
                break;
            }

            // Receive response from server
            memset(buffer, 0, sizeof(buffer));
            if (recv(clientSocket, buffer, sizeof(buffer), 0) == -1) {
                std::cerr << "Error receiving response." << std::endl;
                break;
            }

            std::cout << "Server: " << buffer << std::endl;
        }
    } else {
        // Login failed
        std::cout << "Login failed. Invalid username or password." << std::endl;
    }

    // Close the socket
    close(clientSocket);

    return 0;
}

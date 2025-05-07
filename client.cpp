#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>  // For inet_pton, ntohl
#include <unistd.h>      // For close
#include <sys/socket.h>  // For socket, connect, send, recv

// Constants
#define SERVER_PORT 3000
#define SERVER_ADDRESS "127.0.0.1" // or replace with your server's IP

// Function to send request
void send_request(int sockfd, uint8_t callType, uint8_t resendSeq) {
    uint8_t request[2] = { callType, resendSeq }; // Request payload

    // Send the request to the server
    if (send(sockfd, request, sizeof(request), 0) < 0) {
        std::cerr << "Error sending request." << std::endl;
        exit(1);
    }
    std::cout << "Request sent. Call Type: " << (int)callType << ", ResendSeq: " << (int)resendSeq << std::endl;
}

// Function to handle the response and parse the packets
void handle_response(int sockfd) {
    uint8_t buffer[256];  // Buffer to store response
    int bytes_received;

    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
        // Read the response in chunks
        for (int i = 0; i < bytes_received; i += 15) {
            // Ensure we have enough data for one packet (15 bytes)
            if (bytes_received - i < 15) break;
            
            // Extract fields from the packet (Big Endian)
            uint32_t symbol;
            uint8_t buy_sell;
            uint32_t quantity;
            uint32_t price;
            uint32_t seq;

            // Read symbol (4 bytes)
            symbol = ntohl(*(uint32_t*)&buffer[i]);
            // Read buy/sell indicator (1 byte)
            buy_sell = buffer[i + 4];
            // Read quantity (4 bytes)
            quantity = ntohl(*(uint32_t*)&buffer[i + 5]);
            // Read price (4 bytes)
            price = ntohl(*(uint32_t*)&buffer[i + 9]);
            // Read packet sequence (4 bytes)
            seq = ntohl(*(uint32_t*)&buffer[i + 13]);

            // Convert symbol to a string (4 characters)
            char symbol_str[5];
            symbol_str[0] = (symbol >> 24) & 0xFF;
            symbol_str[1] = (symbol >> 16) & 0xFF;
            symbol_str[2] = (symbol >> 8) & 0xFF;
            symbol_str[3] = symbol & 0xFF;
            symbol_str[4] = '\0';

            // Display the extracted fields
            std::cout << "Packet: Symbol: " << symbol_str
                      << ", Buy/Sell: " << (buy_sell == 'B' ? "Buy" : "Sell")
                      << ", Quantity: " << quantity
                      << ", Price: " << price
                      << ", Seq: " << seq
                      << std::endl;
        }
    }

    if (bytes_received < 0) {
        std::cerr << "Error receiving data." << std::endl;
    }
}

// Function to reconnect for missing sequences (if required)
void handle_missing_sequences(int sockfd, uint8_t missing_seq) {
    std::cout << "Requesting missing sequence: " << (int)missing_seq << std::endl;
    send_request(sockfd, 2, missing_seq);  // Call type 2 for Resend Packet
    handle_response(sockfd);
}

int main() {
    // Initialize socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    // Convert IP address to binary form
    if (inet_pton(AF_INET, SERVER_ADDRESS, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address." << std::endl;
        return 1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed." << std::endl;
        return 1;
    }

    std::cout << "Connected to the server." << std::endl;

    // Sending a request to stream all packets
    send_request(sockfd, 1, 0);  // Call type 1, no resend sequence
    handle_response(sockfd);

    // Close connection after receiving all packets
    close(sockfd);
    std::cout << "Connection closed." << std::endl;

    return 0;
}

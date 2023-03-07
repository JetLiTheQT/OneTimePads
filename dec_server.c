#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_BUFFER_SIZE 100000

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address));
    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

// // Decrypt ciphertext using one-time pad with key
// void decrypt(char* decryptedtext, const char* ciphertext, const char* key) {
//     int i;
//     int length = strlen(ciphertext);
//     for (i = 0; i < length; i++) {
//         decryptedtext[i] = '\0';
//         int c = ciphertext[i] - 'A'; // Ciphertext character as integer (0-25)
//         int k = key[i] - 'A'; // Key character as integer (0-25)
//         int p = (c - k) % 27; // Plaintext character as integer (0-26)
//         if (p < 0) {
//             p += 27; // Ensure positive modulus value
//         }
//         if (p == 26) { // Space
//             decryptedtext[i] = ' ';
//         } else { // A-Z
//             decryptedtext[i] = (p + 'A');
//         }
//     }
//     decryptedtext[length] = '\0';
// }
// Decrypt ciphertext using one-time pad with key
void decrypt(char* decryptedtext, const char* ciphertext, const char* key) {
    int i;
    int length = strlen(ciphertext);
    for (i = 0; i < length; i++) {
        decryptedtext[i] = '\0';
        int c = ciphertext[i] - 'A'; // Ciphertext character as integer (0-25)
        int k = key[i] - 'A'; // Key character as integer (0-25)
        int p;
        if (c == 26) { // Space
            p = 26;
        } else { // A-Z
            p = (c - k) % 26;
            if (p < 0) {
                p += 26; // Ensure positive modulus value
            }
        }
        if (p == 26) { // Space
            decryptedtext[i] = ' ';
        } else { // A-Z
            decryptedtext[i] = p + 'A';
        }
    }
    decryptedtext[length] = '\0';
}

int main(int argc, char *argv[]) {
    int listenSocket, connectionSocket, portNumber, charsRead;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    //char buffer[10];
    char decryptedtext[MAX_BUFFER_SIZE]  = {"\0"};
    char ciphertext[MAX_BUFFER_SIZE]  = {"\0"};
    char key[MAX_BUFFER_SIZE] = {"\0"};

    // Check usage & arguments
    if (argc < 2) {
        fprintf(stderr,"USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Set up the socket
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }

    // Allow reuse of port if already in use
    int yes = 1;
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        error("ERROR setting socket options");
    }

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Enable the socket to begin listening
    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
    }
    listen(listenSocket, 5); // Flip the socket on - it can now receive up to 5 connections

    while (1) {
        // Accept a connection, blocking if one is not available until one connects
        connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
        if (connectionSocket < 0) {
            error("ERROR on accept");
        }

        // Fork a child process to handle this connection
        pid_t pid = fork();
        if (pid < 0) {
            error("ERROR on fork");
        }

        if (pid == 0) {
            // // Verify that the client is dec_client
            // charsRead = 0;            // Reset charsRead
            // memset(buffer, '\0', 10);            // Initialize the buffer with null terminators
            // printf("Received data: %s\n", buffer);
            // charsRead = recv(connectionSocket, buffer, 10, 0);
            // if (charsRead < 0){
            //     error("ERROR reading from socket");
            // }
            // buffer[charsRead] = '\0';
            // if (strcmp(buffer, "dec_client\n") != 0) {
            //     fprintf(stderr, "SERVER: ERROR, client is not dec_client\n");
            //     close(connectionSocket);
            //     continue;
            // }

           // Reset charsRead
            charsRead = 0;

            // Receive ciphertext from the client
                memset(ciphertext, '\0', MAX_BUFFER_SIZE);
                charsRead = recv(connectionSocket, ciphertext, MAX_BUFFER_SIZE - 1, 0);
                if (charsRead < 0) {
                    error("ERROR reading from socket");
                }
                // printf("SERVER POST RECEIVE PLAINTEXT: %s", plaintext);
            // // Reset charsRead
            // charsRead = 0;

            // // Receive key from the client
                memset(key, '\0', MAX_BUFFER_SIZE);
            //     charsRead = recv(connectionSocket, key, MAX_BUFFER_SIZE - 1, 0);
            //     if (charsRead < 0) {
            //         error("ERROR reading from socket");
            //     }
            if (ciphertext[0] != '\0') {  // Check if the buffer isn't null character
                char *token;
                // Tokenize the buffer based on the newline character, starting from the second character
                token = strtok(ciphertext, "\n");
                // If there is a token, copy it to the plaintext buffer
                if (token != NULL) {
                    strcpy(ciphertext, token);
                    // Check for another token (the key)
                    token = strtok(NULL, "\n");

                    if (token != NULL) {
                        // Copy the second token (the key) to the key buffer
                        strcpy(key, token);
                    }
                    
                }
            }

            // Remove trailing newline from ciphertext and key
            ciphertext[strcspn(ciphertext, "\n")] = '\0';
            key[strcspn(key, "\n")] = '\0';

            // // Make sure the key is at least as long as the ciphertext
            // if (strlen(key) < strlen(ciphertext)) {
            //     fprintf(stderr, "SERVER: ERROR key '%s' is too short\n", key);
            //     close(connectionSocket);
            //     exit(1);
            // }

            // Reset charsRead
            charsRead = 0;
            memset(decryptedtext, '\0', MAX_BUFFER_SIZE);

            // Decrypt the ciphertext using the key
            decrypt(decryptedtext, ciphertext, key);
            // sprintf(decryptedtext, "%s\n", decryptedtext);

            // Send the decrypted plaintext back to the client
            charsRead = send(connectionSocket, decryptedtext, strlen(ciphertext), 0);
            if (charsRead < 0) {
                error("ERROR writing to socket");
            }

            close(connectionSocket); // Close the connection socket for this client
            exit(0);
        }else {
            // In parent process
            // Close the connection socket for this client
            close(connectionSocket);
        }
    
    }

    close(listenSocket); // Close the listening socket
    return 0;
}

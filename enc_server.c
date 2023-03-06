#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_BUFFER_SIZE 70001

// Error function used for reporting issues
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

// Encrypt plaintext using one-time pad with key
void encrypt(char* ciphertext, const char* plaintext, const char* key) {
    printf("Plaintext: %s \n Key: %s\n ", plaintext, key);
    int i;
    int length = strlen(plaintext);
    for (i = 0; i < length; i++) {
        ciphertext[i] = '\0';
        int p = plaintext[i] - 'A'; // Plaintext character as integer (0-25)
        int k = key[i] - 'A'; // Key character as integer (0-25)
        int c = (p + k) % 27; // Ciphertext character as integer (0-26)
        if (c == 26) { // Space
            ciphertext[i] = ' ';
        } else { // A-Z
            ciphertext[i] = c + 'A';
        }
    }
    ciphertext[length] = '\0';
    printf("Encryption output: %s", ciphertext);
}




int main(int argc, char *argv[]){
    int listenSocket, connectionSocket, charsRead;
    char buffer[MAX_BUFFER_SIZE], key[MAX_BUFFER_SIZE], ciphertext[MAX_BUFFER_SIZE] = {"\0"}, plaintext[MAX_BUFFER_SIZE];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    // Check usage & args
    if (argc < 2) { 
        fprintf(stderr,"USAGE: %s port\n", argv[0]); 
        exit(1);
    } 

    // Create the socket that will listen for connections
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        error("ERROR on binding");
    }

    // Start listening for connections. Allow up to 5 connections to queue up
    listen(listenSocket, 5); 

    // Accept a connection, blocking if one is not available until one connects
    while(1){
        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
        if (connectionSocket < 0){
            error("ERROR on accept");
        }

        printf("Connection accepted!\n");

        // Fork a child process to handle this connection
        pid_t pid = fork();
        if (pid < 0) {
            error("ERROR on fork");
        }

        if (pid == 0) {
            // In child process
            // Reset charsRead
                charsRead = 0;
            // Initialize the buffer with null terminators
                memset(buffer, '\0', MAX_BUFFER_SIZE);

            // Receive data from the socket into the buffer
            charsRead = recv(connectionSocket, buffer, MAX_BUFFER_SIZE - 1, 0);

            // Check if there was an error
            if (charsRead < 0) {
                error("ERROR reading from socket");
            }
            buffer[charsRead] = '\0';
            // Print the received data
            // printf("Received data: %s\n", buffer);
            // printf("Received identifier: %s\n", buffer); // TESTING
            if (strcmp(buffer, "enc_client") != 0) {
                fprintf(stderr, "SERVER: ERROR, client is not enc_client\n");
                close(connectionSocket);
                exit(1);
            }

        // Reset charsRead
        charsRead = 0;
        // Receive plaintext from the client
        memset(plaintext, '\0', MAX_BUFFER_SIZE);
        charsRead = recv(connectionSocket, plaintext, MAX_BUFFER_SIZE - 1, 0);
        if (charsRead < 0) {
            error("ERROR reading from socket");
        }

        // Reset charsRead
        charsRead = 0;

        // // Receive delimiter from the client
        // char delimiter;
        // charsRead = recv(connectionSocket, &delimiter, 1, 0);
        // if (charsRead < 0) {
        //     error("ERROR reading from socket");
        // }
        // if (delimiter != '#') {
        //     error("ERROR: Invalid delimiter received\n");
        // }


        // // Reset charsRead
        // charsRead = 0;


        // Receive key from the client
        memset(key, '\0', MAX_BUFFER_SIZE);
        charsRead = recv(connectionSocket, key, MAX_BUFFER_SIZE - 1, 0);
        if (charsRead < 0) {
            error("ERROR reading from socket");
        }

        // Remove trailing newline from plaintext and key
        plaintext[strcspn(plaintext, "\n")] = '\0';
        key[strcspn(key, "\n")] = '\0';

        // Make sure the key is at least as long as the plaintext
        if (strlen(key) < strlen(buffer)) {
            fprintf(stderr, "SERVER: ERROR key '%s' is too short\n", key);
            close(connectionSocket);
            exit(1);
        }

        // Reset charsRead
        charsRead = 0;

        //printf("Plaintext: %s \n Key: %s \n ", plaintext, key);
        // Encrypt the plaintext using the key
        //char *ciphertext = malloc((strlen(plaintext) + 1) * sizeof(char));
        memset(ciphertext, '\0', MAX_BUFFER_SIZE);
        encrypt(ciphertext, plaintext, key);
        // printf("Ciphertext: %s", ciphertext);

        

        // Send the ciphertext back to the client
        charsRead = send(connectionSocket, ciphertext, strlen(ciphertext), 0);
        if (charsRead < 0) {
            error("ERROR writing to socket");
        }

        // Close the connection socket for this client
        close(connectionSocket);
        exit(0);
        } else {
            // In parent process
            // Close the connection socket for this client
            close(connectionSocket);
        }
    }

    // Close the listening socket
    close(listenSocket);

    return 0;
}

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>

// // Define maximum message size
// #define MAX_MSG_SIZE 1000

// // Error function used for reporting issues
// void error(const char *msg) {
//     perror(msg);
//     exit(1);
// }

// // Set up the address struct
// void setupAddressStruct(struct sockaddr_in* address, int portNumber) {
//     // Clear out the address struct
//     memset((char*) address, '\0', sizeof(*address));

//     // The address should be network capable
//     address->sin_family = AF_INET;

//     // Store the port number
//     address->sin_port = htons(portNumber);

//     // Allow a client at any address to connect to this server
//     address->sin_addr.s_addr = INADDR_ANY;
// }

// int main(int argc, char *argv[]) {
//     int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
//     socklen_t sizeOfClientInfo;
//     char buffer[MAX_MSG_SIZE];
//     char plaintext[MAX_MSG_SIZE];
//     char key[MAX_MSG_SIZE];
//     char ciphertext[MAX_MSG_SIZE];
//     struct sockaddr_in serverAddress, clientAddress;

//     // Check usage & args
//     if (argc < 2) {
//         fprintf(stderr,"USAGE: %s port\n", argv[0]);
//         exit(1);
//     }

//     // Set up the address struct for this process (the server)
//     setupAddressStruct(&serverAddress, atoi(argv[1]));

//     // Create a socket
//     listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
//     if (listenSocketFD < 0) {
//         error("ERROR opening socket");
//     }

//     // Enable the socket to begin listening
//     if (bind(listenSocketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
//         error("ERROR on binding");
//     }

//     // Flip the socket on - it can now receive up to 5 connections
//     if (listen(listenSocketFD, 5) < 0) {
//         error("ERROR on listening");
//     }

//     while (1) {
//         // Accept a connection, blocking if one is not available until one connects
//         sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
//         establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *) &clientAddress, &sizeOfClientInfo); // Accept
//         if (establishedConnectionFD < 0) {
//             error("ERROR on accept");
//         }
//         else{
//          printf("Server: Successfully connected to the Client!"); // to see if i successfully connected.
//         }     

//         // Fork a new process to handle the connection
//         int childPid = fork();
//         if (childPid < 0) {
//             error("ERROR on fork");
//         } else if (childPid == 0) { // In the child process
//             // Close the listening socket because the child process doesn't need it
//             close(listenSocketFD);

//             // Receive plaintext from client
//             memset(plaintext, '\0', sizeof(plaintext));
//             while (strstr(plaintext, "@@") == NULL) {
//                 memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
//                 charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0);
//                 if (charsRead < 0) {
//                     error("ERROR reading from socket");
//                 }
//                 strcat(plaintext, buffer);
//             }
//             // Remove the terminating characters from the key
//             plaintext[strcspn(key, "@")] = '\0';
//             printf("Received plaintext: %s\n", plaintext);
//             // Receive key from client
//             memset(key, '\0', sizeof(key));
//             while (strlen(key) < sizeof(plaintext)) {
//                 memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
//                 charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0);
//                 if (charsRead < 0) {
//                     error("ERROR reading from socket");
//                 }
//                 strcat(key, buffer);
//             }
//             // Remove the terminating characters from the key
//             key[strcspn(key, "@")] = '\0';
            
//         // Encrypt the plaintext using the key
//         memset(ciphertext, '\0', sizeof(ciphertext));
//         int i;
//         for (i = 0; i < sizeof(plaintext); i++) {
//             // Convert plaintext and key characters to ASCII values
//             int plaintextChar = (int) plaintext[i];
//             int keyChar = (int) key[i];

//             // Subtract 65 from each character to make A = 0, B = 1, etc.
//             plaintextChar -= 65;
//             keyChar -= 65;

//             // Add the plaintext and key values together, modulo 26
//             int ciphertextChar = (plaintextChar + keyChar) % 26;

//             // Add 65 to the ciphertext value to get back into ASCII range
//             ciphertextChar += 65;

//             // Convert the ciphertext value back to a character and add it to the ciphertext string
//             char ciphertextCharStr[2];
//             sprintf(ciphertextCharStr, "%c", ciphertextChar);
//             strcat(ciphertext, ciphertextCharStr);
//         }

//         // Add terminating characters to the ciphertext
//         strcat(ciphertext, "@@");

//         // Send the ciphertext back to the client
//         charsRead = send(establishedConnectionFD, ciphertext, strlen(ciphertext), 0);
//         if (charsRead < 0) {
//             error("ERROR writing to socket");
//         }

//         // Close the existing socket which is connected to the client
//         close(establishedConnectionFD);

//         // Exit the child process
//         exit(0);
//     } else { // In the parent process
//         // Close the established connection because the parent process doesn't need it
//         close(establishedConnectionFD);
//     }
//     }

//     // Close the listening socket
//     close(listenSocketFD);
//     return 0;

// }



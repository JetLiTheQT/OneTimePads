#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_BUFFER_SIZE 100000

// Error function used for reporting issues
void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    char buffer[10]= {'\0'};
    char plaintext[MAX_BUFFER_SIZE] = {'\0'};
    char key[MAX_BUFFER_SIZE]={'\0'};
    char ciphertext[MAX_BUFFER_SIZE]= {'\0'};

    // Check usage & args
    if (argc < 3) {
        fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]);
        exit(0);
    }

    // Set up buffer
    memset(buffer, '\0', 10);
    strcpy(buffer, "enc_client\n");
    

    // Get the plaintext from file
    FILE *plaintextFile = fopen(argv[1], "r");
    if (plaintextFile == NULL) {
        fprintf(stderr, "ERROR opening plaintext file\n");
        exit(1);
    }
    fgets(plaintext, MAX_BUFFER_SIZE, plaintextFile);
    // plaintext[strcspn(plaintext, "\n")] = '\0';
    fclose(plaintextFile);

    // Get the key from file
    FILE *keyFile = fopen(argv[2], "r");
    if (keyFile == NULL) {
        fprintf(stderr, "ERROR opening key file\n");
        exit(1);
    }
    fgets(key, MAX_BUFFER_SIZE, keyFile);
    // key[strcspn(key, "\n")] = '\0';
    fclose(keyFile);

    // Set up the server address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
    if (serverHostInfo == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr_list[0], serverHostInfo->h_length); // Copy in the address

    // Create the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("ERROR opening socket");
    }

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("ERROR connecting");
    }

    
    // Send verification to the server
    
    if (send(socketFD, buffer, strlen(buffer), 0) < 0) {
        error("ERROR writing to socket");
    }

    // Send plaintext to the server
    if (send(socketFD, plaintext, strlen(plaintext), 0) < 0) {
        error("ERROR writing to socket");
    }

    // Send key to the server
    if (send(socketFD, key, strlen(key), 0) < 0) {
        error("ERROR writing to socket");
    }
    // Receive the ciphertext from the server
    memset(ciphertext, '\0', MAX_BUFFER_SIZE);
    charsRead = recv(socketFD, ciphertext, MAX_BUFFER_SIZE - 1, 0);
    if (charsRead < 0) {
        error("CLIENT: ERROR reading from socket");
    }

    // Print the ciphertext to stdout
    printf("%s\n", ciphertext);

    // Close the socket
    close(socketFD);

    return 0;
}




// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <netdb.h> 

// // Define maximum message size
// #define MAX_MSG_SIZE 1000

// // Error function used for reporting issues
// void error(const char *msg) { 
//     perror(msg); 
//     exit(0); 
// }

// // Set up the address struct
// void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname) {
//     // Clear out the address struct
//     memset((char*) address, '\0', sizeof(*address));
//     // The address should be network capable
//     address->sin_family = AF_INET;
//     // Store the port number
//     address->sin_port = htons(portNumber);
//     // Get the DNS entry for this host name
//     struct hostent* hostInfo = gethostbyname(hostname);
//     if (hostInfo == NULL) {
//         fprintf(stderr, "CLIENT: ERROR, no such host\n");
//         exit(0);
//     }
//     // Copy the first IP address from the DNS entry to sin_addr.s_addr
//     memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
// }

// int main(int argc, char *argv[]) {
//     int socketFD, portNumber, charsWritten, charsRead;
//     struct sockaddr_in serverAddress;
//     struct hostent* serverHostInfo;
//     char buffer[MAX_MSG_SIZE];
//     char plaintext[MAX_MSG_SIZE];
//     char key[MAX_MSG_SIZE];
//     char ciphertext[MAX_MSG_SIZE];
//     FILE* plaintextFile;
//     FILE* keyFile;
//     size_t bytesRead = 0;
//     size_t totalBytesRead = 0;
//     size_t totalBytesWritten = 0;
//     size_t plaintextSize = 0;
//     size_t keySize = 0;
           
//     //Testing
//     printf("in main");
//     //


//     // Initialize arrays with null terminators
//     memset(plaintext, '\0', sizeof(plaintext));
//     memset(key, '\0', sizeof(key));
//     memset(ciphertext, '\0', sizeof(ciphertext));

//     // Check usage & args
//     if (argc < 4) { 
//         fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); 
//         exit(0); 
//     }

//     // Open plaintext file
//     plaintextFile = fopen(argv[1], "r");
//     if (plaintextFile == NULL) {
//         fprintf(stderr, "CLIENT: ERROR, unable to open plaintext file\n");
//         exit(1);
//     }

//     // Open key file
//     keyFile = fopen(argv[2], "r");
//     if (keyFile == NULL) {
//         fprintf(stderr, "CLIENT: ERROR, unable to open key file\n");
//         exit(1);
//     }

//     // Get plaintext from file
//     while ((bytesRead = fread(plaintext + totalBytesRead, 1, MAX_MSG_SIZE - totalBytesRead, plaintextFile)) > 0) {
//         totalBytesRead += bytesRead;
//         if (totalBytesRead == MAX_MSG_SIZE) {
//             break;
//         }
//     }
//     plaintextSize = totalBytesRead;
//     fclose(plaintextFile);

       
//     //Testing
//     printf("after getting plaintext from file");
//     //


//     // Get key from file
//     totalBytesRead = 0;
//     while ((bytesRead = fread(key + totalBytesRead, 1, MAX_MSG_SIZE - totalBytesRead, keyFile)) > 0) {
//         totalBytesRead += bytesRead;
//         if (totalBytesRead == MAX_MSG_SIZE) {
//             break;
//         }
//     }
//     keySize = totalBytesRead;
//     fclose(keyFile);

//     // Make sure key is at least as long as plaintext
//     if (keySize < plaintextSize) {
//         fprintf(stderr, "CLIENT: ERROR, key is too short\n");
//         exit(1);
//     }

//     //Testing
//     printf("before setting up server");
//     //

//     // Set up the server address struct
//     setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

//     // Create a socket
//     socketFD = socket(AF_INET, SOCK_STREAM, 0);
//     if (socketFD < 0) {
//         error("CLIENT: ERROR opening socket");
//     }
   
//     //Testing
//     printf("before connecting to server");
//     //

//     // Connect to server
//     if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
//         error("CLIENT: ERROR connecting");
//     }

//     // Combine plaintext and key into single string with delimiter
//     sprintf(buffer, "%s%c%s", plaintext, '\n', key);

//     // Send message to server
//     totalBytesWritten = 0;
//     while (totalBytesWritten < plaintextSize + keySize + 1) {
//         charsWritten = send(socketFD, buffer + totalBytesWritten, plaintextSize + keySize + 1 - totalBytesWritten, 0);
//         if (charsWritten < 0) {
//             error("CLIENT: ERROR writing to socket");
//         }
//         totalBytesWritten += charsWritten;
//     }

//     // Get ciphertext from server
//     totalBytesRead = 0;
//     while (totalBytesRead < plaintextSize) {
//         charsRead = recv(socketFD, ciphertext + totalBytesRead, plaintextSize - totalBytesRead, 0);
//         if (charsRead < 0) {
//             error("CLIENT: ERROR reading from socket");
//         }
//         totalBytesRead += charsRead;
//     }

//     // Print ciphertext to stdout
//     printf("%s\n", ciphertext);

//     // Close the socket
//     close(socketFD);
//     return 0;
// }

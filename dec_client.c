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

int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    // char buffer[10]= {'\0'} 
    char key[MAX_BUFFER_SIZE]= {'\0'}, ciphertext[MAX_BUFFER_SIZE]= {'\0'}, decryptedtext[MAX_BUFFER_SIZE]= {'\0'};

    // Check usage & args
    if (argc < 3) {
        fprintf(stderr,"USAGE: %s ciphertext key port\n", argv[0]);
        exit(0);
    }


    // // Set validator buffer up:
    // memset(buffer, '\0', 10);
    // strcpy(buffer, "dec_client\n");

    // Get the plaintext from file
    FILE *cipherTextFile = fopen(argv[1], "r");
    if (cipherTextFile == NULL) {
        fprintf(stderr, "ERROR opening cipher text file\n");
        exit(1);
    }
    fgets(ciphertext, MAX_BUFFER_SIZE, cipherTextFile);
    fclose(cipherTextFile);

    // Get the key from file
    FILE *keyFile = fopen(argv[2], "r");
    if (keyFile == NULL) {
        fprintf(stderr, "ERROR opening key file\n");
        exit(1);
    }
    fgets(key, MAX_BUFFER_SIZE, keyFile);
    fclose(keyFile);

    //printf("DEC_CLIENT: \nBuffer: %s\nCiphertext: %s\nKey: %s\n", buffer, ciphertext, key);
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

    
    // // Send verification to the server
    
    // if (send(socketFD, buffer, strlen(buffer), 0) < 0) {
    //     error("ERROR writing to socket");
    // }

    // Send cipherText to the server
    if (send(socketFD, ciphertext, strlen(ciphertext), 0) < 0) {
        error("ERROR writing to socket");
    }

    // Send key to the server
    if (send(socketFD, key, strlen(key), 0) < 0) {
        error("ERROR writing to socket");
    }

    memset(decryptedtext, '\0', MAX_BUFFER_SIZE);

    // Receive the decrypted plaintext from the server
    charsRead = recv(socketFD, decryptedtext, MAX_BUFFER_SIZE - 1, 0);
    if (charsRead < 0) {
        error("CLIENT: ERROR reading from socket");
    }

    // Print the decrypted plaintext to stdout
    strcat(decryptedtext, "\n");
    printf("%s", decryptedtext);

    // Close the socket
    close(socketFD);
    return 0;
}
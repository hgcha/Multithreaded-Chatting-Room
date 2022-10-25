#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int client_socket;

void read_msg(){ // The function for the thread which reads messages from the server.

    char read_buffer[1124]; // The buffer for holding the message from the server.

    while(1){
        if(read(client_socket, read_buffer, 1124) == -1){ // Try reading a message from the server.
            printf("Error while receiving a message."); // Print the error message if an error occurs.
            return;
        }
        else{
            printf("%s", read_buffer); // If read() is successful, print the message read from the server.
        }
    }    

    return;
}

void write_msg(){ // The function for the thread which sends a message to the server.

    char write_buffer[1024]; // The buffer for holding the message to send to the server.
    
    while(1){
        fgets(write_buffer, 1024, stdin); //Get input for the message to send to the server, and save it in 'write_buffer'.
        if(write(client_socket, write_buffer, 1024) == -1){ //Try sending the message to the server.
            printf("Error while writing the message."); // Print the error message if an error occurs.
            return;
        }
        if(strcmp(write_buffer, "exit\n") == 0){ // If the message to send to the server is 'exit', break the while loop.
            break;
        }
    }

    close(client_socket); // Close the socket.
    exit(0); // Terminate the process.
}

int main(void)
{
    struct sockaddr_in server_addr; // A struct variable for the server's address.
    char ip[32]; // The server's IP
    int port; // The server's port number.
    pthread_t read_thread; // The thread for reading messages from the server.
    pthread_t write_thread; // The thread for sending messages to the server.
    void *thread_ret; // A void pointer for the return value of pthread_join().

    printf("Server IP Address: "); // Get input for the server's IP.
    scanf("%s", ip);
    printf("Server Port: "); // Get input for the server's port.
    scanf("%d", &port);

    client_socket = socket(AF_INET, SOCK_STREAM, 0); // Create a socket for the connection to the server.

    server_addr.sin_family = AF_INET; // Set the server's address and port.
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    if(connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){ // Try connecting to the server.
        printf("Error while connecting to server.\n"); // Print the error message if an error occurs.
        return 0;
    }
    else{
        printf("Successfully connected to server.\n"); // If the connection is successful, print the message that the connection is successful.
    }

    pthread_create(&read_thread, NULL, (void *)read_msg, NULL); // Create a thread which reads messages from the server.
    pthread_create(&write_thread, NULL, (void*)write_msg, NULL); // Create a thread which sends a message from the server.
    pthread_join(read_thread, &thread_ret); // Wait for 'read_thread' to terminate.
    pthread_join(write_thread, &thread_ret); // Wait for 'write_thread' to terminate.

    return 0;
}
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

struct client_info { // The struct variable for holding client's socket and ID.
    int client_id;
    int socket;
};

struct client_info client[100] = {0}; // An array for holding the information of clients. Can hold up to 100 clients' information.

void write_msg(int id, char *msg_from_client){ // The function for sending a message from a client to other clients.
    
    int i;
    for(i = 0; i < 100; i++){ // Send a message from a client to other clients except the one who sended the message.
        if(client[i].client_id != 0 && client[i].client_id != id){ 
            write(client[i].socket, msg_from_client, 1024);
        }
    }
} 

void read_msg(struct client_info* cli){ // The function for reading a client's message.
    
    char msg[1024] = {0}; // The original message from a client.
    char client_msg[1124] = {0}; // The processed message which includes the client's ID before the message.

    sprintf(client_msg, "Client %d has been connected.\n", cli->client_id); //If a client connects to the server, client_msg holds the message that a client has been connected. 
    printf("%s", client_msg); // Print the message that a client has been connected to the server.
    write_msg(cli->client_id, client_msg); // Notify all the other clients that a client has been connected.
    
    while(1){
        if(read(cli->socket, msg, 1024) == -1){ // Read a message from a client. And save the result in 'msg'.
            printf("Error in recv"); // Print the error message if read() fails.
            break;
        }
        else{
            if((strlen(msg) > 0) && (msg[0] != '\n')){ //If the length of message is longer than 0, and the first character is not '\n'
                if(strcmp(msg, "exit\n") == 0){ // If the client sends 'exit',
                    sprintf(client_msg, "Client %d has been disconnected.\n", cli->client_id); //Save a message that a client has been disconnected in 'client_msg'.
                    printf("%s", client_msg); // Print 'client_msg' to the server.
                    write_msg(cli->client_id, client_msg); // Send 'client_msg' to the other clients who are still connected.
                    break;
                }
                else{ // If the message from a client is not 'exit',
                    sprintf(client_msg, "Client %d: %s", cli->client_id, msg); // Save the client's id with the client's message in 'client_msg'.
                    printf("Message from %s", client_msg); // Print 'client_msg' to the server.
                    write_msg(cli->client_id, client_msg); // Send 'client_msg' to the other clients except the one who sended the message.
                }
            }
        }
    }

    close(cli->socket); // Close the socket for the client if the client is disconnected from the server.
    cli->client_id = 0; // Set the ID of the disconnected client to 0.
    return;
}

int main(void){

    int server_socket;
    char *ip = "127.0.0.1"; //The server's IP is '127.0.0.1'.
    int port;
    struct sockaddr_in server_addr; // A struct variable for server's address.
    struct sockaddr_in client_addr; // A struct variable for client's address.
    socklen_t client_addr_size = sizeof(client_addr);
    int client_num = 0; // Counting the number of clients.
    int client_socket; // The socket used for accepting the client's connection request.
    char msg[1024] = {0}; // A buffer for holding a message for sending to clients.
    pthread_t* client_thread; // pthread_t pointer for creating a new thread.
    int i;

    server_socket = socket(AF_INET, SOCK_STREAM, 0); //Create a socket for server.

    printf("Server Port: "); // Get input for the server's port.
    scanf("%d", &port);

    server_addr.sin_family = AF_INET; //Set the server's IP and port.
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){ //Bind the server's IP and port to the server's socket.
        printf("Error in binding\n");
        return 0;
    }

    if(listen(server_socket, 10) == -1){ //Wait for clients' connection requests.
        printf("Error in listening\n");
        return 0;
    }

    printf("Server is opened.\n"); //If no error occurs, print that the server is successfully opened.
    
    while(1){
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size); //If a client send a connect request, accept it and create a socket for the client. 
        
        if(client_socket == -1){ //If accept() fails, print the error message
            printf("Error in accepting a client.\n");
            return 0;
        }
            
        strcpy(msg, "Server: Welcome to the chatting room!\nServer: Enter \"exit\" to exit the chatting room.\n"); //Save a guiding message to the client in 'msg'.

        write(client_socket, msg, sizeof(msg)); // Send the welcoming message to the client. 

        for(i = 0; i < 100; i++){ 
            if(client[i].client_id == 0){  // Find the empty slot in client[].
                client[i].client_id = ++client_num; // Add the information of the new client to the client[].
                client[i].socket = client_socket;
                break;       
            }
        }

        client_thread = (pthread_t*)malloc(sizeof(pthread_t)); // Allocate memory for the new thread.
        if(pthread_create(client_thread, NULL, (void *)read_msg, (void*)&client[i]) != 0){ // Create a new thread for reading the message from the new client. 
            printf("Error while creating thread\n");
            break;
        }
        
        pthread_detach(*client_thread); //*** When the new client_thread ends, free all the resources allocated for the thread.
    }

    return 0;
}
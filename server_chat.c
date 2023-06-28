#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_ROOMS 5
#define MAX_CLIENTS 10
#define STDIN 0

typedef struct {
    char name[256];
    int clients[MAX_CLIENTS];
    int count;
} Room;

typedef struct {
    int fd;
    int room_id;
    char *host;
    int port;
} Client;

fd_set master, read_fds, write_fds;
struct sockaddr_in myaddr, remoteaddr;
int listener, newfd, fdmax, nbytes, yes = 1;
socklen_t addrlen;
char buf[256];

void send_message(int sender_fd, Client *clients, int room_id) {
    const char* sender_host = clients[sender_fd].host;
    int sender_port = clients[sender_fd].port;

    // build a new string 
    char formatted_str[256];
    snprintf(formatted_str, sizeof(formatted_str), "%s:%d -> %s", sender_host, sender_port, buf);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].room_id == room_id && clients[i].fd != sender_fd) {
            send(clients[i].fd, formatted_str, strlen(formatted_str), 0);
        }
    }
}

void send_room_list(int client_fd, Room *rooms) {
    char roomList[256] = "";
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].count < MAX_CLIENTS) {
            char room[32];
            sprintf(room, "%d. %s\n", i, rooms[i].name);
            strcat(roomList, room);
        }
    }
    send(client_fd, roomList, strlen(roomList), 0);
}

void start_room(Room *rooms) {
    int room_id = create_room("General", rooms);
    rooms[room_id].clients[0] = 0;
    rooms[room_id].count = 1;
}

int create_room(char *name, Room *rooms) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].count == 0) {
            strcpy(rooms[i].name, name);
            rooms[i].count = 1;
            rooms[i].clients[0] = -1;
            return i;
        }
    }
    return -1; // Unable to create the room (all rooms are full)
}

int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("Enter IP and Port for this server\n");
        exit(1);
    }

    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    listener = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = inet_addr(argv[1]);
    myaddr.sin_port = htons(atoi(argv[2]));

    memset(&(myaddr.sin_zero), '\0', 8);
    addrlen = sizeof(remoteaddr);
    bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr));

    listen(listener, 10);
    FD_SET(listener, &master);
    FD_SET(STDIN, &master);
    fdmax = listener;

    // Create an array of clients
    Client *clients = (Client *)malloc(MAX_CLIENTS * sizeof(Client));

    // Initialize the client array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;      // -1 indicates an empty slot
        clients[i].room_id = 0;   // Default room ID
        clients[i].host = NULL; // Empty name
        clients[i].port = 0;
    }

    // Create an array of rooms
    Room *rooms = (Room *)malloc(MAX_ROOMS * sizeof(Room));

    // Initialize the room array
    for (int i = 0; i < MAX_ROOMS; i++) {
        rooms[i].name[0] = '\0';
        rooms[i].count = 0;
        for (int j = 0; j < MAX_CLIENTS; j++) {
            rooms[i].clients[j] = -1;
        }
    }

    start_room(rooms);

    printf("Server started. Waiting for connections...\n");

    while (1) {
        read_fds = master;
        select(fdmax + 1, &read_fds, NULL, NULL, NULL);
        
        for (int i = 0; i <= fdmax; i++) {
            if (!FD_ISSET(i, &read_fds))
                continue;

            if (i == listener) {
                newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
                FD_SET(newfd, &master);
                if (newfd > fdmax)
                    fdmax = newfd;

                clients[newfd].fd = newfd;
                clients[newfd].room_id = 0;
                clients[newfd].host = inet_ntoa(remoteaddr.sin_addr);
                clients[newfd].port = ntohs(remoteaddr.sin_port);

                printf("New connection established. Client address: %s, Client port: %d\n", clients[newfd].host, clients[newfd].port);
                const char* message = "Connection established successfully.\n"
                                      "Select one of the options below:\n"
                                      "1. List available rooms: list_rooms\n"
                                      "2. Create a new room: create_room <room_name>\n"
                                      "3. Connect to an existing room: connect_room <room_id>\n"
                                      "4. Quit Room: quit_room\n";
                send(newfd, message, strlen(message), 0);
            } else {
                memset(&buf, 0, sizeof(buf));
                nbytes = recv(i, buf, sizeof(buf), 0);

                if (nbytes <= 0) {
                    // Handle client disconnection
                    close(i);
                    FD_CLR(i, &master);

                    // Remove client from the corresponding room
                    for (int j = 0; j < MAX_ROOMS; j++) {
                        for (int k = 0; k < MAX_CLIENTS; k++) {
                            if (rooms[j].clients[k] == i) {
                                rooms[j].clients[k] = -1;
                                break;
                            }
                        }
                    }

                    printf("Client %d disconnected.\n", i);
                } 
                else {
                    
                    if (strncmp(buf, "list_rooms", 10) == 0) {
                        send_room_list(i, rooms);
                    } 
                    else if (strncmp(buf, "create_room", 11) == 0) {
                        char room_name[256];
                        sscanf(buf, "create_room %[^\n]", room_name);
                        int room_id = create_room(room_name, rooms);
                        if (room_id != -1) {
                            rooms[room_id].clients[0] = i;
                            rooms[room_id].count = 1;

                            clients[i].room_id = room_id;

                            send_message(i, clients, clients[i].room_id);
                            const char* message = "Room created successfully.\n";
                            send(i, message, strlen(message), 0);
                        } else {
                            const char* message = "Unable to create the room. All rooms are full.\n";
                            send(i, message, strlen(message), 0);
                            close(i);
                            FD_CLR(i, &master);
                        }
                    } 
                    else if (strncmp(buf, "connect_room", 12) == 0) {
                        int room_id;
                        sscanf(buf, "connect_room %d", &room_id);
                        if (room_id >= 0 && room_id < MAX_ROOMS) {
                            int count = rooms[room_id].count;
                            if (count < MAX_CLIENTS) {
                                rooms[room_id].clients[count] = i;
                                rooms[room_id].count++;
                                
                                clients[i].room_id = room_id;

                                send_message(i, clients, clients[i].room_id);

                                printf("Message sent to room %d\n", room_id);
                                char message[256];
                                snprintf(message, sizeof(message), "Message sent to room %d\n", clients[i].room_id);
                                send(i, message, strlen(message), 0);
                            } else {
                                const char* message = "The room is full.\n";
                                send(i, message, strlen(message), 0);
                                close(i);
                                FD_CLR(i, &master);
                            }
                        } else {
                            const char* message = "Invalid room ID.\n";
                            send(i, message, strlen(message), 0);
                        }
                    } 
                    else if (strncmp(buf, "quit_room", 9) == 0) {
                        if (clients[i].room_id != 0) {
                            // Return the client to the General
                            int count = rooms[clients[i].room_id].count;
                            rooms[clients[i].room_id].clients[count] = -1;
                            rooms[clients[i].room_id].count--;
                            clients[i].room_id = 0;

                            send_message(i, clients, clients[i].room_id);

                            printf("Message sent to room %d\n", clients[i].room_id);
                            char message[256];
                            snprintf(message, sizeof(message), "Message sent to room %d\n", clients[i].room_id);
                            send(i, message, strlen(message), 0);
                        } else {
                            // Close program
                            close(i);
                            FD_CLR(i, &master);
                        }
                    } 
                    else {
                        // Chat in the current room
                        send_message(i, clients, clients[i].room_id);
                        printf("Message sent to room %d\n", clients[i].room_id);
                        char message[256];
                        snprintf(message, sizeof(message), "Message sent to room %d\n", clients[i].room_id);
                        send(i, message, strlen(message), 0);
                    }
                    printf("Message received from client %s:%d -> %s", clients[i].host, clients[i].port, buf);
                }
            }
        }
    }

    return 0;
}
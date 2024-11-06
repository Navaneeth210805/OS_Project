#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<semaphore.h>
#include<pthread.h>
#include<time.h>
#include<sys/stat.h>

#define PORT 8010

#define MAX_CLIENTS 10
int read_count;
sem_t read_mutex, write_mutex;

void filereader(int client_socket){
    sem_wait(&read_mutex);
    read_count++;
    if(read_count == 1){
        sem_wait(&write_mutex);
    }

    sem_post(&read_mutex);

    char filename[1024];
    recv(client_socket, filename, 1024, 0);
    printf("File name received %s",filename);
    FILE *file = fopen(filename, "r");
    if(file == NULL){
        perror("File not found\n");
        exit(1);
    }
    char buffer[1024];
    while(fgets(buffer, 1024, file)){
        send(client_socket, buffer, 1024, 0);
    }
    
    sem_wait(&read_mutex);
    read_count--;
    if(read_count == 0){
        sem_post(&write_mutex);
    }
    sem_post(&read_mutex);
}

void file_renamer(int client_socket){
    sem_wait(&write_mutex);
    char filename[1024];
    recv(client_socket, filename, 1024, 0);
    printf("File name received %s",filename);
    fflush(stdout);
    char newname[1024];
    recv(client_socket, newname, 1024, 0);
    printf("New name received %s",newname);
    fflush(stdout);
    if(rename(filename, newname) < 0){
        perror("File renaming failed\n");
        exit(1);
    }
    send(client_socket, "File renamed successfully", 1024, 0);
    sem_post(&write_mutex);
}

void metadata_display(int client_socket) {
    sem_wait(&read_mutex);
    read_count++;
    if(read_count == 1){
        sem_wait(&write_mutex);
    }

    sem_post(&read_mutex);

    char filename[1024];
    struct stat file_stat;

    recv(client_socket, filename, 1024, 0);
    printf("File name received for metadata display: %s\n", filename);

    if (stat(filename, &file_stat) == -1) {
        perror("Error getting file metadata");
        send(client_socket, "Error: File not found or unable to access metadata.\n", 1024, 0);
        return;
    }

    char metadata[4096]; 
    snprintf(metadata, sizeof(metadata), 
             "File Size: %lld bytes\n"
             "Permissions: %c%c%c%c%c%c%c%c%c\n"
             "Last Access Time: %s"
             "Last Modification Time: %s"
             "Last Status Change Time: %s",
             (long long)file_stat.st_size,
             (file_stat.st_mode & S_IRUSR) ? 'r' : '-',
             (file_stat.st_mode & S_IWUSR) ? 'w' : '-',
             (file_stat.st_mode & S_IXUSR) ? 'x' : '-',
             (file_stat.st_mode & S_IRGRP) ? 'r' : '-',
             (file_stat.st_mode & S_IWGRP) ? 'w' : '-',
             (file_stat.st_mode & S_IXGRP) ? 'x' : '-',
             (file_stat.st_mode & S_IROTH) ? 'r' : '-',
             (file_stat.st_mode & S_IWOTH) ? 'w' : '-',
             (file_stat.st_mode & S_IXOTH) ? 'x' : '-',
             ctime(&file_stat.st_atime),
             ctime(&file_stat.st_mtime),
             ctime(&file_stat.st_ctime));

    send(client_socket, metadata, sizeof(metadata), 0);
    
    sem_wait(&read_mutex);
    read_count--;
    if(read_count == 0){
        sem_post(&write_mutex);
    }
    sem_post(&read_mutex);
}

void *client_handler(void *arg){
    int client_socket = *(int*)arg;
    char buffer[1024];
    int choice;
    printf("HI\n");
    while (1)
    {
        printf("Waiting for command from client\n");
        int received = recv(client_socket, &choice, sizeof(choice), 0);
        if(received < 0)
        {
            break;
        }
        printf("%d",choice);
        fflush(stdout);
        if(choice == 1){
            filereader(client_socket);
        }
        if(choice == 4)
        {
            file_renamer(client_socket);
        }
        if(choice == 6)
        {
            metadata_display(client_socket);
        }
        if(choice == 10)
        {
            break;
        }
    }
    
    close(client_socket);
    return NULL;
}


int main(){
    int server_socket;
    struct sockaddr_in server_address;
    socklen_t server_address_size = sizeof(server_address);
    int client_socket[MAX_CLIENTS];
    int client_count = 0;
    pthread_t thread_id[MAX_CLIENTS];
    srand(time(0));

    sem_init(&read_mutex, 0, 1);
    sem_init(&write_mutex, 0, 1);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket <= 0){
        perror("Socket creation failed");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_socket, (struct sockaddr*)&server_address, server_address_size) < 0){
        perror("Binding failed");
        exit(1);
    }

    if(listen(server_socket, MAX_CLIENTS) < 0){
        perror("Listening failed");
        exit(1);
    }

    printf("Waiting for connections....\n");

    while(1){
        client_socket[client_count] = accept(server_socket, (struct sockaddr*)&server_address, &server_address_size);
        if(client_socket[client_count] <0){
            perror("Accept failed\n");
            exit(1);
        }
        printf("Connection established with client %d\n", client_count+1);
        pthread_create(&thread_id[client_count], NULL, client_handler, (void*)&client_socket[client_count]);
        printf("HELLO\n");
        client_count++;
        printf("%d\n", client_count);

        if(client_count == MAX_CLIENTS){
            for(int j=0; j<MAX_CLIENTS; j++){
                pthread_join(thread_id[j], NULL);
            }
            client_count = 0;
        }
    }

    for(int i=0;i<client_count;i++)
    {
        close(client_socket[i]);
    }

    close(server_socket);

    return 0;
}
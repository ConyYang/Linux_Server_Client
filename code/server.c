#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>

int newsockfd;

void error(char *m) {
    perror(m);
}

void *thread_handler() {
    /**
     * Process data and terminate
     */
    int n;
    char buffer[256];

    n = read(newsockfd, buffer, 255);
    if (n < 0) {
        error("ERROR reading from socket");
    }
    printf("Message received: %s\n", buffer);

    int newnum = atoi(buffer);
    newnum *= 5;
    sprintf(buffer, "%d", newnum);

    n = write(newsockfd, buffer, 255);
    if (n < 0) {
        error("ERROR writing back to socket");
    }
    return NULL;
}

void *waiting() {
    /**
     * Waiting for connection
     */
    sleep(10);
    return NULL;
}

int main(int argc, char *argv[]) {
    int sockfd, port, clilen;
    pthread_t pthread[3];
    int i = 0;
    void *new_sock = NULL;
    struct sockaddr_in serv_addr, cli_addr;

    // parse argument
    if (argc < 2)
        error("ERROR, no port provided\n");
    port = atoi(argv[1]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR opening socket");

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port); //host to network

    // (a) ties a socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR binding to socket");

    // (b) inform the OS returns immediately after listen
    // listen(sockfd, 2);
    clilen = sizeof(cli_addr);

    // block until an incoming connection has been
    // detected on the server's socket.
    listen(sockfd, 3); // 3 = backlog. number of incoming connections
    while ((newsockfd = accept(sockfd, (struct sockaddr *)
            &cli_addr, &clilen))) {
        if (newsockfd < 0)
            perror("Accpet Failed");
        else
            pthread_create(&pthread[i], NULL, thread_handler, new_sock);
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (1) {
        if (listen(sockfd, 3) == 0) { // 0 mean no error detected
            pthread_create(&pthread[i], NULL, waiting, i);
        }
    }
#pragma clang diagnostic pop
}

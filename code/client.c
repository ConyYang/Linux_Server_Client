#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <zconf.h>
#include <sys/wait.h>
#include <pthread.h>

void error(char *m) {
    perror(m);
    exit(0);
}

int child_handler(int port, const struct sockaddr *p_serv_addr);

static pthread_mutex_t io_mux;

int main(int argc, char *argv[]) {
    struct sockaddr_in serv_addr;
    struct hostent *server;

    pid_t pid[3];
    pthread_mutex_init(&io_mux, NULL);

    if (argc < 3) error("usage client [hostname] [port]\n");
    int port = atoi(argv[2]);
    server = gethostbyname(argv[1]);
    if (server == NULL) error("ERROR, no such host\n");

    // start with a clean address structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // internet socket
    bcopy((char *) server->h_addr,
          (char *) &serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(port);

    for (int i = 0; i < 3; i++) {
        if ((pid[i] = fork()) < 0) { // error
            printf("\n error in fork");
            abort();
        } else if (pid[i] == 0) {
            return child_handler(port, (const struct sockaddr *) &serv_addr);
        }
    }

    wait(NULL);
    return 0;
}

int child_handler(int port, const struct sockaddr *p_serv_addr) {
    int sockfd, n;

    char buffer[256];

    // this is (one of) the child process
    printf("\n Hello from child:PID = %d\n", getpid());

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    if (connect(sockfd, p_serv_addr, sizeof(*p_serv_addr)) < 0)
        error("ERROR connecting");

    // enter critical
    pthread_mutex_lock(&io_mux);
    printf("Please enter the message: ");
    fgets(buffer, 255, stdin);
    // exit critical
    pthread_mutex_unlock(&io_mux);

    n = write(sockfd, buffer, strlen(buffer));

    if (n < 0)
        error("ERROR writing to socket");

    n = read(sockfd, buffer, 255);
    if (n < 0)
        error("ERROR reading from socket");
    printf("%s\n", buffer);

    return 0; // child terminated itself here
}

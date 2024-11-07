#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"      // Port clients will connect to
#define BACKLOG 10       // Number of pending connections queue will hold

// Signal handler for SIGCHLD to reap zombie processes created by child processes
void sigchld_handler(int s) {
    int saved_errno = errno;           // Save current errno value to restore later
    while (waitpid(-1, NULL, WNOHANG) > 0); // Clean up terminated child processes
    errno = saved_errno;               // Restore errno in case waitpid changed it
}

// Get the IPv4 or IPv6 address from the socket address structure
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {     // IPv4 address
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr); // IPv6 address
}

int main() {
    int sockfd, new_fd;                 // Listening socket (sockfd) and new connection socket (new_fd)
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // Connector's address information
    socklen_t sin_size;
    struct sigaction sa;                // For reaping zombie processes
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char *msg = "Hello There!\n";       // Message to be sent to clients upon connection

    // Initialize hints structure to zero for getaddrinfo configuration
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // Use TCP
    hints.ai_flags = AI_PASSIVE;        // Automatically use server's IP

    // Get server address information, storing result in servinfo
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Loop through all results and bind to the first available
    for (p = servinfo; p != NULL; p = p->ai_next) {
        // Create socket using the specified address, protocol family, and type
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        // Set socket option to allow reuse of local addresses
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        // Bind socket to port
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break; // Successful bind
    }

    // Free servinfo as it is no longer needed
    freeaddrinfo(servinfo);

    // If no address was bound successfully, exit with an error
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    // Listen on the socket with a specified backlog of pending connections
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // Set up signal handling to prevent zombie processes by reaping terminated child processes
    sa.sa_handler = sigchld_handler;    // Define handler function
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    // Main loop to accept and handle incoming connections
    while (1) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        // Convert and print the incoming IP address
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
        printf("server: got connection from %s\n", s);

        // Fork a new process to handle the connection
        if (!fork()) {                  // Child process
            close(sockfd);               // Child doesn't need the listener
            if (send(new_fd, msg, strlen(msg), 0) == -1) {
                perror("send");
            }
            close(new_fd);               // Close connection after sending message
            exit(0);                     // Terminate child process
        }
        close(new_fd);                   // Parent doesn't need this connection socket
    }

    return 0;
}

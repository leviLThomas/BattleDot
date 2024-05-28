#include "server.h"
#include "battledot.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <sys/wait.h>

struct thread_info {
  pthread_t thread_id;
  int sockfd;
};

void sigchld_handler(int s) {
  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void *handle_client(void *arg) {
  struct thread_info *tinfo = (struct thread_info *)arg;
  int client_fd = tinfo->sockfd;

  struct DataPacket packet;
  char buf[MAXDATASIZE];
  memset(&packet, 0, sizeof(packet));

  int bytes_read = read(client_fd, &packet, sizeof(packet));
  if (bytes_read > 0) {
    packet.x = ntohl(packet.x);
    packet.y = ntohl(packet.y);

    fprintf(stdout, "Received from client:\n");
    fprintf(stdout, "Name: %s\n", packet.name);
    fprintf(stdout, "X: %u\n", packet.x);
    fprintf(stdout, "y: %u\n", packet.y);

    switch (packet.flags) {
    case JOIN:
      snprintf(buf, MAXDATASIZE - 1,
               "Join flag received. Please wait for the game to start...");
      break;
    case START:
      snprintf(buf, MAXDATASIZE - 1,
               "Start flag received. The game is now starting...");
      break;
    default:
      snprintf(buf, MAXDATASIZE - 1, "Invalid flag received.");
      break;
    }
  }

  close(client_fd);
  tinfo->sockfd = -1;

  pthread_exit(NULL);
}

int main() {
  int sockfd, new_fd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  struct sigaction sa;
  int yes = 1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  struct DataPacket packet;
  memset(&packet, 0, sizeof(packet));

  char buf[MAXDATASIZE];
  memset(buf, 0, MAXDATASIZE - 1);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  rv = getaddrinfo(NULL, PORT, &hints, &servinfo);
  if (rv != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) {
      perror("server: socket");
      continue;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(EXIT_FAILURE);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(EXIT_FAILURE);
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  printf("server: waiting for connections...\n");

  size_t num_threads;
  size_t tnum = 0;
  ssize_t stack_size;
  pthread_attr_t attr;
  struct thread_info *tinfo;
  void *res;
  int e;
  e = pthread_attr_init(&attr);
  if (e != 0) {
    perror("pthread_attr_init");
    exit(EXIT_FAILURE);
  }

  if (stack_size > 0) {
    e = pthread_attr_setstacksize(&attr, stack_size);
    if (e != 0) {
      perror("pthread_attr_setstacksize");
      exit(EXIT_FAILURE);
    }
  }

  tinfo = calloc(num_threads, sizeof(*tinfo));
  if (tinfo == NULL) {
    perror("calloc");
    exit(EXIT_FAILURE);
  }

  printf("server: thread creation attributes initialized");

  for (;;) {
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
              s, sizeof s);
    printf("server: got connection from %s\n", s);

    int *client_fd = malloc(sizeof(int));
    if (client_fd == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    *client_fd = new_fd;

    if (tnum >= num_threads) {
      // Wait for a thread to finish
      for (size_t i = 0; i < num_threads; ++i) {
        if (tinfo[i].sockfd != -1) {
          int join_err = pthread_join(tinfo[i].thread_id, NULL);
          if (join_err == 0) {
            printf("server: thread %zu finished and slot is reused\n", i);
            tinfo[i].sockfd = -1; // Mark slot as available
            tnum--;
            break;
          } else {
            perror("pthread_join");
          }
        }
      }

      if (tnum >= num_threads) {
        fprintf(stderr, "server: still maximum number of threads reached\n");
        close(new_fd);
        continue;
      }
    }

    tinfo[tnum].sockfd = new_fd;

    int err = pthread_create(&tinfo[tnum].thread_id, &attr, &handle_client,
                             &tinfo[tnum]);
    if (err != 0) {
      perror("pthread_create");
      close(new_fd);
      exit(EXIT_FAILURE);
    }

    // Detach the thread if you do not want to join later
    err = pthread_detach(tinfo[tnum].thread_id);
    if (err != 0) {
      perror("pthread_detach");
      exit(EXIT_FAILURE);
    }

    tnum++;
  }

  pthread_attr_destroy(&attr);
  free(tinfo);
  exit(EXIT_SUCCESS);
}

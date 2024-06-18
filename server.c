#include "server.h"
#include "battledot.h"
#include "circular_linked_list.h"
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

typedef struct {
  pthread_t thread_id;
  pthread_mutex_t mutexsockfd;
  int sockfd;
  PlayerStatus *pstatus;
} thread_info;

thread_info clientThd[MAX_THREADS];
pthread_cond_t lobby_full;
pthread_cond_t game_start;

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
  thread_info *tinfo = (thread_info *)arg;
  int client_fd = tinfo->sockfd;
  PlayerStatus *pstatus = tinfo->pstatus;
  int numbytes = 0;

  struct DataPacket packet;
  memset(&packet, 0, sizeof(packet));

  for (numbytes = read(client_fd, &packet, sizeof(packet)); numbytes > 0;) {
    packet.x = ntohl(packet.x);
    packet.y = ntohl(packet.y);

    pthread_mutex_lock(&pstatus->status_mutex);
    pstatus_update(pstatus, packet.x, packet.y, packet.flags);
    if (pstatus->flags & EXIT) {
      pthread_mutex_unlock(&pstatus->status_mutex);
      break;
    }
    pthread_mutex_unlock(&pstatus->status_mutex);
  }

  snprintf(packet.buffer, MAXBUFSIZE - 1, "Thanks for playing!");
  packet.flags |= EXIT;
  numbytes = send(client_fd, &packet, sizeof(packet), 0);
  if (numbytes == -1) {
    perror("send");
    exit(EXIT_FAILURE);
  }
  close(client_fd);
  tinfo->sockfd = -1;

  pthread_exit(NULL);
}

int main(void) {
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

  size_t num_threads = 4;
  size_t tnum = 0;
  ssize_t stack_size = 65536;
  pthread_attr_t attr;
  thread_info *tinfo;
  void *res;
  int e;

  pthread_cond_init(&lobby_full, NULL);
  pthread_cond_init(&game_start, NULL);

  e = pthread_attr_init(&attr);
  if (e != 0) {
    perror("pthread_attr_init");
    exit(EXIT_FAILURE);
  }

  if (stack_size > 0) {
    e = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (e != 0) {
      perror("pthread_attr_setdetachstate");
      exit(EXIT_FAILURE);
    }
  }

  tinfo = calloc(num_threads, sizeof(*tinfo));
  if (tinfo == NULL) {
    perror("calloc");
    exit(EXIT_FAILURE);
  }

  printf("server: thread creation attributes initialized\n");

  CircularLinkedList cll;

  BattleDotConfig bdot_config;
  bdot_config_new(&bdot_config, 0, 4, stdout);

  BattleDotInstance bdot_instance;
  bdot_instance_new(&bdot_instance, bdot_config);

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

    struct DataPacket packet;
    memset(&packet, 0, sizeof(packet));

    int numbytes = read(new_fd, &packet, sizeof(packet));
    if (numbytes > 0) {
      packet.x = ntohl(packet.x);
      packet.y = ntohl(packet.y);

      fprintf(stdout, "%lu Received from client:\n", tinfo->thread_id);
      fprintf(stdout, "Name: %s\n", packet.buffer);
      fprintf(stdout, "X: %u\n", packet.x);
      fprintf(stdout, "y: %u\n", packet.y);

      PlayerInfo pinfo;
      pinfo_new(&pinfo, packet.x, packet.y, packet.buffer);

      PlayerStatus *pstatus = malloc(sizeof(*pstatus));
      if (pstatus == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
      }
      pstatus_new(pstatus);

      PlayerInstance *pinstance = malloc(sizeof(*pinstance));
      if (pinstance == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
      }
      pinstance_new(pinstance, pinfo, pstatus);

      tinfo[tnum].sockfd = new_fd;

      int err = pthread_create(&tinfo[tnum].thread_id, &attr, &handle_client,
                               &tinfo[tnum]);
      if (err != 0) {
        perror("pthread_create");
        close(new_fd);
        exit(EXIT_FAILURE);
      }
      tnum++;

      if (tnum >= num_threads - 1) {
        fprintf(stdout, "Max players reached!\n");
        pthread_cond_broadcast(&lobby_full);
        break;
      }

      e = bdot_instance_add_player(&bdot_instance, pinstance);
      if (e == 1) {
        fprintf(stdout, "Max Players Reached!\n");
        bdot_instance_run(&bdot_instance);
        break;
      }
    }
  }
  pthread_attr_destroy(&attr);
  free(tinfo);
  bdot_instance_destroy(&bdot_instance);
  printf("Main: program completed. Exiting.\n");
  exit(EXIT_SUCCESS);
}

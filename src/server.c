#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define on_error(...)             \
  {                               \
    fprintf(stderr, __VA_ARGS__); \
    fflush(stderr);               \
    exit(1);                      \
  }
#define PROTOCOL_IP 0

void *handle_client(int *client_fd)
{
  char buf[BUFFER_SIZE];
  while (1)
  {
    size_t msg_length = recv(*client_fd, buf, BUFFER_SIZE, 0);
    if (!msg_length)
    {
      break;
    }
    if (msg_length < 0)
    {
      on_error("Client read failed\n");
    }

    if (send(*client_fd, buf, msg_length, 0) < 0)
    {
      on_error("Client write failed\n");
    }
  }

  return 0;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    on_error("Usage: %s [port]\n", argv[0]);
  }

  int port = atoi(argv[1]);
  if (port <= 0)
  {
    on_error("Bad port: %s\n", argv[1]);
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, PROTOCOL_IP);
  if (server_fd < 0)
  {
    on_error("Could not create socket\n");
  }

  struct sockaddr_in server;
  server.sin_port = htons(port);
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

  if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    on_error("Could not bind socket\n");
  }

  if (listen(server_fd, 128) < 0)
  {
    on_error("Could not listen on socket\n");
  }

  printf("Server listening on :%d\n", port);

  while (1)
  {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);

    int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);
    if (client_fd < 0)
    {
      on_error("Could not establish new connection\n");
    }

    pthread_t handle_client_thread;
    if (pthread_create(&handle_client_thread, NULL, (void *)handle_client, &client_fd))
    {
      on_error("Error creating thread\n");
    }
  }

  return 0;
}

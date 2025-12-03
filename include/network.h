#ifndef NETWORK_H
#define NETWORK_H

typedef struct {
    char *name;
    char *address;
    int port;
    char *username;
    char *password;
    char *conn_type;
} remote_machine;

remote_machine machine_test = {
    .name      = "Machine de Test",
    .address   = "192.168.1.42",
    .port      = 2222,
    .username  = "admin",
    .password  = "password123",
    .conn_type = "ssh"
};

#endif /* NETWORK_H */
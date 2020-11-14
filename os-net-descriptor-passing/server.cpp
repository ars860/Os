#include <cstddef>
#include <iostream>
#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>
#include "utils.h"

const size_t BUFFER_SIZE = 1000;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "usage: ./server socket_name";
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_un addr{};
    socklen_t addrlen = sizeof(addr);

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, argv[1]);
    unlink(argv[1]);

    int listener = socket(AF_UNIX, SOCK_STREAM, 0);

    if (listener == -1) {
        error("Can't create socket");
    }

    if (bind(listener, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        error("Can't bind");
    }

    if (listen(listener, 100) == -1) {
        error("Can't listen");
    }

    bool working = true;
    while (working) {
        int client = accept(listener, (struct sockaddr *) &addr, &addrlen);

        if (client == -1) {
            error("Can't accept");
        }

        int pipefd[2];

        if (pipe(pipefd) == -1) {
            error("Can't pipe");
        }

        struct msghdr msg{};
        struct cmsghdr *cmsg;

        char iobuf[1];
        struct iovec io{};
        io.iov_base = iobuf;
        io.iov_len = sizeof(iobuf);

        msg.msg_iov = &io;
        msg.msg_iovlen = 1;

        char msg_buffer[CMSG_SPACE(sizeof(pipefd[1]))];
        msg.msg_control = msg_buffer;
        msg.msg_controllen = sizeof(msg_buffer);
        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(pipefd[1]));
        memcpy(CMSG_DATA(cmsg), &pipefd[1], sizeof(pipefd[1]));

        if (sendmsg(client, &msg, 0) == -1) {
            perror("Can't sendmsg");
        }

        close(pipefd[1]);

        char buffer[BUFFER_SIZE];
        int len = receive_message(pipefd[0], buffer, BUFFER_SIZE);

        if (len <= 0) {
            perror("Can't receive message");
        } else {
            std::string message(buffer, buffer + len);
            if (message == "stop") {
                working = false;
            }

            std::cout << buffer << '\n';
        }

        close(pipefd[0]);
        close(client);
    }

    close(listener);
}
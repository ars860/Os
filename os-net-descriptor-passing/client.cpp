#include <iostream>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include "utils.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "usage ./client socket_name message";
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_un addr{};

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, argv[1]);

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        error("Can't socket");
    }

    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        error("Can't connect");
    }

    struct msghdr msg{};
    char iobuf[1];
    struct iovec io;
    io.iov_base = iobuf;
    io.iov_len = sizeof(iobuf);

    char msg_buf[CMSG_SPACE(sizeof(int))];
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = msg_buf;
    msg.msg_controllen = sizeof(msg_buf);

    if (recvmsg(sock, &msg, 0) == -1) {
        error("Can't recvmsg");
    }

    auto cmsg = CMSG_FIRSTHDR(&msg);
    int fd;
    memcpy(&fd, CMSG_DATA(cmsg), sizeof(fd));

    send_message(argv[2], fd);

    close(fd);
    close(sock);
}
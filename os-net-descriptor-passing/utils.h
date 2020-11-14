#ifndef OS_MULTIPLEXING_UTILS_H
#define OS_MULTIPLEXING_UTILS_H

void error(std::string const& message){
    perror(message.data());
    exit(EXIT_FAILURE);
}

void send_message(std::string message, int fd) {
    message += 3;
    size_t sended = 0;
    while (sended < message.size()) {
        ssize_t curr_portion = write(fd, message.data() + sended, message.length() - sended);
        if (curr_portion == -1) {
            perror("Can't send message");
            continue;
        }
        sended += curr_portion;
    }
}

size_t receive_message(int fd, char *buffer, size_t buffer_size) {
    size_t received = 0;
    char last_received = 'a';

    while (last_received != 3) {
        ssize_t len = read(fd, buffer, buffer_size);
        if (len == -1) {
            perror("Can't read response./");
            continue;
        }
        if (len == 0) {
            break;
        }
        received += len;
        last_received = buffer[len - 1];
    }

    received--;
    buffer[received] = 0;
    
    return received;
}

#endif //OS_MULTIPLEXING_UTILS_H

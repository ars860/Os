#include <iostream>
#include <dirent.h>
#include <functional>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <wait.h>

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string path;
std::string name;
int inum = -1;
int sizeEq = -1;
int sizeG = -1;
int sizeL = -1;
int nlinks = -1;
std::string exec = "";

std::vector<std::string> filePaths;

void parseArgs(int argc, char *argv[]) {
    path = argv[1];
    for (size_t i = 2; i < argc - 1; i++) {
        if (std::string(argv[i]) == "-inum") {
            i++;
            inum = std::stoi(argv[i]);
        }
        if (std::string(argv[i]) == "-name") {
            i++;
            name = argv[i];
        }
        if (std::string(argv[i]) == "-size") {
            i++;
            switch (argv[i][0]) {
                case '=':
                    sizeEq = std::stoi(argv[i] + 1);
                    break;
                case '+':
                    sizeG = std::stoi(argv[i] + 1);
                    break;
                case '-':
                    sizeL = std::stoi(argv[i] + 1);
                    break;
                default:
                    break;
            }
        }
        if (std::string(argv[i]) == "-nlinks") {
            i++;
            nlinks = std::stoi(argv[i]);
        }
        if (std::string(argv[i]) == "-exec") {
            i++;
            exec = argv[i];
        }
    }
}

void add(dirent *file, const std::string &filePath) {
    if (name != "" && name != split(std::string(file->d_name), '.')[0]) {
        return;
    }
    if (inum != -1 && inum != file->d_ino) {
        return;
    }
    struct stat stat_buf;
    int rc = stat(filePath.c_str(), &stat_buf);
    int fileSize = stat_buf.st_size;
    if (sizeEq != -1 && fileSize != sizeEq) {
        return;
    }
    if (sizeG != -1 && fileSize < sizeG) {
        return;
    }
    if (sizeL != -1 && fileSize > sizeL) {
        return;
    }
    int hardLinksAmount = stat_buf.st_nlink;
    if (nlinks != -1 && nlinks != hardLinksAmount) {
        return;
    }
    filePaths.push_back(filePath);
}

char *strToChar(std::string &s) {
    return const_cast<char *>(s.data());
}

void walk(std::string currPath) {
    DIR *dir = opendir(strToChar(currPath));

    if (dir == nullptr) {
        switch errno {
            case EACCES:
                std::cerr << currPath + " : Permission denied!\n";
                break;
            case ENFILE:
                std::cerr << "To many opened files!\n";
                break;
            case EMFILE:
                std::cerr << "To many opened files!\n";
                break;
            default:
                std::cerr << "Something bad happened! errno :" + std::to_string(errno);
                break;
        }
        return;
    }

    dirent *curr;
    std::vector<std::string> toWalk;
    while ((curr = readdir(dir)) != nullptr) {
        std::string destination = std::string(curr->d_name);
        switch (curr->d_type) {
            case DT_DIR:
                if (destination != "." && destination != "..") {
                    toWalk.push_back(currPath + "/" + destination);
                }
                break;
            case DT_REG:
                add(curr, currPath + "/" + destination);
                break;
        }
    }
    closedir(dir);

    for (auto &nextDir : toWalk) {
        walk(nextDir);
    }
}

void execute(char *argv[]) {
    switch (pid_t pid = fork()) {
        case -1:
            //error
            std::cout << "cant't fork\n";
            break;
        case 0: {
            //child
            if (execve(argv[0], argv, nullptr) == -1) {
                std::cout << errno;
                std::cout << "can't execute";
                exit(-1);
            }
            break;
        }
        default:
            //parent
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                std::cout << "can't execute\n";
            }
            break;
    }
}

bool is_executable(std::string filePath) {
    struct stat stat_buff;
    return stat(strToChar(filePath), &stat_buff) == 0 && stat_buff.st_mode & S_IXUSR;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        return 0;
    }

    parseArgs(argc, argv);

    if (exec != "" && !is_executable(exec)) {
        std::cout << exec + " is not an executable file!" << '\n';
        return 0;
    }


    DIR *dir = opendir(strToChar(path));
    if (dir == nullptr) {
        std::cout << "incorrect path!\n";
        return 0;
    } else {
        closedir(dir);
    }
    walk(path);

    if (exec != "") {
        char *args[3];
        args[0] = strToChar(exec);
        args[2] = nullptr;
        for (auto &filePath : filePaths) {
            args[1] = strToChar(filePath);
            execute(args);
            std::cout << '\n';
        }
    } else {
        for (auto s : filePaths) {
            std::cout << s << '\n';
        }
    }
}


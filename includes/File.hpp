#ifndef FILE_HPP
# define FILE_HPP

# include <unistd.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <string.h>
# include <sys/socket.h>
# include "Logger.hpp"

class File {
private:
    static size_t fds;
public:
	static int open(char const *file, int oflag);
	static int open(char const *file, int oflag, mode_t mode);
	static int close(int fd);
    static int socket(int domain, int type, int protocol);
    static int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    static int pipe(int pipefd[2]);
    static int dup2(int oldfd, int newfd);

    static int const devNull;
    static size_t usedFds(void);
};

#endif /* FILE_HPP */

#include "File.hpp"

size_t File::fds = 0;
int const File::devNull = File::open("/dev/null", O_WRONLY);

int File::open(char const *filename, int flags) {
	int ret = ::open(filename, flags);
	if (ret > 0)
		++File::fds;
	return (ret);
}

int File::open(char const *filename, int flags, mode_t mode) {
	int ret = ::open(filename, flags, mode);
	if (ret > 0)
		++File::fds;
	return (ret);
}

int File::close(int fd) {
	int ret = ::close(fd);
	if (ret == 0)
		--File::fds;
	return (ret);
}

int File::socket(int domain, int type, int protocol) {
    int ret = ::socket(domain, type, protocol);
    if (ret > 0)
        ++File::fds;
    return (ret);
}

int File::accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int ret = ::accept(sockfd, addr, addrlen);
	if (ret == -1 && errno && errno != 2) {
		Logger::error("accept errno ", errno, ": ", strerror(errno));
	}
    if (ret > 0)
        ++File::fds;
    return (ret);
}

int File::pipe(int pipefd[2]) {
    int ret = ::pipe(pipefd);
    if (ret == 0)
        File::fds += 2;
    return (ret);
}

int File::dup2(int oldfd, int newfd) {
    int ret = ::dup2(oldfd, newfd);
    if (ret > 0 && ret != newfd)
        ++File::fds;
    return (ret);
}

size_t File::usedFds(void) {
    return (File::fds);
}

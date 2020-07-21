#include "StreamJob.hpp"

StreamJob::StreamJob(int fd, Client& client, Response& resp, int maxBodySize, Server *server, responseCallback callback, std::string fileTmp):
	fd(fd), client(client),
	response(resp), callback(callback), expired(false),
	maxBodySize(maxBodySize), totalSize(0),
	server(server), fileTmp(fileTmp) {
	list = client.getList();
	Logger::verbose("New stream: ", fd);
}

bool StreamJob::isEmpty(void) const {
	return (list->empty());
}

StreamJob::operator int() {
	return (this->fd);
}

bool StreamJob::isCompleted(void) const {
	return (list->done && list->currentSize == 0);
}

bool StreamJob::stream() {
	BodyPair chunk = list->front();
	list->pop_front();
	if (!list->done && list->currentSize < 8388608)
		client.skip = false;
	Logger::verbose("Stream ", fd, " writing ", chunk.size, " and remaining in list ", list->currentSize - chunk.size);
	int ret = ::write(fd, chunk.data, chunk.size);
	if (ret == -1)
		list->push_front(chunk);
	else if (ret != -1 && ret != (ssize_t)chunk.size) {
		int remainingSize = chunk.size - ret;
		BodyPair remainingChunk = BodyPair();
		remainingChunk.data = ft_strndup(chunk.data + ret, remainingSize);
		remainingChunk.size = remainingSize;
		list->push_front(remainingChunk);
		list->currentSize -= ret;
	} else {
		list->currentSize -= chunk.size;
	}
	if (ret != -1) {
		totalSize += ret;
		delete chunk.data;
	}
	return (this->isCompleted());
}

StreamJob::~StreamJob() {
	Logger::verbose("Stream ", fd, " is over");
	if (!expired) {
		client.deleteBody();
		Request* request = client.returnRequest();
		response.setDontSend(false);
		if (callback) {
			(server->*callback)(*request, response, fd);
			unlink(fileTmp.c_str());
		}
		delete request;
		client.setJob(nullptr);
		client.skip = false;
	}
	if (fd != -1 && fd != File::devNull)
		File::close(fd);
	if (totalSize > maxBodySize) {
		response.setStatus(RFC::RequestEntityTooLarge);
		response.defaultError(RFC::RequestEntityTooLarge);
	}
}

void StreamJob::expire() {
	if (callback)
		unlink(fileTmp.c_str());
	expired = true;
}

bool StreamJob::isExpired() {
	return (this->expired);
}

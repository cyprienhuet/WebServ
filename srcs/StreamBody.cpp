#include "StreamBody.hpp"
#include <cstring>

StreamBody::StreamBody(Client& client, std::list<StreamJob*>& jobList):
	client(client), jobList(jobList) {}

StreamBody::~StreamBody() {}

void StreamBody::pushJob(int fd, Response& response, int maxBodySize, Server *server, responseCallback callback, std::string fileTmp) {
	StreamJob* job = new StreamJob(fd, client, response, maxBodySize, server, callback, fileTmp);
	jobList.push_back(job);
	client.setJob(job);
}

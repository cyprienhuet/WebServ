#include "CGIResponse.hpp"

CGIResponse::CGIResponse(Response const &other, std::vector<std::pair<int, bool> > &outputs):
    Response(other),
    outputs(outputs),
    method(RFC::INVALID),
    strBuffer(),
    readHeaders(false), contentLength(0) {
    this->dontSend = other.getDontSend();
}

CGIResponse::~CGIResponse() {
    // Remove bodyFd from cgiOutputs if client disconnected before the end
    int fd = this->getBodyFd();
    for (auto it = this->outputs.begin(), ite = this->outputs.end(); it != ite; ++it) {
        if ((*it).first == fd) {
            this->outputs.erase(it);
            break ;
        }
    }
}

void CGIResponse::forMethod(RFC::Method method) {
    this->method = method;
}

int CGIResponse::getBodyFd(void) const {
    if (!this->body)
        return (-1);
    return (static_cast<FileBody*>(this->body)->getFd());
}

bool CGIResponse::isBodyReady(void) const {
    int fd = this->getBodyFd();
    auto it = this->outputs.begin();
    auto ite = this->outputs.end();
    for ( ; it != ite; ++it)
        if ((*it).first == fd)
            break ;
    return (it != ite && (*it).second);
}

void CGIResponse::erase(std::vector<std::pair<int, bool> >::iterator it) {
    this->outputs.erase(it);
}

int CGIResponse::proxyResponseSend(Client &client, std::vector<std::pair<int, bool> >::iterator it) {
    if (this->Response::send(client)) {
        this->erase(it);
        return (true);
    }
    return (false);
}

int CGIResponse::send(Client& client) {
    static char buffer[R_BUFFER_SIZE];

    // Check if CGI is ready to be read
    int fd = this->getBodyFd();
    auto it = this->outputs.begin();
    auto ite = this->outputs.end();
    for ( ; it != ite; ++it)
        if ((*it).first == fd)
            break ;
    if (it == ite || !(*it).second)
        return (false);
    (*it).second = false;
    client.setReady(false);

    // If we sent nothing, we need to check if we have the headers
    if (this->progress == NOTHING_SENT) {
        size_t ret = this->body->getBody(buffer, R_BUFFER_SIZE);
        // Check if the process is dead and empty
        if (ret == 0) {
            // If there is an empty response or an HEAD request
            this->addHeader("Content-length", to_string(this->contentLength));
            delete this->body;
            this->body = nullptr;
            return (this->proxyResponseSend(client, it));
        }
        if (this->readHeaders) {
            this->contentLength += ret;
            return (false);
        }
        this->strBuffer.append(buffer, ret);
        // * If we find CLRF for the headers, set the CGIBody to the remaining buffer
        // * and the next calls will just forward to Response::send
        size_t clrf = this->strBuffer.find("\r\n\r\n");
        if (clrf != std::string::npos || (clrf = this->strBuffer.find("\r\n")) == 0) {
            std::vector<std::string> headers = split(this->strBuffer.substr(0, clrf), "\r\n");
            for (auto &h: headers) {
                size_t sepPos = h.find(':');
                if (sepPos != std::string::npos) {
                    this->addHeader(h.substr(0, sepPos), h.substr(sepPos + 1));
                }
            }
            if (this->getHeader("X-Error-Internal").second == "500") {
                this->erase(it);
                return (2);
            }
            if (this->getHeader("Status").second.length() > 0)
                this->setStatus((RFC::Status)std::stoi(this->getHeader("Status").second));
            this->readHeaders = true;
            if (this->method == RFC::HEAD) {
                this->contentLength += this->strBuffer.length();
                this->strBuffer.clear();
                return (false);
            }
            this->strBuffer.erase(0, clrf + 4);
            this->addHeader("Transfer-Encoding", "chunked");
            static_cast<CGIBody*>(this->body)->setBuffer(this->strBuffer);
            return (this->proxyResponseSend(client, it));
        }
        return (false);
    }
    return (this->proxyResponseSend(client, it));
}

int CGIResponse::getType(void) const {
	return (1);
}

bool CGIResponse::hasReadHeaders(void) const {
    return (this->readHeaders);
}

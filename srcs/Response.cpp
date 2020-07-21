#include "Response.hpp"

Response::Response():
	uri("/"), version("1.1"), status(RFC::InvalidStatus),
	body(nullptr), compression(RFC::NONE), headers(),
	stream(), strmOpen(false), progress(NOTHING_SENT), dontSend(false),
	unsendBuffer(), lastSendSize(0), nextInitialProgress(NOTHING_SENT) {}

Response::Response(Response const &other):
	uri(other.uri), version(other.version), status(other.status),
	body(other.body), compression(other.compression), headers(other.headers),
	stream(other.stream), strmOpen(other.strmOpen), progress(other.progress), dontSend(other.dontSend),
	unsendBuffer(other.unsendBuffer), lastSendSize(other.lastSendSize), nextInitialProgress(other.nextInitialProgress) {}

Response::~Response() {
	delete body;
	if (this->strmOpen)
		deflateEnd(&this->stream);
}

std::string const &Response::getURI(void) const {
	return (this->uri);
}

std::string const &Response::getVersion(void) const {
	return (this->version);
}

RFC::Status	Response::getStatus(void) const {
	return (this->status);
}

bool Response::getDontSend(void) const {
	return this->dontSend;
}

Body *Response::getBody(void) const {
	return (this->body);
}
bool Response::isBodyReady(void) const {
	return (true);
}

RFC::Encoding Response::getCompression(void) const
{
	return (this->compression);
}

std::pair<std::string, std::string> Response::getHeader(std::string const &name) const {
	auto ite = this->headers.end();
	for (auto it = this->headers.begin(); it != ite; ++it)
		if (insensitive_equal((*it).first, name))
			return (*it);
	return (std::make_pair("", ""));
}

void Response::setStatus(RFC::Status status) {
	this->status = status;
}

void Response::setUri(std::string const &uri) {
	this->uri = uri;
}

void Response::setDontSend(bool b) {
	this->dontSend = b;
}

bool Response::addHeader(std::string const &name, std::string const &value) {
	auto ite = this->headers.end();
	for (auto it = this->headers.begin(); it != ite; ++it)
		if (insensitive_equal((*it).first, name)) {
			(*it).second = value;
			return (false);
		}
	this->headers.push_back(std::make_pair(name, value));
	return (true);
}

bool Response::removeHeader(std::string const &name) {
	auto it = this->headers.begin();
	auto ite = this->headers.end();
	for ( ; it != ite; ++it)
		if (insensitive_equal((*it).first, name)) {
			this->headers.erase(it);
			return (true);
		}
	return (false);
}

void Response::setCompression(RFC::Encoding encoding) {
	this->compression = encoding;
}

void Response::defaultError(RFC::Status status) {
	std::string strBody = "\
<!DOCTYPE html>\n\
<html lang=\"en\">\n\
	<head>\n\
		<meta charset=\"utf-8\" />\n\
		<title>" + to_string(status) + "</title>\n\
	<head>\n\
	<body>\n\
		<h1>Error " + to_string(status) + " - " + RFC::StatusReason[status] + "</h1>\n\
	</body>\n\
</html>";
	this->addHeader("Content-Language", "en");
	this->addHeader("Content-Type", "text/html; charset=utf-8");
	this->setBody(strBody);
}

void Response::setBody(std::string const &strBody) {
	char *tmp = (char*)operator new (strBody.length());
	ft_memcpy(tmp, strBody.c_str(), strBody.length());
	this->setBody(tmp, strBody.length());
}

void Response::setBody(char const *cbody, size_t size) {
	RawBody *body = new RawBody(cbody, size);
	this->addHeader("Content-Length", to_string(size));
	this->setBody(body);
}

void Response::setBody(Resource const &resource, int fd) {
	delete this->body;
	this->body = nullptr;

	if (fd < 1)
		return ;
	FileBody *body = new FileBody(fd, resource.size());
	this->addHeader("Content-Type", MimeTypes::getType(resource.relativePath()));
	this->addHeader("Content-Length", to_string(resource.size()));
	this->setBody(body);
}

void Response::setBody(Body *body) {
	delete this->body;
	this->body = body;
}

void Response::outputLine(std::string &out) const {
	out.append("HTTP/" + this->version + RFC::SP
		+ to_string(this->status) + RFC::SP
		+ RFC::StatusReason[this->status]
		+ RFC::CRLF);
}

void Response::outputHeaders(std::string &out) const {
	auto it = this->headers.begin();
	auto ite = this->headers.end();
	for ( ; it != ite; ++it)
		out.append((*it).first + ": " + (*it).second + RFC::CRLF);
	out.append(RFC::CRLF);
}

void Response::redirectSSL(int portSSL, const Request& request) {
	std::string location = "https://" + request.conf().server_name[0];
	if (portSSL != 443) {
		location += ":";
		location += to_string(portSSL);
	}
	location += request.getUri();
	this->addHeader("Location", location);
	this->addHeader("Content-Length", "0");
}

void Response::initCompression() {
	this->stream.zalloc = Z_NULL;
	this->stream.zfree = Z_NULL;
	this->stream.opaque = Z_NULL;
	this->strmOpen = true;
	if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, (this->compression == RFC::GZIP) ? 31 : 15, 8, Z_DEFAULT_STRATEGY) != Z_OK)
		throw std::exception();
}

int Response::sendUnsend(Client& client) {
	int totalSend = 0;
	Logger::verbose("Response: sending missing ", this->unsendBuffer.length(), " bytes");
	if ((size_t)(totalSend = client.send(this->unsendBuffer.c_str(), this->unsendBuffer.length())) != this->unsendBuffer.length()) {
		if (totalSend > 0)
			this->unsendBuffer.erase(0, totalSend);
		return (false);
	}
	this->unsendBuffer.clear();
	return (true);
}

bool Response::trySend(Client &client, char const *str, int size) {
	int totalSend = 0;
	if ((totalSend = client.send(str, size)) != size) {
		if (totalSend == -1)
			totalSend = 0;
		Logger::verbose("Response: tried to send ", size, " bytes to Client ", client.getIp(), " but sent ", totalSend, " bytes");
		this->unsendBuffer = std::string(str + totalSend, size - totalSend);
		return (false);
	}
	return (true);
}

int Response::send(Client& client) {
	static char buffer[R_BUFFER_SIZE];
	static char cbuffer[R_BUFFER_SIZE+6];
	static char hex[] = "0123456789ABCDEF";
	bool remove = false;

	if (this->dontSend)
		return (false);
	client.setReady(false);

	if (this->progress == Response::NOTHING_SENT)
	{
		if (unsendBuffer.length() > 0) {
			if (!this->sendUnsend(client))
				return (false);
		} else {
			if (this->getHeader("Transfer-Encoding").second == "chunked") {
				nextInitialProgress = Response::CHUNKED_SEND_MORE_RAW;
			} else if (this->compression != RFC::NONE && this->body && this->body->getBodySize() > 1024) {
				try {
					this->initCompression();
					this->addHeader("Content-Encoding", RFC::EncodingString[this->compression]);
					this->addHeader("Transfer-Encoding", "chunked");
					this->removeHeader("Content-Length");
					nextInitialProgress = Response::CHUNKED_SEND_MORE;
				} catch (std::exception e){
					Logger::error("Could not compress response: zlib error");
					nextInitialProgress = Response::HEADER_SENT;
				}
			} else
				nextInitialProgress = Response::HEADER_SENT;
			this->outputLine(unsendBuffer);
			struct timeval time;
			ft_memset(&time, 0, sizeof(struct timeval));
			::gettimeofday(&time, 0);
			unsendBuffer.append("Date: " + RFC::timeToString(time.tv_sec) + RFC::CRLF);
			this->outputHeaders(unsendBuffer);
			Logger::verbose("########################### Response Headers\n",
							unsendBuffer,
							"###########################");
			if (!this->trySend(client, unsendBuffer.c_str(), unsendBuffer.length()))
				return (false);
			unsendBuffer.clear();
		}
		if (!this->getBody()) remove = true;
		this->progress = nextInitialProgress;
	}
	else if (this->progress == Response::HEADER_SENT)
	{
		if (unsendBuffer.length() > 0) {
			if (!this->sendUnsend(client))
				return (false);
		} else {
			int size = this->body->getBody(buffer, R_BUFFER_SIZE);
			this->lastSendSize = size;
			if (!this->trySend(client, buffer, size))
				return (false);
		}
		if (this->lastSendSize < R_BUFFER_SIZE)	remove = true;
	}
	else if (this->progress == Response::CHUNKED_SEND_MORE_RAW)
	{
		if (unsendBuffer.length() > 0) {
			if (!this->sendUnsend(client))
				return (false);
		} else {
			int ret = this->body->getBody(buffer + 6, R_BUFFER_SIZE - 8);
			if (ret == -1)
				return (false);
			buffer[0] = hex[ret / 0x1000];
			buffer[1] = hex[ret % 4096 / 0x0100];
			buffer[2] = hex[ret % 256 / 0x0010];
			buffer[3] = hex[ret % 0x0010];
			buffer[4] = '\r';
			buffer[5] = '\n';
			buffer[ret + 6] = '\r';
			buffer[ret + 7] = '\n';
			this->lastSendSize = ret;
			if (!this->trySend(client, buffer, ret + 8))
				return (false);
		}
		if (this->lastSendSize == 0) remove = true;
	}
	else if (this->progress == Response::CHUNKED_SEND_MORE)
	{
		if (unsendBuffer.length() > 0) {
			if (!this->sendUnsend(client))
				return (false);
		} else {
			char ret = 0, flush = 0;
			this->stream.avail_out = R_BUFFER_SIZE - 2;
			this->stream.next_out = (Bytef*)cbuffer + 6;
			do {
				this->stream.avail_in = this->body->getBody(buffer, R_BUFFER_SIZE);
				this->stream.next_in = (Bytef*)buffer;
				flush = (this->stream.avail_in == R_BUFFER_SIZE) ? Z_NO_FLUSH : Z_FINISH;
				do {
					ret = deflate(&this->stream, flush);
				} while (this->stream.avail_in != 0);
			} while (this->stream.avail_out == R_BUFFER_SIZE - 2);
			int have = R_BUFFER_SIZE - 2 - this->stream.avail_out;
			// * 0xFFFF = 65535
			cbuffer[0] = hex[have / 0x1000];
			cbuffer[1] = hex[have % 4096 / 0x0100];
			cbuffer[2] = hex[have % 256 / 0x0010];
			cbuffer[3] = hex[have % 0x0010];
			cbuffer[4] = '\r';
			cbuffer[5] = '\n';
			*this->stream.next_out = '\r';
			*(this->stream.next_out + 1) = '\n';
			// flush and ret are in ...{00000000}{00000000}
			this->lastSendSize = 0 | (flush << 8) | (ret);
			if (!this->trySend(client, cbuffer, have + 8))
				return (false);
		}
		// flush == Z_FINISH && ret == 1
		if ((this->lastSendSize >> 8) == Z_FINISH && (this->lastSendSize & 0xFF) == 1)
			this->progress = Response::CHUNKED_NEED_END;
	}
	else if (this->progress == Response::CHUNKED_NEED_END)
	{
		if (unsendBuffer.length() > 0) {
			if (!this->sendUnsend(client))
				return (false);
		} else {
			buffer[0] = '0';
			buffer[1] = '\r';
			buffer[2] = '\n';
			buffer[3] = '\r';
			buffer[4] = '\n';
			if (!this->trySend(client, buffer, 5))
				return (false);
		}
		deflateEnd(&this->stream);
		this->strmOpen = false;
		remove = true;
	}
	if (remove) {
		client.keepAlive();
		struct timeval time;
		ft_memset(&time, 0, sizeof(struct timeval));
		::gettimeofday(&time, 0);
		// Display that we are sending a request
		Logger::info("\033[0;36m", RFC::timeToString(time.tv_sec),
			"\t\033[0;34mSEND\t\033[1;35m", client.getIp(), "\033[0m\t",
			this->getStatus(), " ", RFC::StatusReason[this->getStatus()]);
	}
	return (remove);
}

int Response::getType(void) const {
	return (0);
}

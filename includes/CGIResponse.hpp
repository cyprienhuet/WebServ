#ifndef CGIRESPONSE_HPP
# define CGIRESPONSE_HPP

# include "Response.hpp"
# include "CGIBody.hpp"

class CGIResponse:
    public Response {
private:
    std::vector<std::pair<int, bool> > &outputs;

    RFC::Method method;
    std::string strBuffer;
    bool readHeaders;
    size_t contentLength;
public:
    CGIResponse(Response const &other, std::vector<std::pair<int, bool> > &outputs);
    virtual ~CGIResponse();

    void forMethod(RFC::Method method);
    void erase(std::vector<std::pair<int, bool> >::iterator it);
    int proxyResponseSend(Client &client, std::vector<std::pair<int, bool> >::iterator it);
    virtual int send(Client &client);

    int getBodyFd(void) const;
	bool isBodyReady(void) const;
    virtual int getType(void) const;
    bool hasReadHeaders(void) const;
};

#endif /* CGIRESPONSE_HPP */

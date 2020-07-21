#ifndef HEADERLIST_HPP
# define HEADERLIST_HPP

# include <string>
# include <vector>
# include <list>
# include <utility>
# include "RFC.hpp"

struct ContentType {
	std::string	mediaType;
	std::string	charset;

	ContentType(): mediaType(), charset() {};
};

struct AcceptQuality {
	std::string value;
	float q;

	AcceptQuality(): value(), q(1.f) {};
};

struct Host {
	std::string name;
	int port;

	Host(): name(), port(80) {};
};

/*
struct HeaderList {
	// General Headers
	std::string cacheControl;
	std::string connection;
	std::string date; // * In subject
	std::string pragma;
	std::string trailer;
	std::string transferEncoding; // * In subject
	std::string upgrade;
	std::string via;
	std::string warning;
	// Entity Headers
	std::vector<RFC::Method> allow; // * In subject
	std::string contentEncoding; // * In subject
	std::string contentLanguage; // * In subject
	int contentLength = 0; // * In subject
	std::string contentLocation; // * In subject
	std::string contentRange;
	ContentType contentType; // * In subject
	std::string expires;
	std::string lastModified; // * In subject
	// Request Headers
	std::string accept;
	std::vector<AcceptQuality> acceptCharset; // * In subject
	std::vector<AcceptQuality> acceptEncoding; // * In subject
	std::vector<AcceptQuality> acceptLanguage; // * In subject
	std::pair<std::string, std::string> authorization; // * In subject
	std::string expect;
	std::string from;
	Host host; // * In subject
	std::string ifMatch;
	std::string ifModifiedSince;
	std::string ifNoneMatch;
	std::string ifRange;
	std::string ifUnmodifiedSince;
	std::string maxForwards;
	std::string proxyAuthorization;
	std::string range;
	std::string referrer; // * In subject
	std::string TE;
	std::string	userAgent; // * In subject
	// Response Headers
	std::string age;
	std::string acceptRanges;
	std::string ETag;
	std::string location; // * In subject
	std::string proxyAuthenticate;
	std::string retryAfter; // * In subject
	std::string server; // * In subject
	std::string vary;
	std::string wwwAuthenticate;
	std::string others; // * In subject
};
*/

#endif

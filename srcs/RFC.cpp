#include "RFC.hpp"

const std::string RFC::CRLF = "\r\n";
const std::string RFC::SP = " ";
const std::string RFC::TAB = "\t";
const std::string RFC::ALLSP = " \t";

const std::string RFC::timeToString(time_t const &time) {
	char buffer[30];
	ft_memset(buffer, 0, 30);
	strftime(buffer, 30, "%a, %d %b %Y %T", gmtime(&time));
	return std::string(buffer) + " GMT";
}

time_t RFC::stringToTime(std::string &time) {
	struct tm tm;
	char *ptr;
	if ((ptr = strptime(time.c_str(), "%a, %d %b %Y %T GMT", &tm)) != NULL) {
		return mktime(&tm);
	} else if ((ptr = strptime(time.c_str(), "%A, %d-%b-%y %T GMT", &tm)) != NULL) {
		return mktime(&tm);
	} else if ((ptr = strptime(time.c_str(), "%a %b %d %T %Y", &tm)) != NULL) {
		return mktime(&tm);
	}
	return NULL;
}

std::vector<std::string> RFC::createMethodString(void) {
	std::vector<std::string> MethodStrings(8);
	MethodStrings[OPTIONS] = "OPTIONS";
	MethodStrings[GET] = "GET";
	MethodStrings[HEAD] = "HEAD";
	MethodStrings[POST] = "POST";
	MethodStrings[PUT] = "PUT";
	MethodStrings[DELETE] = "DELETE";
	MethodStrings[TRACE] = "TRACE";
	MethodStrings[CONNECT] = "CONNECT";
	return (MethodStrings);
}
std::vector<std::string> RFC::MethodString = RFC::createMethodString();

RFC::Method RFC::methodFromString(std::string const &method) {
	auto it = RFC::MethodString.begin();
	auto ite = RFC::MethodString.end();
	for (size_t i = 0; it != ite; ++it, ++i)
		if (*it == method)
			return (static_cast<RFC::Method>(i));
	return (INVALID);
}

std::map<RFC::Status, std::string> RFC::createStatusReason(void) {
	std::map<RFC::Status, std::string> StatusReason;
	StatusReason[Continue] = "Continue";
	StatusReason[SwitchingProtocols] = "Switching Protocols";
	StatusReason[Ok] = "OK";
	StatusReason[Created] = "Created";
	StatusReason[Accepted] = "Accepted";
	StatusReason[NonAuthorativeInformation] = "Non-Authoritative Information";
	StatusReason[NoContent] = "No Content";
	StatusReason[ResetContent] = "Reset Content";
	StatusReason[PartialContent] = "Partial Content";
	StatusReason[MultipleChoices] = "Multiple Choices";
	StatusReason[MovedPermanently] = "Moved Permanently";
	StatusReason[Found] = "Found";
	StatusReason[SeeOther] = "See Other";
	StatusReason[NotModified] = "Not Modified";
	StatusReason[TemporaryRedirect] = "Temporary Redirect";
	StatusReason[BadRequest] = "Bad Request";
	StatusReason[Unauthorized] = "Unauthorized";
	StatusReason[PaymentRequired] = "Payment Required";
	StatusReason[Forbidden] = "Forbidden";
	StatusReason[NotFound] = "Not Found";
	StatusReason[MethodNotAllowed] = "Method Not Allowed";
	StatusReason[NotAcceptable] = "Not Acceptable";
	StatusReason[ProxyAuthenticationRequired] = "Proxy Authentication Required";
	StatusReason[RequestTimeout] = "Request Time-out";
	StatusReason[Conflict] = "Conflict";
	StatusReason[Gone] = "Gone";
	StatusReason[LengthRequired] = "Length Required";
	StatusReason[PreconditionFailed] = "Precondition Failed";
	StatusReason[RequestEntityTooLarge] = "Request Entity Too Large";
	StatusReason[RequestURITooLarge] = "Request-URI Too Large";
	StatusReason[UnsupportedMediaType] = "Unsupported Media Type";
	StatusReason[RequestedRangeNotSatisfiable] = "Requested range not satisfiable";
	StatusReason[ExpectationFailed] = "Expectation Failed";
	StatusReason[UpgradeRequired] = "Upgrade Required";
	StatusReason[InternalServerError] = "Internal Server Error";
	StatusReason[NotImplemented] = "Not Implemented";
	StatusReason[BadGateway] = "Bad Gateway";
	StatusReason[ServiceUnavailable] = "Service Unavailable";
	StatusReason[GatewayTimeout] = "Gateway Time-out";
	StatusReason[HTTPVersionNotSupported] = "HTTP Version not supported";
	return (StatusReason);
}
std::map<RFC::Status, std::string> RFC::StatusReason = RFC::createStatusReason();

std::map<RFC::Header, std::string> RFC::createHeaderName(void) {
	std::map<RFC::Header, std::string> HeaderName;
	// General
	HeaderName[CacheControl] = "Cache-Control";
	HeaderName[Connection] = "Connection";
	HeaderName[Date] = "Date";
	HeaderName[Pragma] = "Pragma";
	HeaderName[Trailer] = "Trailer";
	HeaderName[TransferEncoding] = "Transfer-Encoding";
	HeaderName[Upgrade] = "Upgrade";
	HeaderName[Via] = "Via";
	HeaderName[Warning] = "Warning";
	// Entity
	HeaderName[Allow] = "Allow";
	HeaderName[ContentEncoding] = "Content-Encoding";
	HeaderName[ContentLanguage] = "Content-Language";
	HeaderName[ContentLength] = "Content-Length";
	HeaderName[ContentLocation] = "Content-Location";
	HeaderName[ContentRange] = "Content-Range";
	HeaderName[ContentType] = "Content-Type";
	HeaderName[Expires] = "Expires";
	HeaderName[LastModified] = "Last-Modified";
	// Request
	HeaderName[Accept] = "Accept";
	HeaderName[AcceptCharset] = "Accept-Charset";
	HeaderName[AcceptEncoding] = "Accept-Encoding";
	HeaderName[AcceptLanguage] = "Accept-Language";
	HeaderName[Authorization] = "Authorization";
	HeaderName[Expect] = "Expect";
	HeaderName[From] = "From";
	HeaderName[Host] = "Host";
	HeaderName[IfMatch] = "If-Match";
	HeaderName[IfModifiedSince] = "If-Modified-Since";
	HeaderName[IfNoneMatch] = "If-None";
	HeaderName[IfRange] = "If-Range";
	HeaderName[IfUnmodifiedSince] = "If-Unmodified-Since";
	HeaderName[MaxForwards] = "Max-Forwards";
	HeaderName[ProxyAuthorization] = "Proxy-Authorization";
	HeaderName[Range] = "Range";
	HeaderName[Referer] = "Referer";
	HeaderName[TE] = "TE";
	HeaderName[UserAgent] = "User-Agent";
	// Response
	HeaderName[Age] = "Age";
	HeaderName[AcceptRanges] = "Accept-Ranges";
	HeaderName[ETag] = "ETag";
	HeaderName[Location] = "Location";
	HeaderName[ProxyAuthenticate] = "Proxy-Authenticate";
	HeaderName[RetryAfter] = "Retry-After";
	HeaderName[Server] = "Server";
	HeaderName[Vary] = "Vary";
	HeaderName[WWWAuthenticate] = "WWW-Authenticate";
	return (HeaderName);
}
std::map<RFC::Header, std::string> RFC::HeaderName = RFC::createHeaderName();

std::map<RFC::Encoding, std::string> RFC::createEncodingString(void)
{
	std::map<RFC::Encoding, std::string> EncodingString;
	EncodingString[NONE] = "none";
	EncodingString[GZIP] = "gzip";
	EncodingString[DEFLATE] = "deflate";
	return (EncodingString);
}
std::map<RFC::Encoding, std::string> RFC::EncodingString = RFC::createEncodingString();

RFC::Header RFC::headerFromString(std::string const &header) {
	auto it = RFC::HeaderName.begin();
	auto ite = RFC::HeaderName.end();
	for ( ; it != ite; ++it) {
		if (insensitive_equal((*it).second, header))
			return ((*it).first);
	}
	return (OtherHeader);
}

std::string RFC::base64_encode(const std::string &in) {
    std::string out;

    int val = 0, valb = -6;
    for (auto c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
		out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4)
		out.push_back('=');
    return out;
}

std::string RFC::base64_decode(const std::string &in) {
    std::string out;

    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; ++i)
		T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

    int val = 0, valb = -8;
    for (auto c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

bool RFC::isRFCSpace(int c) {
	return (c == RFC::SP[0] || c == RFC::TAB[0]);
}

void RFC::decodeURIComponent(std::string &uri) {
	static std::string const &validHex = "abcdefABCDEF0123456789";

	for (auto it = uri.begin(); it != uri.end(); ++it) {
		if (*it == '%') {
			auto end = uri.end();
			auto first = it + 1;
			auto second = first + 1;
			if (first == end || second == end)
				continue ;
			int c1 = std::tolower(*first);
			int c2 = std::tolower(*second);
			if (validHex.find(c1) == std::string::npos
				|| validHex.find(c2) == std::string::npos)
				continue ;
			int decimal = ((std::isdigit(c1) ? c1 - '0' : c1 - 'a') * 16)
						+ (std::isdigit(c2) ? c2 - '0' : c2 - 'a');
			if (decimal < 32 || decimal > 126)
				continue ;
			it = uri.erase(it, ++second);
			uri.insert(it, (char)decimal);
			if (it != uri.begin())
				--it;
		}
	}
}

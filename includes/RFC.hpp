#ifndef RFC_HPP
# define RFC_HPP

# include <string>
# include <vector>
# include <map>
# include "Utility.hpp"

class RFC {
public:
	static const std::string CRLF;
	static const std::string SP;
	static const std::string TAB;
	static const std::string ALLSP;

	static const std::string timeToString(time_t const &time);
	static time_t stringToTime(std::string &time);

	enum Method {
		OPTIONS = 0,
		GET,
		HEAD,
		POST,
		PUT,
		DELETE,
		TRACE,
		CONNECT,
		INVALID
	};
	static std::vector<std::string> MethodString;
	static Method methodFromString(std::string const &method);

	enum Status {
		Continue = 100,
		SwitchingProtocols = 101,
		Ok = 200,
		Created = 201,
		Accepted = 202,
		NonAuthorativeInformation = 203,
		NoContent = 204,
		ResetContent = 205,
		PartialContent = 206,
		MultipleChoices = 300,
		MovedPermanently = 301,
		Found = 302,
		SeeOther = 303,
		NotModified = 304,
		TemporaryRedirect = 307,
		BadRequest = 400,
		Unauthorized = 401,
		PaymentRequired = 402,
		Forbidden = 403,
		NotFound = 404,
		MethodNotAllowed = 405,
		NotAcceptable = 406,
		ProxyAuthenticationRequired = 407,
		RequestTimeout = 408,
		Conflict = 409,
		Gone = 410,
		LengthRequired = 411,
		PreconditionFailed = 412,
		RequestEntityTooLarge = 413,
		RequestURITooLarge = 414,
		UnsupportedMediaType = 415,
		RequestedRangeNotSatisfiable = 416,
		ExpectationFailed = 417,
		UpgradeRequired = 426,
		InternalServerError = 500,
		NotImplemented = 501,
		BadGateway = 502,
		ServiceUnavailable = 503,
		GatewayTimeout = 504,
		HTTPVersionNotSupported = 505,
		InvalidStatus = 999
	};
	static std::map<Status, std::string> StatusReason;

	enum Header {
		// General
		CacheControl = 0,
		Connection,
		Date,
		Pragma,
		Trailer,
		TransferEncoding,
		Upgrade,
		Via,
		Warning,
		// Entity
		Allow,
		ContentEncoding,
		ContentLanguage,
		ContentLength,
		ContentLocation,
		ContentRange,
		ContentType,
		Expires,
		LastModified,
		// Request
		Accept,
		AcceptCharset,
		AcceptEncoding,
		AcceptLanguage,
		Authorization,
		Expect,
		From,
		Host,
		IfMatch,
		IfModifiedSince,
		IfNoneMatch,
		IfRange,
		IfUnmodifiedSince,
		MaxForwards,
		ProxyAuthorization,
		Range,
		Referer,
		TE,
		UserAgent,
		// Response
		Age,
		AcceptRanges,
		ETag,
		Location,
		ProxyAuthenticate,
		RetryAfter,
		Server,
		Vary,
		WWWAuthenticate,
		OtherHeader
	};
	static std::map<Header, std::string> HeaderName;
	static Header headerFromString(std::string const &header);

	enum Encoding {
		NONE = 0,
		GZIP = 1,
		DEFLATE = 2
	};
	static std::map<Encoding, std::string> EncodingString;

	// Utility
	static std::string base64_decode(const std::string &in);
	static std::string base64_encode(const std::string &in);
	static bool isRFCSpace(int c);
	static void decodeURIComponent(std::string &uri);
private:
	static std::vector<std::string> createMethodString(void);
	static std::map<Status, std::string> createStatusReason(void);
	static std::map<Header, std::string> createHeaderName(void);
	static std::map<Encoding, std::string> createEncodingString(void);
};

#endif

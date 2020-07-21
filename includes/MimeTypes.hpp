#ifndef MIMETYPES_HPP
# define MIMETYPES_HPP

# include <iostream>
# include <string>

class MimeTypes {
private:
	struct mime {
		const std::string extension;
		const std::string type;
	};
	static const MimeTypes::mime mimes[348];
public:
	static const std::string getType(const std::string path);
	static const std::string getExtension(const std::string type);
};

#endif

# include "Utility.hpp"

std::string to_string(int value) {
	std::string res;
	if (value == 0)
		return ("0");
	std::string::iterator pos = res.end();
	while (value != 0) {
		pos = res.insert(pos, '0' + (value % 10));
		value /= 10;
	}
	if (value < 0)
		res.insert(pos, '-');
	return (res);
}

std::string to_string(size_t value) {
	std::string res;
	if (value == 0)
		return ("0");
	std::string::iterator pos = res.end();
	while (value != 0) {
		pos = res.insert(pos, '0' + (value % 10));
		value /= 10;
	}
	return (res);
}

std::vector<std::string> split(std::string const &string, std::string const &sep) {
	size_t pos, offset = 0;
	std::vector<std::string> ret;
	if (string.length() == 0 || sep.length() == 0) {
		ret.push_back(string);
		return (ret);
	}
	while ((pos = string.find(sep, offset)) != std::string::npos) {
		if (offset == pos)
			++offset;
		else {
			ret.push_back(string.substr(offset, pos - offset));
			offset = pos + sep.length();
		}
	}
	if (offset < pos) {
		ret.push_back(string.substr(offset, pos - offset));
	}
	return ret;
}

std::string join(std::vector<std::string> const &vec, std::string const &glue) {
	std::string str;
	auto ite = vec.end();
	for (auto it = vec.begin(); it != ite; it++) {
		str.append(*it);
		if (--(vec.end()) != it) {
			str.append(glue);
		}
	}
	return str;
}

void trim(std::string &str) {
	size_t pos = 0;
	size_t length = str.length();
	pos = 0;
	while (pos < length && RFC::isRFCSpace(str[pos]))
		++pos;
	if (pos > 0)
		str.erase(0, pos);
	length = str.length();
	if (length) {
		pos = length - 1;
		while (pos > 1 && RFC::isRFCSpace(str[pos]))
			--pos;
		if (pos < length - 1)
			str.erase(pos);
	}
}

bool insensitive_equal(std::string const &s1, std::string const &s2) {
	size_t l1 = s1.length();
	if (l1 != s2.length())
		return (false);
	for (size_t i = 0; i < l1; i++)
		if (std::tolower(s1[i]) != std::tolower(s2[i]))
			return (false);
	return (true);
}

size_t ft_strlen(char const *src) {
	size_t i = 0;

	while (src[i])
		++i;
	return (i);
}

char *ft_strdup(char const *src) {
	return (ft_strndup(src, ft_strlen(src)));
}

char *ft_strndup(char const *src, size_t length) {
	if (length == 0 || !src)
		return (nullptr);
	char *tmp = (char*)operator new (length + 1);
	size_t i = 0;

	for ( ; i < length; ++i)
		tmp[i] = src[i];
	tmp[i] = 0;
	return (tmp);
}

char *ft_strnstr(char const *str, char const *to_find, size_t len, size_t find_len) {
	unsigned int i;
	unsigned int o;

	if (len == 0 || find_len == 0 || to_find[0] == '\0' || !str)
		return ((char *)str);
	i = 0;
	while (i < len) {
		o = 0;
		while (str[i + o] == to_find[o] && (i + o) < len && o < find_len)
			o++;
		if (o == find_len)
			return ((char *)&str[i]);
		i++;
	}
	return (nullptr);
}

void *ft_memcpy(void *dest, const void *src, size_t n) {
	if (!dest && !src)
		return (0);
	unsigned char *d = (unsigned char *)dest;
	const unsigned char	*s = (unsigned char *)src;
	for (size_t i = 0; i < n; ++i)
		d[i] = s[i];
	return (dest);
}

void *ft_memset(void *str, int c, size_t n) {
	unsigned char	*m;

	m = (unsigned char *)str;
	while (n-- > 0)
		*(m++) = (unsigned char)c;
	return (str);
}

char* ft_atoi_hex_chunked(char *str, size_t& res) {
	int	i = 0;
	int lower = 0;
	bool isHex = false;
	if (!str)
		throw std::exception();
	while (std::isdigit(str[i]) || (isHex = ((lower = std::tolower(str[i])) >= 'a' && lower <= 'f'))) {
		if (isHex) {
			res = (res * 16) + (lower - 'a' + 10);
		} else {
			res = (res * 16) + (str[i] - '0');
		}
		i++;
		isHex = false;
	}
	// if (str[i] && !std::isspace(str[i]))
	// 	throw std::exception();
	return (str + i);
}

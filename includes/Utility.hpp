#ifndef UTILITY_HPP
# define UTILITY_HPP

# include <stdlib.h>
# include <unistd.h>
# include <errno.h>
# include <string>
# include <vector>
# include "RFC.hpp"

std::string to_string(size_t value);
std::string to_string(int value);
std::vector<std::string> split(std::string const &string, std::string const &sep);
std::string join(std::vector<std::string> const &vec, std::string const &glue);
template<typename T>
std::string join(std::vector<T> const &vec, std::string const &glue) {
	std::string str;
	auto ite = vec.end();
	for (auto it = vec.begin(); it != ite; it++) {
		str.append(to_string(*it));
		if (--(vec.end()) != it) {
			str.append(glue);
		}
	}
	return (str);
}
void trim(std::string &str);
bool insensitive_equal(std::string const &s1, std::string const &s2);
size_t ft_strlen(char const *src);
char *ft_strdup(char const *src);
char *ft_strndup(char const *src, size_t length);
char *ft_strnstr(char const *str, char const *to_find, size_t len, size_t find_len);
void *ft_memcpy(void *dest, const void *src, size_t n);
void *ft_memset(void *str, int c, size_t n);
char* ft_atoi_hex_chunked(char *str, size_t& value);

#endif

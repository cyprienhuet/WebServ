#ifndef RESPONSEBODY_HPP
#define RESPONSEBODY_HPP

class ResponseBody {
	char* ptr;
public:
	ResponseBody(char* ptr) : ptr(ptr) {};
	operator char*() { return this->ptr; }
	operator const char*() { return this->ptr; }
	ResponseBody& operator=(char* ptr) { this->ptr = ptr; return *this; };
};

#endif /* RESPONSEBODY_HPP */

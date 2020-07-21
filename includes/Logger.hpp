#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <iostream>

class Logger
{
public:
	enum LoggerOptions {
		NO_STDOUT = 0x1,
		SHOW_INFO = 0x2,
		SHOW_DEBUG = 0x4,
		SHOW_VERBOSE = 0x8,
		SHOW_ERROR = 0x16,
		DEFAULT = SHOW_INFO | SHOW_DEBUG | SHOW_ERROR
	};
	enum LoggerType {
		INFO,
		DEBUG,
		VERBOSE,
		ERROR,
		RUN
	};
private:
	static int options;
	static int mode;

	static bool flagEnabled(LoggerOptions flag);
	static bool shouldDisplay(LoggerType type);

	static void log(LoggerType type) {
		if (shouldDisplay(type))
			std::cout << '\n';
	}

	template<typename T, typename ...Args>
	static void log(LoggerType type, T arg, Args... args) {
		if (shouldDisplay(type))
			std::cout << arg;
		else return ;
		Logger::log(type, args...);
	}
public:
	static void setOptions(int flags);

	template<typename ...Args>
	static void info(Args... args) {
		Logger::log(INFO, args...);
	}
	template<typename ...Args>
	static void error(Args... args) {
		Logger::log(ERROR, args...);
	}
	template<typename ...Args>
	static void debug(Args... args) {
		Logger::log(DEBUG, args...);
	}
	template<typename ...Args>
	static void verbose(Args... args) {
		Logger::log(VERBOSE, args...);
	}
	template<typename ...Args>
	static void run(Args... args) {
		Logger::log(RUN, args...);
	}
};

#endif

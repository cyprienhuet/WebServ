#include "Logger.hpp"

int Logger::options = 0 ^ Logger::DEFAULT;

bool Logger::flagEnabled(LoggerOptions flag) {
	return (Logger::options & flag);
}

bool Logger::shouldDisplay(LoggerType type) {
	return (!Logger::flagEnabled(NO_STDOUT)
			&& (type == RUN
			|| (type == INFO 	&& Logger::flagEnabled(SHOW_INFO))
			|| (type == DEBUG 	&& Logger::flagEnabled(SHOW_DEBUG))
			|| (type == ERROR 	&& Logger::flagEnabled(SHOW_ERROR))
			|| (type == VERBOSE	&& Logger::flagEnabled(SHOW_VERBOSE))));
}

void Logger::setOptions(int flags) {
	Logger::options = 0 ^ flags;
}

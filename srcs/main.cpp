#include "Server.hpp"

int main(int argc, char **argv) {
	std::string conf = "conf/default.conf";
	if (argc >= 2)
		conf = argv[1];
	Server s(conf);

	if (s.isValidConf()) {
		try {
			s.init();
		} catch(const std::exception& e) {
			Logger::error(e.what());
			return (EXIT_FAILURE);
		}
		s.start();
	}
	File::close(File::devNull);
	return (EXIT_SUCCESS);
}

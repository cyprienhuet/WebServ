#ifndef RESOURCE_HPP
# define RESOURCE_HPP

# include <unistd.h>
# include <errno.h>
# include <sys/param.h>
# include <sys/stat.h>
# include <string>
# include "RFC.hpp"

/**
 * Represent a file on the system.
 * 	absolute: cwd/root/path/filename.extension
 * 	relative: path/filename.extension
 * 	rootRelative: root/path/filename.extension
 **/
class Resource {
public:
	enum ResourceStatus {
		VALID = 0,
		FORBIDDEN,
		NOT_FOUND,
		IS_DIR,
		GENERAL
	};
private:
	static std::string getCwd(void);
	static size_t fds;

	//localhost:8000/get/resource/test.html

	std::string	r_absoluteRoot; // CWD + r_root
	std::string	r_root; // www
	std::string	r_relativePath; // get/resource/test.html
	std::string	r_filename; // test
	std::string	r_extension; // html
	std::string	r_pathInfo; //
	std::string r_query; //
	std::string	r_fullFilename; // test.html
	bool		r_isDirectory; //

	std::string	r_relativeRootPath; // r_root + relative_path
	std::string	r_absolutePath; // CWD + r_relativeRootPath
	std::string	r_folder; // relative_rootpath - filename
	std::string r_relativeFolder; //relative_path - filename

	struct stat		r_stat;
	ResourceStatus	r_status;

	static void fixPath(std::string &path);
	static void appendToRelativePath(std::string &path, std::string const &filename);
	std::string const &pathWithFilename(std::string const &path, std::string const &filename) const;
	void generatePaths(void);
public:
	Resource();
	Resource(std::string const &relativePath);
	Resource(std::string const &root, std::string const &relativePath);
	Resource(Resource const &other);
	virtual ~Resource();

	static std::string cwd;

	void load(std::string const &relativePath);
	void load(std::string const &root, std::string const &relativePath);
	Resource &operator=(Resource const &other);

	std::string const &filename(void) const;
	std::string const &extension(void) const;
	std::string const &pathInfo(void) const;
	std::string const &query(void) const;
	std::string const &fullFilename(void) const;
	std::string const &relativePath(std::string const &filename="") const;
	std::string const &absoluteRoot(void) const;
	std::string const &root(void) const;
	std::string const &relativeRootPath(std::string const &filename="") const;
	std::string const &absolutePath(std::string const &filename="") const;
	std::string const &folder(std::string const &filename="") const;
	std::string const &relativeFolder(std::string const &filename="") const;

	struct stat const &getStats(void) const;
	ResourceStatus status(void) const;
	RFC::Status RFCStatus() const;
	bool isValid(void) const;
	time_t const &mtime(void) const;
	size_t size(void) const;
	bool isDirectory(void) const;
	bool unlink(void);
};

#endif

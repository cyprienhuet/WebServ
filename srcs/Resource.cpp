#include "Resource.hpp"
#include <iostream>

std::string Resource::cwd = Resource::getCwd();
std::string Resource::getCwd(void) {
	char buffer[MAXPATHLEN + 1];
	if (getcwd(buffer, MAXPATHLEN + 1)) {
		std::string cwd = buffer;
		Resource::fixPath(cwd);
		return (cwd);
	}
	return ("");
}

Resource::Resource():
	r_absoluteRoot(), r_root(), r_relativePath(),
	r_filename(), r_extension(), r_pathInfo(), r_query(),
	r_fullFilename(), r_isDirectory(false),
	r_relativeRootPath(), r_absolutePath(), r_folder(), r_relativeFolder(),
	r_stat(), r_status(GENERAL) {}

Resource::Resource(std::string const &relativePath) {
	this->load("", relativePath);
}

Resource::Resource(std::string const &root, std::string const &relativePath) {
	this->load(root, relativePath);
}

Resource::Resource(Resource const &other):
	r_absoluteRoot(other.r_absoluteRoot),  r_root(other.r_root), r_relativePath(other.r_relativePath),
	r_filename(other.r_filename), r_extension(other.r_extension),
	r_pathInfo(other.r_pathInfo), r_query(other.r_query),
	r_fullFilename(other.r_fullFilename),
	r_isDirectory(other.r_isDirectory),
	r_relativeRootPath(other.r_relativeRootPath), r_absolutePath(other.r_absolutePath),
	r_folder(other.r_folder), r_relativeFolder(other.r_relativeFolder),
	r_stat(other.r_stat),
	r_status(other.r_status) {}

Resource::~Resource() {}

void Resource::load(std::string const &relativePath) {
	this->load("", relativePath);
}

void Resource::load(std::string const &root, std::string const &relativePath) {
	this->r_isDirectory = false;
	this->r_root = root;
	this->fixPath(this->r_root);
	std::string fixedRelativedPath = relativePath;
	this->fixPath(fixedRelativedPath);
	size_t qPos = fixedRelativedPath.find('?');
	this->r_query.clear();
	if (qPos != std::string::npos) {
		this->r_query = fixedRelativedPath.substr(qPos + 1);
		fixedRelativedPath.erase(qPos);
		this->fixPath(fixedRelativedPath);
	}
	size_t pPos = fixedRelativedPath.find_last_of('.');
	this->r_pathInfo = fixedRelativedPath;
	if (pPos != std::string::npos) {
		size_t pathPos = fixedRelativedPath.find('/', pPos);
		if (pathPos != std::string::npos) {
			this->r_pathInfo = fixedRelativedPath.substr(pathPos);
			fixedRelativedPath.erase(pathPos);
			this->fixPath(fixedRelativedPath);
		}
	}
	this->r_relativePath = fixedRelativedPath;
	// If relative path end with a / it's a folder, no filename nor extension
	if ((qPos == std::string::npos && relativePath.back() != '/')
		|| (qPos != std::string::npos && relativePath[qPos - 1] != '/')) {
		// Find last / for filename - none means no subdirectoy
		size_t fPos = this->r_relativePath.find_last_of('/');
		if (fPos == std::string::npos) {
			fPos = 0;
		}
		// Find the last . for the filename and the ext - none means no ext
		this->r_filename = this->r_relativePath.substr(fPos, pPos);
		this->r_extension.clear();
		if (pPos != std::string::npos) {
			this->r_extension = this->r_relativePath.substr(pPos + 1);
		}
	} else {
		this->r_filename.clear();
		this->r_extension.clear();
		this->r_isDirectory = true;
	}
	this->generatePaths();
	// Check the resource as a file
	this->r_status = VALID;
	::ft_memset(&this->r_stat, 0, sizeof(this->r_stat));
	if (lstat(this->r_absolutePath.c_str(), &this->r_stat) == -1) {
		if (errno == EACCES)
			this->r_status = FORBIDDEN;
		else if (errno == ENOENT || errno == ENOTDIR)
			this->r_status = NOT_FOUND;
		else
			this->r_status = GENERAL;
	} else if (S_ISDIR(this->r_stat.st_mode)) {
		this->r_status = IS_DIR;
	}
	// Folder name - default to ./ if it's only a filename
	if (this->isDirectory()) {
		this->r_folder = this->r_relativePath;
		this->r_relativeFolder = this->r_relativePath;
	} else {
		size_t bPos = this->r_relativeRootPath.find_last_of('/');
		if (bPos == std::string::npos) {
			this->r_folder = "./";
		} else {
			this->r_folder = this->r_relativeRootPath.substr(0, bPos);
		}
		size_t cPos = this->r_relativePath.find_last_of('/');
		if (cPos == std::string::npos) {
			this->r_relativeFolder = "./";
		} else {
			this->r_relativeFolder = this->r_relativePath.substr(0, cPos);
		}
	}

	// std::cout << "this->r_absoluteRoot = <" << this->r_absoluteRoot << ">\n" <<
	// 		"this->r_root = <" << this->r_root << ">\n" <<
	// 		"this->r_relativePath = <" << this->r_relativePath << ">\n" <<
	// 		"this->relativePath(pathInfo) = <" << this->relativePath(this->pathInfo()) << ">\n" <<
	// 		"this->r_filename = <" << this->r_filename << ">\n" <<
	// 		"this->r_extension = <" << this->r_extension << ">\n" <<
	// 		"this->r_pathInfo = <" << this->r_pathInfo << ">\n" <<
	// 		"this->r_query = <" << this->r_query << ">\n" <<
	// 		"this->r_fullFilename = <" << this->r_fullFilename << ">\n" <<
	// 		"this->r_isDirectory = <" << this->r_isDirectory << ">\n" <<
	// 		"this->r_relativeRootPath = <" << this->r_relativeRootPath << ">\n" <<
	// 		"this->r_absolutePath = <" << this->r_absolutePath << ">\n" <<
	// 		"this->r_folder = <" << this->r_folder << ">\n" << std::flush;
}

Resource &Resource::operator=(Resource const &other) {
	this->r_absoluteRoot = other.r_absoluteRoot;
	this->r_root = other.r_root;
	this->r_relativePath = other.r_relativePath;
	this->r_filename = other.r_filename;
	this->r_extension = other.r_extension;
	this->r_pathInfo = other.r_pathInfo;
	this->r_query = other.r_query;
	this->r_fullFilename = other.r_fullFilename;
	this->r_isDirectory = other.r_isDirectory;
	this->r_relativeRootPath = other.r_relativeRootPath;
	this->r_absolutePath = other.r_absolutePath;
	this->r_folder = other.r_folder;
	this->r_stat = other.r_stat;
	this->r_status = other.r_status;
	return (*this);
}

/**
 * ltrim "./" and delete the last / if there is one
 **/
void Resource::fixPath(std::string &path) {
	size_t length = path.length();
	size_t offset = 0;
	while (offset < length && (path[offset] == '.' || path[offset] == '/'))
		++offset;
	if (offset > 0)
		path.erase(0, offset);
	if (path.length() > 0 && path.back() == '/')
		path.erase(--path.end());
}

/**
 * Append filename to path
 * path is a relative path
 **/
void Resource::appendToRelativePath(std::string &path, std::string const &filename) {
	static std::string tmp;
	if (filename.length() > 0) {
		if (filename == "..") {
			size_t bPos = path.find('/');
			if (bPos == 0 || bPos == std::string::npos) {
				path.clear();
			} else {
				path.erase(bPos);
			}
		} else {
			tmp = filename;
			Resource::fixPath(tmp);
			if (path.length() > 0) {
				path += ("/" + tmp);
			} else {
				path = tmp;
			}
		}
	}
}

std::string const &Resource::pathWithFilename(std::string const &path, std::string const &filename) const {
	static std::string tmp;
	if (filename.length() > 0) {
		tmp = path;
		Resource::appendToRelativePath(tmp, filename);
		return (tmp);
	}
	return (path);
}

void Resource::generatePaths(void) {
	// Relative to root path
	if (this->r_root.length() == 0) {
		this->r_relativeRootPath = this->r_relativePath;
	} else if (this->r_relativePath.length() == 0) {
		this->r_relativeRootPath = this->r_root;
	} else {
		this->r_relativeRootPath = this->r_root + "/" + this->r_relativePath;
	}
	// Absolute root path
	if (Resource::cwd.length() == 0) {
		this->r_absoluteRoot = "/" + this->r_root;
	} else {
		this->r_absoluteRoot = "/" + Resource::cwd + "/" + this->r_root;
	}
	// Absolute path - empty cwd will make it /
	if (Resource::cwd.length() == 0) {
		this->r_absolutePath = "/" + this->r_relativeRootPath;
	} else {
		this->r_absolutePath = "/" + Resource::cwd + "/" + this->r_relativeRootPath;
	}
	// Full filename (no relative path before)
	if (this->r_extension.length() == 0) {
		this->r_fullFilename = this->r_filename;
	} else {
		this->r_fullFilename = this->r_filename + "." + this->r_extension;
	}
}

std::string const &Resource::filename(void) const {
	return (this->r_filename);
}

std::string const &Resource::extension(void) const {
	return (this->r_extension);
}

std::string const &Resource::pathInfo(void) const {
	return (this->r_pathInfo);
}

std::string const &Resource::query(void) const {
	return (this->r_query);
}

std::string const &Resource::fullFilename(void) const {
	return (this->r_fullFilename);
}

std::string const &Resource::relativePath(std::string const &filename) const {
	return (this->pathWithFilename(this->r_relativePath, filename));
}

std::string const &Resource::absoluteRoot(void) const {
	return (this->r_absoluteRoot);
}

std::string const &Resource::root(void) const {
	return (this->r_root);
}

std::string const &Resource::relativeRootPath(std::string const &filename) const {
	return (this->pathWithFilename(this->r_relativeRootPath, filename));
}

std::string const &Resource::absolutePath(std::string const &filename) const {
	return (this->pathWithFilename(this->r_absolutePath, filename));
}

std::string const &Resource::folder(std::string const &filename) const {
	return (this->pathWithFilename(this->r_folder, filename));
}

std::string const &Resource::relativeFolder(std::string const &filename) const {
	return (this->pathWithFilename(this->r_relativeFolder, filename));
}

struct stat const &Resource::getStats(void) const {
	return (this->r_stat);
}

Resource::ResourceStatus Resource::status(void) const {
	return (this->r_status);
}

RFC::Status Resource::RFCStatus(void) const {
	if (this->r_status == FORBIDDEN) {
		return (RFC::Forbidden);
	} else if (this->r_status == NOT_FOUND) {
		return (RFC::NotFound);
	} else if (this->r_status == GENERAL) {
		return (RFC::InternalServerError);
	}
	return (RFC::Ok);
}

bool Resource::isValid(void) const {
	return (this->r_status == VALID);
}

bool Resource::isDirectory(void) const {
	return (S_ISDIR(this->r_stat.st_mode));
}

time_t const &Resource::mtime(void) const {
	#ifdef __MACH__
		return (this->r_stat.st_mtimespec.tv_sec);
	#else
		return (this->r_stat.st_mtim.tv_sec);
	#endif
}

size_t Resource::size(void) const {
	return (this->r_stat.st_size);
}

bool Resource::unlink(void) {
	int res = ::unlink(this->absolutePath().c_str());
	if (res == -1) {
		if (errno == EACCES || errno == EISDIR || errno == EPERM) {
			this->r_status = FORBIDDEN;
		} else {
			this->r_status = NOT_FOUND;
		}
		return (false);
	}
	return (true);
}

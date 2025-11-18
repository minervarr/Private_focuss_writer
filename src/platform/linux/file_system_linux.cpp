#include "file_system_linux.h"
#include "utils/logger.h"
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <cstdlib>

namespace phantom {

FileSystemLinux::FileSystemLinux() {
    LOG_TRACE(LogCategory::PLATFORM, "FileSystemLinux constructor");
}

FileSystemLinux::~FileSystemLinux() {
    LOG_TRACE(LogCategory::PLATFORM, "FileSystemLinux destructor");
}

std::string FileSystemLinux::getConfigDirectory() const {
    const char* xdgConfig = getenv("XDG_CONFIG_HOME");
    if (xdgConfig) {
        return std::string(xdgConfig) + "/phantom-writer";
    }

    const char* home = getenv("HOME");
    if (home) {
        return std::string(home) + "/.config/phantom-writer";
    }

    return ".phantom-writer";
}

std::string FileSystemLinux::getDocumentsDirectory() const {
    const char* home = getenv("HOME");
    if (home) {
        return std::string(home) + "/Documents";
    }
    return ".";
}

std::string FileSystemLinux::getTempDirectory() const {
    const char* tmpDir = getenv("TMPDIR");
    if (tmpDir) {
        return tmpDir;
    }
    return "/tmp";
}

bool FileSystemLinux::fileExists(const std::string& path) const {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool FileSystemLinux::createDirectory(const std::string& path) const {
    return mkdir(path.c_str(), 0755) == 0;
}

char FileSystemLinux::getPathSeparator() const {
    return '/';
}

} // namespace phantom

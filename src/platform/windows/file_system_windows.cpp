#include "file_system_windows.h"
#include "utils/logger.h"
#include <windows.h>
#include <shlobj.h>
#include <sys/stat.h>

namespace phantom {

FileSystemWindows::FileSystemWindows() {
    LOG_TRACE(LogCategory::PLATFORM, "FileSystemWindows constructor");
}

FileSystemWindows::~FileSystemWindows() {
    LOG_TRACE(LogCategory::PLATFORM, "FileSystemWindows destructor");
}

std::string FileSystemWindows::getConfigDirectory() const {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
        return std::string(path) + "\\PhantomWriter";
    }
    return ".\\phantom-writer";
}

std::string FileSystemWindows::getDocumentsDirectory() const {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_MYDOCUMENTS, nullptr, 0, path))) {
        return std::string(path);
    }
    return ".";
}

std::string FileSystemWindows::getTempDirectory() const {
    char path[MAX_PATH];
    DWORD result = GetTempPathA(MAX_PATH, path);
    if (result > 0 && result < MAX_PATH) {
        return std::string(path);
    }
    return ".\\temp";
}

bool FileSystemWindows::fileExists(const std::string& path) const {
    struct _stat buffer;
    return (_stat(path.c_str(), &buffer) == 0);
}

bool FileSystemWindows::createDirectory(const std::string& path) const {
    return CreateDirectoryA(path.c_str(), nullptr) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
}

char FileSystemWindows::getPathSeparator() const {
    return '\\';
}

} // namespace phantom

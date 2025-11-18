#include "file_system_android.h"
#include "utils/logger.h"
#include <sys/stat.h>
#include <unistd.h>

namespace phantom {

FileSystemAndroid::FileSystemAndroid() {
    LOG_TRACE(LogCategory::PLATFORM, "FileSystemAndroid constructor");
}

FileSystemAndroid::~FileSystemAndroid() {
    LOG_TRACE(LogCategory::PLATFORM, "FileSystemAndroid destructor");
}

void FileSystemAndroid::setAndroidApp(android_app* app) {
    app_ = app;
}

std::string FileSystemAndroid::getConfigDirectory() const {
    if (!app_ || !app_->activity) {
        LOG_ERROR(LogCategory::PLATFORM, "Android app not initialized");
        return "/data/local/tmp";
    }

    // Use internal storage directory
    const char* internalDataPath = app_->activity->internalDataPath;
    if (internalDataPath) {
        return std::string(internalDataPath) + "/config";
    }

    return "/data/local/tmp";
}

std::string FileSystemAndroid::getDocumentsDirectory() const {
    if (!app_ || !app_->activity) {
        LOG_ERROR(LogCategory::PLATFORM, "Android app not initialized");
        return "/sdcard/Documents";
    }

    const char* externalDataPath = app_->activity->externalDataPath;
    if (externalDataPath) {
        return std::string(externalDataPath) + "/documents";
    }

    return "/sdcard/Documents";
}

std::string FileSystemAndroid::getTempDirectory() const {
    if (!app_ || !app_->activity) {
        return "/data/local/tmp";
    }

    const char* internalDataPath = app_->activity->internalDataPath;
    if (internalDataPath) {
        return std::string(internalDataPath) + "/cache";
    }

    return "/data/local/tmp";
}

bool FileSystemAndroid::fileExists(const std::string& path) const {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool FileSystemAndroid::createDirectory(const std::string& path) const {
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
}

char FileSystemAndroid::getPathSeparator() const {
    return '/';
}

} // namespace phantom

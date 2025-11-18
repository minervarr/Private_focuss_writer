#ifndef PHANTOM_FILE_SYSTEM_ANDROID_H
#define PHANTOM_FILE_SYSTEM_ANDROID_H

#include "../platform_interface.h"
#include <android_native_app_glue.h>

namespace phantom {

class FileSystemAndroid : public IPlatformFileSystem {
public:
    FileSystemAndroid();
    ~FileSystemAndroid() override;

    void setAndroidApp(android_app* app);

    // IPlatformFileSystem interface
    std::string getConfigDirectory() const override;
    std::string getDocumentsDirectory() const override;
    std::string getTempDirectory() const override;
    bool fileExists(const std::string& path) const override;
    bool createDirectory(const std::string& path) const override;
    char getPathSeparator() const override;

private:
    android_app* app_ = nullptr;
};

} // namespace phantom

#endif // PHANTOM_FILE_SYSTEM_ANDROID_H

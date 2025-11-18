#ifndef PHANTOM_FILE_SYSTEM_LINUX_H
#define PHANTOM_FILE_SYSTEM_LINUX_H

#include "../platform_interface.h"

namespace phantom {

class FileSystemLinux : public IPlatformFileSystem {
public:
    FileSystemLinux();
    ~FileSystemLinux() override;

    // IPlatformFileSystem interface
    std::string getConfigDirectory() const override;
    std::string getDocumentsDirectory() const override;
    std::string getTempDirectory() const override;
    bool fileExists(const std::string& path) const override;
    bool createDirectory(const std::string& path) const override;
    char getPathSeparator() const override;
};

} // namespace phantom

#endif // PHANTOM_FILE_SYSTEM_LINUX_H

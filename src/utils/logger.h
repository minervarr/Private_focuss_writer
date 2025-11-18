#ifndef PHANTOM_LOGGER_H
#define PHANTOM_LOGGER_H

#include <string>
#include <cstdarg>

namespace phantom {

enum class LogLevel {
    TRACE,    // Solo Debug - Detalles extremos
    DEBUG,    // Solo Debug - Info de desarrollo
    INFO,     // Ambos - Info general
    WARN,     // Ambos - Advertencias
    ERR,      // Ambos - Errores (renamed from ERROR to avoid Windows macro conflict)
    FATAL     // Ambos - Errores críticos
};

namespace LogCategory {
    constexpr const char* INIT = "INIT";
    constexpr const char* RENDER = "RENDER";
    constexpr const char* INPUT = "INPUT";
    constexpr const char* BUFFER = "BUFFER";
    constexpr const char* VULKAN = "VULKAN";
    constexpr const char* PLATFORM = "PLATFORM";
    constexpr const char* PERSISTENCE = "PERSISTENCE";
    constexpr const char* UI = "UI";
}

class Logger {
public:
    static void init(const std::string& logFilePath = "phantom_writer.log");
    static void shutdown();

    static void log(LogLevel level, const char* file, int line, const char* category, const char* format, ...);

    static void setConsoleOutput(bool enabled);
    static void setFileOutput(bool enabled);

private:
    Logger() = default;
    static Logger& getInstance();

    void logInternal(LogLevel level, const char* file, int line, const char* category, const char* format, va_list args);

    const char* getLevelString(LogLevel level);

    bool consoleOutput_ = true;
    bool fileOutput_ = true;
    std::string logFilePath_;
    void* logFile_ = nullptr; // FILE* pero sin incluir stdio aquí
};

// Macros convenientes
#ifdef NDEBUG
    #define LOG_TRACE(...) ((void)0)
    #define LOG_DEBUG(...) ((void)0)
#else
    #define LOG_TRACE(category, ...) phantom::Logger::log(phantom::LogLevel::TRACE, __FILE__, __LINE__, category, __VA_ARGS__)
    #define LOG_DEBUG(category, ...) phantom::Logger::log(phantom::LogLevel::DEBUG, __FILE__, __LINE__, category, __VA_ARGS__)
#endif

#define LOG_INFO(category, ...)  phantom::Logger::log(phantom::LogLevel::INFO, __FILE__, __LINE__, category, __VA_ARGS__)
#define LOG_WARN(category, ...)  phantom::Logger::log(phantom::LogLevel::WARN, __FILE__, __LINE__, category, __VA_ARGS__)
#define LOG_ERROR(category, ...) phantom::Logger::log(phantom::LogLevel::ERR, __FILE__, __LINE__, category, __VA_ARGS__)
#define LOG_FATAL(category, ...) phantom::Logger::log(phantom::LogLevel::FATAL, __FILE__, __LINE__, category, __VA_ARGS__)

} // namespace phantom

#endif // PHANTOM_LOGGER_H

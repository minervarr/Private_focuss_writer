#include "logger.h"
#include <cstdio>
#include <ctime>
#include <cstring>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace phantom {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::init(const std::string& logFilePath) {
    Logger& logger = getInstance();
    logger.logFilePath_ = logFilePath;

    if (logger.fileOutput_) {
        logger.logFile_ = fopen(logFilePath.c_str(), "w");
        if (!logger.logFile_) {
            fprintf(stderr, "[LOGGER] Failed to open log file: %s\n", logFilePath.c_str());
        }
    }
}

void Logger::shutdown() {
    Logger& logger = getInstance();
    if (logger.logFile_) {
        fclose(static_cast<FILE*>(logger.logFile_));
        logger.logFile_ = nullptr;
    }
}

void Logger::setConsoleOutput(bool enabled) {
    getInstance().consoleOutput_ = enabled;
}

void Logger::setFileOutput(bool enabled) {
    getInstance().fileOutput_ = enabled;
}

void Logger::log(LogLevel level, const char* file, int line, const char* category, const char* format, ...) {
    va_list args;
    va_start(args, format);
    getInstance().logInternal(level, file, line, category, format, args);
    va_end(args);
}

const char* Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERR:   return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

void Logger::logInternal(LogLevel level, const char* file, int line, const char* category, const char* format, va_list args) {
    // Obtener timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    struct tm* timeinfo = localtime(&time_t_now);

    char timestamp[64];
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
        timeinfo->tm_year + 1900,
        timeinfo->tm_mon + 1,
        timeinfo->tm_mday,
        timeinfo->tm_hour,
        timeinfo->tm_min,
        timeinfo->tm_sec,
        static_cast<long>(ms.count()));

    // Extraer solo el nombre del archivo (sin path completo)
    const char* filename = strrchr(file, '/');
    if (filename) {
        filename++; // Saltar el '/'
    } else {
        filename = strrchr(file, '\\');
        if (filename) {
            filename++; // Saltar el '\'
        } else {
            filename = file;
        }
    }

    // Formatear el mensaje
    char message[1024];
    vsnprintf(message, sizeof(message), format, args);

    // Formato: [TIMESTAMP][LEVEL][CATEGORY][FILE:LINE] Message
    char logLine[2048];
    snprintf(logLine, sizeof(logLine), "[%s][%s][%s][%s:%d] %s\n",
        timestamp,
        getLevelString(level),
        category,
        filename,
        line,
        message);

    // Output a consola
    if (consoleOutput_) {
        // Colorear según nivel (solo en terminal)
        const char* colorCode = "";
        const char* resetCode = "\033[0m";

        switch (level) {
            case LogLevel::TRACE: colorCode = "\033[90m"; break;  // Gris
            case LogLevel::DEBUG: colorCode = "\033[36m"; break;  // Cyan
            case LogLevel::INFO:  colorCode = "\033[32m"; break;  // Verde
            case LogLevel::WARN:  colorCode = "\033[33m"; break;  // Amarillo
            case LogLevel::ERR:   colorCode = "\033[31m"; break;  // Rojo
            case LogLevel::FATAL: colorCode = "\033[91m"; break;  // Rojo brillante
        }

        fprintf(stdout, "%s%s%s", colorCode, logLine, resetCode);
        fflush(stdout);
    }

    // Output a archivo
    if (fileOutput_ && logFile_) {
        fprintf(static_cast<FILE*>(logFile_), "%s", logLine);
        fflush(static_cast<FILE*>(logFile_));
    }
}

} // namespace phantom

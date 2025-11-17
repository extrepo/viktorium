#ifndef UNILOGGER_H
#define UNILOGGER_H
#include "experimental/filesystem"
#include <sstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/logger.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/bundled/printf.h>
#include "unilog/log_utils.h"
#include "unilog/logger_entity.h"
#include "unilog/sink_entity.h"
#ifdef QT_CORE_LIB
#  include <QString>
#  include <QDebug>
#endif
/**
 * Класс логгера
 */
class UniLogger final {
public:
    struct LogStream : public std::ostringstream {
    public:
        LogStream(std::string l_name, const spdlog::source_loc& _loc, spdlog::level::level_enum _lvl, std::string _prefix);
        ~LogStream();
        void flush();
    private:
        std::string log_name;
        spdlog::source_loc loc;
        spdlog::level::level_enum lvl = spdlog::level::info;
        std::string prefix;
    };
#ifdef QT_CORE_LIB
    /**
     * Вспомогательный класс для Qt.
     * Перенаправляет в поток QDebug и затем записывает в соответствующий лог
     */
    class LogStreamQt {
    public:
        explicit LogStreamQt(QString _log_name, spdlog::level::level_enum _lvl) :
            lvl(_lvl),
            log_name(_log_name),
            qtDebug(&buffer) {}
        ~LogStreamQt() noexcept(false);
        QDebug &stream() { return qtDebug; }
    private:
        void writeToLog();
        spdlog::level::level_enum lvl;
        QString log_name;
        QString buffer;
        QDebug qtDebug;
    };
#endif
public:
    /**
     * Функция возвращающая указатель на логгер-синглтон
     * Многопоточная реализация
     */
    static UniLogger& getInstance();
    /**
     * Инициализация синглтона логгера
     */
    bool init(std::string conf_file_path, std::string log_file_path);
    /**
     * Зарегистрировать логгер из сущности
     * Возвращает указатель на зарегистрированный логгер
     */
    std::shared_ptr<spdlog::logger> regLogger(LoggerEntity &log_ent, std::string &pattern);
    /**
     * Зарегистрировать синк из сущности
     * Возвращает указатель на зарегистрированный синк spdlog::sinks::sink
     */
    std::shared_ptr<spdlog::sinks::sink> regSink(SinkEntity *ent_sinks);
    /**
     * Указатель на логгер spdlog::logger по его наименованию
     */
    std::shared_ptr<spdlog::logger> getLogger(std::string logger_name);
    void shutdown() { spdlog::shutdown(); }
    template <typename... Args>
    void log(std::string l_name,
             const spdlog::source_loc &loc,
             spdlog::level::level_enum lvl,
             const char* fmt,
             const Args &... args);
    template <typename... Args>
    void printf(const spdlog::source_loc& loc,
                spdlog::level::level_enum lvl,
                const char* fmt,
                const Args &... args);
private:
    UniLogger() = default;
    ~UniLogger() = default;
    UniLogger(const UniLogger&) = delete;
    void operator=(const UniLogger&) = delete;
private:
    /**
     * Признак инициализирован ли логгер
     */
    std::atomic_bool is_inited_;
    /**
     * Расположение и имя файла конфигурации
     */
    std::string conf_path_;
    /**
     * Расположение файлов-логов
     */
    std::string log_path_;
    /**
     * Мьютекс для контроля создания логгера при многопоточности
     */
    static std::mutex locker_;
};
// получить имя файла (отбрасываем путь к файлу)
#ifdef QT_CORE_LIB
/// Independent loggers
#define L_TRACE(logger) UniLogger::LogStreamQt(logger, spdlog::level::trace).stream()
#define L_DEBUG(logger) UniLogger::LogStreamQt(logger, spdlog::level::debug).stream()
#define L_INFO(logger) UniLogger::LogStreamQt(logger, spdlog::level::info).stream()
#define L_WARN(logger) UniLogger::LogStreamQt(logger, spdlog::level::warn).stream()
#define L_ERROR(logger) UniLogger::LogStreamQt(logger, spdlog::level::err).stream()
#define L_FATAL(logger) UniLogger::LogStreamQt(logger, spdlog::level::critical).stream()
/// Main logger
#define G_TRACE() L_TRACE("Main")
#define G_DEBUG() L_DEBUG("Main")
#define G_INFO() L_INFO("Main")
#define G_WARN() L_WARN("Main")
#define G_ERROR() L_ERROR("Main")
#define G_FATAL() L_FATAL("Main")
#else
#define LOGGER_CALL(logger, level, ...) (UniLogger::getInstance().getLogger(logger))->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, level, __VA_ARGS__)
/// Independent loggers
#define L_TRACE(logger) UniLogger::LogStream(logger, {__FILE__, __LINE__, __FUNCTION__}, spdlog::level::trace, "")
#define L_DEBUG(logger) UniLogger::LogStream(logger, {__FILE__, __LINE__, __FUNCTION__}, spdlog::level::debug, "")
#define L_INFO(logger) UniLogger::LogStream(logger, {__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, "")
#define L_WARN(logger) UniLogger::LogStream(logger, {__FILE__, __LINE__, __FUNCTION__}, spdlog::level::warn, "")
#define L_ERROR(logger) UniLogger::LogStream(logger, {__FILE__, __LINE__, __FUNCTION__}, spdlog::level::err, "")
#define L_FATAL(logger) UniLogger::LogStream(logger, {__FILE__, __LINE__, __FUNCTION__}, spdlog::level::critical, "")
/// Main logger
#define G_TRACE() L_TRACE("Main")
#define G_DEBUG() L_DEBUG("Main")
#define G_INFO() L_INFO("Main")
#define G_WARN() L_WARN("Main")
#define G_ERROR() L_ERROR("Main")
#define G_FATAL() L_FATAL("Main")
//#define LOG_ERROR(logger, ...) LOGGER_CALL(logger, spdlog::level::err, __VA_ARGS__)
#define STM_ERROR(logger) UniLogger::LogStream(logger,{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::err, "")
#endif
#endif // UNILOGGER_H

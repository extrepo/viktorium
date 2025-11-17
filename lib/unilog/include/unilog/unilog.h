#ifndef UNILOG_H
#define UNILOG_H
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
class UniLog final {
public:
    /**
     * Наименование глобального логгера по умолчанию
     */
    static const char* LOGGER_MAIN_NAME;
    /**
     * Наименование синка глобального логгера - для записи сообщений error-level
     */
    static const char* SINK_ERROR_LOG_NAME;
    /**
     * Наименование синка глобального логгера - для записи сообщений любого уровня
     */
    static const char* SINK_FULL_LOG_NAME;
public:
    /**
     * Функция возвращающая указатель на логгер-синглтон
     * Многопоточная реализация
     */
    static UniLog& getInstance();
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
    /**
     * Функция для высвобождения всех ресурсов и потоков
     */
    void shutdown() { spdlog::shutdown(); }
    /**
     * Вспомогательная структура для логгера.
     * Перенаправляет в поток и затем записывает в соответствующий лог при вызове деструктора
     */
    struct LogStream : public std::ostringstream {
    public:
        /**
         * Конструктор для перенаправления в поток
         */
        LogStream(std::string l_name, const spdlog::source_loc& _loc, spdlog::level::level_enum _lvl, std::string _prefix) :
            log_name(l_name),
            loc(_loc),
            lvl(_lvl),
            prefix(_prefix) {}
        /**
         * Деструктор
         */
        ~LogStream() {
            flush();
        }
        /**
         * Функция для сброса сообщения в логгер
         */
        void flush();
    private:
        /**
         * Наименование логгера для записи сообщения
         */
        std::string log_name;
        /**
         * Расположение вызова логгера
         */
        spdlog::source_loc loc;
        /**
         * Уровень лог-сообщения
         */
        spdlog::level::level_enum lvl = spdlog::level::info;
        /**
         * Префикс сообщения
         */
        std::string prefix;
    };
#ifdef QT_CORE_LIB
    /**
     * Вспомогательный класс для Qt.
     * Перенаправляет в поток QDebug и затем записывает в соответствующий лог
     */
    class LogStreamQt {
    public:
        /**
         * Конструктор для перенаправления в поток
         */
        explicit LogStreamQt(QString _log_name, spdlog::level::level_enum _lvl) :
            lvl(_lvl),
            log_name(_log_name),
            qtDebug(&buffer) {}
        /**
         * Деструктор
         */
        ~LogStreamQt() noexcept(false);
        /**
         * Функция возвращает поток для записи сообщения
         */
        QDebug &stream() { return qtDebug; }
    private:
        /**
         * Функция для сброса сообщения в логгер
         */
        void flush();
        /**
         * Наименование логгера для записи сообщения
         */
        QString log_name;
        /**
         * Уровень лог-сообщения
         */
        spdlog::level::level_enum lvl;
        /**
         * Буфер для хранения сообщения
         */
        QString buffer;
        /**
         * Встроенный консольный Qt логгер qDebug
         */
        QDebug qtDebug;
    };
#endif
private:
    UniLog() = default;
    ~UniLog() = default;
    UniLog(const UniLog&) = delete;
    void operator=(const UniLog&) = delete;
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
     * Мьютекс для контроля операций логгера при многопоточности
     */
    static std::mutex locker_;
};
#ifdef QT_CORE_LIB
/**
 * Запись сообщения в заданный логгер
 */
#define L_TRACE(logger) UniLog::LogStreamQt(logger, spdlog::level::trace).stream()
#define L_DEBUG(logger) UniLog::LogStreamQt(logger, spdlog::level::debug).stream()
#define L_INFO(logger) UniLog::LogStreamQt(logger, spdlog::level::info).stream()
#define L_WARN(logger) UniLog::LogStreamQt(logger, spdlog::level::warn).stream()
#define L_ERROR(logger) UniLog::LogStreamQt(logger, spdlog::level::err).stream()
#define L_FATAL(logger) UniLog::LogStreamQt(logger, spdlog::level::critical).stream()
/// Main logger
#define G_TRACE() L_TRACE(UniLog::LOGGER_MAIN_NAME)
#define G_DEBUG() L_DEBUG(UniLog::LOGGER_MAIN_NAME)
#define G_INFO() L_INFO(UniLog::LOGGER_MAIN_NAME)
#define G_WARN() L_WARN(UniLog::LOGGER_MAIN_NAME)
#define G_ERROR() L_ERROR(UniLog::LOGGER_MAIN_NAME)
#define G_FATAL() L_FATAL(UniLog::LOGGER_MAIN_NAME)
#else
#define LOGGER_CALL(logger, level, ...) (UniLog::getInstance().getLogger(logger))->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, level, __VA_ARGS__)
/// Independent loggers
#define L_TRACE(logger) UniLog::LogStream(logger, {__FILE__, __LINE__, __FUNCTION__}, spdlog::level::trace, "")
#define L_DEBUG(logger) UniLog::LogStream(logger, {__FILE__, __LINE__, __FUNCTION__}, spdlog::level::debug, "")
#define L_INFO(logger) UniLog::LogStream(logger, {__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, "")
#define L_WARN(logger) UniLog::LogStream(logger, {__FILE__, __LINE__, __FUNCTION__}, spdlog::level::warn, "")
#define L_ERROR(logger) UniLog::LogStream(logger, {__FILE__, __LINE__, __FUNCTION__}, spdlog::level::err, "")
#define L_FATAL(logger) UniLog::LogStream(logger, {__FILE__, __LINE__, __FUNCTION__}, spdlog::level::critical, "")
/// Main logger
#define G_TRACE() L_TRACE(UniLog::LOGGER_MAIN_NAME)
#define G_DEBUG() L_DEBUG(UniLog::LOGGER_MAIN_NAME)
#define G_INFO() L_INFO(UniLog::LOGGER_MAIN_NAME)
#define G_WARN() L_WARN(UniLog::LOGGER_MAIN_NAME)
#define G_ERROR() L_ERROR(UniLog::LOGGER_MAIN_NAME)
#define G_FATAL() L_FATAL(UniLog::LOGGER_MAIN_NAME)
#endif
#endif // UNILOG_H

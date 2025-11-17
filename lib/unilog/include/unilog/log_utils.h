#ifndef LOG_UTILS_H
#define LOG_UTILS_H
#include <string>
#include <vector>
#include <mutex>
#include <map>
#include "unilog/sink_entity.h"
#include "unilog/logger_entity.h"
#include "unilog/tpool_entity.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
/**
 * Полезные статические функции
 */
class LogUtils {
public:
    /**
     * Параметр - Синки в конфигурации
     */
    static const char* CONFIG_KEYWORD_SINKS;
    /**
     * Параметр - Логгеры в конфигурации
     */
    static const char* CONFIG_KEYWORD_LOGGERS;
    /**
     * Параметр - Паттерны лога в конфигурации
     */
    static const char* CONFIG_KEYWORD_PATTERNS;
    /**
     * Значение параметра - паттерн лога по умолчанию
     */
    static const char* DEFAULT_PATTERN;
    /**
     * Параметр - Пулл потоков для логера
     */
    static const char* CONFIG_KEYWORD_THREADPOOL;
public:
    /**
     * Чтение содержимого из файла по указанному пути
     */
    static rapidjson::Document readJSON(std::string &file_path);
    /**
     * Запись JSON-документа в файл по указанному пути
     */
    static bool writeJSON(rapidjson::Document &doc_json, std::string &file_path);
    /**
     * Получить параметры логгеров из JSON
     */
    static bool readParamLoggers(std::string &file_path, std::vector<LoggerEntity> &loggers);
    /**
     * Записать параметры логгера в JSON
     */
    static bool writeParamLogger(std::string &file_path, LoggerEntity &logger);
    /**
     * Получить параметры паттернов из JSON
     */
    static bool readParamPatterns(std::string &file_path, std::map<std::string, std::string> &patterns);
    /**
     * Записать параметры паттернов в JSON
     */
    static bool writeParamPatterns(std::string &file_path, std::string p_name, std::string p_value);
    /**
     * Получить параметры пула потоков
     */
    static bool readParamTPool(std::string &file_path, TPoolEntity &pool_entity);
    /**
     * Записать параметры пула потоков
     */
    static bool writeParamTPool(std::string &file_path, TPoolEntity &pool_entity);
    /**
     * Записать ключевые метки в файл конфигурации
     * (используется при создании файла конфигурации)
     */
    static bool writeKeywords(std::string &file_path);
    /**
     * Установить расположение для всех лог-файлов
     * [заданная_директория]/[имя_синка].log
     */
    static bool setLogPath(std::vector<LoggerEntity> &loggers, std::string log_path);
public:
    /**
     * Зарегистрированные синки для логгеров
     */
    static std::vector<std::string> reg_sinks;
    /**
     * Зарегистрированные логгеры
     */
    static std::vector<std::string> reg_loggers;
    /**
     * Зарегистрированные паттерны для сообщений логгера
     */
    static std::vector<std::string> reg_patterns;
private:
    /**
     * Получить параметры синка из JSON
     */
    static bool readParamSink(rapidjson::Document &doc_json, SinkEntity *sink);
    /**
     * Записать параметры синка в JSON
     */
    static bool writeParamSink(std::string &file_path, SinkEntity *sink);
    /**
     * Проверка содержится ли элемент в векторе
     */
    static bool isUnique(std::vector<std::string> vector, std::string element);
    /**
     * Мьютекс на процедуры чтения/записи в конфигурационный файл
     */
    static std::mutex file_lock;
};
#endif // LOG_UTILS_H

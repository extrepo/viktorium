#ifndef LOGGERENTITY_H
#define LOGGERENTITY_H
#include <string>
#include <vector>
#include "sink_entity.h"
/**
 * Класс сущности логгера
 */
class LoggerEntity
{
public:
    /**
     * Значение параметра - наименование логгера по умолчанию
     */
    static const char* LOGGER_NAME_DEFAULT;
    /**
     * Наименование параметра - синки подсоединенные к логгеру
     */
    static const char* LOGGER_SINKS;
    /**
     * Наименование параметра - паттерн логгера для выводимых сообщений
     */
    static const char* LOGGER_PATTERN;
    /**
     * Значение параметра - паттерн логгера для выводимых сообщений по-умолчанию
     */
    static const char* LOGGER_PATTERN_DEFAULT;
    /**
     * Наименование параметра - уровень логирования
     * (начиная с которого должна производится запись в файл лога)
     */
    static const char* LOGGER_LOG_LEVEL;
    /**
     * Значение параметра - уровень логирования по-умолчанию
     */
    static const char* LOGGER_LOG_LEVEL_DEFAULT;
    /**
     * Наименование параметра - Тип записи логируемых сообщений
     */
    static const char* LOGGER_TYPE;
    /**
     * Значение параметра - синхронная запись
     */
    static const char* LOGGER_TYPE_SYNCH;
    /**
     * Значение параметра - асинхронная запись
     */
    static const char* LOGGER_TYPE_ASYNCH;
    /**
     * Значение параметра - асинхронная запись
     */
    static const char* LOGGER_TYPE_ASYNCH_NB;
    /**
     * Значение параметра - по-умолчанию
     */
    static const char* LOGGER_TYPE_DEFAULT;
public:
    /**
     * Конструктор логгера
     */
    LoggerEntity(std::string _logger_name = LOGGER_NAME_DEFAULT,
                 std::vector<SinkEntity*> _logger_sinks = std::vector<SinkEntity*>(),
                 std::string _logger_pattern = LOGGER_PATTERN_DEFAULT,
                 std::string _logger_level = "info",
                 std::string _logger_type = LOGGER_TYPE_DEFAULT);
    /**
     * Установить имя логгера
     */
    void setLoggerName(std::string _logger_name);
    /**
     * Получить имя логгера
     */
    std::string getLoggerName();
    /**
     * Установить список синков, подключенных к логгеру
     */
    void setSinks(std::vector<SinkEntity*> _logger_sinks);
    /**
     * Получить список синков, подключенных к логгеру
     */
    std::vector<SinkEntity*> getSinks();
    /**
     * Установить паттерн для выводимых логгером сообщений
     */
    void setPatternName(std::string _pattern);
    /**
     * Получить паттерн выводимых логгером сообщений
     */
    std::string getPatternName();
    /**
     * Установить уровень логирования
     */
    void setLevel(std::string _level);
    /**
     * Получить текущий уровень логирования
     */
    std::string getLevel();
    /**
     * Установить тип записи логируемых сообщений
     */
    void setSynchType(std::string _synch_type);
    /**
     * Получить текущий тип записи логируемых сообщений
     */
    std::string getSynchType();
    /**
     * Сериализовать объект класса в JSON
     */
    rapidjson::Document toJSON();
    /**
     * Десериализовать в объект класса из JSON
     */
    bool fromJSON(const rapidjson::Value& doc_json);
private:
    /**
     * Имя логгера
     */
    std::string logger_name;
    /**
     * Синки логгера
     */
    std::vector<SinkEntity*> sinks;
    /**
     * Паттерн логгера для записываемых сообщений
     */
    std::string pattern;
    /**
     * Уровень логирования
     */
    std::string log_level;
    /**
     * Тип записи логируемых сообщений
     */
    std::string synch_type;
    /**
     * Возможные значения для типа записи логируемых сообщений
     */
    std::vector<std::string> exist_types;
};

#endif // LOGGERENTITY_H

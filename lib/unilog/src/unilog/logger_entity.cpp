#include "unilog/logger_entity.h"
#include <spdlog/common.h>
#include <iostream>
/**
 * Значение параметра - наименование логгера по умолчанию
 */
const char* LoggerEntity::LOGGER_NAME_DEFAULT = "console_logger";
/**
 * Наименование параметра - синки подсоединенные к логгеру
 */
const char* LoggerEntity::LOGGER_SINKS = "sinks";
/**
 * Наименование параметра - паттерн логгера для выводимых сообщений
 */
const char* LoggerEntity::LOGGER_PATTERN = "pattern";
/**
 * Значение параметра - паттерн логгера для выводимых сообщений по-умолчанию
 */
const char* LoggerEntity::LOGGER_PATTERN_DEFAULT = "general_pattern";
/**
 * Наименование параметра - уровень логирования
 * (начиная с которого должна производится запись в файл лога)
 */
const char* LoggerEntity::LOGGER_LOG_LEVEL = "level";
/**
 * Значение параметра - уровень логирования по-умолчанию
 */
const char* LoggerEntity::LOGGER_LOG_LEVEL_DEFAULT = "info";
/**
 * Наименование параметра - Тип записи логируемых сообщений
 */
const char* LoggerEntity::LOGGER_TYPE = "sync_type";
/**
 * Значение параметра - синхронная запись
 */
const char* LoggerEntity::LOGGER_TYPE_SYNCH = "sync";
/**
 * Значение параметра - асинхронная запись
 */
const char* LoggerEntity::LOGGER_TYPE_ASYNCH = "async";
/**
 * Значение параметра - асинхронная запись
 */
const char* LoggerEntity::LOGGER_TYPE_ASYNCH_NB = "async_nb";
/**
 * Значение параметра - по-умолчанию
 */
const char* LoggerEntity::LOGGER_TYPE_DEFAULT = LOGGER_TYPE_ASYNCH;
/**
 * Конструктор логгера
 */
LoggerEntity::LoggerEntity(std::string _logger_name,
                           std::vector<SinkEntity*> _logger_sinks,
                           std::string _logger_pattern,
                           std::string _logger_level,
                           std::string _logger_type):
    logger_name(_logger_name),
    sinks(_logger_sinks),
    pattern(_logger_pattern),
    log_level(_logger_level),
    synch_type(_logger_type)
{
    // формируем список из доступных типов логирования
    exist_types.push_back(LOGGER_TYPE_SYNCH);
    exist_types.push_back(LOGGER_TYPE_ASYNCH);
    exist_types.push_back(LOGGER_TYPE_ASYNCH_NB);
}
/**
 * Установить имя логгера
 */
void LoggerEntity::setLoggerName(std::string _logger_name) {
    if(_logger_name.empty()) _logger_name = LOGGER_NAME_DEFAULT;
    if(logger_name.compare(_logger_name) == 0) return;
    logger_name = _logger_name;
}
/**
 * Получить имя логгера
 */
std::string LoggerEntity::getLoggerName() {
    return logger_name;
}
/**
 * Установить список синков, подключенных к логгеру
 */
void LoggerEntity::setSinks(std::vector<SinkEntity*> _logger_sinks) {
    if(_logger_sinks.empty()) {
        std::cerr << "Attempted to set empty sinks to logger \"" << logger_name << "\"";
        return;
    }
    sinks = _logger_sinks;
}
/**
 * Получить список синков, подключенных к логгеру
 */
std::vector<SinkEntity*> LoggerEntity::getSinks() {
    return sinks;
}
/**
 * Установить паттерн для выводимых логгером сообщений
 */
void LoggerEntity::setPatternName(std::string _pattern) {
    if(_pattern.empty()) _pattern = LOGGER_PATTERN_DEFAULT;
    if(_pattern.compare(pattern) == 0) return;
    pattern = _pattern;
}
/**
 * Получить паттерн выводимых логгером сообщений
 */
std::string LoggerEntity::getPatternName() {
    return pattern;
}
/**
 * Установить уровень логирования
 */
void LoggerEntity::setLevel(std::string _level) {
    if(log_level == _level) return;
    // проверяем, есть ли такой тип среди известных
    spdlog::string_view_t exist_levels[] SPDLOG_LEVEL_NAMES;
    for (const auto &level_str : exist_levels)
    {
        if (level_str == _level) {
            log_level = _level;
            return;
        }
    }
    // если совпадений не нашлось - попадаем сюда
    std::cerr << "Attempted to set unknown type of level \"" << _level
              << "\" for logger: \"" << logger_name << "\"";
    return;
}
/**
 * Получить текущий уровень логирования
 */
std::string LoggerEntity::getLevel() {
    return log_level;
}
/**
 * Установить тип записи логируемых сообщений
 */
void LoggerEntity::setSynchType(std::string _synch_type) {
    // если пустой - задаем тип по-умолчанию
    if(_synch_type.empty()) _synch_type = LOGGER_TYPE_DEFAULT;
    // проверяем, есть ли такой тип среди известных
    std::vector<std::string>::iterator it = std::find(exist_types.begin(), exist_types.end(), _synch_type);
    if(it == exist_types.end()) {
        std::cerr << "Attempted to set unknown type \"" << _synch_type
                  << "\" for logger: \"" << logger_name << "\"" << std::endl;
        return;
    }
    // если не изменился - выходим
    if(synch_type.compare(_synch_type) == 0) return;
    synch_type = _synch_type;
}
/**
 * Получить текущий тип записи логируемых сообщений
 */
std::string LoggerEntity::getSynchType() {
    return synch_type;
}
/**
 * Сериализовать объект класса в JSON
 */
rapidjson::Document LoggerEntity::toJSON() {
    rapidjson::Document d_write;
    d_write.SetObject();
    auto &allocator =  d_write.GetAllocator();
    // Создаем объект логгера
    rapidjson::Value logger_objects;
    logger_objects.SetObject();
    // массив синков
    std::vector<SinkEntity*> sinks = this->getSinks();
    rapidjson::Value array_main;
    array_main.SetArray();
    for(size_t i = 0; i < sinks.size(); i++) {
        rapidjson::Value value(sinks.at(i)->getSinkName().c_str(), allocator);
        array_main.PushBack(value, allocator);
    }
    // записываем параметры
    logger_objects.AddMember(rapidjson::StringRef(LoggerEntity::LOGGER_SINKS),
                             array_main,
                             allocator);
    rapidjson::Value pattern_value = rapidjson::Value(this->getPatternName().c_str(), allocator);
    logger_objects.AddMember(rapidjson::StringRef(LoggerEntity::LOGGER_PATTERN), pattern_value, allocator);
    rapidjson::Value log_value = rapidjson::Value(this->getLevel().c_str(), allocator);
    logger_objects.AddMember(rapidjson::StringRef(LoggerEntity::LOGGER_LOG_LEVEL), log_value, allocator);
    rapidjson::Value type_value = rapidjson::Value(this->getSynchType().c_str(), allocator);
    logger_objects.AddMember(rapidjson::StringRef(LoggerEntity::LOGGER_TYPE), type_value, allocator);
    d_write.CopyFrom(logger_objects, allocator);
    return d_write;
}
/**
 * Десериализовать в объект класса из JSON
 */
bool LoggerEntity::fromJSON(const rapidjson::Value& params) {
    // массив синков логгера
    std::vector<SinkEntity*> logger_sinks;
    if(params.HasMember(LoggerEntity::LOGGER_SINKS) && params[LoggerEntity::LOGGER_SINKS].IsArray()) {
        rapidjson::Document::ConstArray sinks = params[LoggerEntity::LOGGER_SINKS].GetArray();
        for(rapidjson::Value::ConstValueIterator i = sinks.Begin(); i != sinks.End(); i++) {
            if(!i->IsString()) {
                std::cerr << "Error parsing sink - incorrect sink typename in configuration" << std::endl;
                return false;
            }
            // выставляем для всех синков параметр - наименование
            SinkEntity *sink = new SinkEntity;
            sink->setSinkName(i->GetString());
            logger_sinks.push_back(sink);
        }
    }
    this->setSinks(logger_sinks);
    // Параметр - паттерн сообщений логера
    if(params.HasMember(LoggerEntity::LOGGER_PATTERN) && params[LoggerEntity::LOGGER_PATTERN].IsString()) {
        this->setPatternName(params[LoggerEntity::LOGGER_PATTERN].GetString());
    }
    // Параметр - уровень логирования
    if(params.HasMember(LoggerEntity::LOGGER_LOG_LEVEL) && params[LoggerEntity::LOGGER_LOG_LEVEL].IsString()) {
        this->setLevel(params[LoggerEntity::LOGGER_LOG_LEVEL].GetString());
    }
    // Параметр - тип записи сообщений логгером
    if(params.HasMember(LoggerEntity::LOGGER_TYPE) && params[LoggerEntity::LOGGER_TYPE].IsString()) {
        this->setSynchType(params[LoggerEntity::LOGGER_TYPE].GetString());
    }
    return true;
}

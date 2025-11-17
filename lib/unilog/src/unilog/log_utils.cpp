#include "unilog/log_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <algorithm>
#include <unordered_map>
/**
 * Мьютекс на процедуры чтения/записи в конфигурационный файл
 */
std::mutex LogUtils::file_lock;
/**
 * Зарегистрированные синки для логгеров
 */
std::vector<std::string> LogUtils::reg_sinks;
/**
 * Зарегистрированные логгеры
 */
std::vector<std::string> LogUtils::reg_loggers;
/**
 * Зарегистрированные паттерны для сообщений логгера
 */
std::vector<std::string> LogUtils::reg_patterns;
/**
 * Параметр - Синки в конфигурации
 */
const char* LogUtils::CONFIG_KEYWORD_SINKS = "SINKS";
/**
 * Параметр - Логгеры в конфигурации
 */
const char* LogUtils::CONFIG_KEYWORD_LOGGERS = "LOGGERS";
/**
 * Параметр - Паттерны лога в конфигурации
 */
const char* LogUtils::CONFIG_KEYWORD_PATTERNS = "PATTERNS";
/**
 * Параметр - Пулл потоков для логера
 */
const char* LogUtils::CONFIG_KEYWORD_THREADPOOL = "THREAD_POOL";
/**
 * Значение параметра - паттерн по умолчанию
 */
const char* LogUtils::DEFAULT_PATTERN = "%Y-%m-%d %H:%M:%S,%e %L [thread-%t] %n : %v";
/**
 * Чтение содержимого из файла по указанному пути
 */
rapidjson::Document LogUtils::readJSON(std::string &file_path)
{
    file_lock.lock();
    rapidjson::Document doc_json;
    // читаем файл
    std::ifstream is;
    is.open(file_path, std::ifstream::in);
    if(is.is_open()) {
        std::stringstream ss;
        ss << is.rdbuf();
        doc_json.Parse(ss.str().c_str());
        if(!doc_json.IsObject()) doc_json = nullptr;
        is.close();
    }
    file_lock.unlock();
    return doc_json;
}
/**
 * Запись JSON-документа в файл по указанному пути
 */
bool LogUtils::writeJSON(rapidjson::Document &doc_json, std::string &file_path) {
    file_lock.lock();
    std::ofstream f;
    f.open(file_path, std::ofstream::trunc);
    if(!f.is_open()) return false;
    else {
        rapidjson::StringBuffer buf;
        buf.Clear();
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
        doc_json.Accept(writer);
        f << buf.GetString();
        f.flush();
    }
    f.close();
    file_lock.unlock();
    return true;
}
/**
 * Записать ключевые метки в файл конфигурации
 * (используется при создании файла конфигурации)
 */
bool LogUtils::writeKeywords(std::string &file_path) {
    rapidjson::Document w_doc;
    // Опредляем документ как объект, а не массив
    w_doc.SetObject();
    auto &allocator = w_doc.GetAllocator();
    w_doc.AddMember(rapidjson::Value(CONFIG_KEYWORD_SINKS, allocator), rapidjson::Value(), allocator);
    w_doc.AddMember(rapidjson::Value(CONFIG_KEYWORD_PATTERNS, allocator), rapidjson::Value(),allocator);
    w_doc.AddMember(rapidjson::Value(CONFIG_KEYWORD_LOGGERS, allocator), rapidjson::Value(), allocator);
    w_doc.AddMember(rapidjson::Value(CONFIG_KEYWORD_THREADPOOL, allocator), rapidjson::Value(), allocator);
    return LogUtils::writeJSON(w_doc, file_path);
}
/**
 * Получить параметры синка из JSON
 */
bool LogUtils::readParamSink(rapidjson::Document &doc_json, SinkEntity *sink) {
    std::string sink_name = sink->getSinkName();
    if(sink_name.empty()) {
        std::cerr << "Error parsing file configuration for logger - searching sink has empty input name" << std::endl;;
        return false;
    }
    if(!doc_json.HasMember(CONFIG_KEYWORD_SINKS)) {
        std::cerr << "Error parsing file configuration for logger" << std::endl;;
        return false;
    }
    // читаем информацию о всех синках
    const rapidjson::Value& r_sinks = doc_json.FindMember(CONFIG_KEYWORD_SINKS)->value;
    // читаем параметры из json
    if(!sink->fromJSON(r_sinks)) return false;
    // смотрим есть ли он среди известных
    if(LogUtils::isUnique(reg_sinks, sink_name)) {
        reg_sinks.push_back(sink_name);
    }
    return true;
}
/**
 * Записать параметры синка в JSON
 */
bool LogUtils::writeParamSink(std::string &file_path, SinkEntity *sink) {
    // читаем файл
    rapidjson::Document d_read = LogUtils::readJSON(file_path);
    if(!d_read.HasMember(LogUtils::CONFIG_KEYWORD_SINKS)) {
        std::cerr << "Error writing sink to file configuration" << std::endl;
        return false;
    }
    // проверяем на уникальность
    if(!isUnique(reg_sinks, sink->getSinkName())) return true;
    reg_sinks.push_back(sink->getSinkName());
    // Добавление синка в конфигурационный файл
    rapidjson::Value &r_sinks = d_read.FindMember(CONFIG_KEYWORD_SINKS)->value;
    if(!r_sinks.IsObject()) r_sinks.SetObject();
    rapidjson::Document sink_json = sink->toJSON();
    auto &allocator = sink_json.GetAllocator();
    rapidjson::Value name(sink->getSinkName().c_str(), allocator);
    r_sinks.AddMember(name, sink_json, allocator);
    // Запись объектов в конфигурационный файл
    return LogUtils::writeJSON(d_read, file_path);
}
/**
 * Получить параметры логгеров из JSON
 */
bool LogUtils::readParamLoggers(std::string &path, std::vector<LoggerEntity> &loggers) {
    // читаем файл
    rapidjson::Document doc_json = LogUtils::readJSON(path);
    if(!doc_json.IsObject() || !doc_json.HasMember(CONFIG_KEYWORD_LOGGERS)) {
        std::cerr << "Error parsing file configuration for logger" << std::endl;;
        return false;
    }
    // читаем информацию о всех логгерах
    const rapidjson::Value& r_loggers = doc_json.FindMember(CONFIG_KEYWORD_LOGGERS)->value;
    for(rapidjson::Value::ConstMemberIterator it = r_loggers.MemberBegin(); it != r_loggers.MemberEnd(); it++) {
        // смотрим есть ли он среди известных
        if(!LogUtils::isUnique(reg_loggers, it->name.GetString())) {
            std::cerr << "Error parsing configuration - redefenition logger " << it->name.GetString() << " for logger" << std::endl;
            return false;
        }
        LoggerEntity logger;
        logger.setLoggerName(it->name.GetString());
        // читаем параметры логгера
        logger.fromJSON(it->value);
        // массив синков логгера
        for(size_t i = 0; i < logger.getSinks().size(); i++) {
            // читаем параметры для синка
            if(!LogUtils::readParamSink(doc_json, logger.getSinks().at(i)))
                return false;
        }
        loggers.push_back(logger);
        // добавляем в список зарегистрированных
        reg_loggers.push_back(logger.getLoggerName());
    }
    return true;
}
/**
 * Записать параметры логгера в JSON
 */
bool LogUtils::writeParamLogger(std::string &file_path, LoggerEntity &logger) {
    // читаем файл
    rapidjson::Document d_read = LogUtils::readJSON(file_path);
    if(!d_read.IsObject() || !d_read.HasMember(LogUtils::CONFIG_KEYWORD_LOGGERS)) {
        std::cerr << "Error writing sink to file configuration" << std::endl;
        return false;
    }
    if(!isUnique(reg_loggers, logger.getLoggerName())) {
         std::cerr << "Error writing logger to file configuration - attempted to rewrite exist logger "
                    << logger.getLoggerName() << std::endl;
        return false;
    }
    reg_loggers.push_back(logger.getLoggerName());
    // Добавляем логгер в конфигурационный файл
    rapidjson::Value &r_loggers = d_read.FindMember(CONFIG_KEYWORD_LOGGERS)->value;
    if(!r_loggers.IsObject()) r_loggers.SetObject();
    rapidjson::Document log_json = logger.toJSON();
    auto &allocator = log_json.GetAllocator();
    rapidjson::Value name(logger.getLoggerName().c_str(), allocator);
    r_loggers.AddMember(name, log_json, allocator);
    // Запись объектов в конфигурационный файл
    if(!LogUtils::writeJSON(d_read, file_path)) return false;
    // записываем синки логгера
    for(size_t i = 0; i < logger.getSinks().size(); i++) {
        SinkEntity *sink = logger.getSinks().at(i);
        if(!LogUtils::writeParamSink(file_path, sink)) return false;
    }
    return true;
}
/**
 * Получить параметры паттернов из JSON
 */
bool LogUtils::readParamPatterns(std::string &file_path, std::map<std::string, std::string> &patterns) {
    // читаем файл
    rapidjson::Document doc_json = LogUtils::readJSON(file_path);
    if(!doc_json.IsObject() || !doc_json.HasMember(CONFIG_KEYWORD_PATTERNS)) {
        std::cerr << "Error parsing file configuration for logger" << std::endl;
        return false;
    }
    // читаем информацию о всех логгерах
    const rapidjson::Value& r_pat = doc_json.FindMember(CONFIG_KEYWORD_PATTERNS)->value;
    if(r_pat.IsNull()) return false;
    for(rapidjson::Value::ConstMemberIterator it = r_pat.MemberBegin(); it != r_pat.MemberEnd(); it++) {
        if(!it->name.IsString() || !it->value.IsString()) {
            std::cerr << "Error parsing file configuration for logger - incorrect type of pattern" << std::endl;
            return false;
        }
        std::string pat_name = it->name.GetString();
        std::string pat_value = it->value.GetString();
        if(!LogUtils::isUnique(reg_patterns, pat_name)) {
            std::cerr << "Error parsing file configuration for logger - attempted to redefenition exist pattern" << std::endl;
            return false;
        }
        patterns[pat_name] = pat_value;
        reg_patterns.push_back(pat_name);
    }
    return true;
}
/**
 * Записать параметры паттернов в JSON
 */
bool LogUtils::writeParamPatterns(std::string &file_path, std::string p_name, std::string p_value) {
    // читаем файл
    rapidjson::Document doc_json = LogUtils::readJSON(file_path);
    if(!doc_json.HasMember(CONFIG_KEYWORD_PATTERNS)) {
        std::cerr << "Error parsing file configuration for logger" << std::endl;
        return false;
    }
    if(!LogUtils::isUnique(reg_patterns, p_name)) {
        std::cerr << "Attempted to rewrite exist logger patern: " << p_name << std::endl;
        return false;
    }
    rapidjson::Value &r_patterns = doc_json.FindMember(CONFIG_KEYWORD_PATTERNS)->value;
    if(!r_patterns.IsObject()) r_patterns.SetObject();
    rapidjson::Document d_write;
    d_write.SetObject();
    auto &allocator = d_write.GetAllocator();
    rapidjson::Value name(p_name.c_str(), allocator);
    rapidjson::Value value(p_value.c_str(), allocator);
    r_patterns.AddMember(name, value, allocator);
    // Запись объектов в конфигурационный файл
    if(!LogUtils::writeJSON(doc_json, file_path)) return false;
    // добавляем в список зарегистрированных
    reg_patterns.push_back(p_name);
    return true;
}
/**
 * Получить параметры пула потоков
 */
bool LogUtils::readParamTPool(std::string &file_path, TPoolEntity &pool_entity) {
    // читаем файл
    rapidjson::Document doc_json = LogUtils::readJSON(file_path);
    if(!doc_json.HasMember(CONFIG_KEYWORD_THREADPOOL)) {
        std::cerr << "Error parsing file configuration for logger" << std::endl;
        return false;
    }
    // читаем информацию о пуле потоков
    const rapidjson::Value& r_tpool = doc_json.FindMember(CONFIG_KEYWORD_THREADPOOL)->value;
    // размер очереди в байтах
    return pool_entity.fromJSON(r_tpool);
}
/**
 * Записать параметры пула потоков
 */
bool LogUtils::writeParamTPool(std::string &file_path, TPoolEntity &pool_entity) {
    // читаем файл
    rapidjson::Document d_read = LogUtils::readJSON(file_path);
    if(!d_read.HasMember(LogUtils::CONFIG_KEYWORD_THREADPOOL)) {
        std::cerr << "Error writing sink to file configuration" << std::endl;
        return false;
    }
    if(!d_read.IsObject()) d_read.SetObject();
    // Добавление в конфигурационный файл
    auto &r_allocator = d_read.GetAllocator();
    rapidjson::Value name(LogUtils::CONFIG_KEYWORD_THREADPOOL, r_allocator);
    rapidjson::Document pool_json = pool_entity.toJSON();
    auto &p_allocator = pool_json.GetAllocator();
    d_read.FindMember(name)->value.CopyFrom(pool_json, p_allocator);
    // Запись объектов в конфигурационный файл
    return LogUtils::writeJSON(d_read, file_path);
}
/**
 * Установить расположение для всех лог-файлов
 * [заданная_директория]/[имя_синка].log
 */
bool LogUtils::setLogPath(std::vector<LoggerEntity> &loggers, std::string log_path) {
    // проходим по всем сущностям логера
    for(size_t i = 0; i < loggers.size(); i++) {
        std::vector<SinkEntity*> sinks = loggers.at(i).getSinks();
        for(size_t j = 0; j < sinks.size(); j++) {
            SinkEntity *sink = sinks.at(j);
            // если в файле не была задана директория - назначаем расположение синку
            if(sink->getFileName().empty() || sink->getFileName() == SinkEntity::SINK_FILENAME_DEFAULT) {
                std::string path = log_path + sink->getSinkName() + ".log";
                sink->setFileName(path);
            }
        }
        loggers.at(i).setSinks(sinks);
    }
    return true;
}
/**
 * Проверка содержится ли элемент в векторе
 */
bool LogUtils::isUnique(std::vector<std::string> vector, std::string element) {
    std::vector<std::string>::iterator i = std::find(vector.begin(), vector.end(), element);
    if(i != vector.end()) return false;
    return true;
}

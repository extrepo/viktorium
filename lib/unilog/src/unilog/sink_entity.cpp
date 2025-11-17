#include "unilog/sink_entity.h"
#include <algorithm>
#include <iostream>
#include <spdlog/common.h>
/**
 * Значение параметра - наименование синка по умолчанию
 */
const char* SinkEntity::SINK_NAME_DEFAULT = "color_stdout_sink";
/**
 * Наименование параметра - имя и расположение файла с логами
 */
const char* SinkEntity::SINK_FILENAME = "base_file_name";
/**
 * Значение параметра - имя и расположение файла с логами по умолчанию
 */
const char* SinkEntity::SINK_FILENAME_DEFAULT = "./default.log";
/**
 * Наименование параметра - тип синка
 */
const char* SinkEntity::SINK_TYPE = "type";
/**
 * Консольный синк
 * Однопоточное исполнение (потокоНЕбезопасное)
 */
const char* SinkEntity::SINK_TYPE_STDOUT_SINK_ST = "stdout_sink_st";
/**
 * Консольный синк
 * Многопоточное исполнение (потокобезопасное)
 */
const char* SinkEntity::SINK_TYPE_STDOUT_SINK_MT = "stdout_sink_mt";
/**
 * Консольный синк
 * Однопоточное исполнение (потокоНЕбезопасное)
 */
const char* SinkEntity::SINK_TYPE_STDERR_SINK_ST = "stderr_sink_st";
/**
 * Консольный синк
 * Многопоточное исполнение (потокобезопасное)
 */
const char* SinkEntity::SINK_TYPE_STDERR_SINK_MT = "stderr_sink_mt";
/**
 * Консольный синк с поддержкой различных цветов подсветки
 * Однопоточное исполнение (потоНЕкобезопасное)
 */
const char* SinkEntity::SINK_TYPE_STDOUT_COLOR_SINK_ST = "stdout_color_sink_st";
/**
 * Консольный синк с поддержкой различных цветов подсветки
 * Многопоточное исполнение (потокобезопасное)
 */
const char* SinkEntity::SINK_TYPE_STDOUT_COLOR_SINK_MT = "stdout_color_sink_mt";
/**
 * Консольный синк с поддержкой различных цветов подсветки
 * Однопоточное исполнение (потоНЕкобезопасное)
 */
const char* SinkEntity::SINK_TYPE_STDERR_COLOR_SINK_ST = "stderr_color_sink_st";
/**
 * Консольный синк с поддержкой различных цветов подсветки
 * Многопоточное исполнение (потокобезопасное)
 */
const char* SinkEntity::SINK_TYPE_STDERR_COLOR_SINK_MT = "stderr_color_sink_mt";
/**
 * Синк системного логгера POSIX(3) который пишет в системный лог
 * Однопоточное исполнение (потокоНЕбезопасное)
 */
const char* SinkEntity::SINK_TYPE_SYSLOG_SINK_ST = "syslog_sink_st";
/**
 * Синк системного логгера POSIX(3) который пишет в системный лог
 * Многопоточное исполнение (потокобезопасное)
 */
const char* SinkEntity::SINK_TYPE_SYSLOG_SINK_MT = "syslog_sink_mt";
/**
 * Простой синк, который просто пишет в лог-файл
 * Параметры:
 * - расположение лог-файлов
 * Однопоточное исполнение (потоНЕкобезопасное)
 */
const char* SinkEntity::SINK_TYPE_BASIC_FILE_SINK_ST = "basic_file_sink_st";
/**
 * Простой синк, который просто пишет в лог-файл
 * Параметры:
 * - расположение лог-файлов
 * Многопоточное исполнение (потокобезопасное)
 */
const char* SinkEntity::SINK_TYPE_BASIC_FILE_SINK_MT = "basic_file_sink_mt";
/**
 * Синк с ротацией файлов по времени
 * Параметры:
 * - время(в часах и минутах) когда надо создавать новый лог-файл
 * - расположение лог-файлов
 * Однопоточное исполнение (потоНЕкобезопасное)
 */
const char* SinkEntity::SINK_TYPE_DAILY_FILE_SINK_ST = "daily_file_sink_st";
/**
 * Синк с ротацией файлов по времени
 * Параметры:
 * - время(в часах и минутах) когда надо создавать новый лог-файл
 * - расположение лог-файлов
 * Многопоточное исполнение (потокобезопасное)
 */
const char* SinkEntity::SINK_TYPE_DAILY_FILE_SINK_MT = "daily_file_sink_mt";
/**
 * Синк с ротацией файлов по размеру, количеству и времени
 * Параметры:
 * - количество лог-файлов
 * - размер каждого лог-файла
 * - время(в часах и минутах) когда надо создавать новый лог-файл
 * - расположение лог-файлов
 * Однопоточное исполнение (потокоНЕбезопасное)
 */
const char* SinkEntity::SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST = "rotate_daily_file_sink_st";
/**
 * Синк с ротацией файлов по размеру, количеству и времени
 * Параметры:
 * - количество лог-файлов
 * - размер каждого лог-файла
 * - время(в часах и минутах) когда надо создавать новый лог-файл
 * - расположение лог-файлов
 * Многопоточное исполнение (потокобезопасное)
 */
const char* SinkEntity::SINK_TYPE_ROTATING_DAILY_FILE_SINK_MT = "rotate_daily_file_sink_mt";
/**
 * Синк с ротацией файлов по размеру и количеству
 * Параметры:
 * - количество лог-файлов
 * - размер каждого лог-файла
 * - расположение лог-файлов
 * Однопоточное исполнение (потокоНЕбезопасное)
 */
const char* SinkEntity::SINK_TYPE_ROTATING_FILE_SINK_ST = "rotating_file_sink_st";
/**
 * Синк с ротацией файлов по размеру и количеству
 * Параметры:
 * - количество лог-файлов
 * - размер каждого лог-файла
 * - расположение лог-файлов
 * Многопоточное исполнение (потокобезопасное)
 */
const char* SinkEntity::SINK_TYPE_ROTATING_FILE_SINK_MT = "rotating_file_sink_mt";
/**
 * Значение параметра - тип синка по умолчанию
 */
const char* SinkEntity::SINK_TYPE_DEFAULT = SINK_TYPE_STDOUT_COLOR_SINK_MT;
/**
 * Наименование параметра - час - когда нужно делать ротацию файлов логов по времени
 */
const char* SinkEntity::SINK_ROTATION_HOUR = "rotation_hour";
/**
 * Значение параметра по умолчанию - час - когда нужно делать ротацию файлов логов по времени
 */
const int SinkEntity::SINK_ROTATION_HOUR_DEFAULT = 0;
/**
 * Наименование параметра - минуты - когда нужно делать ротацию файлов логов по времени
 */
const char* SinkEntity::SINK_ROTATION_MINUTE = "rotation_minute";
/**
 * Значение параметра по умолчанию - минуты - когда нужно делать ротацию файлов логов по времени
 */
const int SinkEntity::SINK_ROTATION_MINUTE_DEFAULT = 0;
/**
 * Наименование параметра - максимально допустимый размер одного файла лога в байтах
 * (когда нужно создавать и открывать новый файл для записи логов)
 */
const char* SinkEntity::SINK_ROTATION_MAX_FILESIZE = "max_size";
/**
 * Значение параметра - максимально допустимый размер одного файла лога в байтах по умолчанию
 * (когда нужно создавать и открывать новый файл для записи логов)
 */
const int SinkEntity::SINK_ROTATION_MAX_FILESIZE_DEFAULT = 25 * 1024 * 1024;
/**
 * Наименование параметра - максимальное допустимое количество файлов логов
 * (когда нужно делать ротацию по количеству файлов)
 */
const char* SinkEntity::SINK_ROTATION_MAX_FILES = "max_files";
/**
 * Значение параметра - максимальное допустимое количество файлов логов по умолчанию
 * (когда нужно делать ротацию по количеству файлов)
 */
const int SinkEntity::SINK_ROTATION_MAX_FILES_DEFAULT = 30;
/**
 * Наименование параметра - уровень логирования
 * (начиная с которого должна производится запись в файл лога)
 */
const char* SinkEntity::SINK_LOG_LEVEL = "level";
/**
 * Значение параметра - уровень логирования по умолчанию
 * (начиная с которого должна производится запись в файл лога)
 */
const char* SinkEntity::SINK_LOG_LEVEL_DEFAULT = "info";
/**
 * Конструктор Синка для логгера
 */
SinkEntity::SinkEntity(std::string _sink_name,
                       std::string _sink_type,
                       std::string _sfile_name,
                       int _rotation_h,
                       int _rotation_m,
                       int _max_size,
                       int _max_files,
                       std::string _level) :
    sink_name(_sink_name),
    sink_type(_sink_type),
    sfile_name(_sfile_name),
    rotation_h(_rotation_h),
    rotation_m(_rotation_m),
    max_size(_max_size),
    max_files(_max_files),
    level(_level)
{
    // Заполняем массив наименованиями поддерживаемых типов синков
    // Типы синков, определенные в библиотеке SPDLOG
    exist_sink_types.push_back(SINK_TYPE_STDOUT_SINK_ST);
    exist_sink_types.push_back(SINK_TYPE_STDOUT_SINK_MT);
    exist_sink_types.push_back(SINK_TYPE_STDERR_SINK_ST);
    exist_sink_types.push_back(SINK_TYPE_STDERR_SINK_MT);
    exist_sink_types.push_back(SINK_TYPE_STDOUT_COLOR_SINK_ST);
    exist_sink_types.push_back(SINK_TYPE_STDOUT_COLOR_SINK_MT);
    exist_sink_types.push_back(SINK_TYPE_STDERR_COLOR_SINK_ST);
    exist_sink_types.push_back(SINK_TYPE_STDERR_COLOR_SINK_MT);
    exist_sink_types.push_back(SINK_TYPE_SYSLOG_SINK_ST);
    exist_sink_types.push_back(SINK_TYPE_SYSLOG_SINK_MT);
    exist_sink_types.push_back(SINK_TYPE_BASIC_FILE_SINK_ST);
    exist_sink_types.push_back(SINK_TYPE_BASIC_FILE_SINK_MT);
    exist_sink_types.push_back(SINK_TYPE_DAILY_FILE_SINK_ST);
    exist_sink_types.push_back(SINK_TYPE_DAILY_FILE_SINK_MT);
    exist_sink_types.push_back(SINK_TYPE_ROTATING_FILE_SINK_ST);
    exist_sink_types.push_back(SINK_TYPE_ROTATING_FILE_SINK_MT);
    // "Кастомные" типы синков. Если появятся новые - добавлять их здесь ниже!
    exist_sink_types.push_back(SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST);
    exist_sink_types.push_back(SINK_TYPE_ROTATING_DAILY_FILE_SINK_MT);
}
/**
 * Установить наименование синка
 */
void SinkEntity::setFileName(std::string _sfile_name) {
    // если пустой - задаем имя по-умолчанию
    if(_sfile_name.empty()) _sfile_name = SINK_FILENAME_DEFAULT;
    // если не изменилось - выходим
    if(sfile_name.compare(_sfile_name) == 0) return;
    sfile_name = _sfile_name;
}
/**
 * Получить имя и расположение файла с логами
 */
std::string SinkEntity::getFileName() {
    // проверяем есть ли для данного синка вообще такой параметр
    if(sink_type != SINK_TYPE_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_DAILY_FILE_SINK_MT &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_MT &&
            sink_type != SINK_TYPE_ROTATING_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_FILE_SINK_MT &&
            sink_type != SINK_TYPE_BASIC_FILE_SINK_MT &&
            sink_type != SINK_TYPE_BASIC_FILE_SINK_ST)
        return "";
    return sfile_name;
}
/**
 * Установить наименование синка
 */
void SinkEntity::setSinkName(std::string _sink_name) {
    // если пустой - задаем имя по-умолчанию
    if(_sink_name.empty()) _sink_name = SINK_NAME_DEFAULT;
    // если не изменилось - выходим
    if(sink_name.compare(_sink_name) == 0) return;
    sink_name = _sink_name;
}
/**
 * Получить наименование синка
 */
std::string SinkEntity::getSinkName() {
    return sink_name;
}
/**
 * Установить тип синка (входной параметр - из списка поддерживаемых типов)
 */
void SinkEntity::setSinkType(std::string _sink_type) {
    // если пустой - задаем тип по-умолчанию
    if(_sink_type.empty()) _sink_type = SINK_TYPE_DEFAULT;
    // проверяем, есть ли такой тип среди известных
    std::vector<std::string>::iterator it = std::find(exist_sink_types.begin(), exist_sink_types.end(), _sink_type);
    if(it == exist_sink_types.end()) {
        std::cerr << "Attempted to set unknown type \"" << _sink_type
                  << "\" of sink: \"" << sink_name << "\"";
        return;
    }
    // если не изменился - выходим
    if(sink_type.compare(_sink_type) == 0) return;
    sink_type = _sink_type;
}
/**
 * Получить тип синка (возвращаемый параметр - из списка поддерживаемых типов)
 */
std::string SinkEntity::getSinkType() {
    return sink_type;
}
/**
 * Установить параметр - час - когда нужно делать ротацию файлов логов по времени
 */
void SinkEntity::setRotationHour(int _rotation_h) {
    // проверяем есть ли для данного синка вообще такой параметр
    if(sink_type != SINK_TYPE_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_DAILY_FILE_SINK_MT &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_MT)
        return;
    // если не изменилось - выходим
    if(rotation_h == _rotation_h) return;
    // проверяем задаваемое время на адекватность
    if(_rotation_h < 0 || _rotation_h > 24) {
        std::cerr << "Attempted to set incorrect rotation hour \"" << _rotation_h
                  << "\" of sink: \"" << sink_name << "\"";
        return;
    }
    rotation_h = _rotation_h;
}
/**
 * Получить параметр - час - когда нужно делать ротацию файлов логов по времени
 */
int SinkEntity::getRotationHour() {
    // проверяем есть ли для данного синка вообще такой параметр
    if(sink_type != SINK_TYPE_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_DAILY_FILE_SINK_MT &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_MT)
        return -1;
    return rotation_h;
}
/**
 * Установить параметр - минуты - когда нужно делать ротацию файлов логов по времени
 */
void SinkEntity::setRotationMinute(int _rotation_m) {
    // проверяем есть ли для данного синка вообще такой параметр
    if(sink_type != SINK_TYPE_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_DAILY_FILE_SINK_MT &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_MT)
        return;
    // если не изменилось - выходим
    if(rotation_m == _rotation_m) return;
    // проверяем задаваемое время на адекватность
    if(_rotation_m < 0 || _rotation_m > 60) {
        std::cerr << "Attempted to set incorrect rotation minute \"" << _rotation_m
                  << "\" of sink: \"" << sink_name << "\"";
        return;
    }
    rotation_m = _rotation_m;
}
/**
 * Получить параметр - минуты - когда нужно делать ротацию файлов логов по времени
 */
int SinkEntity::getRotationMinute() {
    // проверяем есть ли для данного синка вообще такой параметр
    if(sink_type != SINK_TYPE_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_DAILY_FILE_SINK_MT &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_MT)
        return -1;
    return rotation_m;
}
/**
 * Установить параметр - максимально допустимый размер одного файла лога в байтах
 * (когда нужно создавать и открывать новый файл для записи логов)
 */
void SinkEntity::setMaxFileSize(int _max_size) {
    // проверяем есть ли для данного синка вообще такой параметр
    if(sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_MT &&
            sink_type != SINK_TYPE_ROTATING_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_FILE_SINK_MT)
        return;
    // если не изменилось - выходим
    if(max_size == _max_size) return;
    // проверяем задаваемый параметр на адекватность
    if(_max_size < 0) {
        std::cerr << "Attempted to set incorrect rotation filesize \"" << _max_size
                  << "\" of sink: \"" << sink_name << "\"";
        return;
    }
    max_size = _max_size;
}
/**
 * Получить параметр - максимально допустимый размер одного файла лога в байтах
 * (когда нужно создавать и открывать новый файл для записи логов)
 */
int SinkEntity::getMaxFileSize() {
    // проверяем есть ли для данного синка вообще такой параметр
    if(sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_MT &&
            sink_type != SINK_TYPE_ROTATING_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_FILE_SINK_MT)
        return -1;
    return max_size;
}
/**
 * Установить параметр - максимальное допустимое количество файлов логов
 * (когда нужно делать ротацию по количеству файлов)
 */
void SinkEntity::setMaxFiles(int _max_files) {
    // проверяем есть ли для данного синка вообще такой параметр
    if(sink_type != SINK_TYPE_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_DAILY_FILE_SINK_MT &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_MT &&
            sink_type != SINK_TYPE_ROTATING_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_FILE_SINK_MT)
        return;
    // если не изменилось - выходим
    if(max_files == _max_files) return;
    // проверяем задаваемый параметр на адекватность
    if(_max_files < 0) {
        std::cerr << "Attempted to set incorrect rotation filesize \"" << _max_files
                  << "\" of sink: \"" << sink_name << "\"";
        return;
    }
    max_files = _max_files;
}
/**
 * Получить параметр - максимальное допустимое количество файлов логов
 * (когда нужно делать ротацию по количеству файлов)
 */
int SinkEntity::getMaxFiles() {
    // проверяем есть ли для данного синка вообще такой параметр
    if(sink_type != SINK_TYPE_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_DAILY_FILE_SINK_MT &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_DAILY_FILE_SINK_MT &&
            sink_type != SINK_TYPE_ROTATING_FILE_SINK_ST &&
            sink_type != SINK_TYPE_ROTATING_FILE_SINK_MT)
        return -1;
    return max_files;
}

/**
 * Установить параметр - уровень логирования
 * (начиная с которого должна производится запись в файл лога)
 */
void SinkEntity::setLevel(std::string _level) {
    if(level == _level) return;
    // проверяем, есть ли такой тип среди известных
    spdlog::string_view_t exist_levels[] SPDLOG_LEVEL_NAMES;
    for (const auto &level_str : exist_levels)
    {
        if (level_str == _level) {
            level = _level;
            return;
        }
    }
    // если совпадений не нашлось - попадаем сюда
    std::cerr << "Attempted to set unknown type of level \"" << _level
              << "\" for sink: \"" << sink_name << "\"";
    return;
}
/**
 * Получить параметр - уровень логирования
 * (начиная с которого должна производится запись в файл лога)
 */
std::string SinkEntity::getLevel() {
    return level;
}
/**
 * Сериализовать объект класса в JSON
 */
rapidjson::Document SinkEntity::toJSON() {
    // Записываем параметры в файл
    rapidjson::Document d_write;
    d_write.SetObject();
    auto &allocator = d_write.GetAllocator();
    // Создаем объект синка
    rapidjson::Value sinks_objects(rapidjson::kObjectType);
    if(!this->getSinkType().empty()) {
        rapidjson::Value name(SinkEntity::SINK_TYPE, allocator);
        rapidjson::Value value(this->getSinkType().c_str(), allocator);
        sinks_objects.AddMember(name, value, allocator);
    }
    if(!this->getFileName().empty()) {
        rapidjson::Value name(SinkEntity::SINK_FILENAME, allocator);
        rapidjson::Value value(this->getFileName().c_str(), allocator);
        sinks_objects.AddMember(name, value, allocator);
    }
    if(this->getRotationHour() >= 0 && this->getRotationMinute() >= 0) {
        sinks_objects.AddMember(rapidjson::StringRef(SinkEntity::SINK_ROTATION_HOUR), this->getRotationHour(), allocator);
        sinks_objects.AddMember(rapidjson::StringRef(SinkEntity::SINK_ROTATION_MINUTE), this->getRotationMinute(), allocator);
    }
    if(this->getMaxFileSize() > 0)
        sinks_objects.AddMember(rapidjson::StringRef(SinkEntity::SINK_ROTATION_MAX_FILESIZE), this->getMaxFileSize(), allocator);
    if(this->getMaxFiles() > 0)
        sinks_objects.AddMember(rapidjson::StringRef(SinkEntity::SINK_ROTATION_MAX_FILES), this->getMaxFiles(), allocator);
    if(!this->getLevel().empty()) {
        rapidjson::Value name(SinkEntity::SINK_LOG_LEVEL, allocator);
        rapidjson::Value value(this->getLevel().c_str(), allocator);
        sinks_objects.AddMember(name, value, allocator);
    }
    d_write.CopyFrom(sinks_objects, allocator);
    return d_write;
}
/**
 * Десериализовать в объект класса из JSON
 */
bool SinkEntity::fromJSON(const rapidjson::Value& doc_json) {
    // смотрим есть ли синк с таким именем
    if(!doc_json.HasMember(this->getSinkName().c_str())) {
        std::cerr << "Error parsing file configuration for logger - sink " << this->getSinkName() << " not found!" << std::endl;
        return false;
    }
    // читаем параметры
    const rapidjson::Value &params = doc_json[this->getSinkName().c_str()];
    if(params.HasMember(SinkEntity::SINK_TYPE)) {
        this->setSinkType(params[SinkEntity::SINK_TYPE].GetString());
    }
    if(params.HasMember(SinkEntity::SINK_ROTATION_HOUR)) {
        this->setRotationHour(params[SinkEntity::SINK_ROTATION_HOUR].GetInt());
    }
    if(params.HasMember(SinkEntity::SINK_ROTATION_MINUTE)) {
        this->setRotationMinute(params[SinkEntity::SINK_ROTATION_MINUTE].GetInt());
    }
    if(params.HasMember(SinkEntity::SINK_ROTATION_MAX_FILESIZE)) {
        this->setMaxFileSize(params[SinkEntity::SINK_ROTATION_MAX_FILESIZE].GetInt());
    }
    if(params.HasMember(SinkEntity::SINK_ROTATION_MAX_FILES)) {
        this->setMaxFiles(params[SinkEntity::SINK_ROTATION_MAX_FILES].GetInt());
    }
    if(params.HasMember(SinkEntity::SINK_LOG_LEVEL)) {
        this->setLevel(params[SinkEntity::SINK_LOG_LEVEL].GetString());
    }
    return true;
}

#include "unilog/unilogger.h"
#include <typeinfo>
#include <typeindex>
#include <iostream>
#include <fstream>
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "unilog/rotate_daily_file_sink.h"

/**
 * Мьютекс для контроля создания логгера при многопоточности
 */
std::mutex UniLogger::locker_;

UniLogger::LogStream::LogStream(std::string l_name, const spdlog::source_loc& _loc, spdlog::level::level_enum _lvl, std::string _prefix)
    : log_name(l_name),
      loc(_loc),
      lvl(_lvl),
      prefix(_prefix)
{
}

void UniLogger::LogStream::flush()
{
    auto logger = UniLogger::getInstance().getLogger(log_name);
    logger->log(loc,lvl,(prefix + str()).c_str());
}

UniLogger::LogStream::~LogStream()
{
    UniLogger::LogStream::flush();
}
/**
 * Функция возвращающая указатель на логгер-синглтон
 * Многопоточная реализация
 */
UniLogger& UniLogger::getInstance()
{
    // указатель на логгер
    static UniLogger* volatile pt;
    // если указатель пустой - создаем экземпляр логгера
    if(pt == 0) {
        // временный указатель
        UniLogger* tmp;
        {
            // блокируем доступ для других потоков
            std::lock_guard<std::mutex> lock(locker_);
            // создание экземпляра
            static UniLogger logger;
            // ставим статус логгера - неинициализирован
            logger.is_inited_ = false;
            // предаем ссылку
            tmp = &logger;
        }
        pt = tmp;
    }
    return *pt;
}
/**
 * Инициализация логгера
 */
bool UniLogger::init(std::string conf_file_path, std::string log_file_path)
{
    // блокируем доступ для других потоков
    std::lock_guard<std::mutex> lock(locker_);
    if(is_inited_) return true;
    try {
        conf_path_ = conf_file_path;
        log_path_ = log_file_path;
        // проверяем есть ли файл конфигурации
        std::ifstream is;
        is.open(conf_path_, std::ifstream::in);
        // если файла не обнаружено - создаем и записываем базовую конфигурацию
        if(!is.is_open()) {
            // раскидываем ключевые наименования параметров - Синки, Паттерны, Логгеры и Пул потоков
            if(!LogUtils::writeKeywords(conf_path_)) return false;
            // записываем параметры паттерна со значениями по-умолчанию
            if(!LogUtils::writeParamPatterns(conf_path_,
                                         LoggerEntity::LOGGER_PATTERN_DEFAULT,
                                         LogUtils::DEFAULT_PATTERN)) return false;
            // записываем пул потоков с параметрами по-умолчанию
            TPoolEntity pool;
            pool.setQueueSize(TPoolEntity::TPOOL_QUEUE_SIZE_DEFAULT);
            pool.setThreadCount(TPoolEntity::TPOOL_THREAD_COUNT_DEFAULT);
            LogUtils::writeParamTPool(conf_path_, pool);
            // записываем логгер MAIN
            LoggerEntity log_main;
            log_main.setLoggerName("Main");
            log_main.setPatternName(LoggerEntity::LOGGER_PATTERN_DEFAULT);
            log_main.setLevel(spdlog::level::to_string_view(spdlog::level::trace).data());
            // делаем синки по умолчанию
            // синк для записи логов FULL
            SinkEntity* sink_full = new SinkEntity;
            sink_full->setSinkName("Full");
            sink_full->setFileName(log_path_ + sink_full->getSinkName() + ".log");
            sink_full->setSinkType(SinkEntity::SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST);
            sink_full->setLevel(spdlog::level::to_string_view(spdlog::level::trace).data());
            sink_full->setMaxFiles(365);
            sink_full->setMaxFileSize(SinkEntity::SINK_ROTATION_MAX_FILESIZE_DEFAULT);
            sink_full->setRotationHour(SinkEntity::SINK_ROTATION_HOUR_DEFAULT);
            sink_full->setRotationMinute(SinkEntity::SINK_ROTATION_MINUTE_DEFAULT);
            // синк для записи логов ERROR
            SinkEntity* sink_error = new SinkEntity;
            sink_error->setSinkName("Error");
            sink_error->setFileName(log_path_ + sink_error->getSinkName() + ".log");
            sink_error->setSinkType(SinkEntity::SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST);
            sink_error->setLevel(spdlog::level::to_string_view(spdlog::level::err).data());
            sink_error->setMaxFiles(365);
            sink_error->setMaxFileSize(SinkEntity::SINK_ROTATION_MAX_FILESIZE_DEFAULT);
            sink_error->setRotationHour(SinkEntity::SINK_ROTATION_HOUR_DEFAULT);
            sink_error->setRotationMinute(SinkEntity::SINK_ROTATION_MINUTE_DEFAULT);
            // синк для вывода сообщений в консоль
            SinkEntity* sink_console = new SinkEntity;
            sink_console->setSinkName(SinkEntity::SINK_NAME_DEFAULT);
            sink_console->setSinkType(SinkEntity::SINK_TYPE_DEFAULT);
            // добавляем их к сущности логгера
            log_main.setSinks(std::vector<SinkEntity*>({sink_full, sink_error, sink_console}));
            // дописываем его в файл конфигурации
            if(!LogUtils::writeParamLogger(conf_path_, log_main)) {
                std::cerr << "Error writing configuration for logger: " << log_main.getLoggerName();
                return false;
            }
            LogUtils::reg_loggers.clear();
            LogUtils::reg_sinks.clear();
            LogUtils::reg_patterns.clear();
        }
        // читаем общие параметры - паттерны и пул потоков
        std::map<std::string, std::string> patterns;
        if(!LogUtils::readParamPatterns(conf_path_, patterns)) return false;
        TPoolEntity thread_pool;
        if(!LogUtils::readParamTPool(conf_path_, thread_pool)) return false;
        // читаем сущности логгеров
        std::vector<LoggerEntity> loggers;
        if(!LogUtils::readParamLoggers(conf_path_, loggers)) return false;
        // Костыль но пока пусть так - назначаем пути для лог файлов
        LogUtils::setLogPath(loggers, log_path_);
        // начинаем инициализировать логгеры
        // создаем пул потоков
        std::shared_ptr<spdlog::details::thread_pool> tp = std::make_shared<spdlog::details::thread_pool>(thread_pool.getQueueSize(),
                                                                                                          thread_pool.getThreadCount());
        spdlog::details::registry::instance().set_tp(tp);
        // проходим по всем логгерам
        for(size_t i = 0; i < loggers.size(); i++) {
            LoggerEntity &logger = loggers.at(i);
            // если паттерн не нашелся
            if(!patterns.count(logger.getPatternName())) return false;
            std::string log_pattern = patterns.at(logger.getPatternName());
            // пытаемся зарегистрировать и создать каждый логгер
            if(regLogger(logger, log_pattern) == nullptr) return false;
        }
    }  catch (std::exception_ptr e) {
        assert(false);
        return false;
    }
    is_inited_ = true;
    return true;
}
/**
 * Создать логгер из сущности
 */
std::shared_ptr<spdlog::logger> UniLogger::regLogger(LoggerEntity &log_ent, std::string &pattern) {
    std::vector<SinkEntity*> ent_sinks = log_ent.getSinks();
    std::vector<std::shared_ptr<spdlog::sinks::sink>> reg_sinks;
    // вначале регистрируем синки логгера и запоминаем их
    for(size_t i = 0; i < ent_sinks.size(); i ++) {
        std::shared_ptr<spdlog::sinks::sink> reg_sink = regSink(ent_sinks.at(i));
        if(reg_sink == nullptr) {
            std::cerr << "Error register sink: " << ent_sinks.at(i)->getSinkName() << std::endl;
            return nullptr;
        }
        reg_sinks.push_back(reg_sink);
    }
    // регистрируем логгер в соответствии с параметрами и синками
    std::shared_ptr<spdlog::logger> logger = nullptr;
    // синхронный тип логгера
    if(log_ent.getSynchType() == LoggerEntity::LOGGER_TYPE_SYNCH) {
        logger = std::make_shared<spdlog::logger>(log_ent.getLoggerName(), begin(reg_sinks), end(reg_sinks));
    }
    // асинхронный тип логгера
    if(log_ent.getSynchType() == LoggerEntity::LOGGER_TYPE_ASYNCH ||
            log_ent.getSynchType() == LoggerEntity::LOGGER_TYPE_ASYNCH_NB) {
        auto &registry_inst = spdlog::details::registry::instance();
        // создаем глобальный пул потоков, если он еще не существует
        std::lock_guard<std::recursive_mutex> tp_lock(registry_inst.tp_mutex());
        auto tp = registry_inst.get_tp();
        if (tp == nullptr) {
            std::cerr << "Logger::createAsync: Thread pull created" << std::endl;
            return nullptr;
        }
        spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block;
        if(log_ent.getSynchType() == LoggerEntity::LOGGER_TYPE_ASYNCH_NB)
            policy = spdlog::async_overflow_policy::overrun_oldest;
        logger = std::make_shared<spdlog::async_logger>(log_ent.getLoggerName(), begin(reg_sinks), end(reg_sinks), std::move(tp), policy);
    }
    logger->set_level(spdlog::level::from_str(log_ent.getLevel()));
    logger->flush_on(spdlog::level::from_str(log_ent.getLevel()));
    logger->set_pattern(pattern);
    spdlog::register_logger(logger);

    return logger;
}
/**
 * Зарегистрировать синки из сущностей
 */
std::shared_ptr<spdlog::sinks::sink> UniLogger::regSink(SinkEntity *s_ent) {
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_STDOUT_SINK_MT) {
        return std::make_shared<spdlog::sinks::stdout_sink_mt>();
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_STDOUT_SINK_ST) {
        return std::make_shared<spdlog::sinks::stdout_sink_st>();
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_STDERR_SINK_MT) {
        return std::make_shared<spdlog::sinks::stderr_sink_st>();
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_STDERR_SINK_ST) {
        return std::make_shared<spdlog::sinks::stdout_sink_st>();
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_STDOUT_COLOR_SINK_MT) {
        return std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_STDOUT_COLOR_SINK_ST) {
        return std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_STDERR_COLOR_SINK_MT) {
        return std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_STDERR_COLOR_SINK_ST) {
        return std::make_shared<spdlog::sinks::stderr_color_sink_st>();
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_BASIC_FILE_SINK_MT) {
        return std::make_shared<spdlog::sinks::basic_file_sink_mt>(s_ent->getFileName());
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_BASIC_FILE_SINK_ST) {
        return std::make_shared<spdlog::sinks::basic_file_sink_st>(s_ent->getFileName());
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_DAILY_FILE_SINK_ST) {
        return std::make_shared<spdlog::sinks::daily_file_sink_st>(s_ent->getFileName(),
                                                                   s_ent->getRotationHour(),
                                                                   s_ent->getRotationMinute(),
                                                                   false,
                                                                   s_ent->getMaxFiles());
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_DAILY_FILE_SINK_MT) {
        return std::make_shared<spdlog::sinks::daily_file_sink_mt>(s_ent->getFileName(),
                                                                   s_ent->getRotationHour(),
                                                                   s_ent->getRotationMinute(),
                                                                   false,
                                                                   s_ent->getMaxFiles());
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_ROTATING_DAILY_FILE_SINK_MT) {
        return std::make_shared<spdlog::sinks::rotate_daily_file_sink_mt>(s_ent->getFileName(),
                                                                          s_ent->getRotationHour(),
                                                                          s_ent->getRotationMinute(),
                                                                          s_ent->getMaxFileSize(),
                                                                          false,
                                                                          s_ent->getMaxFiles());
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST) {
        return std::make_shared<spdlog::sinks::rotate_daily_file_sink_st>(s_ent->getFileName(),
                                                                          s_ent->getRotationHour(),
                                                                          s_ent->getRotationMinute(),
                                                                          s_ent->getMaxFileSize(),
                                                                          false,
                                                                          s_ent->getMaxFiles());
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_ROTATING_FILE_SINK_MT) {
        return std::make_shared<spdlog::sinks::rotating_file_sink_mt>(s_ent->getFileName(),
                                                                      s_ent->getMaxFileSize(),
                                                                      s_ent->getMaxFiles());
    }
    if(s_ent->getSinkType() == SinkEntity::SINK_TYPE_ROTATING_FILE_SINK_ST) {
        return std::make_shared<spdlog::sinks::rotating_file_sink_st>(s_ent->getFileName(),
                                                                      s_ent->getMaxFileSize(),
                                                                      s_ent->getMaxFiles());
    }
    return nullptr;
}
/**
 * Указатель на логгер spdlog::logger по его наименованию
 */
std::shared_ptr<spdlog::logger> UniLogger::getLogger(std::string logger_name) {
    // возвращаем логгер по имени из зарегистрированных после init
    auto logger = spdlog::get(logger_name);
    // если логгер не нашелся, пробуем создать его с параметрами по умолчанию и вернуть
    if(not logger) {
        // блокируем доступ для других потоков
        std::lock_guard<std::mutex> lock(locker_);
        // создаем сущность логгера
        LoggerEntity log_ent;
        log_ent.setLoggerName(logger_name);
        log_ent.setPatternName(LoggerEntity::LOGGER_PATTERN_DEFAULT);
        log_ent.setSynchType(LoggerEntity::LOGGER_TYPE_DEFAULT);
        log_ent.setLevel(LoggerEntity::LOGGER_LOG_LEVEL_DEFAULT);
        // делаем синки по умолчанию
        // синк для записи логов
        SinkEntity* sink = new SinkEntity;
        sink->setSinkName(logger_name);
        sink->setFileName(log_path_ + sink->getSinkName() + ".log");
        sink->setSinkType(SinkEntity::SINK_TYPE_ROTATING_DAILY_FILE_SINK_ST);
        sink->setLevel(SinkEntity::SINK_LOG_LEVEL_DEFAULT);
        sink->setMaxFiles(SinkEntity::SINK_ROTATION_MAX_FILES_DEFAULT);
        sink->setMaxFileSize(SinkEntity::SINK_ROTATION_MAX_FILESIZE_DEFAULT);
        sink->setRotationHour(SinkEntity::SINK_ROTATION_HOUR_DEFAULT);
        sink->setRotationMinute(SinkEntity::SINK_ROTATION_MINUTE_DEFAULT);
        // синк для вывода сообщений в консоль
        SinkEntity* sink_console = new SinkEntity;
        sink_console->setSinkName(SinkEntity::SINK_NAME_DEFAULT);
        sink_console->setSinkType(SinkEntity::SINK_TYPE_DEFAULT);
        // добавляем их к сущности логгера
        log_ent.setSinks(std::vector<SinkEntity*>({sink, sink_console}));
        // регистрируем логгер
        std::string pattern = LogUtils::DEFAULT_PATTERN;
        logger = regLogger(log_ent, pattern);
        // дописываем его в файл конфигурации
        if(!LogUtils::writeParamLogger(conf_path_, log_ent)) {
            std::cerr << "Error writing configuration for logger: " << log_ent.getLoggerName();
        }
    }
    return logger;
}

template <typename... Args>
void UniLogger::log(std::string l_name,
         const spdlog::source_loc &loc,
         spdlog::level::level_enum lvl,
         const char* fmt,
         const Args &... args)
{
    auto logger = spdlog::get(l_name);
    logger->log(loc, lvl, fmt, args...);
}

template <typename... Args>
void UniLogger::printf(const spdlog::source_loc& loc,
            spdlog::level::level_enum lvl,
            const char* fmt,
            const Args &... args)
{
    spdlog::log(loc, lvl, fmt::sprintf(fmt, args...).c_str());
}
#ifdef QT_CORE_LIB
void UniLogger::LogStreamQt::writeToLog() {
    UniLogger::getInstance().getLogger(log_name.toStdString())->log(lvl, buffer.toStdString());
}

UniLogger::LogStreamQt::~LogStreamQt() noexcept(false) {
    try {
        writeToLog();
    } catch(std::exception&) {
        assert(!"exception in logger helper destructor");
        throw;
    }
}
#endif

#include "unilog/tpool_entity.h"
#include <iostream>
/**
 * Наименование параметра - Количество потоков
 */
const char* TPoolEntity::TPOOL_THREAD_COUNT = "thread_count";
/**
 * Значение параметра - Количество потоков по-умолчанию
 */
const int TPoolEntity::TPOOL_THREAD_COUNT_DEFAULT = 5;
/**
 * Наименование параметра - Размер очереди
 */
const char* TPoolEntity::TPOOL_QUEUE_SIZE = "queue_size";
/**
 * Значение параметра - Размер очереди по-умолчанию
 */
const int TPoolEntity::TPOOL_QUEUE_SIZE_DEFAULT = 4096;
/**
 * Конструктор сущности
 */
TPoolEntity::TPoolEntity(int _thread_count, int _queue_size) :
    thread_count_(_thread_count),
    queue_size_(_queue_size)
{
}
/**
 * Установить количество потоков
 */
void TPoolEntity::setThreadCount(int _thread_count) {
    if(thread_count_ == _thread_count) return;
    // проверяем на адекватность
    if(_thread_count < 0 || _thread_count > 100*1000) {
        std::cerr << "Attempted to set incorrect \"thread_count\" value " << _thread_count;
        return;
    }
    thread_count_ = _thread_count;
}
/**
 * Получить количество потоков
 */
int TPoolEntity::getThreadCount() {
    if(thread_count_ < 1) return TPOOL_QUEUE_SIZE_DEFAULT;
    return thread_count_;
}
/**
 * Установить размер очереди
 */
void TPoolEntity::setQueueSize(int _queue_size) {
    if(queue_size_ == _queue_size) return;
    // проверяем на адекватность
    if(_queue_size < 0 || _queue_size > 1000*1000) {
        std::cerr << "Attempted to set incorrect \"queue_size\" value " << _queue_size;
        return;
    }
    queue_size_ = _queue_size;
}
/**
 * Получить размер очереди
 */
int TPoolEntity::getQueueSize() {
    if(queue_size_ < 1) return TPOOL_QUEUE_SIZE_DEFAULT;
    return queue_size_;
}
/**
 * Сериализовать объект класса в JSON
 */
rapidjson::Document TPoolEntity::toJSON() {
    // Записываем параметры в файл
    rapidjson::Document d_write;
    d_write.SetObject();
    auto &allocator = d_write.GetAllocator();
    // Создаем объект пула потоков
    rapidjson::Value pool_objects(rapidjson::kObjectType);
    // размер очереди
    rapidjson::Value name(TPoolEntity::TPOOL_QUEUE_SIZE, allocator);
    pool_objects.AddMember(name, this->getQueueSize(), allocator);
    // количество потоков
    name = rapidjson::Value(TPoolEntity::TPOOL_THREAD_COUNT, allocator);
    pool_objects.AddMember(name, this->getThreadCount(), allocator);
    d_write.CopyFrom(pool_objects, allocator);
    return d_write;
}
/**
 * Десериализовать в объект класса из JSON
 */
bool TPoolEntity::fromJSON(const rapidjson::Value& r_tpool) {
    // размер очереди в байтах
    if(r_tpool.HasMember(TPoolEntity::TPOOL_QUEUE_SIZE) && r_tpool[TPoolEntity::TPOOL_QUEUE_SIZE].IsInt()) {
        this->setQueueSize(r_tpool[TPoolEntity::TPOOL_QUEUE_SIZE].GetInt());
    }
    else {
        std::cerr << "Sets default queue size for loggers - " << TPoolEntity::TPOOL_QUEUE_SIZE_DEFAULT << std::endl;
        this->setQueueSize(TPoolEntity::TPOOL_QUEUE_SIZE_DEFAULT);
    }
    // количество потоков
    if(r_tpool.HasMember(TPoolEntity::TPOOL_THREAD_COUNT) && r_tpool[TPoolEntity::TPOOL_THREAD_COUNT].IsInt()) {
        this->setQueueSize(r_tpool[TPoolEntity::TPOOL_THREAD_COUNT].GetInt());
    }
    else {
        std::cerr << "Sets default thread count for loggers - " << TPoolEntity::TPOOL_THREAD_COUNT_DEFAULT << std::endl;
        this->setQueueSize(TPoolEntity::TPOOL_THREAD_COUNT_DEFAULT);
    }
    return true;
}

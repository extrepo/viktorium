#ifndef TPOOLENTITY_H
#define TPOOLENTITY_H
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
/**
 * Класс сущности Пула потоков логгера
 */
class TPoolEntity
{
public:
    /**
     * Наименование параметра - Количество потоков
     */
    static const char* TPOOL_THREAD_COUNT;
    /**
     * Значение параметра - Количество потоков по-умолчанию
     */
    static const int TPOOL_THREAD_COUNT_DEFAULT;
    /**
     * Наименование параметра - Размер очереди
     */
    static const char* TPOOL_QUEUE_SIZE;
    /**
     * Значение параметра - Размер очереди по-умолчанию
     */
    static const int TPOOL_QUEUE_SIZE_DEFAULT;
public:
    /**
     * Конструктор сущности
     */
    TPoolEntity(int _thread_count = TPOOL_THREAD_COUNT_DEFAULT,
                int _queue_size = TPOOL_QUEUE_SIZE_DEFAULT);
    /**
     * Установить количество потоков
     */
    void setThreadCount(int _thread_count);
    /**
     * Получить количество потоков
     */
    int getThreadCount();
    /**
     * Установить размер очереди
     */
    void setQueueSize(int _queue_size);
    /**
     * Получить размер очереди
     */
    int getQueueSize();
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
     * Количество потоков
     */
    int thread_count_;
    /**
     * Размер очереди
     */
    int queue_size_;
};

#endif // TPOOLENTITY_H

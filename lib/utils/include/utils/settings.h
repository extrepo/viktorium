#ifndef UNI_SETTINGS_H
#define UNI_SETTINGS_H
#include <string>
#include <map>
/**
 * Класс для доступа к настройкам и константам приложения
 */
class Settings {
public:
    /**
     * Каталог для логов (оканчивается '/')
     */
    static std::string logDir();
    /**
     * Каталог для файлов конфигурации (оканчивается '/')
     */
    static std::string configDir();
    /**
     * Каталог для файлов БД (оканчивается '/')
     */
    static std::string dbDir();
    /**
     * Файл в котором хранится конфигурация логгера
     */
    static std::string logConfig();
    /**
     * Получить системный параметр
     */
    static std::string getParam(std::string id);
    /**
     * Установить системный параметр
     */
    static void setParam(std::string id, std::string value);
private:
    /**
     * Считывали ли параметры из файла
     */
    static bool paramsReaded;
    /**
     * Хранение параметров
     */
    static std::map<std::string, std::string> _params;
    /**
     * Создать каталог
     * @return при ошибке -1
     */
    static int mkfolder(std::string path);
    /**
     * Выдаём домашний каталог приложения для пользователя
     */
    static std::string getUserAppDir();
    /**
     * Выдаём домашний каталог для системного приложения
     */
    static std::string getSystemAppDir();
};

#endif // UNI_SETTINGS_H

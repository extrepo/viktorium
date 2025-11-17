#ifndef UNI_UTILS_H
#define UNI_UTILS_H
#include <string>
#include <vector>
#include <mutex>
class NetIface {
public:
    std::string name;
    std::string ip;
    std::string mask;
    std::string broadcast;
};
/**
 * Полезные статические функции
 */
class Utils {
public:
    /**
     * Выдаёт относительное время в миллисекундах
     */
    static long millis();
    /**
     * Останавливает поток на определенное количество миллисекунд
     */
    static void delay(long millis);
    /**
     * Генератор UUID
     */
    static std::string genUUID();
    /**
     * Преобразование целочисленного значения в hex строку
     */
    static std::string digitToHex(int d);
    /**
     * Преобразование hex строки в целочисленное значение
     */
    static int hexToDigit(std::string hex);
    /**
     * Замена спецсимволов JSON
     */
    static std::string jsonEscape(std::string in);
    /**
     * Возвращает сетевые интерфейсы системы
     */
    static std::vector<NetIface> interfaces();
    /**
     * Наименование и версия компилятора
     */
    static std::string getCompiler();
    /**
     * Наименование и версия ОС под которой работаем
     */
    static std::string getOS();
    /**
     * Пользователь, от которого запущено приложение
     */
    static std::string getOSUser();
    /**
     * Имя хоста, на котором запущено приложение
     */
    static std::string getHostName();
    /**
     * Кодирование base64
     */
    static std::string base64Encode(unsigned char const *data, size_t in_len);
    /**
     * Декодирование base64
     */
    static std::string base64Decode(std::string encodedString);
};
#endif // UNI_UTILS_H

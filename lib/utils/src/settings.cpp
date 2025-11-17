#include "utils/settings.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WIN32
#  include <direct.h>
#  include <windows.h>
#  define PATH_SEP '\\'
#else
#  include <limits.h>
#  include <unistd.h>
#  define PATH_SEP '/'
#endif

/**
 * Хранение параметров
 */
std::map<std::string, std::string> Settings::_params;
/**
 * Выдаём домашний каталог приложения для пользователя
 */
std::string Settings::getUserAppDir() {
    char const *home = getenv("HOME");
    std::string shome = "";
    if(home == nullptr) home = getenv("USERPROFILE");
    if(home == nullptr) {
        char const *hdrive = getenv("HOMEDRIVE");
        char const *hpath = getenv("HOMEPATH");
        if(hdrive == nullptr || hpath == nullptr) {
            shome = ".";
        } else {
            shome = std::string(hdrive) + hpath;
        }
    } else shome = home;
    std::string userAppDir = shome + PATH_SEP + ".viktorium" + PATH_SEP;
    if(mkfolder(userAppDir) != 0) {
        std::cerr << "Error: Home application dir not accessible: " + userAppDir << std::endl;
        exit(1);
    }
    return userAppDir;
}
/**
 * Выдаём домашний каталог для системного приложения
 */
std::string Settings::getSystemAppDir() {
    // Используем в качестве рабочего каталог уровнем выше
    // Находим текущий путь к исполняемому файлу
#ifdef _WIN32
    char cpath[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, cpath, MAX_PATH);
    std::string sep = "\\\\";
#else
    char cpath[PATH_MAX];
    int z = readlink("/proc/self/exe", cpath, PATH_MAX);
    cpath[z > 0 ? z : 0] = 0;
    std::string sep = "/";
#endif
    // Получаем каталог на один выше того где лежит бинарник
    std::regex regexp("^(.*)" + sep + "[^" + sep + "]*" + sep + "[^" + sep + "]*");
    std::smatch match;
    std::string binFile = cpath;
    if(!std::regex_match(binFile, match, regexp) || match.size() != 2) {
        std::cerr << "Error: Can't find application binary path" << std::endl;
        exit(1);
    }
    std::string systemAppDir = match[1];
    if(systemAppDir.size() > 0 && systemAppDir[systemAppDir.size()-1] != PATH_SEP) systemAppDir += PATH_SEP;
    if(mkfolder(systemAppDir) < 0) {
        std::cerr << "Error: Home application dir not accessible: " + systemAppDir << std::endl;
        exit(1);
    }
    return systemAppDir;
}
/**
 * Каталог для логов
 */
std::string Settings::logDir() {
    // Располагаем логи в log/
    std::string dir = getUserAppDir() + "log" + PATH_SEP;
    if(mkfolder(dir) != 0) {
        std::cerr << "Error: Log dir is not accessible: " + dir << std::endl;
        exit(1);
    }
    return dir;
}
/**
 * Каталог для файлов конфигурации
 */
std::string Settings::configDir() {
    std::string dir = getUserAppDir() + "conf" + PATH_SEP;
    int r = mkfolder(dir);
    if(r < 0) {
        std::cerr << "Error: Config dir is not accessible: " + dir << std::endl;
        exit(1);
    }
    if(r > 0) {
        std::cerr << "Error: Config dir is not writable: " + dir << std::endl;
        exit(1);
    }
    return dir;
}
/**
 * Каталог для файлов БД
 */
std::string Settings::dbDir() {
    // Располагаем базу данных в getUserAppDir/db/
    std::string dir = getUserAppDir() + "db" + PATH_SEP;
    if(mkfolder(dir) != 0) {
        std::cerr << "Error: Database dir is not accessible: " + dir << std::endl;
        exit(1);
    }
    return dir;
}
/**
 * Файл в котором хранится конфигурация логгера
 */
std::string Settings::logConfig() {
    return configDir() + "log.json";
}
/**
 * Получить системный параметр
 */
std::string Settings::getParam(std::string id) {
    // Читаем из файла параметры (если надо)
    if(!paramsReaded) {
        _params.clear();
        std::ifstream is;
        is.open(configDir() + "settings.json", std::ifstream::in);
        if(is.is_open()) {
            std::stringstream ss;
            ss << is.rdbuf();
            rapidjson::Document d;
            d.Parse(ss.str().c_str());
            if(d.IsObject()) {
                for(rapidjson::Value::ConstMemberIterator i=d.MemberBegin();i!=d.MemberEnd();++i) {
                    std::string idx = i->name.GetString();
                    std::transform(idx.begin(), idx.end(), idx.begin(), [](unsigned char c){ return std::tolower(c); });
                    _params[idx] = i->value.GetString();
                }
            }
            is.close();
        }
        paramsReaded = true;
    }
    // Возвращаем параметр
    std::transform(id.begin(), id.end(), id.begin(), [](unsigned char c){ return std::tolower(c); });
    return _params[id];
}
/**
 * Считывали ли параметры из файла
 */
bool Settings::paramsReaded = false;
/**
 * Установить системный параметр
 */
void Settings::setParam(std::string id, std::string value) {
    // Приводим к нижнему регистру
    std::transform(id.begin(), id.end(), id.begin(), [](unsigned char c){ return std::tolower(c); });
    if(getParam(id) == value) return;
    // Перечитываем параметры из файла, чтобы не перезаписать то что за это время исправил инсталлятор
    paramsReaded = false;
    if(getParam(id) == value) return;
    _params[id] = value;
    // Записываем параметры в файл
    rapidjson::Document d;
    d.SetObject();
    for(auto &i : _params) {
        rapidjson::Value name(i.first.c_str(), d.GetAllocator());
        rapidjson::Value value(i.second.c_str(), d.GetAllocator());
        d.AddMember(name, value, d.GetAllocator());
    }
    std::ofstream f;
    f.open(configDir() + "settings.json", std::ofstream::trunc);
    if(f.is_open()) {
        rapidjson::StringBuffer buf;
        buf.Clear();
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
        d.Accept(writer);
        f << buf.GetString();
        f.flush();
    }
    f.close();
}
/**
 * Создать каталог и проверить на доступность для записи
 * -1 - ошибка создания каталога
 *  0 - каталог создан успешно
 *  1 - каталог создан, но недоступен для записи
 */
int Settings::mkfolder(std::string path) {
    // Проверяем есть ли такой каталог
    struct stat info;
    // Если каталога нет - создаём его
    if(stat(path.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
#ifdef WIN32
        return _mkdir(path.c_str());
#else
        return mkdir(path.c_str(), 0755);
#endif
    }
    // Каталог есть, проверяем на возможность записи в него
    if(access(path.c_str(), W_OK)) return 1;
    return 0;
}

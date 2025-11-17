#include "utils/utils.h"
#include <chrono>
#include <random>
#include <thread>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>
#ifdef WIN32
#  include <winsock2.h>
#  include <windows.h>
#  include <process.h>
#  include <iphlpapi.h>
#else
#  include <unistd.h>
#  include <limits.h>
#  include <pwd.h>
#  include <ifaddrs.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#endif
/**
 * Выдаёт относительное время в миллисекундах
 */
long Utils::millis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
/**
 * Останавливает поток на определенное количество миллисекунд
 */
void Utils::delay(long millis) {
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
}
/**
 * Генератор UUID
 */
std::string Utils::genUUID() {
    static std::random_device dev;
    static std::mt19937 rng(dev());
    std::uniform_int_distribution<int> dist(0, 15);
    const char *v = "0123456789abcdef";
    const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };
    std::string res;
    for(int i=0;i<16;i++) {
        if(dash[i]) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}
/**
 * Преобразование целочисленного значения в hex строку
 */
std::string Utils::digitToHex(int d) {
    std::stringstream stream;
    stream << std::hex << d;
    return stream.str();
}
/**
 * Преобразование hex строки в целочисленное значение
 */
int Utils::hexToDigit(std::string hex) {
    int i;
    std::istringstream iss(hex);
    iss >> std::hex >> i;
    return i;
}
/**
 * Замена спецсимволов JSON
 */
std::string Utils::jsonEscape(std::string in) {
    std::string out = "";
    out.reserve(out.length() * 1.5);
    for(size_t i=0;i<in.size();i++) {
        switch(in[i]) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            default: out += in[i];
        }
    }
    return out;
}
/**
 * Возвращает сетевые интерфейсы системы
 */
std::vector<NetIface> Utils::interfaces() {
    std::vector<NetIface> ifaces;
#ifdef WIN32
    DWORD r, addrtable_size = 16 * 1024;
    PMIB_IPADDRTABLE addrtable = (PMIB_IPADDRTABLE)malloc(addrtable_size);
    for(int i = 0;i < 5;i++) {
        r = GetIpAddrTable(addrtable, &addrtable_size, 0);
        if(r != ERROR_INSUFFICIENT_BUFFER) break;
        free(addrtable);
        addrtable = (PMIB_IPADDRTABLE)malloc(addrtable_size);
    }
    if(r != NO_ERROR) {
        free(addrtable);
        return ifaces;
    }
    for(unsigned int i=0;i<addrtable->dwNumEntries;i++) {
        NetIface iface;
        // Адрес и имя
        struct in_addr addr;
        addr.S_un.S_addr = addrtable->table[i].dwAddr;
        iface.ip = inet_ntoa(addr);
        iface.name = inet_ntoa(addr);
        // Бродкаст
        DWORD maddress = (addrtable->table[i].dwAddr & addrtable->table[i].dwMask) | (addrtable->table[i].dwMask ^ (DWORD)0xffffffff);
        addr.S_un.S_addr = maddress;
        iface.broadcast = inet_ntoa(addr);
        // Маска
        addr.S_un.S_addr = addrtable->table[i].dwMask;
        iface.mask = inet_ntoa(addr);
        ifaces.push_back(iface);
        break;
    }
    free(addrtable);
#else
    struct ifaddrs *ifAddrStruct = nullptr;
    struct ifaddrs *ifa = nullptr;
    getifaddrs(&ifAddrStruct);
    for(ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        if(!ifa->ifa_addr) continue;
        // Только IPv4
        if(ifa->ifa_addr->sa_family != AF_INET) continue;
        NetIface iface;
        // Наименование интерфейса
        iface.name = ifa->ifa_name;
        // Адрес
        struct sockaddr_in *sin = (struct sockaddr_in *)(ifa->ifa_addr);
        iface.ip = inet_ntoa(sin->sin_addr);
        // Бродкаст
        sin = (struct sockaddr_in *)(ifa->ifa_ifu.ifu_broadaddr);
        iface.broadcast = inet_ntoa(sin->sin_addr);
        // Маска
        sin = (struct sockaddr_in *)(ifa->ifa_netmask);
        iface.mask = inet_ntoa(sin->sin_addr);
        ifaces.push_back(iface);
    }
    if(ifAddrStruct != nullptr) freeifaddrs(ifAddrStruct);
#endif
    return ifaces;
}
/**
 * Наименование и версия компилятора
 */
std::string Utils::getCompiler() {
    std::string arch = "";
#if defined(__x86_64__) || defined(_M_X64)
    arch = "x86_64";
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
    arch = "x86_32";
#elif defined(__ARM_ARCH_2__)
    arch = "ARM2";
#elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
    arch = "ARM3";
#elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
    arch = "ARM4T";
#elif defined(__ARM_ARCH_5_) || defined(__ARM_ARCH_5E_)
    arch = "ARM5"
#elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2_)
    arch = "ARM6T2";
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
    arch = "ARM6";
#elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
    arch = "ARM7";
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
    arch = "ARM7A";
#elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
    arch = "ARM7R";
#elif defined(__ARM_ARCH_7M__)
    arch = "ARM7M";
#elif defined(__ARM_ARCH_7S__)
    arch = "ARM7S";
#elif defined(__aarch64__) || defined(_M_ARM64)
    arch = "ARM64";
#elif defined(mips) || defined(__mips__) || defined(__mips)
    arch = "MIPS";
#elif defined(__sh__)
    arch = "SUPERH";
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
    arch = "POWERPC";
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
    arch = "POWERPC64";
#elif defined(__sparc__) || defined(__sparc)
    arch = "SPARC";
#elif defined(__m68k__)
    arch = "M68K";
#elif defined(__e2k__)
    arch = "E2K";
#endif
    std::string compiler = "Undefined";
    std::string ver = "Undefined";
#if defined(__GNUC__)
    #if defined(__LCC__)
        compiler = "LCC";
        ver = std::to_string(__LCC__ / 100) + ".";
        ver += std::to_string(__LCC__ % 100);
	#elif defined(__llvm__)
		#if defined(__clang__)
			compiler = "Clang/LLVM";
            ver = std::to_string(__clang_major__) + ".";
            ver += std::to_string(__clang_minor__) + ".";
            ver += std::to_string(__clang_patchlevel__);
		#elif defined(__APPLE_CC__)
			compiler = "Apple GCC/LLVM";
		#else
			compiler = "GCC/LLVM";
		#endif
	#elif defined(__APPLE_CC__)
		compiler = "GCC";
	#elif defined(__MINGW32__) || defined(__MINGW64__)
		compiler = "MinGW GCC";
	#else
		compiler = "GCC";
	#endif
    if(ver == "Undefined") {
        ver = std::to_string(__GNUC__) + ".";
        ver += std::to_string(__GNUC_MINOR__) + ".";
        ver += std::to_string(__GNUC_PATCHLEVEL__);
    }
#endif
#if defined(__INTEL_COMPILER)
	compiler "Intel";
	ver = std::to_string(__INTEL_COMPILER / 100) + ".";
    ver += std::to_string((__INTEL_COMPILER % 100) / 10);
#endif
#if defined(_MSC_VER)
    compiler = "MSVC";
    ver = std::to_string(_MSC_VER / 100) + ".";
    ver += std::to_string(_MSC_VER % 100);
#endif
    return compiler + " " + arch + " " + ver;
}
/**
 * Наименование и версия ОС под которой работаем
 */
std::string Utils::getOS() {
#ifdef _WIN32
    return "Windows";
#else
    std::string os, ver;
    std::ifstream is;
    is.open("/etc/os-release", std::ifstream::in);
    while(is.is_open() && !is.eof()) {
        std::string s;
        getline(is, s);
        std::string id, v;
        int n = s.find("=");
        if(n == std::string::npos) continue;
        id = s.substr(0, n);
        v = s.substr(n + 1);
        while(v.size() > 0 && v[0] == '\"') v = v.substr(1);
        while(v.size() > 0 && v[v.size()-1] == '\"') v.resize(v.size() - 1);
        if(id == "NAME") os = v;
        if(id == "ID" && os.empty()) os = v;
        if(id == "VERSION") ver = v;
        if(id == "VERSION_ID" && ver.empty()) ver = v;
    }
    if(is.is_open()) is.close();
    if(os.empty()) os = "Unknown OS";
    is.open("/etc/astra_version", std::ifstream::in);
    if(is.is_open()) {
        std::string s;
        is >> s;
        if(!s.empty()) ver += " (" + s + ")";
        is.close();
    }
    return os + " " + ver;
#endif
}
/**
 * Пользователь, от которого запущено приложение
 */
std::string Utils::getOSUser() {
#ifdef WIN32
    DWORD bufSize = 1024;
    char buf[bufSize];
    if(GetUserName(buf, &bufSize)) return buf;
#else
    struct passwd *pwd = getpwuid(getuid());
    if(pwd) return pwd->pw_name;
#endif
    return "";
}
/**
 * Имя хоста, на котором запущено приложение
 */
std::string Utils::getHostName() {
#ifdef WIN32
    DWORD bufSize = 1024;
    char buf[bufSize];
    if(GetComputerName(buf, &bufSize)) return buf;
#else
    size_t bufSize = 1024;
    char buf[bufSize];
    if(gethostname(buf, bufSize) == 0) return buf;
#endif
    return "";
}
/**
 * Кодирование base64
 */
std::string Utils::base64Encode(unsigned char const *data, size_t in_len) {
    static const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t len_encoded = (in_len + 2) / 3 * 4;
    unsigned char trailing_char = '=';
    std::string ret;
    ret.reserve(len_encoded);
    unsigned int pos = 0;
    while (pos < in_len) {
        ret.push_back(base64_chars[(data[pos + 0] & 0xfc) >> 2]);
        if (pos+1 < in_len) {
           ret.push_back(base64_chars[((data[pos + 0] & 0x03) << 4) + ((data[pos + 1] & 0xf0) >> 4)]);
           if (pos+2 < in_len) {
              ret.push_back(base64_chars[((data[pos + 1] & 0x0f) << 2) + ((data[pos + 2] & 0xc0) >> 6)]);
              ret.push_back(base64_chars[  data[pos + 2] & 0x3f]);
           } else {
              ret.push_back(base64_chars[(data[pos + 1] & 0x0f) << 2]);
              ret.push_back(trailing_char);
           }
        } else {
            ret.push_back(base64_chars[(data[pos + 0] & 0x03) << 4]);
            ret.push_back(trailing_char);
            ret.push_back(trailing_char);
        }
        pos += 3;
    }
    return ret;
}
/**
 * Декодирование base64
 */
std::string Utils::base64Decode(std::string encodedString) {
    if(encodedString.empty()) return std::string();
    encodedString.erase(std::remove(encodedString.begin(), encodedString.end(), '\n'), encodedString.end());
    size_t length_of_string = encodedString.length();
    size_t pos = 0;
    size_t approx_length_of_decoded_string = length_of_string / 4 * 3;
    std::string ret;
    ret.reserve(approx_length_of_decoded_string);
    while(pos < length_of_string) {
        size_t pos_of_char[4];
        for(int i=0;i<4;i++) {
            pos_of_char[i] = 0;
            char chr = encodedString.at(pos + i);
            if(chr >= 'A' && chr <= 'Z') pos_of_char[i] = chr - 'A';
            if(chr >= 'a' && chr <= 'z') pos_of_char[i] = chr - 'a' + ('Z' - 'A') + 1;
            if(chr >= '0' && chr <= '9') pos_of_char[i] = chr - '0' + ('Z' - 'A') + ('z' - 'a') + 2;
            if(chr == '+' || chr == '-') pos_of_char[i] = 62;
            if(chr == '/' || chr == '_') pos_of_char[i] = 63;
        }
        ret.push_back(static_cast<std::string::value_type>((pos_of_char[0] << 2) + ((pos_of_char[1] & 0x30) >> 4)));
        if((pos + 2 < length_of_string) &&  // Check for data that is not padded with equal signs (which is allowed by RFC 2045)
                encodedString.at(pos + 2) != '=' &&
                encodedString.at(pos + 2) != '.') { // accept URL-safe base 64 strings, too, so check for '.' also.
            ret.push_back(static_cast<std::string::value_type>(((pos_of_char[1] & 0x0f) << 4) + ((pos_of_char[2] & 0x3c) >> 2)));
            if((pos + 3 < length_of_string) && encodedString.at(pos + 3) != '=' && encodedString.at(pos + 3) != '.') {
                ret.push_back(static_cast<std::string::value_type>(((pos_of_char[2] & 0x03) << 6) + pos_of_char[3]));
            }
        }
        pos += 4;
    }
    return ret;
}

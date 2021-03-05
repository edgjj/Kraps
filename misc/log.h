#pragma once

#include <mutex>
#include <iostream>
#include <vector>
#include <string>
#include <ctime> 

static std::mutex mtx_cout;
struct acout
{
    std::unique_lock<std::mutex> lk;
    acout()
        :
        lk(std::unique_lock<std::mutex>(mtx_cout))
    {

    }

    template<typename T>
    acout& operator<<(const T& _t)
    {
        std::cout << _t;
        return *this;
    }

    acout& operator<<(std::ostream& (*fp)(std::ostream&))
    {
        std::cout << fp;
        return *this;
    }
};

namespace LOG {
    enum LVL {
        INFO,
        WARNING,
        ERR
    };
    static std::vector<std::string> errdesc = { "INFO", "WARNING", "ERROR" };

    static void log(uint8_t lvl, std::string what) {
        std::time_t t = std::time(nullptr);
        char mbstr[100];
        std::strftime(mbstr, 100, "[%Y-%m-%e %H:%M:%S]", std::localtime(&t));

        acout() << mbstr << ' '
            << errdesc[lvl] << ": " << what << std::endl;
    }
    static void info(std::string str) {
        log(LVL::INFO, str);
    }
    static void warning(std::string str) {
        log(LVL::WARNING, str);
    }
    static void error(std::string str) {
        log(LVL::ERR, str);
    }
}
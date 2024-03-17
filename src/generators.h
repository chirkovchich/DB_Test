#include <iostream>
#include <sstream>
#include <pqxx/pqxx>
#include <chrono>
#include <string>
#include <iomanip>
#include <random>
#include <algorithm>

using namespace std::literals;
using pqxx::operator"" _zv;
using time_point = std::chrono::system_clock::time_point;

class GeneratorInt{
public:
    GeneratorInt(){
        engine_.seed(time(0));
    }
    int operator()(int min, int max){
        std::uniform_int_distribution<int> rand_int(min, max);
        int n1 = rand_int(engine_);
    return n1;
    }
private:
    std::mt19937 engine_;
};

class GeneratorTime{
public:
    GeneratorTime(){
        engine_.seed(time(0));
    }
    std::chrono::system_clock::time_point operator()(){
        std::uniform_int_distribution<int> rand_int(0, 100000);

    return m_start_ + std::chrono::hours(rand_int(engine_));
    }

private:
    std::mt19937 engine_;
    std::chrono::system_clock::time_point m_start_;
};

char GenerateUpString() {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[ rand() % max_index ];
}

std::string GenerateLowerString(int lenth) {
    auto randchar = []() -> char
    {
        const char charset[] = "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(lenth,0);
    std::generate_n( str.begin(), lenth, randchar );
    return str;
}

std::string GenerateName(){
    return GenerateUpString() + GenerateLowerString(4) + " "s +
           GenerateUpString() + GenerateLowerString(4) + " "s +
           GenerateUpString() + GenerateLowerString(4);
}

std::string GenerateFName(){
    return 'F' + GenerateLowerString(4) + " "s +
           GenerateUpString() + GenerateLowerString(4) + " "s +
           GenerateUpString() + GenerateLowerString(4);
}

std::string GenerateDate(GeneratorTime& g){
    const auto t_c = std::chrono::system_clock::to_time_t(g());
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&t_c), "%Y-%m-%d");
    return oss.str() ;
}
std::string GenerateGender(){
    return GenderToString(static_cast<Gender>(rand() % 2));
}
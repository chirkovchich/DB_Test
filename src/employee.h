#pragma once

#include <string>
#include <sstream>
#include <chrono>
#include <pqxx/pqxx>

using namespace std::literals;
using pqxx::operator"" _zv;
using time_point = std::chrono::system_clock::time_point;

enum class Gender {
    MALE,
    FEMALE
};

Gender StrToGender(std::string& gender_str){
    if (gender_str == "Male"s){
        return Gender::MALE;
    }
    return Gender::FEMALE;
}

std::string GenderToString(Gender gender) {
    if (gender == Gender::MALE){
        return "Male"s;
    }
    return "Female"s;
}

std::ostream& operator<<(std::ostream& out, Gender& gender) {
    out << GenderToString(gender);
    return out;
}

class Employee {
public:
    Employee(std::string name, std::string birthday, std::string gender_str)
        :name_(name),
         gender_(StrToGender(gender_str))
        {
            birthday_tm_ = ParseTime(birthday);
        }
   
    Employee(const Employee&) = default;
    Employee& operator=(const Employee&) = delete;
    Employee(Employee&&) = default;
    Employee& operator=(Employee&&) = delete;

    void SaveToDB(pqxx::connection& conn) const {
        pqxx::work w(conn);
        w.exec_params(
                R"(
        INSERT INTO workers (name, birthday, gender) VALUES ($1, $2, $3);
        )"_zv,
            name_, this->GetBirthdayStr(), GenderToString(gender_));
        w.commit();
    }

    std::string GetBirthdayStr() const {
        const auto t_c = std::chrono::system_clock::to_time_t(birthday_tm_);
        std::ostringstream oss;
        oss << std::put_time(std::gmtime(&t_c), "%Y-%m-%d");
        return oss.str();
    }
    std::string GetName() const {
        return name_;
    }

    std::string GetGender() const {
        return GenderToString(gender_);
    }

    int CalcAge(){
        auto t_now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::hours>(t_now - birthday_tm_).count();
        return age / 8760 ;
    }

private:
    time_point ParseTime(std::string& time_str){
        std::tm tm = {};
        std::stringstream ss(time_str);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        
        return std::chrono::system_clock::from_time_t(std::mktime(&tm)) + std::chrono::hours(10) ;
    };

    std::string name_;
    time_point birthday_tm_;
    Gender gender_;
}; 

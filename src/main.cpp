
#include <iostream>
#include <sstream>
#include <pqxx/pqxx>
#include <chrono>
#include <string>
#include <iomanip>
#include <random>
#include <algorithm>

#include "employee.h"
#include "generators.h"

using namespace std::literals;
// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;
using time_point = std::chrono::system_clock::time_point;

int main(int argc, const char* argv[]) {
    try {
        if (argc == 1) {
            std::cout << "Invalid cmd parameters\n"sv;
            return EXIT_SUCCESS;
        } 
        // Подключаемся к БД, указывая её параметры в качестве аргумента
        pqxx::connection conn{ R"(postgres://postgres:mish@localhost:30432/test_db)"};
       
        int action = std::stoi(argv[1]);
        std::vector<std::string> all_args;
        all_args.assign(argv + 1, argv + argc);

        if (action == 1){
            pqxx::work w(conn);
            // создаем enam для пола
            w.exec(
            "DO $$ "
                "BEGIN "
                    "IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'gen') THEN "
                        "CREATE TYPE gen AS ENUM ('Male', 'Female');"
                " END IF; "
            " END$$; "_zv);

            w.exec(
                "CREATE TABLE IF NOT EXISTS workers (id SERIAL PRIMARY KEY, name varchar(200) NOT NULL,"
                                                    "birthday date NOT NULL,"
                                                    "gender gen NOT NULL DEFAULT('Male'));"_zv);

            w.exec("DELETE FROM workers;"_zv);
            // Применяем все изменения
            w.commit();
        }
        
        if (action == 2){
            
            Employee worker{all_args.at(1), all_args.at(2), all_args.at(3)};
            std::cout << "Name: "s << worker.GetName() << "; "s
                              << "Birthday: "s << worker.GetBirthdayStr() << "; "s 
                              << "Gender: "s << worker.GetGender() << "; "s 
                              << "Age: "s << worker.CalcAge()
                              << std::endl;
            worker.SaveToDB(conn);
            return 0;
        }
            
        if (action == 3){
            pqxx::read_transaction r(conn);

            auto query_text = "SELECT DISTINCT ON (name , birthday) name, birthday, gender FROM workers ORDER BY name; "_zv;

            for (auto [name, date, gender] : r.query<std::string, std::string, std::string>(query_text)) {
                    Employee worker1{name, date, gender};

                    std::cout << "Name: "s << name << "; "s
                              << "Birthday: "s << date << "; "s 
                              << "Gender: "s << gender << "; "s 
                              << "Age: "s << worker1.CalcAge()
                              << std::endl;
            }
             return 0;
        }

        if (action == 4){
            pqxx::work w(conn);

            w.exec(
            "CREATE OR REPLACE FUNCTION insertintoboosted(valuesforinsert TEXT) RETURNS VOID AS "
                "$$ "
                "BEGIN "
                "EXECUTE 'INSERT INTO workers(name, birthday, gender) VALUES (' || valuesforinsert || ')';"
                "END;"
                "$$"
                "LANGUAGE plpgsql;"_zv);
            
            GeneratorTime g;

            w.exec("DELETE FROM workers;"_zv);

            conn.prepare("prep", "SELECT insertintoboosted($1::text)");
            for (size_t j = 0; j < 1000; ++j){
                size_t n = 1000;
                std::stringstream ss;
                for (size_t i = 0; i < n; ++i)
                {
                    if (i == 0)
                        ss  << "'" << GenerateName() << "', '" << GenerateDate(g) << " ' , '" << GenerateGender() << "'";
                    else
                        ss << "('" << GenerateName() << "', '" << GenerateDate(g) << " ' , '" << GenerateGender() << "'" ;

                    if (i < n - 1)
                        ss << "),";
                }
            w.exec_prepared("prep", ss);
            }
            w.commit();

            std::vector<Employee> men_employee;

            for (int i = 0; i < 100; ++i){
                const std::string male = "Male"s;
                men_employee.push_back({GenerateFName(), GenerateDate(g), male});
            }
            w.commit();
            
            size_t men_size = men_employee.size();
            std::stringstream ss1;
            for (size_t i = 0; i < men_size; ++i)
            {
                if (i == 0)
                    ss1  << "'" << men_employee[i].GetName() << "', '" << men_employee[i].GetBirthdayStr() << " ' , '" << men_employee[i].GetGender() << "'";
                else
                    ss1 << "('" << men_employee[i].GetName() << "', '" << men_employee[i].GetBirthdayStr() << " ' , '" << men_employee[i].GetGender() << "'" ;

                if (i < men_size - 1)
                    ss1 << "),";
            }
            w.exec_prepared("prep", ss1);
          
            w.commit();
 
            return 0;
        }

        if (action == 5){
            //поиск без индексов

            pqxx::work w(conn);
            w.exec("DROP INDEX IF EXISTS worker_name;"_zv); 
            w.commit();

            pqxx::read_transaction r(conn);
            auto query_text = "SELECT name, birthday, gender FROM workers WHERE name LIKE 'F%' AND gender = 'Male'; "_zv;
            
            const auto start_time = std::chrono::steady_clock::now();
            auto ans =  r.query<std::string, std::string, std::string>(query_text);
            const auto end_time = std::chrono::steady_clock::now();
            std::cout << "Duration: "s << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " ms"s << std::endl; 
            //65 ms
        }

        if (action == 6){
            //поиск с индексом

            pqxx::work w(conn);
            w.exec("CREATE INDEX worker_name ON workers (name);"_zv); 
            w.commit();

            pqxx::read_transaction r(conn);
            auto query_text = "SELECT name, birthday, gender FROM workers WHERE name LIKE 'F%' AND gender = 'Male'; "_zv;
            
            const auto start_time = std::chrono::steady_clock::now();
            auto ans =  r.query<std::string, std::string, std::string>(query_text);
            const auto end_time = std::chrono::steady_clock::now();
            std::cout << "Duration: "s << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " ms"s << std::endl;
            //45 ms
        }
        
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
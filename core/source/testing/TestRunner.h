#include <iostream>
#include <iomanip>
#include <string>
#include <functional>
#include <vector>

#define TEST_FAILED(file, line)                                             \
    std::cerr << "Test failed: "  << name << std::endl                      \
    << "\t" << std::setw(20) << std::left << "Description"  << " |\t" << description << std::endl        \
    << "\t" << std::setw(20) << std::left << "Expected"     << " |\t" << "(" << condition << ")" << std::endl    \
    << "\t" << std::setw(20) << std::left << "File"         << " |\t"  << std::right << file << std::endl                     \
    << "\t" << std::setw(20) << std::left << "Line"         << " |\t"  << std::right << std::to_string(line) << std::endl;


class TestRunner {
public:
    struct Test {

        Test* Then(const std::string& description) {
            this->description = description;
            return this;
        }


        template<typename T>
        Test* Require(std::string condition, const T& func, std::string file, long line) {
            if (!func()) {
                auto idx = condition.find_first_of('=');

                condition.replace(idx, 1, "!");

                TEST_FAILED(file, line);
            }

            return this;
        }


        std::string name;
        std::string category;
        std::string description;
        bool condition;
    };

    static Test* RegisterTest(const std::string& name, const std::string& category) {
        tests.push_back(Test{ name, category });
        return &tests.back();
    }

    static void Run() {
        int passed = 0;
        std::vector<Test> failed;


		for (const auto& test : tests)
		{
            bool pass = test.condition;
            auto name = test.name;
			
            if (!pass) {
                failed.push_back(test);
                continue;
            }
			
            passed++;
			
		}

        std::cout << "Testing Finished..." << std::endl
            << std::to_string(passed) << "/" << std::to_string(tests.size()) << " passed ("  << 
            failed.size() << " failed)" << std::endl;
    }

private:
    static std::vector<Test> tests;
};



#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "IntroSort.h"
#include "Student.h"

constexpr const char* INPUT_FILE =
    "students_1000000.csv";

constexpr size_t THREAD_COUNT = 4;

constexpr int WARMUP_RUNS = 2;
constexpr int TEST_RUNS = 20;

std::vector<Student> loadStudents(
    const std::string& filename)
{
    std::vector<Student> students;

    std::ifstream file(filename);

    if (!file.is_open())
    {
        return students;
    }

    std::string line;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);

        std::string value;

        std::vector<std::string> tokens;

        while (std::getline(ss, value, ','))
        {
            tokens.push_back(value);
        }

        if (tokens.size() != 7)
        {
            continue;
        }

        Student student;

        student.id = std::stoi(tokens[0]);
        student.firstName = tokens[1];
        student.lastName = tokens[2];
        student.faculty = tokens[3];
        student.rating = std::stod(tokens[4]);
        student.age = std::stoi(tokens[5]);
        student.course = std::stoi(tokens[6]);

        students.push_back(
            std::move(student));
    }

    return students;
}

bool isSorted(
    const std::vector<Student>& arr)
{
    for (size_t i = 1;
         i < arr.size();
         ++i)
    {
        if (arr[i].rating <
            arr[i - 1].rating)
        {
            return false;
        }
    }

    return true;
}

int main()
{
    std::vector<Student> students =
        loadStudents(INPUT_FILE);

    if (students.empty())
    {
        return 1;
    }

    for (int i = 0;
         i < WARMUP_RUNS;
         ++i)
    {
        auto warmupData =
            students;

        parallelIntroSort(
            warmupData,
            THREAD_COUNT);
    }

    long long totalTime = 0;

    for (int i = 0;
         i < TEST_RUNS;
         ++i)
    {
        auto testData =
            students;

        auto start =
            std::chrono::
            high_resolution_clock::
            now();

        parallelIntroSort(
            testData,
            THREAD_COUNT);

        auto end =
            std::chrono::
            high_resolution_clock::
            now();

        if (!isSorted(testData))
        {
            return 1;
        }

        totalTime +=
            std::chrono::
            duration_cast<
                std::chrono::milliseconds>(
                    end - start)
                .count();
    }

    double averageTime =
        static_cast<double>(
            totalTime) / TEST_RUNS;

    std::cout
        << "Time(ms): "
        << averageTime
        << '\n';

    return 0;
}

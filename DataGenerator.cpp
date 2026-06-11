#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "Student.h"

int main()
{
    size_t count;

    std::cout << "Enter number of students: ";
    std::cin >> count;

    std::string filename =
        "students_" +
        std::to_string(count) +
        ".csv";

    std::vector<std::string> firstNames =
    {
        "Ivan",
        "Anna",
        "Dmytro",
        "Olena",
        "Maksym",
        "Sophia",
        "Andrii",
        "Maria"
    };

    std::vector<std::string> lastNames =
    {
        "Shevchenko",
        "Koval",
        "Tkachenko",
        "Bondarenko",
        "Kravchenko",
        "Melnyk"
    };

    std::vector<std::string> faculties =
    {
        "FIOT",
        "FPM",
        "FEL",
        "IASA",
        "FBMI"
    };

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int>
        firstNameDist(
            0,
            firstNames.size() - 1);

    std::uniform_int_distribution<int>
        lastNameDist(
            0,
            lastNames.size() - 1);

    std::uniform_int_distribution<int>
        facultyDist(
            0,
            faculties.size() - 1);

    std::uniform_real_distribution<double>
        ratingDist(
            60.0,
            100.0);

    std::uniform_int_distribution<int>
        ageDist(
            17,
            25);

    std::uniform_int_distribution<int>
        courseDist(
            1,
            6);

    std::ofstream file(filename);

    if (!file.is_open())
    {
        std::cout << "File open error\n";
        return 1;
    }

    file << std::fixed << std::setprecision(2);

    for (size_t i = 0; i < count; ++i)
    {
        Student student;

        student.id = static_cast<int>(i);
        student.firstName =
            firstNames[firstNameDist(gen)];
        student.lastName =
            lastNames[lastNameDist(gen)];
        student.faculty =
            faculties[facultyDist(gen)];
        student.rating =
            ratingDist(gen);
        student.age =
            ageDist(gen);
        student.course =
            courseDist(gen);

        file
            << student.id << ","
            << student.firstName << ","
            << student.lastName << ","
            << student.faculty << ","
            << student.rating << ","
            << student.age << ","
            << student.course
            << "\n";
    }

    file.close();

    std::cout
        << "Generated file: "
        << filename
        << '\n';

    return 0;
}
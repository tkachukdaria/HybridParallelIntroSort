#pragma once

#include <vector>

#include "Student.h"

void introSort(std::vector<Student>& arr);

void parallelIntroSort(
    std::vector<Student>& arr,
    size_t threadCount);
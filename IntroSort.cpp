#include "IntroSort.h"

#include <algorithm>
#include <atomic>
#include <cmath>

#include "ThreadPool.h"

namespace
{
    constexpr int INSERTION_SORT_THRESHOLD = 32;
    constexpr int PARALLEL_THRESHOLD = 50000;

    bool lessByRating(
        const Student& a,
        const Student& b)
    {
        return a.rating < b.rating;
    }

    void insertionSort(
        std::vector<Student>& arr,
        int left,
        int right)
    {
        for (int i = left + 1; i <= right; ++i)
        {
            Student key = std::move(arr[i]);

            int j = i - 1;

            while (j >= left &&
                   arr[j].rating > key.rating)
            {
                arr[j + 1] = std::move(arr[j]);
                --j;
            }

            arr[j + 1] = std::move(key);
        }
    }

    void heapSort(
        std::vector<Student>& arr,
        int left,
        int right)
    {
        auto begin = arr.begin() + left;
        auto end = arr.begin() + right + 1;

        std::make_heap(begin, end, lessByRating);
        std::sort_heap(begin, end, lessByRating);
    }

    int medianOfThreeIndex(
        std::vector<Student>& arr,
        int left,
        int right)
    {
        int mid = left + (right - left) / 2;

        double a = arr[left].rating;
        double b = arr[mid].rating;
        double c = arr[right].rating;

        if ((a <= b && b <= c) ||
            (c <= b && b <= a))
        {
            return mid;
        }

        if ((b <= a && a <= c) ||
            (c <= a && a <= b))
        {
            return left;
        }

        return right;
    }

    int partition(
        std::vector<Student>& arr,
        int left,
        int right)
    {
        int pivotIndex =
            medianOfThreeIndex(
                arr,
                left,
                right);

        std::swap(
            arr[pivotIndex],
            arr[right]);

        double pivot =
            arr[right].rating;

        int i = left;

        for (int j = left; j < right; ++j)
        {
            if (arr[j].rating < pivot)
            {
                std::swap(
                    arr[i],
                    arr[j]);

                ++i;
            }
        }

        std::swap(
            arr[i],
            arr[right]);

        return i;
    }

    void introSortImpl(
        std::vector<Student>& arr,
        int left,
        int right,
        int depthLimit)
    {
        int size = right - left + 1;

        if (size <= 1)
        {
            return;
        }

        if (size <= INSERTION_SORT_THRESHOLD)
        {
            insertionSort(
                arr,
                left,
                right);

            return;
        }

        if (depthLimit == 0)
        {
            heapSort(
                arr,
                left,
                right);

            return;
        }

        int pivot =
            partition(
                arr,
                left,
                right);

        introSortImpl(
            arr,
            left,
            pivot - 1,
            depthLimit - 1);

        introSortImpl(
            arr,
            pivot + 1,
            right,
            depthLimit - 1);
    }

    void parallelIntroSortImpl(
        std::vector<Student>& arr,
        int left,
        int right,
        int depthLimit,
        ThreadPool& pool,
        std::atomic<int>& remainingTasks)
    {
        int size = right - left + 1;

        if (size <= 1)
        {
            return;
        }

        if (size <= INSERTION_SORT_THRESHOLD)
        {
            insertionSort(
                arr,
                left,
                right);

            return;
        }

        if (depthLimit == 0)
        {
            heapSort(
                arr,
                left,
                right);

            return;
        }

        int pivot =
            partition(
                arr,
                left,
                right);

        int leftSize = pivot - left;
        int rightSize = right - pivot;

        bool canCreateTask =
            size >= PARALLEL_THRESHOLD &&
            remainingTasks.load() > 0;

        if (canCreateTask &&
            remainingTasks.fetch_sub(1) > 0)
        {
            if (leftSize > rightSize)
            {
                pool.submit(
                    [&arr,
                     left,
                     pivot,
                     depthLimit,
                     &pool,
                     &remainingTasks]()
                {
                    parallelIntroSortImpl(
                        arr,
                        left,
                        pivot - 1,
                        depthLimit - 1,
                        pool,
                        remainingTasks);
                });

                parallelIntroSortImpl(
                    arr,
                    pivot + 1,
                    right,
                    depthLimit - 1,
                    pool,
                    remainingTasks);
            }
            else
            {
                pool.submit(
                    [&arr,
                     pivot,
                     right,
                     depthLimit,
                     &pool,
                     &remainingTasks]()
                {
                    parallelIntroSortImpl(
                        arr,
                        pivot + 1,
                        right,
                        depthLimit - 1,
                        pool,
                        remainingTasks);
                });

                parallelIntroSortImpl(
                    arr,
                    left,
                    pivot - 1,
                    depthLimit - 1,
                    pool,
                    remainingTasks);
            }
        }
        else
        {
            introSortImpl(
                arr,
                left,
                pivot - 1,
                depthLimit - 1);

            introSortImpl(
                arr,
                pivot + 1,
                right,
                depthLimit - 1);
        }
    }
}

void introSort(
    std::vector<Student>& arr)
{
    if (arr.empty())
    {
        return;
    }

    int depthLimit =
        2 *
        static_cast<int>(
            std::log2(arr.size()));

    introSortImpl(
        arr,
        0,
        static_cast<int>(arr.size()) - 1,
        depthLimit);
}

void parallelIntroSort(
    std::vector<Student>& arr,
    size_t threadCount)
{
    if (arr.empty())
    {
        return;
    }

    if (threadCount <= 1)
    {
        introSort(arr);
        return;
    }

    ThreadPool pool(threadCount);

    int depthLimit =
        2 *
        static_cast<int>(
            std::log2(arr.size()));

    std::atomic<int> remainingTasks(
        static_cast<int>(threadCount * 4));

    parallelIntroSortImpl(
        arr,
        0,
        static_cast<int>(arr.size()) - 1,
        depthLimit,
        pool,
        remainingTasks);

    pool.wait();
}
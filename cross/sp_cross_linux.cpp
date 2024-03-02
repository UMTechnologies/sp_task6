#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <chrono>
#include <string>
#include <stdexcept> 

std::mutex mutex;
double global_sum = 0;

void calculate_squares(const std::vector<double>& numbers, size_t start, size_t end) {
    std::this_thread::sleep_for(std::chrono::seconds(10));
    double local_sum = 0;
    for (size_t i = start; i < end; ++i) {
        local_sum += numbers[i] * numbers[i];
    }
    std::lock_guard<std::mutex> guard(mutex);
    global_sum += local_sum;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <filename> <number of threads>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error opening file\n";
        return 1;
    }

    std::vector<double> numbers;
    double number;
    while (file >> number) {
        numbers.push_back(number);
    }

    if (numbers.size() < 2) {
        std::cerr << "File must contain at least two numbers without text and other specific symbols.\n";
        return 1;
    }

    unsigned long num_threads;
    try {
        long requested_threads = std::stol(argv[2]); 
        if (requested_threads <= 0) {
            std::cerr << "Invalid number of threads. Setting to 1.\n";
            num_threads = 1;
        } else {
            num_threads = static_cast<unsigned long>(requested_threads);
        }
    } catch (const std::invalid_argument& ia) {
        std::cerr << "Invalid argument. Using 1 thread.\n";
        num_threads = 1;
    } catch (const std::out_of_range& oor) {
        std::cerr << "Number of threads is out of range. Using the number of processor cores.\n";
        num_threads = std::thread::hardware_concurrency(); 
    }

    if (num_threads > numbers.size()) {
        num_threads = numbers.size();
        std::cerr << "Warning: Number of threads is greater than number of numbers. Reducing threads to " << numbers.size() << ".\n";
    }

    std::vector<std::thread> threads;
    size_t chunk_size = numbers.size() / num_threads;
    size_t start = 0;
    for (size_t i = 0; i < num_threads; ++i) {
        size_t end = start + chunk_size + (i < (numbers.size() % num_threads));
        threads.emplace_back(calculate_squares, std::cref(numbers), start, end);
        start = end;
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Final sum of squares: " << global_sum << std::endl;
    return 0;
}

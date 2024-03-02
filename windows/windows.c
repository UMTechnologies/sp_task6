#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <limits.h>

double global_sum = 0.0;
CRITICAL_SECTION criticalSection;

typedef struct {
    double* numbers;
    int start;
    int end;
} ThreadData;

DWORD WINAPI calculate_squares(LPVOID param) {
    ThreadData* data = (ThreadData*)param;
    double local_sum = 0.0;
    for (int i = data->start; i < data->end; ++i) {
        local_sum += data->numbers[i] * data->numbers[i];
    }

    // Добавлена задержка в 10 секунд
    Sleep(10000);

    EnterCriticalSection(&criticalSection);
    global_sum += local_sum;
    LeaveCriticalSection(&criticalSection);

    free(data);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <number of threads>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
        return 1;
    }

    long long num_threads_input = strtoll(argv[2], NULL, 10);
    if (num_threads_input > INT_MAX || num_threads_input <= 0) {
        fprintf(stderr, "Error: The number of threads is out of valid range.\n");
        fclose(file);
        return 1;
    }
    int num_threads = (int)num_threads_input;

    double numbers[1024];
    int count = 0;

    while (fscanf(file, "%lf", &numbers[count]) == 1 && count < 1024) {
        count++;
    }
    fclose(file);

    if (count < 2) {
        fprintf(stderr, "Error: The file must contain at least two numbers.\n");
        return 1;
    }

    if (num_threads > count) {
        num_threads = count;
        printf("Note: Number of threads set to %d, as it exceeds the number of numbers.\n", num_threads);
    }

    InitializeCriticalSection(&criticalSection);

    HANDLE* handles = (HANDLE*)malloc(num_threads * sizeof(HANDLE));
    int nums_per_thread = count / num_threads;
    int start = 0;

    for (int i = 0; i < num_threads; ++i) {
        ThreadData* data = (ThreadData*)malloc(sizeof(ThreadData));
        data->numbers = numbers;
        data->start = start;
        data->end = start + nums_per_thread + (i < (count % num_threads) ? 1 : 0);
        start = data->end;

        handles[i] = CreateThread(NULL, 0, calculate_squares, data, 0, NULL);
    }

    WaitForMultipleObjects(num_threads, handles, TRUE, INFINITE);

    for (int i = 0; i < num_threads; ++i) {
        CloseHandle(handles[i]);
    }
    free(handles);
    DeleteCriticalSection(&criticalSection);

    printf("The sum of squares is: %.2lf\n", global_sum);

    return 0;
}

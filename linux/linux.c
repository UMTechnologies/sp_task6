#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

double global_sum = 0;
pthread_mutex_t mutex;

typedef struct {
    double* numbers;
    int start;
    int end;
} ThreadData;

void* calculate_squares(void* arg) {
    sleep(10); // Задержка в начале работы потока
    ThreadData* data = (ThreadData*)arg;
    double local_sum = 0;
    for (int i = data->start; i < data->end; ++i) {
        local_sum += data->numbers[i] * data->numbers[i];
    }

    pthread_mutex_lock(&mutex);
    global_sum += local_sum;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <filename> <number of threads>\n", argv[0]);
        return 1;
    }

    char *end;
    long num_threads_temp = strtol(argv[2], &end, 10);
    if (end == argv[2] || *end != '\0' || errno == ERANGE || num_threads_temp <= 0 || num_threads_temp > INT_MAX) {
        printf("Invalid number of threads.\n");
        return 1;
    }
    int num_threads = (int)num_threads_temp;

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    double* numbers = malloc(10000 * sizeof(double));
    int count = 0;
    while (fscanf(file, "%lf", &numbers[count]) == 1) {
        count++;
    }
    fclose(file);

    if (count < 2) {
        printf("File must contain at least two numbers\n");
        free(numbers);
        return 1;
    }

    if (num_threads > count) {
        printf("Warning: Number of threads is greater than number of numbers. Reducing threads to %d.\n", count);
        num_threads = count;
    }

    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData* data = malloc(num_threads * sizeof(ThreadData));
    pthread_mutex_init(&mutex, NULL);

    int chunk_size = count / num_threads; // Объявление и инициализация chunk_size
    int start = 0; // Объявление и инициализация start

    for (int i = 0; i < num_threads; i++) {
        data[i].numbers = numbers;
        data[i].start = start;
        data[i].end = i < count % num_threads ? start + chunk_size + 1 : start + chunk_size;
        start = data[i].end;

        pthread_create(&threads[i], NULL, calculate_squares, &data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final sum of squares: %.2lf\n", global_sum);

    pthread_mutex_destroy(&mutex);
    free(numbers);
    free(threads);
    free(data);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

long long int totalSum;
atomic_flag lockHeld = ATOMIC_FLAG_INIT;

struct chunkInfo{
    char *v;
    int start, end;
};

struct arraySumInfo{
    long long int sum;
    double secondsElapsed;
};

void acquire(atomic_flag *lock){
    while (atomic_flag_test_and_set(lock));
}

void release(atomic_flag *lock){
    atomic_flag_clear(lock);
}

char *random_array(int arrayLen){
    char *v = malloc (arrayLen);
    for (int i = 0; i < arrayLen; i++)
        v[i] = (rand() % 201)-100;
    return v;
}

void *chunkSum(void *args){
    struct chunkInfo *chunk = args;
    int i;
    long long int localStorage = 0;
    for(i=chunk->start; i<=chunk->end; i++)
        localStorage += chunk->v[i];
    acquire(&lockHeld);
    totalSum += localStorage;
    release(&lockHeld);
}

long long int arraySum(char *v, int arrayLen){
    long long int sum = 0, i;
    for(i=0; i<arrayLen; i++){
        sum += v[i];
    }
    return sum;
}

struct arraySumInfo distributedArraySum(char *array, long long int arraySize, int threadsCount){
    /* 
        Recebe um array de tamanho <arraySize> e cria <threadsCount> (no máximo 256) threads para realizar
        a soma de seus elementos de maneira distribuída.
        Retorna o tempo total utilizado no cálculo da soma.
    */
    totalSum = 0;
    pthread_t threadArray[256];
    struct chunkInfo chunkArray[256];
    clock_t start, end;

    long long int stepSize = arraySize / threadsCount;
    long long int stepResidue = arraySize % threadsCount;
    
    int step = 0, i;
    
    start = clock();
    for(i=0; i<threadsCount; i++){
        chunkArray[i].start = step;
        chunkArray[i].end = step+(stepSize-1);
        chunkArray[i].v = array;
        pthread_create(&threadArray[i], NULL, chunkSum, &chunkArray[i]);
        step += stepSize;
        if(i==0){
            chunkArray[i].end += stepResidue;
            step += stepResidue;
        }
    }
    for(i=0; i<threadsCount; i++)
        pthread_join(threadArray[i], NULL);
    end = clock();

    struct arraySumInfo result;
    result.sum = totalSum;
    result.secondsElapsed = ((double)(end - start) / CLOCKS_PER_SEC);

    return result;
}


int main(int argc, char **argv){
    long long int arraySize;
    int threadsCount;
    srand(time(NULL));
    for(arraySize=pow(10, 7); arraySize <= pow(10, 9); arraySize*=10){
        long long int stepSize, stepResidue;

        for(threadsCount=1; threadsCount <= 256; threadsCount*=2){
            int loop = 0;
            double totalSeconds;

            // Loop para calcular tempo médio de calculo distribuído
            for(loop = 0; loop <= 10; loop++){
                char *array = random_array(arraySize);
                struct arraySumInfo result = distributedArraySum(array, arraySize, threadsCount);
                totalSeconds += result.secondsElapsed;
                if(result.sum != arraySum(array, arraySize)){
                    printf("Soma distribuida diverge.\n"); 
                    return 0;
                }
                free(array);
            }

            printf("Para N = %lli e K = %d:\n", arraySize, threadsCount);
            printf("Tempo medio: %.4f segundos\n", totalSeconds/10.0);
            printf("----------------------------------------------\n");
        }
    }
}
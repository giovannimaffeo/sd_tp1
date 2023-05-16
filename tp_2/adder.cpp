#include <cstdlib>
#include <cstdio>
#include <vector>
#include <atomic>
#include <thread>
#include <utility>
#include <chrono>
#include <cmath>
#include <iostream>


using namespace std;
using namespace std::chrono;

long long int totalSum;
atomic_flag lockHeld = ATOMIC_FLAG_INIT;

void acquire(atomic_flag *lock){
    while (lock->test_and_set());
}

void release(atomic_flag *lock){
    lock->clear();
}

vector<char> randomArray(int arrayLen){
    vector<char> v;
    for (int i = 0; i < arrayLen; i++)
        v.push_back( (char) (rand() % 201)-100);
    return v;
}

long long int arraySum(vector<char> array){
    long long int sum = 0, i;
    for(i=0; i<array.size(); i++){
        sum += (int) array[i];
    }
    return sum;
}

void chunkSum(vector<char> array){

    int i;
    long long int localStorage = 0;
    for(int v: array){
        localStorage += v;
    }
    
    acquire(&lockHeld);
    totalSum += localStorage;
    release(&lockHeld);
}


pair<long long int, double> distributedArraySum(vector <char> array, int threadsCount){
    /* 
        Recebe um array e cria <threadsCount> (no máximo 256) threads para realizar
        a soma de seus elementos de maneira distribuída.
        Retorna o tempo total utilizado no cálculo da soma.
    */

    long long int stepSize = array.size() / threadsCount;
    long long int stepResidue = array.size() % threadsCount;
    totalSum = 0;
    
    int step = 0, i;
    size_t begin = 0, end = 0;
    vector<vector<char>> chunkArray;
    for (i = 0; i < threadsCount; ++i){
	    if(stepResidue > 0){
	        end += stepSize + 1;
	        stepResidue--;
	    }
	    else{
	        end += stepSize;
	    }

        chunkArray.emplace_back(array.begin() + begin, array.begin() + end);

        begin = end;
    }
    
    thread threadArray[256] = {};
    auto start = high_resolution_clock::now();
    for(i=0; i<threadsCount; i++){
        threadArray[i] = thread(chunkSum, chunkArray[i]);
    }
    for(i=0; i<threadsCount; i++)
        threadArray[i].join();
    auto finish = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(finish - start);
    double timeElapsed = (double) (duration.count())/1000.0;

    pair<long long int, double> result;
    acquire(&lockHeld);
    long long int resultSum = totalSum;
    release(&lockHeld);
    result = make_pair(resultSum, timeElapsed);

    return make_pair(resultSum, timeElapsed);
}


int main(){
    srand(time(NULL));

    long long int arraySize;
    int threadsCount;
    srand(time(NULL));
    for(arraySize=pow(10, 7); arraySize <= pow(10, 9); arraySize*=10){
        long long int stepSize, stepResidue;

        for(threadsCount=1; threadsCount <= 256; threadsCount*=2){
            int loop = 0;
            double totalSeconds = 0;

            printf("Para N = %lli e K = %d:\n", arraySize, threadsCount);
            // Loop para calcular tempo médio de calculo distribuído
            for(loop = 0; loop <= 10; loop++){
                vector<char> array = randomArray(arraySize);
                pair<long long int, double> result = distributedArraySum(array, threadsCount);
                int i;
                totalSeconds += result.second;
                if(result.first != arraySum(array)){
                    printf("Soma distribuida diverge.\n");
                    printf("Distribuida: %lli\nProva Real:%lli\n", result.first, arraySum(array)); 
                    return 0;
                }
            }

            printf("Tempo medio: %.4f segundos\n", totalSeconds/10.0);
            printf("----------------------------------------------\n");
        }
    }
}

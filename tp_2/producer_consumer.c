#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

pthread_mutex_t mutex; // semáforo mutex
sem_t full, empty; // semáforos contadores

// buffer
typedef int buffer_item;
#define BUFFER_SIZE 5
buffer_item buffer[BUFFER_SIZE]; 
int counter; // contador do buffer para inserir elementos

// função que insere item ao buffer
int insertItem(buffer_item item) {
   if(counter < BUFFER_SIZE) {
      buffer[counter] = item; 
      counter++;
      return 0;
   } else { 
      // erro quando o buffer está cheio: nunca deveria ocorrer
      return -1; 
   }
}

// função "producer"
void *producer() {
   buffer_item item; // delcara variável item 
   srand(time(NULL)); // incializa semente usando tempo atual

   while(1) {
      item = (rand() % 10000000) + 1; // incializa item a ser inserido no buffer

      sem_wait(&empty); // espera se o semáforo "empty" for zero
      pthread_mutex_lock(&mutex); // tranca o buffer (impede manipulação por outras threads)

      if(insertItem(item)) {
        // erro quando o buffer está cheio: nunca deveria ocorrer
        fprintf(stderr, "produtor: erro ao inserir item no buffer\n");
      } else {
        printf("produtor produziu %d\n", item);
      }

      pthread_mutex_unlock(&mutex); // liberta o buffer 
      sem_post(&full); // incrementa semáforo "full"
   }
}

// função que remove item do buffer
int removeItem(buffer_item *item) {
   if(counter > 0) {
      *item = buffer[(counter - 1)]; 
      counter--;
      return 0;
   } else { 
      // erro quando o buffer está vazio: nunca deveria ocorrer
      return -1;
   }
}

// função que verifica se um número é primo
int isPrime(int num) {
    if (num < 2) return 0;
    for (int i = 2; i < num; i++) {
        if (num % i == 0) return 0;
    }
    return 1;
}

// função "consumer"
void *consumer() {
    buffer_item item; // delcara variável item 

   while(1) {
      sem_wait(&full); // espera se o semáforo "full" for zero
      pthread_mutex_lock(&mutex); // tranca o buffer (impede manipulação por outras threads)

      if(removeItem(&item)) {
        // erro quando o buffer está vazio: nunca deveria ocorrer
        fprintf(stderr, "consumidor: erro ao consumir item do buffer\n");
      } else {
        printf("consumidor consumiu %d que %s primo\n", item, isPrime(item) ? "é" : "não é");
      }

      pthread_mutex_unlock(&mutex); // liberta o buffer 
      sem_post(&empty); // incrementa semáforo "empty"
   }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Erro: número incorreto de argumentos\n");
        printf("Uso correto: %s <N> <Np> <Nc>\n", argv[0]);
        return -1;
    }

    int np = atoi(argv[2]); // número de threads produtoras
    int nc = atoi(argv[3]); // número de threads consumidoras

    // inicialização dos semáforos
    pthread_mutex_init(&mutex, NULL);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);
    counter = 0; // incializa contador do buffer

    // armazena em attr os atributos padrão de uma thread
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // declara arrays para armazenar ids das threads
    pthread_t pThreads[np];
    pthread_t cThreads[nc];

    int i; // loop counter
    // inicia threads produtoras que irão executar função "producer"
    for(i = 0; i < np; i++) {
      pthread_create(&pThreads[i], &attr, producer, NULL);
    }
    // inicia threads produtoras que irão executar função "consumer"
    for(i = 0; i < nc; i++) {
      pthread_create(&cThreads[i], &attr, consumer, NULL);
   }

   /* Sleep for the specified amount of time in milliseconds */
   usleep(10000);

   /* Exit the program */
   printf("Exit the program\n");
   return 0;
}
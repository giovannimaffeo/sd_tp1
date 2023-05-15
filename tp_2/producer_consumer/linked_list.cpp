#include <stdio.h>
#include <cstdio>

// classe para o nó da lista encadeada
class Node {
public:
    int value;
    long long time;
    Node* next;

    // construtor do nó
    Node(int value, long long time) : value(value), time(time), next(nullptr) {}
};

// classe para a lista encadeada
class LinkedList {
public:
    Node* head;
    Node* tail;

    // construtor da lista encadeada
    LinkedList() : head(nullptr), tail(nullptr) {}

    // método para inserir nó na lista encadeada
    void append(int value, long long time) {
        // cria novo nó a ser inserido
        Node* newNode = new Node(value, time);

        if (head == nullptr) {
            // caso a lista esteja vazia, insere nó como head e tail
            head = tail = newNode;
        } else {
            // insere novo nó como último elemento e atualiza o tail
            tail->next = newNode;
            tail = newNode;
        }
    }

    // método para printar a lista encadeada enquanto apaga da memória
    void printWhileDestroyList() {
        FILE* file = fopen("occupancy_list_output", "a"); // abre arquivo em modo "append"

        if (file == nullptr) {
            printf("erro ao abrir o arquivo\n");
            return;
        }

        fprintf(file, "\ncount time\n");
        // percorre a lista encadeada
        while (head != nullptr) {
            Node* current = head; // define nó atual da iteração como head

            fprintf(file, "%d %lld\n", current->value, current->time); // imprime valor e tempo do nó atual
            head = head->next; // define head da lista como o próximo
            delete current; // deleta da memória nó atual
        }
        
        fclose(file); // fecha o arquivo
    }
};
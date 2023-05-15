#include <stdio.h>

// classe para o nó da lista encadeada
class Node {
public:
    int value;
    Node* next;

    // construtor do nó
    Node(int value) : value(value), next(nullptr) {}
};

// classe para a lista encadeada
class LinkedList {
public:
    Node* head;

    // construtor da lista encadeada
    LinkedList() : head(nullptr) {}

    // método para inserir nó na lista encadeada
    void append(int value) {
        // cria novo nó a ser inserido
        Node* newNode = new Node(value);

        if (head == nullptr) {
            // caso a lista esteja vazia, insere nó como head 
            head = newNode;
        } else {
            // percorre lista encadeada até o último nó
            Node* current = head;
            while (current->next != nullptr) { 
                current = current->next;
            }
            // insere novo nó como último elemento
            current->next = newNode;
        }
    }

    // método para printar a lista encadeada enquanto apaga da memória
    void printWhileDestroyList() {
        // percorre a lista encadeada
        while (head != nullptr) {
            Node* current = head; // define nó atual da iteração como head

            printf("%d ", current->value); // imprime valor do nó atual
            head = head->next; // define head da lista como o próximo
            delete current; // deleta da memória nó atual
        }
        printf("\n");
    }
};
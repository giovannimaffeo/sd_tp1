#include <stdio.h>
#include <cstdio>

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
    Node* tail;

    // construtor da lista encadeada
    LinkedList() : head(nullptr), tail(nullptr) {}

    // método para inserir nó na lista encadeada
    void append(int value) {
        // cria novo nó a ser inserido
        Node* newNode = new Node(value);

        if (head == nullptr) {
            // caso a lista esteja vazia, insere nó como head e tail
            head = tail = newNode;
        } else {
            // insere novo nó como último elemento e atualiza o tail
            tail->next = newNode;
            tail = newNode;
        }
    }

    // método para remover o elemento head da fila
    void pop() {
        if (head != nullptr) {
            Node* temp = head;  // armazena o nó da cabeça atual

            head = head->next;  // atualiza o head para o próximo nó

            // Verifica se o nó da cabeça era o único nó na lista
            if (head == nullptr) {
                tail = nullptr;  // atualiza o tail para nullptr
            }

            delete temp;  // libera a memória do nó removido
        }
    }

    // método para obter o comprimento da lista encadeada
    int length() {
        int count = 0;  // contador de nós
        Node* currentNode = head;  // começa a contagem pelo nó da cabeça

        while (currentNode != nullptr) {
            count++;  // incrementa o contador
            currentNode = currentNode->next;  // move para o próximo nó
        }

        return count;
    }

    // método para obter o valor da cabeça da lista encadeada
    int getHeadValue() {
        if (head != nullptr) {
            return head->value;
        } else {
            std::cerr << "A lista está vazia. Não há valor na cabeça." << std::endl;
            // Você pode escolher como lidar com essa situação.
            // Neste exemplo, estamos retornando -1 como um valor inválido.
            return -1;
        }
    }
};
#include <stdio.h>
#include <cstdio>

// Class for the linked list node
class Node {
public:
    int value;
    Node* next;

    // Node constructor
    Node(int value) : value(value), next(nullptr) {}
};

// Class for the linked list
class LinkedList {
public:
    Node* head;
    Node* tail;

    // Linked list constructor
    LinkedList() : head(nullptr), tail(nullptr) {}

    // Method to insert a node into the linked list
    void append(int value) {
        // Create a new node to be inserted
        Node* newNode = new Node(value);

        if (head == nullptr) {
            // If the list is empty, insert the node as both head and tail
            head = tail = newNode;
        } else {
            // Insert the new node as the last element and update the tail
            tail->next = newNode;
            tail = newNode;
        }
    }

    // Method to remove the head element from the list
    void pop() {
        if (head != nullptr) {
            Node* temp = head;  // Store the current head node

            head = head->next;  // Update the head to the next node

            // Check if the head node was the only node in the list
            if (head == nullptr) {
                tail = nullptr;  // Update the tail to nullptr
            }

            delete temp;  // Free the memory of the removed node
        }
    }

    // Method to get the value of the head of the linked list
    int getHeadValue() {
        if (head != nullptr) {
            return head->value;
        } else {
            std::cerr << "The list is empty. There is no value in the head." << std::endl;
            // You can choose how to handle this situation.
            // In this example, we are returning -1 as an invalid value.
            return -1;
        }
    }

    // Method to traverse the linked list nodes and print the value of each node in sequence
    void print(const std::string& phrase) {
        Node* currentNode = head;
        std::string result;

        while (currentNode != nullptr) {
            result += std::to_string(currentNode->value);
            if (currentNode->next != nullptr) {
                result += "-";
            }
            currentNode = currentNode->next;
        }

        std::cout << phrase << result << std::endl;
    }
};

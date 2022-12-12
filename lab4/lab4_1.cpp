#include <cstdio>
#include <process.h>
#include <windows.h>
#include <thread>
#include <iostream>

using namespace std;

// Структура узла списка
struct Node{
    explicit Node(unsigned x) : value(x) {}
    Node* next = nullptr;
    unsigned value = 0;
};

// Класс списка (очереди на односвязном списке)
class Queue{
    public:
    Queue() = default; Queue(const Queue&) = delete;
    Queue& operator = (const Queue&) = delete;
    ~Queue(){
        while (head) {
            auto temp = head;
            head = temp->next;
            delete temp;
        }
    }

    void Push(unsigned x) {
        auto node = new Node(x); // создаем новый узел
        if (tail) tail->next = node; // добавляем в конец
        else head = node;
        tail = node;
    }

    bool Pop(unsigned* out_val) {
        if (!head) return false;
        *out_val = head->value; // пишем извлекаемое значение
        Node* tmp = head;
        head = tmp->next;
        if (!head) tail = nullptr;
        delete tmp;
        return true;
    }

    private:
    Node* head = nullptr; // первый узел
    Node* tail = nullptr; // последний узел
};

//Очереди, общие для всех потоков
Queue S, R;

//Функция потока А
unsigned __stdcall ThreadProcA(void*) {
    unsigned x = 0;
    while(true){
        S.Push(++x);
        cout << x << endl;
        Sleep(1000);
    }
    return 0;
}

// Функция потока B
unsigned __stdcall ThreadProcB(void*) {
    while (true) {
        unsigned x;
        cout << "b: " << x << endl;
        if (S.Pop(&x)) R.Push(x * x);
        else Sleep(1000);
    }
    return 0;
}

// Функция потока C
unsigned __stdcall ThreadProcC(void*) {
    while (true) {
        unsigned x;
        cout << "c: " << x << endl;
        if (S.Pop(&x)) R.Push(x / 3);
        else Sleep(1000);
    }
    return 0;
}

// Функция потока D
unsigned __stdcall ThreadProcD(void*) {
    while (true) {
        unsigned x;
        if (R.Pop(&x)) printf("%u\n", x); // печатаем его
        else {
            puts("R is empty"); // сообщаем, что R - пуста
            Sleep(1000); // ждем секунду
        }
    }
    return 0;
}

int main() {
    const size_t threads_count = 4;
    HANDLE threads[threads_count];
    // создание потоков
    threads[0] = (HANDLE)_beginthreadex(nullptr, 0, ThreadProcA, nullptr, 0, nullptr);
    threads[1] = (HANDLE)_beginthreadex(nullptr, 0, ThreadProcB, nullptr, 0, nullptr);
    threads[2] = (HANDLE)_beginthreadex(nullptr, 0, ThreadProcC, nullptr, 0, nullptr);
    threads[3] = (HANDLE)_beginthreadex(nullptr, 0, ThreadProcD, nullptr, 0, nullptr);

    // ожидание всех потоков
    WaitForMultipleObjects(threads_count, threads, TRUE, INFINITE);
    for (auto h : threads) CloseHandle(h);
    return 0;

}

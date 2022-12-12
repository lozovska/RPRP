#include <cstdio>
#include <process.h>
#include <windows.h>

// Структура узла списка
struct Node {
    explicit Node(unsigned x) : value(x) {}
    Node* next = nullptr; // следующий узел
    unsigned value = 0; // хранимое значение
};

// Класс списка (очереди на односвязном списке)
class Queue {
public:
    Queue() = default; Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;
    ~Queue() { // удаление всех узлов очереди
        while (head) {// пока есть узлы
            auto temp = head; // получаем первый узел
            head = temp->next; // сдвигаем начало очереди на следующий узел
            delete temp; // удаляем узел
        }
    }
    
    // Вставка значения в очередь
    void Push(unsigned x) {
        auto node = new Node(x); // создаем новый узел
        if (tail) tail->next = node; // добавляем в конец
        else head = node;
        tail = node;
    }

    //Извлечение значения из очереди.
    //Если очередь непуста, в out_val записывается
    //извлекаемое значение и возвращается true.
    //Иначе, возвращается false, а out_val не изменяется.
    bool Pop(unsigned* out_val) {
        if (!head) return false;
        *out_val = head->value; // пишем извлекаемое значение
        Node* tmp = head; // этот узел удалим ниже
        head = tmp->next; // передвигаем начало очереди на следующий узел
        
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

//КО для общих очередей
CRITICAL_SECTION csec_s, csec_r;

//Функция потока А
unsigned __stdcall ThreadProcA(void*) {
    unsigned x = 0;
    
    // в цикле добавляем в S значения 1, 2, 3 и т.д.
    while (true) {
        EnterCriticalSection(&csec_s); // Вход в КО для S
        S.Push(++x); // добавляем очередное значение в S
        LeaveCriticalSection(&csec_s); // Выход из КО для S
    }
    return 0;
}

// Функция потока B
unsigned __stdcall ThreadProcB(void*) {
    while (true) {
        unsigned x;
        EnterCriticalSection(&csec_s); // Вход в КО для S
        EnterCriticalSection(&csec_r); // Вход в КО для R
        
        auto extracted = S.Pop(&x); // пытаемся извлечь значение из S
        if (extracted) R.Push(x * x);
        
        LeaveCriticalSection(&csec_r);
        LeaveCriticalSection(&csec_s);

        if (!extracted) Sleep(1000);
    }
    return 0;
}

// Функция потока C
unsigned __stdcall ThreadProcC(void*) {
    while (true) {
        unsigned x;
        EnterCriticalSection(&csec_r);
        EnterCriticalSection(&csec_s);
        
        auto extracted = S.Pop(&x); // пытаемся извлечь значение из S
        if (extracted) R.Push(x / 3);
        
        LeaveCriticalSection(&csec_s);
        LeaveCriticalSection(&csec_r);
        if (!extracted) Sleep(1000);
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
            Sleep(1000);
        }
    }
    return 0;
}

 

int main() {
    const size_t threads_count = 4;
    HANDLE threads[threads_count];
    
    // Инициализация КО
    InitializeCriticalSection(&csec_s);
    InitializeCriticalSection(&csec_r);
    
    // создание потоков
    
    threads[0] = (HANDLE)_beginthreadex(nullptr, 0, ThreadProcA, nullptr, 0, nullptr);
    threads[1] = (HANDLE)_beginthreadex(nullptr, 0, ThreadProcB, nullptr, 0, nullptr);
    threads[2] = (HANDLE)_beginthreadex(nullptr, 0, ThreadProcC, nullptr, 0, nullptr);
    threads[3] = (HANDLE)_beginthreadex(nullptr, 0, ThreadProcD, nullptr, 0, nullptr);
    
    // ожидание всех потоков

    WaitForMultipleObjects(threads_count, threads, TRUE, INFINITE);
    for (auto h : threads)
    CloseHandle(h);

    // Удаление КО
    DeleteCriticalSection(&csec_r);
    DeleteCriticalSection(&csec_s);
    return 0;
}

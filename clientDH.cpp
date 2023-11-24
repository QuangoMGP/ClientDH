#include <iostream>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <mutex>

using namespace std;
long long int endrand = 1000000000; //length of params
const int numt = 2; // количество клиентов

// Метод для создания случайного числа
long long int irand(long long int a, long long int b){
    return a + time(NULL)*rand()%(b-a+1);
}

class Barrier {
public:
    explicit Barrier(size_t numThreads) : threshold(numThreads), count(numThreads), generation(0) {}

    void Wait() {
        unique_lock<mutex> lock(mut);

        // Текущее поколение барьера
        size_t gen = generation;

        // Уменьшаем счетчик потоков
        if (--count == 0) {
            // Если это последний поток, сбрасываем барьер
            count = threshold;
            ++generation;
            // Оповещаем все потоки, что барьер сброшен
            cv.notify_all();
        } else {
            // Если не последний, ждем, пока барьер не сбросится
            cv.wait(lock, [this, gen] { return gen != generation; });
        }
    }

private:
    mutex mut;
    condition_variable cv;
    size_t threshold;
    size_t count;
    size_t generation;
};

class Client {
    private:
        pid_t pid;
        long long int a;
        long long int g;
        long long int p;
        long long int pubK;
        long long int privK;
        long long int B;

        void calcPubKey(){
            pubK = modPow(g, a, p);
        }
        
        void init(){
            a = irand(1, endrand);
            calcPubKey();
        }

        long long int modPow(long long int base, long long int exponent, long long int& modulus) {
            if (modulus == 1) return 0; // Деление на 0 не допускается
            long long int result = 1;
            base = base % modulus;
            while (exponent > 0) {
                // Если exponent нечетный, умножаем результат на base и берем по модулю
                if (exponent % 2 == 1)
                    result = (result * base) % modulus;
                // Теперь exponent четный, делим его пополам и берем base^2 по модулю
                exponent = exponent >> 1;
                base = (base * base) % modulus;
            }
            return result;
        }


    public:
        Client(long long int g, long long int p): g(g), p(p){
            // isWaiting = true;
            // cout << getPid();
            init();
        }

        pid_t getPid(){
            pid = getpid();
            return pid;
        }
        long long int getPubKey(){
            return pubK;
        }
        void calcPrivKey(long long int& exKey){
            B = exKey;
            privK = modPow(B, a, p);
        }
        long long int getPrivKey(){
            return privK;
        }
        void print(){
            cout << "a: " << a << "\ng: " << g << "\np: " << p << "\nB: " << B << "\npubKey: " << pubK << "\nprivKey: " << privK << '\n';
        }
};


// Генерирует буффер
long long int buff[numt];
void genBuff(){
    for (int i = 0; i < numt; ++i)
        buff[i] = irand(1, endrand);
}

// Потоки клиентов
Barrier barrier(numt);
void clientsHandler(int threadNum) {
    // Считываем из буфера
    Client cli(buff[0], buff[1]);
    long long int pubKey = cli.getPubKey();
    cout << "Client num: " << threadNum+1 <<"   PubKey:  "<< pubKey << "\n";
    barrier.Wait();
    // Кладём публичный ключ в свою ячейку
    buff[threadNum] = pubKey;
    barrier.Wait();
    // Берем чужой ключ из другой ячейки
    cli.calcPrivKey(buff[(threadNum+1)%numt]);
    cout << "Client num: " << threadNum+1 << "   PrivKey: "<< cli.getPrivKey() << '\n';
}

// дочерний поток для подготовки буффера и начала генерации ключей
mutex myMutex;
void queueHandler(int threadMainNum){
        cout << "Создан запрос №" << threadMainNum << " на инициализацию безопасного соединения" << endl;
        // Блокируем доступ к рессурсам пока не пройдёт инициализация
        {
            unique_lock<mutex> lock(myMutex);
            genBuff();
            thread threads[numt];
            for (int i = 0; i< numt; ++i)
                threads[i] = thread(clientsHandler, i);
            for (int i = 0; i< numt; ++i)
                threads[i].join();
            lock.unlock();
        }
}


int queue = 1;
int num = queue;
thread mainThreads[5];
// Обработкич сигналов создаёт поток
void signalHandler(int signum) {
    if (signum == SIGUSR1) {
        mainThreads[queue] = thread(queueHandler, num);
        mainThreads[queue].join();
        ++queue;
        ++num;
        if (queue == 100){
            queue = 1;
        }
    }
}

int main() {
    signal(SIGUSR1, signalHandler);
    pid_t pid = getpid();
    cout << "\nPID: " << pid << "\n";

    while (true)
    {
        sleep(1);
    }
    return 0;
}
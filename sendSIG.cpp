#include <iostream>
#include <csignal>
#include <cstdlib>
#include <unistd.h>


int main() {
    // Получаем PID (идентификатор процесса) другого процесса
    pid_t targetProcessPID;
    int queue; // количество повторов сигнала
    std::cout << "\nPID: ";
    std::cin >> targetProcessPID;
    std::cout << "Amount: ";
    std::cin >> queue;

    int i = 0;
    while (i<queue){
        // Посылаем сигнал SIGUSR1
        if (kill(targetProcessPID, SIGUSR1) == 0) {
            std::cout << "Сигнал №"<< i+1 <<" SIGUSR1 успешно отправлен процессу с PID " << targetProcessPID << std::endl;
        } else {
            std::cerr << "Ошибка при отправке сигнала SIGUSR1" << std::endl;
            return EXIT_FAILURE;
        }
        sleep(1);
        ++i;
    }
    return EXIT_SUCCESS;
}
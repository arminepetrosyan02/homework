#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY 1234
#define SEM_KEY 5678

struct SharedMemory {
    int function;
    int operand1;
    int operand2;
    int result;
};

void semWait(int semId) {
    struct sembuf semBuf;
    semBuf.sem_num = 0;
    semBuf.sem_op = -1;
    semBuf.sem_flg = 0;
    semop(semId, &semBuf, 1);
}

void semSignal(int semId) {
    struct sembuf semBuf;
    semBuf.sem_num = 0;
    semBuf.sem_op = 1;
    semBuf.sem_flg = 0;
    semop(semId, &semBuf, 1);
}

int main() {
    int shmId = shmget(SHM_KEY, sizeof(SharedMemory), 0666);
    if (shmId == -1) {
        std::cerr << "Failed to get shared memory segment!" << std::endl;
        return 1;
    }

    SharedMemory* sharedMemory = static_cast<SharedMemory*>(shmat(shmId, nullptr, 0));
    if (sharedMemory == reinterpret_cast<SharedMemory*>(-1)) {
        std::cerr << "Failed to attach shared memory segment!" << std::endl;
        return 1;
    }
    int semId = semget(SEM_KEY, 1, 0666);
    if (semId == -1) {
        std::cerr << "Failed to get semaphore!" << std::endl;
        return 1;
    }

    while (true) {
        int function;
        std::cout << "Enter the function number (0 for addition, 1 for subtraction, 2 for division, 3 for multiplication): ";
        std::cin >> function;

        int operand1, operand2;
        std::cout << "Enter two integers: ";
        std::cin >> operand1 >> operand2;

        sharedMemory->function = function;
        sharedMemory->operand1 = operand1;
        sharedMemory->operand2 = operand2;
        semSignal(semId);
        semWait(semId);
        std::cout << "Result: " << sharedMemory->result << std::endl;
    }
    if (shmdt(sharedMemory) == -1) {
        std::cerr << "Failed to detach shared memory segment!" << std::endl;
        return 1;
    }

    return 0;
}


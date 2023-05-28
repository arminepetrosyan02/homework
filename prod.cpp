#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

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
    int shmId = shmget(SHM_KEY, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shmId == -1) {
        std::cerr << "Failed to create shared memory segment!" << std::endl;
        return 1;
    }
    SharedMemory* sharedMemory = static_cast<SharedMemory*>(shmat(shmId, nullptr, 0));
    if (sharedMemory == reinterpret_cast<SharedMemory*>(-1)) {
        std::cerr << "Failed to attach shared memory segment!" << std::endl;
        return 1;
    }
    int semId = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semId == -1) {
        std::cerr << "Failed to create semaphore!" << std::endl;
        return 1;
    }
    semctl(semId, 0, SETVAL, 0);

    while (true) {
        semWait(semId);
        if (sharedMemory->function < 0 || sharedMemory->function > 3) {
            std::cerr << "Invalid function number!" << std::endl;
            continue;
        }
        switch (sharedMemory->function) {
            case 0:
                sharedMemory->result = sharedMemory->operand1 + sharedMemory->operand2;
                break;
            case 1:
                sharedMemory->result = sharedMemory->operand1 - sharedMemory->operand2;
                break;
            case 2:
                if (sharedMemory->operand2 == 0) {
                    std::cerr << "Cannot divide by zero!" << std::endl;
                    continue;
                }
                sharedMemory->result = sharedMemory->operand1 / sharedMemory->operand2;
                break;
            case 3:
                sharedMemory->result = sharedMemory->operand1 * sharedMemory->operand2;
                break;
            default:
                std::cerr << "Invalid function number!" << std::endl;
        }
        semSignal(semId);
    }
    if (shmdt(sharedMemory) == -1) {
        std::cerr << "Failed to detach shared memory segment!" << std::endl;
        return 1;
    }

    if (shmctl(shmId, IPC_RMID, nullptr) == -1) {
        std::cerr << "Failed to remove shared memory segment!" << std::endl;
        return 1;
    }
    if (semctl(semId, 0, IPC_RMID) == -1) {
        std::cerr << "Failed to remove semaphore!" << std::endl;
        return 1;
    }

    return 0;
}


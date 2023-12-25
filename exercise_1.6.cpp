#include <iostream>
#include <sys/wait.h>
#include <chrono>

int main() {
    // Main process
    int pd1[2];
    int pd2[2];
    int count = 20;

    if (pipe(pd1) < 0 || pipe(pd2) < 0) {
        std::cout << "Error creating the pipes" << std::endl;
        return -1;
    }

    int pid = fork();
    if (pid == 0) {
        // Child process
        close(pd1[1]); // Closes the writing head for the first pipeline
        close(pd2[0]); // Closes the reading head for the second pipeline

        char message[1]; // container to be read into
        while (count > 0) {
            read(pd1[0], message, 1); // read 1 byte from the reading head
            std::cout << "In child process: " << message << std::endl;
            write(pd2[1], message, 1); // Sends the message through the pipe
            --count;
        }
        close(pd1[0]);
        close(pd2[1]);
    }
    else if (pid < 0) {
        std::cout << "Error creating the process" << std::endl;
        return -1;
    }
    else {
        // Main process
        close(pd1[0]); // Closes the reading head for the first pipeline
        close(pd2[1]); // Closes the writing head for the second pipeline

        char message[1] = {'a'}; // Message to be sent
        auto start = std::chrono::high_resolution_clock::now();
        const int countCache = count;
        while (count > 0) {            
            write(pd1[1], message, 1); // Sends the message through the pipe
            read(pd2[0], message, 1); // read 1 byte from the reading head
            std::cout << "In parent process: " << message << std::endl; // Print message
            --count;
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time per exchange in ns: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / countCache << std::endl;

        wait(0);
        close(pd1[1]);
        close(pd2[0]);
    }
}

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*
This program requires 2 additional programs to be present in the same directory:
- left
- right

The left and right programs should take 2 integers from the standard input and print the result to the standard output.
Rest is handled by this program.
*/

int currentDepth;
int maxDepth;
int leftRight;

// function to handle child processing
int processChild(int leftRight, int toChild) {
    char s_currDepth[10], s_maxDepth[10];
    snprintf(s_currDepth, sizeof(s_currDepth), "%d", currentDepth + 1);
    snprintf(s_maxDepth, sizeof(s_maxDepth), "%d", maxDepth);

    int parentToChild[2], childToParent[2];

    // create pipes
    if (pipe(parentToChild) < 0 || pipe(childToParent) < 0) {
        fprintf(stderr, "Failed to create pipes for child: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // child processes
        dup2(parentToChild[0], STDIN_FILENO);
        dup2(childToParent[1], STDOUT_FILENO);

        close(parentToChild[0]);
        close(parentToChild[1]);
        close(childToParent[0]);
        close(childToParent[1]);

        char *args[] = {"./treePipe", s_currDepth, s_maxDepth, leftRight ? "1" : "0", NULL};
        execvp(args[0], args);

        // fail 
        fprintf(stderr, "Execvp failed: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // parent process
    close(parentToChild[0]);
    close(childToParent[1]);

    // send to child
    dprintf(parentToChild[1], "%d\n", toChild);
    close(parentToChild[1]);

    // read child output
    char buffer[32];
    wait(NULL); // wait for the child
    read(childToParent[0], buffer, sizeof(buffer) - 1);
    close(childToParent[0]);

    buffer[sizeof(buffer) - 1] = '\0';
    return atoi(buffer);
}

int processParent(int leftRight, int num1, int num2) {
    int parentToChild[2], childToParent[2];

    // create pipes
    if (pipe(parentToChild) < 0 || pipe(childToParent) < 0) {
        fprintf(stderr, "Failed to create pipes for parent: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // child processes
        dup2(parentToChild[0], STDIN_FILENO);
        dup2(childToParent[1], STDOUT_FILENO);

        close(parentToChild[0]);
        close(parentToChild[1]);
        close(childToParent[0]);
        close(childToParent[1]);

        char *args[] = {leftRight == 0 ? "./left" : "./right", NULL};
        execvp(args[0], args);

        // Execvp failed
        fprintf(stderr, "Execvp failed: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    close(parentToChild[0]);
    close(childToParent[1]);

    // send to child
    dprintf(parentToChild[1], "%d %d\n", num1, num2);
    close(parentToChild[1]);

    // read result from the child
    char buffer[32];
    wait(NULL); // wait for the child process
    read(childToParent[0], buffer, sizeof(buffer) - 1);
    close(childToParent[0]);

    buffer[sizeof(buffer) - 1] = '\0';
    return atoi(buffer);
}


void printState(int depth, int lr) {
    for (int i = 0; i < depth; i++) {
        fprintf(stderr, "---");
    }
    fprintf(stderr, "> depth: %d, lr: %d\n", depth, lr);
}


void printOutput(const char *message, int value, int depth) {
    for (int i = 0; i < depth; i++) {
        fprintf(stderr, "---");
    }
    fprintf(stderr, "> %s: %d\n", message, value);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <current depth> <max depth> <left or right>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    currentDepth = atoi(argv[1]);
    maxDepth = atoi(argv[2]);
    leftRight = atoi(argv[3]);

    int num1 = 0, num2 = 1, result = -1;

    // leaf node behavior
    if (currentDepth == maxDepth) {
        printState(currentDepth, leftRight);
        scanf("%d", &num1);
        printOutput("num1", num1, currentDepth);

        result = processParent(leftRight, num1, num2);
        printOutput("result", result, currentDepth);

        printf("%d\n", result);
        return 0;
    }

    // intermediate or root behavior
    printState(currentDepth, leftRight);
    if (currentDepth == 0) {
        fprintf(stderr, "Enter num1 for root: ");
    }
    scanf("%d", &num1);
    printOutput("num1", num1, currentDepth);

    // process left child
    num2 = processChild(0, num1);

    // process parent node
    result = processParent(leftRight, num1, num2);
    printOutput("result", result, currentDepth);

    // process right child
    num2 = processChild(1, result);

    if (currentDepth == 0) {
        fprintf(stderr, "Final result: %d\n", num2);
    } else {
        printf("%d\n", num2);
    }

    return 0;
}

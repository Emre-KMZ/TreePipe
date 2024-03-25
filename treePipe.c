#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int currentDepth;
int maxDepth;
int leftRight;

int processChild(int leftRight, int toChild){
    // if (currentDepth > maxDepth){ // Base case
    //     return 1;
    // }

    char s_currDepth[10];
    char s_maxDepth[10];
    snprintf(s_currDepth, sizeof(s_currDepth), "%d", currentDepth+1);
    snprintf(s_maxDepth, sizeof(s_maxDepth), "%d", maxDepth);

    int p2c_l[2]; // Parent to child left pipe, p2c[0] is read end, p2c[1] is write end from parent perspective
    int c2p_l[2]; // Child to parent left pipe

    // Create pipes
    if (pipe(p2c_l) < 0 || pipe(c2p_l) < 0){fprintf(stderr, "Failed to create pipes for left child: %d %s\n", errno, strerror(errno));exit(EXIT_FAILURE);}

    int retval = -1;
    int pid1 = fork();
    if (pid1 < 0){fprintf(stderr, "Fork 1 failed: %d %s\n", errno, strerror(errno));exit(EXIT_FAILURE);}
    if (pid1 == 0){   // Processing child

        dup2(p2c_l[0], 0);  // STDIN_FILENO?
        dup2(c2p_l[1], 1);  // STDOUT_FILENO?

        close(c2p_l[0]);
        close(c2p_l[1]);
        close(p2c_l[0]);
        close(p2c_l[1]);

    
        char *args[] = {"./treePipe", s_currDepth, s_maxDepth, "0", NULL};
        if (leftRight == 1){
            args[3] = "1";
        }
        execvp(args[0], args); // Recursive Call

        fprintf(stderr, "processing child process failed: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    else{     // Parent process
        close(c2p_l[1]);
        close(p2c_l[0]);
        // close(p2c_l[1]);

        char toChildStr[10];
        snprintf(toChildStr, sizeof(toChildStr), "%d ", toChild);
        write(p2c_l[1], toChildStr, sizeof(toChildStr));
        close(p2c_l[1]);

        //read new number from left child and make it a int
        char newNumStr[10];
        wait(NULL);
        read(c2p_l[0], newNumStr, sizeof(newNumStr));
        int newNum = atoi(newNumStr);
        // fprintf(stderr, "readed inside processChild: %d\n", newNum);
        close(c2p_l[0]);

        retval = newNum;
    }
    return retval;
}

int processParent(int leftRight, int num1, int num2){
    
    int p2c[2]; // Parent to child right pipe
    int c2p[2]; // Child to parent right pipe

    // Create pipes
    if (pipe(p2c) < 0 || pipe(c2p) < 0){fprintf(stderr, "Failed to create pipes for right child: %d %s\n", errno, strerror(errno));exit(EXIT_FAILURE);}
    int pid2 = fork();
    if (pid2 < 0){fprintf(stderr, "Fork 2 failed: %d %s\n", errno, strerror(errno));exit(EXIT_FAILURE);}
    if (pid2 == 0){   // Processing child
        
        dup2(p2c[0], 0);  // STDIN_FILENO?
        dup2(c2p[1], 1);  // STDOUT_FILENO?

        close(c2p[0]);
        close(c2p[1]);
        close(p2c[0]);
        close(p2c[1]);

        if (leftRight == 0){
            char *args[] = {"./left", NULL};
            execvp(args[0], args);
        }
        else{
            char *args[] = {"./right", NULL};
            execvp(args[0], args);
        }

        fprintf(stderr, "Processor process failed: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    else{     // Parent process
        close(c2p[1]);
        close(p2c[0]);

        char toChildStr[10];
        // snprintf(toChildStr, sizeof(toChildStr), "%d %d", num1, num2); //
        // write(p2c[1], toChildStr, sizeof(toChildStr));
        // THERE IS A POTENTIAL BUG HERE ############################################################################################################

        dprintf(p2c[1], "%d %d\n", num1, num2);

        close(p2c[1]);

        //read new number from right child and make it a int
        char newNumStr[10];
        wait(NULL);
        read(c2p[0], newNumStr, sizeof(newNumStr));

        
        int newNum = atoi(newNumStr);
        close(c2p[0]);

        return newNum;
    }

}

void statePrinter(int currentDepth, int leftRight){
    char output[150] = "";
    
    for (int i = 0; i < currentDepth; i++){
        strcat(output, "---");
    }
    strcat(output, "> current depth: ");
    char currDepthStr[10];
    snprintf(currDepthStr, sizeof(currDepthStr), "%d", currentDepth);
    strcat(output, currDepthStr);

    strcat(output, ", lr: ");
    char lrStr[10];
    snprintf(lrStr, sizeof(lrStr), "%d\n", leftRight);
    strcat(output, lrStr);

    fprintf(stderr, "%s", output);
   
}
void outputGenerator(char* outString, int outVal){
    char output[150] = "";
    
    for (int i = 0; i < currentDepth; i++){
        strcat(output, "---");
    }
    strcat(output, "> ");
    strcat(output, outString);
    char outValStr[10];
    snprintf(outValStr, sizeof(outValStr), "%d\n", outVal);
    strcat(output, outValStr);

    fprintf(stderr, "%s", output);


    
}
void bigOutputGenerator(int currentDepth, int leftRight, int num1, int num2){
    // ---> current depth: 1, lr: 0, my num1: 2, my num2: 3
    char output[150] = "";
    
    for (int i = 0; i < currentDepth; i++){
        strcat(output, "---");
    }
    strcat(output, "> current depth: ");
    char currDepthStr[10];
    snprintf(currDepthStr, sizeof(currDepthStr), "%d", currentDepth);
    strcat(output, currDepthStr);

    strcat(output, ", lr: ");
    char lrStr[10];
    snprintf(lrStr, sizeof(lrStr), "%d", leftRight);
    strcat(output, lrStr);

    strcat(output, ", my num1: ");
    char num1Str[10];
    snprintf(num1Str, sizeof(num1Str), "%d", num1);
    strcat(output, num1Str);

    strcat(output, ", my num2: ");
    char num2Str[10];
    snprintf(num2Str, sizeof(num2Str), "%d\n", num2);
    strcat(output, num2Str);

    fprintf(stderr, "%s", output);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <current depth> <max depth> <left- right >\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num1, num2, res = -1;

    currentDepth = atoi(argv[1]);
    maxDepth = atoi(argv[2]);
    leftRight = atoi(argv[3]);

    if (currentDepth == maxDepth){ // Leaf node behaviour
        num2 = 1;

        statePrinter(currentDepth, leftRight);

        
        scanf("%d", &num1);

        outputGenerator("my num1 is: ", num1);

        res = processParent(leftRight, num1, num2);
        // fprintf(stderr, "~res: %d, num1: %d, num2: %d\n", res, num1, num2);

        outputGenerator("my result is: ", res);

        printf("%d", res);

        return 0;
        
    }

    // outputGenerator((int[]){leftRight,currentDepth, -1, -1, -1});

    
    statePrinter(currentDepth, leftRight);

    if (currentDepth == 0){
        fprintf(stderr, "Please enter num1 for the root: ");
    }
    scanf("%d", &num1);

    outputGenerator("my num1 is: ", num1);

    //Left child here
    num2 = processChild(0, num1);

    bigOutputGenerator(currentDepth, leftRight, num1, num2);



    res = processParent(leftRight, num1, num2);

    outputGenerator("my result is: ", res);    
  

    num2 = processChild(1, res); // I am not sure about this line - should I really give res as input? ################################################################



    if (currentDepth == 0){
        fprintf(stderr,"The final result is: %d\n", num2);
    }    
    else{
        printf("%d", num2);
    }

    return 0;
}


/*
    potential bugs:
        base case/leaf node handling,
        variables names and usage,
        communication problems between parent and child processes?,



We give parent res to right childs num2
then we print right childs result to parent(stdout)
*/

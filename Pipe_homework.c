#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv){
	if (argc < 3) {
        printf("Invalid number of arguments.\n");
        return 1;
    }

    int arg_count = argc - 1;
    int* fd = (int*)calloc(2 * (arg_count - 1), sizeof(int));
    
    for(int i = 0; i < arg_count - 1; ++i) {
        if(pipe(&(fd[2 * i]))<0) {
            perror("Failed to create pipe");
            return 2;
        }
    }
    
    pid_t* pid = (pid_t*) calloc(arg_count, sizeof(pid_t));
    pid[0] = fork();
    if(pid[0] < 0) {
        perror("Failed to fork 1st child.");
        return 3;
    }
    if(pid[0] == 0) {
        close(fd[0]);
        for(int i = 2; i < (argc - 1) * 2; ++i){
            close(fd[i]);
        }

        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        if (execlp(argv[1], argv[1],NULL)<0) {
            perror("Failed to execute 1st process.");
            return 4;
        }
    }

    for(int i = 1; i < arg_count - 1; ++i){
        pid[i] = fork();
        if(pid[i] < 0) {
            perror("Failed to fork i-th child.");
            return 5;
        }
        if(pid[i]==0) {
            dup2(fd[(i-1) * 2], STDIN_FILENO);
            dup2(fd[i * 2 + 1], STDOUT_FILENO);
            for(int j = 0; j < (arg_count - 1) * 2; ++j){
                close(fd[j]);
            }

            if(execlp(argv[i + 1 ],argv[i + 1],NULL)<0) {
                perror("Failed to execute i-th process.");
                return 6;
            }
        }
    }
    pid[arg_count-1] = fork();
    if(pid[arg_count - 1] < 0) {
        perror("Failed to fork last child.");
        return 7;
    }
    if(pid[arg_count - 1] == 0) {
        dup2(fd[2 * (arg_count - 2)], STDIN_FILENO);
        for (int i = 0; i < 2 * (arg_count - 1); ++i){
            close(fd[i]);
        }

        if(execlp(argv[arg_count],argv[arg_count],NULL)<0) {
            perror("Failed to execute last process.");
            return 8;
        }
    }

    for (int i = 0; i < 2 * (arg_count - 1); ++i){
        close(fd[i]);
    }

    for(int i = 0; i < arg_count; ++i) {
        waitpid(pid[i], NULL, 0);
    }
    
    free(fd);
    free(pid);
    return 0;
	
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#define BUFFER_SIZE 100

void handle_interrupt(int signal) {
	printf("\n 인터럽트 시그널이 발생하였습니다.\n");
    exit(signal);  // 프로그램 종료
}

void ls(int narg, char **argv) {
    char temp[256];
    if (narg == 1) {
        getcwd(temp, 256);
        printf("%s\n", temp);
        argv[1] = temp;
    }

    DIR *pdir;
    struct dirent *pde;
    int i = 0;
    if ((pdir = opendir(argv[1])) == NULL) {
        perror("(Error) Opendir: ");
        return;
    }
    printf("\n");
    while ((pde = readdir(pdir)) != NULL) {
        printf("%-20s", pde->d_name);
        if (++i % 3 == 0)
            printf("\n");
    }
    printf("\n");
    closedir(pdir);
}

void pwd() {
    char cwd[BUFFER_SIZE];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
}

void rm(char *path) {
    if (remove(path) != 0) {
        perror("(Error) Rm: ");
    }
}

void mv(char *src, char *dest) {
    if (rename(src, dest) != 0) {
        perror("(Error) Mv: ");
    }
}

void cat(char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("(Error) Cat: ");
        return;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        printf("%s", buffer);
    }

    fclose(file);
}

void cp(char *src, char *dest) {
    FILE *source = fopen(src, "rb");
    if (source == NULL) {
        perror("(Error) Cp (Source): ");
        return;
    }

    FILE *destination = fopen(dest, "wb");
    if (destination == NULL) {
        fclose(source);
        perror("(Error) Cp (Destination): ");
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, source)) > 0) {
        fwrite(buffer, 1, bytesRead, destination);
    }

    fclose(source);
    fclose(destination);
}

void ln(char *src, char *dest) {
    if (symlink(src, dest) != 0) {
        perror("(Error) Ln: ");
    }
}

void mkdir_func(char *path) {
    if (mkdir(path, 0777) != 0) {
        perror("(Error) Mkdir: ");
    }
}

void rmdir_func(char *path) {
    if (rmdir(path) != 0) {
        perror("(Error) Rmdir: ");
    }
}

void cd(char *path) {
    if (chdir(path) != 0) {
        perror("(Error) Cd: ");
   }
}
void background() {
    printf("\n함수가 백그라운드로 돌아갑니다.\n");
}
int main() {
    char command[BUFFER_SIZE];
    char fullCommand[BUFFER_SIZE + 20];  // 경로 포함한 명령어 저장할 변수

    signal(SIGINT, handle_interrupt);
    signal(SIGQUIT, handle_interrupt);

    while (1) {
        char cwd[BUFFER_SIZE];
        getcwd(cwd, sizeof(cwd));  // 현재 작업 디렉토리 경로 얻기

        printf("%s $ ", cwd);  // 현재 작업 디렉토리 경로와 함께 프롬프트 출력
        fgets(command, BUFFER_SIZE, stdin);  // 사용자 입력 받기

        // 개행 문자 제거
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0) {
	    printf(" \n bye bye \n " );
            break;  // "exit" 입력 시 종료
        }

        // 파이프라인 또는 리다이렉션 기호 확인
        char* pipe = strchr(command, '|');
        char* redirect_in = strchr(command, '<');
        char* redirect_out = strchr(command, '>');

        if (pipe != NULL || redirect_in != NULL || redirect_out != NULL) {
            // 파이프라인 또는 리다이렉션을 포함한 명령어 실행
            pid_t pid = fork();  // 자식 프로세스 생성
            if (pid == -1) {
                perror("(Error) fork: ");
                continue;
            } else if (pid == 0) {
                // 자식 프로세스에서 명령어 실행
                system(command);
                exit(0);  // 자식 프로세스 종료
            } else {
                // 부모 프로세스는 자식 프로세스의 종료를 기다림
                wait(NULL);
            }
        } else if (strcmp(command, "ls") == 0) {
            char *args[] = {"", NULL};  // 첫 번째 인자는 실행 파일명이므로 비워둠
            ls(1, args);  // ls 함수 호출
        } else if (strcmp(command, "pwd") == 0) {
            pwd();  // pwd 함수 호출
        } else if (strncmp(command, "rm ", 3) == 0) {
            char *path = command + 3;  // "rm "을 제외한 경로
            rm(path);  // rm 함수 호출
        } else if (strncmp(command, "mv ", 3) == 0) {
            char *src = strtok(command + 3, " ");
            char *dest = strtok(NULL, " ");
            mv(src, dest);  // mv 함수 호출
        } else if (strncmp(command, "cat ", 4) == 0) {
            char *path = command + 4;  // "cat "을 제외한 경로
            cat(path);  // cat 함수 호출
        } else if (strncmp(command, "cp ", 3) == 0) {
            char *src = strtok(command + 3, " ");
            char *dest = strtok(NULL, " ");
            cp(src, dest);  // cp 함수 호출
        } else if (strncmp(command, "ln ", 3) == 0) {
            char *src = strtok(command + 3, " ");
            char *dest = strtok(NULL, " ");
            ln(src, dest);  // ln 함수 호출
        } else if (strncmp(command, "mkdir ", 6) == 0) {
            char *path = command + 6;  // "mkdir "을 제외한 경로
            mkdir_func(path);  // mkdir 함수 호출
        } else if (strncmp(command, "rmdir ", 6) == 0) {
            char *path = command + 6;  // "rmdir "을 제외한 경로
            rmdir_func(path);  // rmdir 함수 호출
        } else if (strncmp(command, "cd ", 3) == 0) {
            char *path = command + 3;  // "cd "을 제외한 경로
            cd(path);  // cd 함수 호출
	}else if (strcmp(command, "&") != 0  ) {
            // 백그라운드로 돌아가는 함수 호출
            background();
        } else {
            // 일반적인 명령어 실행
            system(command);
        }
    }

    return 0;
}

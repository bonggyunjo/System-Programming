/**


- System Programming
- Team project 1
- 20213012 조은진 (팀장)
- 20193182 조봉균 (팀원)
- 20213074 백현서 (팀원)


**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#define BUFFER_SIZE 100


char* trim(char* str) {
    char* end;
    // 앞쪽 공백 제거
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0) 
	{
  // 모든 문자가 공백일 경우
        return str;
	}

    // 뒤쪽 공백 제거
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // null character 추가
    end[1] = '\0';

    return str;
}

void pipe_execution(char* command1, char* command2) {
    int fd[2];
    pid_t pid;
    if (pipe(fd) == -1) 
	{
        perror("(Error) pipe: ");
        return;
    }

    pid = fork();

    if (pid == -1) 
	{
        perror("(Error) fork: ");
        return;
    }

    if (pid == 0) 
	{
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        if(system(command1) == -1)
	{
            perror("(Error) system: ");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } 	
	else
	{
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

        if(system(command2) == -1){
            perror("(Error) system: ");
            exit(EXIT_FAILURE);
        }
        wait(NULL);
    }
}


void redirect_in_execution(char* command, char* file) {
    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        perror("(Error) open: ");
        return;
    }
    pid_t pid = fork();
    if (pid == -1)
	{
        perror("(Error) fork: ");
        return;
    }

    if (pid == 0) {
        dup2(fd, STDIN_FILENO);
        close(fd);

        system(command);
        exit(0);
    }
	 else
	 {
        wait(NULL);
    }
}

void redirect_out_execution(char* command, char* file) {
 int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("(Error) open: ");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("(Error) fork: ");
        return;
    }

    if (pid == 0) {
        dup2(fd, STDOUT_FILENO);
        close(fd);

        system(command);
        exit(0);
    } else {
        wait(NULL);
    }
}


void execute_in_background(char* command) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("(Error) fork: ");
        return;
    } else if (pid == 0) {
        system(command);
        exit(0);
    } else {
    }
}

void handler_SIGINT(int signal){
	printf("Ctrl + C\n");
	printf("quit (core dumped)\n");
	exit(1);
}

void handler_SIGQUIT(int signal){
	printf("Ctrl + Z \n");
	printf("quit (core dumped)\n");
	
	exit(1);
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
int main() {
    char command[BUFFER_SIZE];
    char fullCommand[BUFFER_SIZE + 20];  // 경로 포함한 명령어 저장할 변수

	signal(SIGINT, handler_SIGINT);
	signal(SIGTSTP, handler_SIGQUIT);

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
 	char* pipe = strchr(command, '|');
        char* redirect_in = strchr(command, '<');
        char* redirect_out = strchr(command, '>');
        char* background = strchr(command, '&');

        if(background != NULL) {
            *background = '\0';
            printf("\nRunning in background\n");
            execute_in_background(command);
        } else if (pipe != NULL) {
            *pipe = '\0';
            char* command1 = trim(command);
            char* command2 = trim(pipe + 1);

            pipe_execution(command1, command2);
        } else if (redirect_in != NULL) {
            *redirect_in = '\0';
            char* command = trim(command);
            char* file = trim(redirect_in + 1);

            redirect_in_execution(command, file);
        } else if (redirect_out != NULL) {
            *redirect_out = '\0';
       char* command = trim(command);
            char* file = trim(redirect_out + 1);

            redirect_out_execution(command, file);
        } else if (strcmp(command, "ls") == 0) {
            char *args[] = {"", NULL};
            ls(1, args);
        } else if (strcmp(command, "pwd") == 0) {
            pwd();
        } else if (strncmp(command, "rm ", 3) == 0) {
            char *path = command + 3;
            rm(path);
        } else if (strncmp(command, "mv ", 3) == 0) {
            char *src = strtok(command + 3, " ");
            char *dest = strtok(NULL, " ");
            mv(src, dest);
        } else if (strncmp(command, "cat ", 4) == 0) {
            char *path = command + 4;
       cat(path);
        } else if (strncmp(command, "cp ", 3) == 0) {
            char *src = strtok(command + 3, " ");
            char *dest = strtok(NULL, " ");
            cp(src, dest);
        } else if (strncmp(command, "ln ", 3) == 0) {
            char *src = strtok(command + 3, " ");
            char *dest = strtok(NULL, " ");
            ln(src, dest);
        } else if (strncmp(command, "mkdir ", 6) == 0) {
            char *path = command + 6;
           mkdir_func(path);
        } else if (strncmp(command, "rmdir ", 6) == 0) {
            char *path = command + 6;
            rmdir_func(path);
        } else if (strncmp(command, "cd ", 3) == 0) {
            char *path = command + 3;
            cd(path);
    } else {
            system(command);
        }
    }

    return 0;
}



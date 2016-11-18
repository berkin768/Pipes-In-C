    //gcc BERKIN_MYCAT.c -o myCat
    //./myCat text = myMore

    // Created by berkin on 05.11.2016.
    // Finished by berkin on 14.11.2016 at 01:45
    //Edited project name on 15.11.2016 at 18:42

    #include "sys/wait.h"    //WAIT function's library
    #include "termio.h"       //GETCH function's library
    #include "stdlib.h"
    #include "stdio.h"
    #include "string.h"
    #include "unistd.h"

    #define BUFFER_SIZE 500   //Becuase if line is too long, console split the sentence. If i dont use a limitation, line number may increase and i cant control if it happen.
    #define MAXIMUM_LINE_NUMBER 100  //In a file, maximum line number
    #define LINES_PER_TURN 24    //Required lines in document
    #define READ_END 0
    #define WRITE_END 1

    //This function used for reading key from user, termio.h library is related with this function.
    int getch(void)
    {
        struct termios oldattr, newattr;
        int ch;
        tcgetattr(STDIN_FILENO, &oldattr);
        newattr=oldattr;
        newattr.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
        ch=getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);

        return ch;
    }

    //This function returns file size in bytes
    int fileSize(FILE *fp){
      fseek(fp, 0L, SEEK_END);
      int fileSize = ftell(fp);
      fseek(fp, 0L, SEEK_SET);

      return fileSize;
    }

    //This function returns file pointer
    FILE *getFile(char * filePath){
        FILE *fp;
        fp = fopen(filePath, "r");

        if(fp != NULL)
        return fp;

        else{
            perror("Error");
            exit(1);
        }
    }

    //This function reads the file and prints in the same time.
    int readAllFile(FILE *fp){

      int byteOfFile = fileSize(fp);
      char * records = malloc(byteOfFile +1);

      if (!fp)
      return -1;

      while (fgets(records,byteOfFile, fp)!=NULL)  //reads file until \0
      printf("%s",records);

      fclose(fp);

      return 0;
    }

    int main(int argc, char * argv[]) {

        FILE *fp = getFile(argv[1]);

        if (argc == 2) {   //Read entire file.  INPUT > ./myCat <filename>
            readAllFile(fp);
        }

        else if (!strcmp(argv[2], "=") && !strcmp(argv[3], "myMore")) {   //Read file part by part. Third argument must be = and 4th myMore

                char buffer[BUFFER_SIZE];
                char inputChar;   //This variable keeps user's input. If user doesn't enter q, loop can continue
                int isFinished=0;  //This variable checks file read's situation. If read complete, process has to stop.
                char write_msg[MAXIMUM_LINE_NUMBER][BUFFER_SIZE];  //Sending message by using pipe.

                do{

                    pid_t pid;
                    int fd[2];
                    pipe(fd);
                    pid = fork();  //Second process created

                    if (pid < 0) { //Fork failed, exit.
                        fprintf(stderr, "Fork Failed");
                        return 1;
                    }

                    if (pid == 0) {   //Child process

                        close(fd[WRITE_END]);

                        char read_end_pipe[10];  //This variable stores pipe's reading point.
                        sprintf(read_end_pipe, "%d", fd[READ_END]);

                        char *arguments[3]={argv[3],read_end_pipe,NULL}; //Execv's arguments. I have to send some params to myMore
                        if(execv(argv[3],arguments)==-1)  //myMore starts
                        {
                            perror("Failed To Execute 'client.c' Program");
                            return -1;
                        }
                    }

                    if (pid > 0) {   //Main process
                        int lineCount = 0;  //This variable stores how many lines have been read?
                        for (int i = 0; i < LINES_PER_TURN; ++i) {  //Read 24 lines if there are 24 lines
                            if (fgets(buffer, sizeof(buffer), fp) > 0) {  //Read until detect \n
                                strcpy(write_msg[i], buffer);
                                lineCount++;
                            }
                            else{  //If whole file has been read, break the loop.
                                isFinished = 1;
                                break;
                            }
                        }

                        close(fd[READ_END]);
                        write(fd[WRITE_END], &write_msg, lineCount * BUFFER_SIZE);
                        close(fd[WRITE_END]);

                        wait(NULL);
                    }

                    if(isFinished == 0)
                        printf("----------PRESS KEY TO CONTINIUE----------\n");
                    else{
                        printf("----------END OF FILE----------\n");
                        return 0;
                    }

                    inputChar = getch();   //User's input

                }while(isFinished != 1 && inputChar != 'q');
            }

        else{
            printf("Wrong Input, Sorry\n");
            return 1;
        }

      return 0;
    }

#include <unistd.h> //fork, execvp, _exit, wait
#include <wait.h>   //wait
#include <stdio.h>  //printf
#include <string.h> //strlen
#include <stdlib.h>  //exit
#define PATH_SIZE 512
#define CMD_SIZE 10
int main() {
        char cmd[256];                                            //コマンドを保存するための配列
        char path_name[PATH_SIZE];                                //パスを保存するための配列

        //カレントディレクトリを取得
        if(getcwd(path_name, sizeof(path_name)) == NULL) {
                perror("getcwd failed");
                exit(EXIT_FAILURE);
        }

        printf("\033[1;34m%s$ \033[0m", path_name);                      //プロンプトとしてドル記号を表示

        while(fgets(cmd, 256, stdin) != NULL) {                   //標準入力からコマンドを読み込むループ
                cmd[strlen(cmd) - 1] = '\0';                      //fgetsは改行も読み込むため、文字列の終わりを示すヌル文字で置換

                if(strcmp(cmd, "pwd") == 0) {
                        if(getcwd(path_name, sizeof(path_name)) != NULL) {
                                printf("current dir: %s\n", path_name);
                        } else {
                                perror("getcwd failed");
                        }
                } else {
                        if(fork() == 0) {                                 //子プロセスを生成
                                printf("(*_*) (%d)\n", getpid());         //子プロセスのPIDを表示
                                char *args[] = {cmd, NULL};
                                execvp(args[0], args);                    //入力されたコマンドを実行
                                perror("execvp failed");
                                _exit(1);                                 //exexlpが失敗した場合、子プロセスを終了
                        }
                        wait(NULL);                                       //親プロセスが子プロセスの終了を待つ
                }
                //カレントディレクトリを再取得
                if(getcwd(path_name, sizeof(path_name)) == NULL) {
			perror("getcwd failed");
                        exit(EXIT_FAILURE);
                }
                printf("\033[1;34m%s$ \033[0m", path_name);                               //ユーザーが次のコマンドを入力できるようにプロンプトを再表示
        }
        return 0;                                                         //メイン関数の終了
}
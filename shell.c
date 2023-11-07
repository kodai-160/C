#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 1024

int main() {
    char cmd[BUFF_SIZE]; // コマンドを保存するための配列
    char *args[BUFF_SIZE]; // execvpに渡すための引数配列
    char *cmd1_args[BUFF_SIZE];
    char *cmd2_args[BUFF_SIZE];
    char *token;
    int pipefd[2];
    int status;
    pid_t pid1, pid2;
    char path_name[BUFF_SIZE]; // 現在のディレクトリパスを保存する配列

    while(1) {
        // カレントディレクトリを取得してプロンプトに表示
        if(getcwd(path_name, sizeof(path_name)) == NULL) {
            perror("getcwd failed");
            exit(EXIT_FAILURE);
        }
        printf("\033[1;34m%s$ \033[0m", path_name); // プロンプト表示

        if(fgets(cmd, BUFF_SIZE, stdin) == NULL) {
            break; // 入力が終了したらループを抜ける
        }
        cmd[strlen(cmd) - 1] = '\0'; // 改行文字を取り除く

        // "pwd"コマンドが入力されたら処理する
        if(strcmp(cmd, "pwd") == 0) {
            printf("%s\n", path_name);
            continue; // 次のコマンドを待つ
        }

        // コマンドラインを"|"で分割してパイプの前後のコマンドを取得
        char *cmd1 = strtok(cmd, "|");
        char *cmd2 = strtok(NULL, "|");

        // コマンド1の引数を解析
        int i = 0;
        token = strtok(cmd1, " ");
        while(token != NULL) {
            cmd1_args[i++] = token;
            token = strtok(NULL, " ");
        }
        cmd1_args[i] = NULL; // 引数リストはNULLで終了する必要がある

        // パイプがあるかどうかチェック
        if (cmd2 != NULL) {
            // コマンド2の引数を解析
            i = 0;
            token = strtok(cmd2, " ");
            while(token != NULL) {
                cmd2_args[i++] = token;
                token = strtok(NULL, " ");
            }
            cmd2_args[i] = NULL;

            // パイプを作成
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        // 最初の子プロセスを生成
        pid1 = fork();
        if (pid1 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid1 == 0) {
            // 子プロセス1
            if (cmd2 != NULL) {
                // 標準出力をパイプの書き込み端に接続
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }
            execvp(cmd1_args[0], cmd1_args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        if (cmd2 != NULL) {
            // パイプを介して２つ目のコマンドを実行する
            pid2 = fork();
            if (pid2 == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid2 == 0) {
                // 子プロセス2
                // 標準入力をパイプの読み取り端に接続
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[1]);
                close(pipefd[0]);
                execvp(cmd2_args[0], cmd2_args);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else {
                // 親プロセスはパイプを使用しない
                close(pipefd[0]);
                close(pipefd[1]);
                waitpid(pid2, &status, 0);
            }
        }
        // 親プロセスは最初の子プロセスの終了を待つ
        waitpid(pid1, &status, 0);
    }
    return 0;
}

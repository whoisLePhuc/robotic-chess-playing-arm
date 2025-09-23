#include "chess_logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

static int engine_in[2];   // ghi vào stockfish (stdin)
static int engine_out[2];  // đọc từ stockfish (stdout)
static pid_t sf_pid;
static char response_buffer[4096];

// Function to start UCI chess engine
int uci_start(const char *engine_path) {
    if (pipe(engine_in) < 0 || pipe(engine_out) < 0) {
        perror("pipe failed");
        return -1;
    }
    sf_pid = fork();
    if (sf_pid == 0) {
        // Child process
        dup2(engine_in[0], STDIN_FILENO);
        dup2(engine_out[1], STDOUT_FILENO);

        close(engine_in[1]);
        close(engine_out[0]);

        execlp(engine_path, engine_path, NULL);
        perror("exec failed");
        exit(1);
    }
    // Parent process
    close(engine_in[0]);
    close(engine_out[1]);
    return 0;
}

// Function to send command to UCI engine
void uci_send(const char *cmd) {
    write(engine_in[1], cmd, strlen(cmd));
    write(engine_in[1], "\n", 1);
    fsync(engine_in[1]);
}

// Function to read response best move from UCI engine
const char *uci_read_bestmove() {
    int n;
    while ((n = read(engine_out[0], response_buffer, sizeof(response_buffer)-1)) > 0) {
        response_buffer[n] = '\0';
        printf("SF> %s", response_buffer);

        // Find bestmove
        char *bm = strstr(response_buffer, "bestmove");
        if (bm) {
            return bm + 9; // Response string "bestmove "
        }

        // Stop when seeing "uciok" or "readyok"
        if (strstr(response_buffer, "uciok") || strstr(response_buffer, "readyok")) {
            return NULL;
        }
    }
    return NULL;
}

// Function to start a new game
void uci_newgame() {
    uci_send("ucinewgame");
    uci_send("isready");
    uci_read_bestmove(); // đọc đến khi readyok
}

// Function to stop stockfish engine
void uci_stop() {
    uci_send("quit");
    waitpid(sf_pid, NULL, 0);
}
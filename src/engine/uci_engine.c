#include "engine/uci_engine.h"
#include "game/chess_state.h"

bool uci_start_engine(uci_engine_t *engine, const char *path) {
    if (!engine || !path) return false;
    
    strcpy(engine->engine_path, path);
    
    if (pipe(engine->engine_in) < 0 || pipe(engine->engine_out) < 0) {
        perror("Failed to create pipes");
        return false;
    }
    
    engine->pid = fork();
    if (engine->pid < 0) {
        perror("Fork failed");
        return false;
    }
    
    if (engine->pid == 0) {
        // Child process
        dup2(engine->engine_in[0], STDIN_FILENO);
        dup2(engine->engine_out[1], STDOUT_FILENO);
        dup2(engine->engine_out[1], STDERR_FILENO);
        
        close(engine->engine_in[0]);
        close(engine->engine_in[1]);
        close(engine->engine_out[0]);
        close(engine->engine_out[1]);
        
        execlp(path, path, NULL);
        perror("Failed to exec engine");
        exit(1);
    }
    
    // Parent process
    close(engine->engine_in[0]);
    close(engine->engine_out[1]);
    
    // Set non-blocking
    int flags = fcntl(engine->engine_out[0], F_GETFL, 0);
    fcntl(engine->engine_out[0], F_SETFL, flags | O_NONBLOCK);
    
    engine->is_running = true;
    
    // Initialize engine
    uci_send_command(engine, "uci");
    usleep(500000); // 0.5 second
    uci_send_command(engine, "isready");
    usleep(200000); // 0.2 second
    
    return true;
}

void uci_stop_engine(uci_engine_t *engine) {
    if (!engine || !engine->is_running) return;
    
    uci_send_command(engine, "quit");
    
    if (engine->pid > 0) {
        int status;
        if (waitpid(engine->pid, &status, WNOHANG) == 0) {
            usleep(100000); // 0.1 second
            if (waitpid(engine->pid, &status, WNOHANG) == 0) {
                kill(engine->pid, SIGTERM);
                waitpid(engine->pid, &status, 0);
            }
        }
    }
    
    if (engine->engine_in[1] >= 0) close(engine->engine_in[1]);
    if (engine->engine_out[0] >= 0) close(engine->engine_out[0]);
    
    engine->is_running = false;
}

bool uci_send_command(uci_engine_t *engine, const char *command) {
    if (!engine || !command || !engine->is_running) return false;
    
    printf("→ Engine: %s\n", command);
    
    size_t len = strlen(command);
    ssize_t written = write(engine->engine_in[1], command, len);
    if (written != (ssize_t)len) return false;
    
    written = write(engine->engine_in[1], "\n", 1);
    if (written != 1) return false;
    
    return true;
}

bool uci_read_response(uci_engine_t *engine, char *buffer, size_t buffer_size, int timeout_ms) {
    if (!engine || !buffer || !engine->is_running) return false;
    
    fd_set readfds;
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    FD_SET(engine->engine_out[0], &readfds);
    
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    
    int ready = select(engine->engine_out[0] + 1, &readfds, NULL, NULL, &timeout);
    if (ready <= 0) return false;
    
    ssize_t n = read(engine->engine_out[0], buffer, buffer_size - 1);
    if (n > 0) {
        buffer[n] = '\0';
        if (strstr(buffer, "bestmove") == NULL) {
            printf("← Engine: %s", buffer);
        }
        return true;
    }
    
    return false;
}

bool uci_get_best_move(uci_engine_t *engine, char *move_buffer, size_t buffer_size) {
    if (!engine || !move_buffer) return false;
    
    char response[1024];
    int attempts = 0;
    
    while (attempts < 100) { // Max 10 seconds
        if (uci_read_response(engine, response, sizeof(response), 100)) {
            char *bestmove = strstr(response, "bestmove ");
            if (bestmove) {
                bestmove += 9; // Skip "bestmove "
                
                // Extract move
                char *space = strchr(bestmove, ' ');
                char *newline = strchr(bestmove, '\n');
                char *end = space;
                if (newline && (!space || newline < space)) end = newline;
                
                size_t move_len = end ? (size_t)(end - bestmove) : strlen(bestmove);
                if (move_len >= buffer_size) move_len = buffer_size - 1;
                
                strncpy(move_buffer, bestmove, move_len);
                move_buffer[move_len] = '\0';
                
                // Remove trailing whitespace
                for (int i = (int)move_len - 1; i >= 0 && isspace((unsigned char)move_buffer[i]); i--) {
                    move_buffer[i] = '\0';
                }
                
                printf("← Engine: bestmove %s\n", move_buffer);
                return strlen(move_buffer) > 0 && strcmp(move_buffer, "(none)") != 0;
            }
        }
        attempts++;
    }
    
    return false;
}

bool uci_set_position(uci_engine_t *engine, const chess_state_t *chess) {
    if (!engine || !chess) return false;
    
    char command[1024] = "position startpos";
    
    if (chess->move_count > 0) {
        strcat(command, " moves");
        for (int i = 0; i < chess->move_count; i++) {
            strcat(command, " ");
            strcat(command, chess->move_history[i].notation);
        }
    }
    
    return uci_send_command(engine, command);
}
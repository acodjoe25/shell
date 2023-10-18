#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "./jobs.h"
#include "string.h"

int redirect_count_out;  // counts the number of redirect symbols
int redirect_idx_out;    // records the index of the redirect symbol
int redirect_count_in;   // counts the number of redirect symbols
int redirect_idx_in;     // records the index of the redirect symbol
int job_counter;
job_list_t *job_list;

// Notes:
// 1) make sure jobs command works (actually might noe be part of 5)
// 2) delayed echo shouldn't print out &

/*
    This function returns the length of the buffer.
    args:
     * buffer - the inputted list that we would like
        to determine the size of
*/
int sizeOfBuffer(char *buffer[]) {
    int i = 0;
    while (buffer[i] != NULL) {
        i++;
    }
    return i;
}

/*
    This function populates an array of tokens based on
    a given array. The tokens array is meant to model the
    array from lab 2.

     buffer - typically a read array containing user input
     tokens - an initially "empty" array that we will populate
        with tokens of the buffer
*/
void tokenize(char buffer[1024], char *tokens[512]) {
    char *token = strtok(buffer, " \t\n");
    int i = 0; /*pointer*/
    redirect_count_out = 0;
    redirect_idx_out = -1;
    redirect_count_in = 0;
    redirect_idx_in = -1;

    while ((token != NULL)) {
        /* check if token is redirect symbol*/
        if (!strcmp((token), ">") || !strcmp((token), ">>")) {
            redirect_count_out++;
            redirect_idx_out = i;
        } else if (!strcmp((token), "<")) {
            redirect_count_in++;
            redirect_idx_in = i;
        }
        /* inserting the portion to the array */
        tokens[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    tokens[i] = NULL;
}

/*
    This function extracts the filneame from a string
    that may contain a filepath or file na,e

    str- the string in which we want to get the file
     name
*/
char *get_file_name(char *str) {
    if (str == NULL) {
        return NULL;
    }
    char *file_name =
        /*getting the pointer to after the backslash*/
        strrchr(str, '/');
    if (file_name == NULL) {
        return str;
    }

    if (*(file_name + 1) == '\0') {
        return "";
    }
    /*returning the pointer gives string, but putting a
    star gives a chararcter*/
    return file_name + 1;
}

/*
    This function prints a given array. It was mainly
    used for testing purposes.

    arr - the name of the aarray we want to print
*/
void print_array(char *arr[]) {
    for (int i = 0; arr[i] != NULL; i++) {
        printf("%s \t", arr[i]);
    }
    printf("\n");
}

/*
    This function parses a user inputted/read array. It
    seperates each grouping by spaces.

    buffer - typically a read array containing user input
    tokens - an initially "empty" array that we will populate
        with tokens of the buffer
    argv - an intially "empty" areay tjat will populate with
        the argv of the buffer (as used in lab 2)
*/
void parse(char buffer[1024], char *tokens[512], char *argv[512]) {
    /* Tokenize the string */
    tokenize(buffer, tokens);

    /* Get the file name and set it to first element in argv */
    char *file_name = get_file_name(tokens[0]);
    argv[0] = file_name;
    int i = 1;
    while (tokens[i] != NULL) {
        /* where you would separate the things */
        char *temp = tokens[i];
        argv[i] = temp;
        i++;
    }
    argv[i] = NULL;
}

/*
 Method that cleans the redirection and the following
 file name from the  array

 buffer - the list that we would like to extract redirections
  from
 cleaned_arr - a list that is initally empty and will be
  popualted by the contents of the buffer with redirections
  removed
 */

void clean_buffer(char *buffer[], char *cleaned_arr[]) {
    int i = 0; /* represents input_buffer index*/
    int j = 0; /* represents cleaned_arr index*/
    while (buffer[i] != NULL) {
        if ((i == redirect_idx_in) | (i == redirect_idx_out)) {
            i += 2;
        } else {
            char *temp = buffer[i];
            cleaned_arr[j] = temp;
            i++;
            j++;
        }
    }
    cleaned_arr[j] = NULL;
}

/* CREATING OUR FOUR BUILT IN COMMANDS*/
/*
this function handles the case in which the user uses the
cd command.

buffer - the parsed, user, inputed information containing
  the cd call
*/
int cd(char *buffer[]) {
    if (sizeOfBuffer(buffer) < 2) {
        fprintf(stderr, "cd: syntax error\n");
        return -1;
    }
    int i = chdir(buffer[1]);
    if (i == -1) {
        perror("cd: syntax error");
        return -1;
    } else {
        return 0;
    }
}
/*
this function handles the case in which the user uses the
ln command.

buffer - the parsed, user, inputed information containing
  the ln call
*/
int ln(char *buffer[]) {
    if (sizeOfBuffer(buffer) < 3) {
        fprintf(stderr, "Wrong number of arguments. Pls try again\n");
        return -1;
    }
    int i = link(buffer[1], buffer[2]);
    if (i == -1) {
        perror("ln returned in error");
        return -1;
    } else {
        return 0;
    }
}

/*
this function handles the case in which the user uses the
rm command.

buffer - the parsed, user, inputed information containing
  the rm call
*/
int rm(char *buffer[]) {
    if (sizeOfBuffer(buffer) < 2) {
        fprintf(stderr, "Wrong number of arguments. Pls try again\n");
        return -1;
    }
    int i = unlink(buffer[1]);
    if (i == -1) {
        perror("rm returned in error");
        return -1;
    } else {
        return 0;
    }
}
/*
this function handles reaping. it determines the
appropiate way to respond to processes after a change
in state.

staus-- the status of the process
jid--- tje jid of the given process
pid--- the process id of a given process
*/
void reap_co(int status, int jid, pid_t pid) {
    // terminated by signnal
    if (WIFSIGNALED(status) != 0) {
        int sig = WTERMSIG(status);
        printf("here!!! 3");
        printf("[%d] (%d) terminated by signal  %d\n", jid, pid, sig);
        if (remove_job_jid(job_list, jid) == -1) {
            fprintf(stderr, "error occured in removing job :(");
        }
    }
    // terminated normally
    else if (WIFEXITED(status) != 0) {
        printf("[%d] (%d) terminated with exit status  %d\n", jid, pid, status);
        if (remove_job_jid(job_list, jid) == -1) {
            fprintf(stderr, "error occured remove job p2");
        }

    }
    // suspended by signal
    else if (WIFSTOPPED(status) != 0) {
        int sig = WSTOPSIG(status);
        printf("[%d] (%d) suspended by signal %d\n", jid, pid, sig);
        if (update_job_jid(job_list, jid, STOPPED) == -1) {
            fprintf(stderr, "update job failure");
        }

    } else if (WIFCONTINUED(status) != 0) {
        // printf("resumed\n");
        if (update_job_jid(job_list, jid, RUNNING) == -1) {
            fprintf(stderr, "update job failure p2");
        }
        printf("[%d] (%d) resumed\n", jid, pid);
    }
}

/*
  This function is the main function. It takes in no inputs.
  it handles

*/
int main() {
    char buffer[1024];
    char *tokens[512];
    char *argv[512];
    job_counter = 0;
    job_list = init_job_list();

    /* ignoring signals */
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    while (1) {
        int status;
        pid_t pid;
        // printf("we got to before the while loop\n");
        while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) >
               0) {
            int jid = get_job_jid(job_list, pid);
            if (jid == -1) {
                fprintf(stderr, "get job error");
            } else {
                reap_co(status, jid, pid);
            }
        }
#ifdef PROMPT
        if (write(2, "33sh> ", 5) < 0) {
            fprintf(stderr, "eek! error");
            exit(1);
        }
        if (fflush(stdout) < 0) {
            fprintf(stderr, "eek! error");
            exit(1);
        }
#endif
        ssize_t check = read(0, buffer, 1024);

        if (check == 0) {
            cleanup_job_list(job_list);
            exit(0);
        } else if (check < 0) {
            fprintf(stderr, "eek! an error occured during read\n");
            exit(1);
        } else {
            /* WHILE READING USER INPUT*/
            buffer[check] = '\0';
            parse(buffer, tokens, argv);

            /*ERROR CHECKING FOR INVALID # OF REDIRECTS and syntax error*/
            if (sizeOfBuffer(tokens) == 0) {
                // ari edit
                continue;
                // exit(0);
            } else if ((redirect_count_in > 1) | (redirect_count_out > 1)) {
                fprintf(stderr, "syntax error: multiple output files\n");
                exit(1);
            } else if ((tokens[redirect_idx_in + 1] == NULL) |
                       (tokens[redirect_idx_out + 1] == NULL)) {
                fprintf(stderr, "No redirection file specified.\n");
                exit(1);
            }

            /* CREATING 2 NEW ARRAYS AFTER CLEANING THE TOKENS AND ARGV
             * BUFFERS*/
            char *cleaned_tokens[512];
            char *cleaned_argv[512];

            /* making an empty array for both*/
            for (int i = 0; i < 512; i++) {
                cleaned_tokens[i] = NULL;
                cleaned_argv[i] = NULL;
            }
            clean_buffer(tokens, cleaned_tokens); /* our cleaned tokens array
                                                     (no redirections) */
            clean_buffer(
                argv,
                cleaned_argv); /* our cleanes argv array (no redirections) */

            /* DEALING WITH SYSTEM CALLS*/
            if (!strcmp(cleaned_tokens[0], "cd")) {
                cd(cleaned_tokens);
            } else if (!strcmp(cleaned_tokens[0], "rm")) {
                rm(cleaned_tokens);
            } else if (!strcmp(cleaned_tokens[0], "ln")) {
                ln(cleaned_tokens);
            } else if (!strcmp(cleaned_tokens[0], "exit")) {
                exit(0);  // this is the only exit
                return 0;
            } else if (!strcmp(cleaned_tokens[0], "jobs")) {
                // printf("%d \n", job_counter);
                jobs(job_list);
            } else if (!strcmp(cleaned_tokens[0], "bg")) {
                if ((sizeOfBuffer(cleaned_tokens) != 2)) {
                    fprintf(stderr, "bg : syntax error");
                } else {
                    int pid_loc =
                        get_job_pid(job_list, atoi(&cleaned_tokens[1][1]));
                    if (pid_loc == -1) {
                        fprintf(stderr, "job not found\n");
                    } else {
                        kill(-pid_loc, SIGCONT);
                    }
                }
            } else if (!strcmp(cleaned_tokens[0], "fg")) {
                if ((sizeOfBuffer(cleaned_tokens) != 2)) {
                    fprintf(stderr, "fg : syntax error");
                } else {
                    int pid_loc =
                        get_job_pid(job_list, atoi(&cleaned_tokens[1][1]));
                    if (pid_loc == -1) {
                        fprintf(stderr, "job not found\n");
                    } else {
                        kill(-pid_loc, SIGCONT);
                        tcsetpgrp(0, pid_loc);

                        int special_status;
                        waitpid(pid_loc, &special_status, WUNTRACED);
                        if (WIFSIGNALED(special_status) != 0) {
                            int sig = WTERMSIG(special_status);
                            printf("[%d] (%d) terminated by signal %d\n",
                                   atoi(&cleaned_tokens[1][1]), pid_loc, sig);
                            remove_job_jid(job_list,
                                           atoi(&cleaned_tokens[1][1]));
                        }
                        // terminated normally
                        else if (WIFEXITED(special_status) != 0) {
                            if (remove_job_jid(job_list,
                                               atoi(&cleaned_tokens[1][1])) ==
                                -1) {
                                fprintf(stderr, "error occured remove job p2");
                            }
                        } else if (WIFSTOPPED(special_status) != 0) {
                            int sig = WSTOPSIG(special_status);
                            if (update_job_jid(job_list,
                                               atoi(&cleaned_tokens[1][1]),
                                               STOPPED) == -1) {
                                fprintf(stderr, "update job failure");
                            }
                            printf("[%d] (%d) suspended by signal %d\n",
                                   atoi(&cleaned_tokens[1][1]), pid_loc, sig);
                        }
                        tcsetpgrp(0, getpgrp());
                    }
                }
            } else {
                /* handle other function calls*/
                pid_t pid;

                /*fork() returns 0 for child process, and p for the parent*/
                if ((pid = fork()) == 0) {
                    // change the childs process group id from its paretns to
                    // its own
                    int child_pid = getpid();
                    setpgid(child_pid, child_pid);

                    if (strcmp(cleaned_tokens[sizeOfBuffer(cleaned_tokens) - 1],
                               "&")) {
                        tcsetpgrp(0, getpgrp());
                    } else if (!strcmp(
                                   cleaned_tokens[sizeOfBuffer(cleaned_tokens) -
                                                  1],
                                   "&")) {
                        cleaned_argv[sizeOfBuffer(cleaned_tokens) - 1] = NULL;
                    }
                    // setting signals back to default
                    signal(SIGINT, SIG_DFL);
                    signal(SIGTSTP, SIG_DFL);
                    signal(SIGTTOU, SIG_DFL);

                    if (redirect_count_in != 0) {
                        char *file_name_in = tokens[redirect_idx_in + 1];
                        if (close(0) == -1) {
                            perror("error when closing file 1");
                            exit(1);
                        }
                        if (open(file_name_in, O_RDONLY | O_CREAT, 0600) ==
                            -1) {
                            perror("error when opening file 1 ");
                            exit(1);
                        }
                    }
                    if (redirect_count_out != 0) {
                        char *file_name_out = tokens[redirect_idx_out + 1];
                        if (!strcmp(tokens[redirect_idx_out], ">")) {
                            if (close(1) == -1) {
                                perror("error wehn closing file 2");
                                exit(1);
                            }
                            if (open(file_name_out,
                                     O_WRONLY | O_CREAT | O_TRUNC,
                                     0600) == -1) {
                                perror("error when opening file 2");
                                exit(1);
                            }
                        } else {
                            if (close(1) == -1) {
                                perror("error when closing file 3");
                                exit(1);
                            }
                            if (open(file_name_out,
                                     O_WRONLY | O_CREAT | O_APPEND,
                                     0600) == -1) {
                                perror("error when opening file 3");
                                exit(1);
                            }
                        }
                    }

                    /* executes the command*/
                    execv(cleaned_tokens[0], cleaned_argv);
                    /* only reaches this line if execv fails*/
                    perror("execv");
                    cleanup_job_list(job_list);
                    exit(1);
                }

                /*** NOW WE ARE IN THE PARENT PROCESS***/
                // IF FOREGROUND PROCESS
                if (strcmp(cleaned_tokens[sizeOfBuffer(cleaned_tokens) - 1],
                           "&")) {
                    int status;
                    waitpid(pid, &status, WUNTRACED);
                    if (!WIFEXITED(status) &&
                        (get_job_jid(job_list, pid) < 0)) {
                        job_counter++;
                        if (add_job(job_list, job_counter, pid, STOPPED,
                                    cleaned_tokens[0]) == -1) {
                            fprintf(stderr, "add job resulted in error");
                        }

                        if (WIFSIGNALED(status) != 0) {
                            printf("[%d] (%d) terminated by signal %d\n",
                                   job_counter, pid, WTERMSIG(status));
                            if (remove_job_jid(job_list, job_counter) == -1) {
                                fprintf(stderr, "remove job error p3");
                            }
                        }
                        // if foreground process was suspended
                        else if (WIFSTOPPED(status) != 0) {
                            printf("[%d] (%d) suspended by signal %d\n",
                                   job_counter, pid, WSTOPSIG(status));
                        }
                    }
                    pid_t parent_pid = getpgrp();
                    tcsetpgrp(0, parent_pid);
                }
                // IF BACKGROUND PROCESS
                else if ((!strcmp(
                             cleaned_tokens[sizeOfBuffer(cleaned_tokens) - 1],
                             "&")) &&
                         (get_job_jid(job_list, pid) < 0)) {
                    job_counter++;
                    if (add_job(job_list, job_counter, pid, RUNNING,
                                cleaned_tokens[0]) == -1) {
                        fprintf(stderr, "add job resulted in error p2");
                    }
                    printf("[%d] (%d)\n", job_counter, pid);
                }
            }
        }
    }
    return 0;
}
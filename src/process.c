#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "process.h"

void child(struct ExecveParams execveParams, struct StandardStreams* streams)
{
    // Reset child's signal mask
    sigset_t mask;
    sigemptyset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    close(streams->fdin[PIPE_WRITE]);
    close(streams->fdout[PIPE_READ]);
    close(streams->fderr[PIPE_READ]);

    execve(execveParams.cmd, execveParams.args, execveParams.env);

    perror("execve");
    fprintf(stderr, "%s\n", execveParams.cmd);
    // This line is only reached, if execve fails.
    exit(EXIT_FAILURE);
}

void parent(struct StandardStreams* streams)
{
    close(streams->fdin[PIPE_READ]);
    close(streams->fdout[PIPE_WRITE]);
    close(streams->fderr[PIPE_WRITE]);
}

pid_t spawnChild(struct ExecveParams execveParams,
                 struct StandardStreams* streams)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // The child calls execve and thus never returns
        child(execveParams, streams);
    } else {
        parent(streams);
    }

    return pid;
}

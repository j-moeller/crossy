#ifndef PROCESS_H
#define PROCESS_H

#define PIPE_READ 0
#define PIPE_WRITE 1

struct ExecveParams {
    const char* cmd;
    char** args;
    char** env;
};

struct StandardStreams {
    int fdin[2];
    int fdout[2];
    int fderr[2];
};

#endif

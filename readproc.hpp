#ifndef READPROC_HPP_
#define READPROC_HPP_

#include <string>

#include <spawn.h>

class ReadProc {

public:
    ReadProc(char ** args);
    ~ReadProc();

    int killproc();

    int waitproc();

    int nowaitproc();

    std::string readoutput();

private:
    pid_t pid;
    int status;
    int out[2];
    posix_spawn_file_actions_t action;
};


#endif
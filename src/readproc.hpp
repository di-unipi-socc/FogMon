#ifndef READPROC_HPP_
#define READPROC_HPP_

#include <string>

#include <spawn.h>

class ReadProc {

public:
    ReadProc(char ** args);
    ~ReadProc();

    //kill the process
    //return    0 if success
    //          -1 if error or not running
    int killproc();

    //wait for the process to terminate
    //return    exitcode
    //          0 if unnatural death
    //          -1 if not running
    int waitproc();

    //test if is still running
    //return    exitcode if terminated
    //          0 if is terminated with no exitcode
    //          -1 if still running
    int nowaitproc();

    std::string readoutput();

private:
    pid_t pid;
    int status;
    int out[2];
    posix_spawn_file_actions_t action;
};


#endif
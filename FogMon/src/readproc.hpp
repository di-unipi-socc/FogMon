#ifndef READPROC_HPP_
#define READPROC_HPP_

#include <string>
#include <vector>
#include <spawn.h>

class ReadProc {

public:
    ReadProc(std::vector<std::string> args);
    ~ReadProc();

    //kill the process
    //return    0 if success
    //          -1 if error or not running
    int killproc();

    //wait for the process to terminate
    //return    0 if exited normally
    //          -1 if unnatural death
    int waitproc();

    //test if is still running
    //return    0 if terminated
    //          -1 if still running
    //          -2 if error
    int nowaitproc();

    //get the exit code
    int getexitcode();

    std::string readoutput();

private:
    pid_t pid;
    int status;
    int out[2];
    int exitcode;
    posix_spawn_file_actions_t action;
};


#endif
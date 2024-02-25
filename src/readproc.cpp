#include "readproc.hpp"

#include <sstream>
#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
char **environ;

using namespace std;

ReadProc::ReadProc(vector<string> args) {
    status = -1;

    if(args.size() == 0 || args[0].c_str() == NULL)
        return;
    
    posix_spawn_file_actions_init(&action);

    pipe(out);

    posix_spawn_file_actions_adddup2(&action, out[1], STDOUT_FILENO); // redirect stdout to pipe
    posix_spawn_file_actions_adddup2(&action, out[1], STDERR_FILENO); // redirect stderr to pipe
    posix_spawn_file_actions_addclose(&action, out[0]);
    
    // build args into a char* array
    char *cargs[args.size()+1];
    for (int i=0; i<args.size(); i++)
    {
        cargs[i] = (char*)args[i].c_str();
    }
    cargs[args.size()] = NULL;
    status = posix_spawn(&pid, cargs[0], &action, NULL, cargs, environ);
    if (status != 0)
    {
        cerr << "posix_spawn: " << strerror(status) << endl;
        close(out[1]);
    }
}

ReadProc::~ReadProc() {
    this->killproc();
    posix_spawn_file_actions_destroy(&action);
    sleep(1); // wait for the other process to exit before destroying variables
}

int ReadProc::killproc() {
    if (status != 0)
        return 0;
    int w;
    if ((w = waitpid(pid, &status, WNOHANG)) < 0)
    {
        return -1;
    }
    else if(w > 0)
    {
        // already exited
        status = -1;
        close(out[1]);
        return 0;
    }else {
        if (kill(pid, SIGKILL) == 0) {
            waitpid(pid, &status, WNOHANG);
            close(out[1]);
            status = -1;
            exitcode = 1;
            return 0;
        } else {
            cerr << "Error killing.\n";

            return -1;
        }
    }    
}

int ReadProc::waitproc() {
    if (status != 0)
        return 0;
    int stat;
    if (waitpid(pid, &stat, 0) < 0)
    {
        return -1;
    }
    else
    {
        status = -1;
        close(out[1]);
        exitcode = WEXITSTATUS(stat);
        if (WIFEXITED(stat))
        {
            return 0;
        }
        else
        {
            //cerr << "child died an unnatural death.\n";
            return -1;
        }
    }
}

int ReadProc::nowaitproc() {
    if (status != 0)
        return 0;
    int stat;
    int w = waitpid(pid, &stat, WNOHANG);
    if (w < 0)
    {
        return -2;
    }
    else if(w > 0)
    {
        status = -1;
        close(out[1]);
        exitcode = WEXITSTATUS(stat);
        return 0;
    }else
    {
        //still running
        return -1;
    }
}

int ReadProc::getexitcode() {
    return exitcode;
}

std::string ReadProc::readoutput() {
    string ret;
    ssize_t num_read;
    char buf[256]; // buffer for reading from pipe 256 bytes at a time

    int res = fcntl(out[0],F_SETFL, fcntl(out[0],F_GETFL) | O_NONBLOCK);
    if(res<0)
        return ret;
    for (int i=0;true;i++)
    {
        num_read = read(out[0], buf, sizeof(buf)-1);
        if (num_read > 0)
        {
            buf[num_read] = 0x00;
            ret += string(buf);
        }
        else
        {
            break;
        }
    }
    return ret;
}
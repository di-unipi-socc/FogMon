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

ReadProc::ReadProc(char** args) {
    status = -1;

    if(args == NULL)
        return;
    
    posix_spawn_file_actions_init(&action);

    pipe(out);

    posix_spawn_file_actions_adddup2(&action, out[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&action, out[1], STDERR_FILENO);
    posix_spawn_file_actions_addclose(&action, out[0]);
    
    status = posix_spawn(&pid, args[0], &action, NULL, args, environ);
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
    if (status == 0)
    {
        int w;
        if ((w = waitpid(pid, &status, WNOHANG)) < 0)
        {
            return -1;
        }
        else if(w > 0)
        {
            if (WIFEXITED(status))
            {
                //printf("child exit status: %d\n", WEXITSTATUS(status));
            }
            else
            {
                //printf("child died an unnatural death.\n");
            }
            status = -1;
            close(out[1]);
            return 0;
        }else {
            if (kill(pid, SIGKILL) == 0) {
                waitpid(pid, &status, WNOHANG);
                close(out[1]);
                status = -1;
                return 0;
            } else {
                cerr << "Error killing.\n";

                return -1;
            }
        }
    }
    return -1;
}

int ReadProc::waitproc() {
    if (status == 0)
    {
        if (waitpid(pid, &status, 0) < 0)
        {
            return -1;
        }
        else
        {
            status =-1;
            close(out[1]);
            if (WIFEXITED(status))
            {
                close(out[1]);
                return WEXITSTATUS(status);
            }
            else
            {
                //cerr << "child died an unnatural death.\n";
                return 0;
            }
        }
    }
    status =-1;
    return -1;
}

int ReadProc::nowaitproc() {
    if (status == 0)
    {
        int w = waitpid(pid, &status, WNOHANG);
        if (w < 0)
        {
            return 0;
        }
        else if(w > 0)
        {
            status =-1;
            close(out[1]);
            if (WIFEXITED(status))
            {
                close(out[1]);
                return WEXITSTATUS(status);
            }
            else
            {
                //cerr << "child died an unnatural death.\n";
                return 0;
            }
        }else
        {
            //still running
            return -1;
        }
    }
    status = -1;
    return 0;
}

std::string ReadProc::readoutput() {
    string ret;
    ssize_t num_read;
    char buf[256];

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
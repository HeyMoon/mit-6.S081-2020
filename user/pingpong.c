#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main(int argc, char const *argv[])
{

    int p1[2];
    int p2[2];
    char buf[1] = {'a'};
    char buf2[1];
    char buf3[1];

    if (pipe(p1) < 0)
    {
        fprintf(2, "%s\n", "create pipe fail");
        exit(1);
    }

    if (pipe(p2) < 0)
    {
        fprintf(2, "%s\n", "create pipe fail");
        exit(1);
    }

    if (fork() == 0)
    {
        close(0);
        dup(p1[0]);
        close(p1[0]);
        close(p1[1]);

        if (read(0, buf2, sizeof(buf2) > 0))
        {
            int pid = getpid();
            fprintf(1, "%d: received ping\n", pid);

            close(1);
            dup(p2[1]);
            close(p2[0]);
            close(p2[1]);
            if (write(1, buf, 1) != 1)
            {
                fprintf(2, "%d: write error\n", pid);
                exit(1);
            }
            exit(0);
        }
    }

    close(1);
    dup(p1[1]);
    int pid = getpid();

    if (write(1, buf, 1) != 1)
    {
        fprintf(2, "%d: write error\n", pid);
        exit(1);
    }

    close(0);
    dup(p2[0]);

    if (read(0, buf3, sizeof(buf3) > 0))
    {
        fprintf(1, "%d: received pong\n", pid);
        exit(0);
    }

    close(p1[0]);
    close(p1[1]);
    close(p2[0]);
    close(p2[1]);
    
    exit(0);
}

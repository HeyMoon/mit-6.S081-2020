#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int readline(char buf[], int max)
{
    int i;
    char c;
    for (i = 0; i < max - 1 && read(0, &c, 1) > 0 && c != '\n'; ++i){
        //fprintf(2, "ch: %c",c);
        buf[i] = c;
    }

    if (c == '\n')
    {
        buf[i] = c;
        ++i;
    }
    buf[i] = '\0';
    return i;
}

int main(int argc, char *argv[])
{
    if (argc > MAXARG)
    {
        fprintf(2, "too much arguments");
        exit(-1);
    }

    if (argc < 2)
    {
        fprintf(2, "Useage: xargs cmd ...");
        exit(-1);
    }

    char *cmd[argc];
    for (int i = 1; i < argc; i++)
    {
        cmd[i - 1] = argv[i];
    }

    char buf[512];
    int n;
    while ((n = readline(buf, 512)) > 0)
    {
        //fprintf(2, "line: %s", buf);
        int pid = fork();
        if (pid == -1)
        {
            fprintf(2, "fork fail");
            exit(-1);
        }

        if (pid == 0)
        {
            cmd[argc - 1] = buf;
            exec(cmd[0], cmd);
        }
        else
        {
            wait(0);
        }
    }

    return 0;
}
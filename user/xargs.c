#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *readline(void)
{
    int length = 512;
    int n;
    char *buf;
    buf = (char *)malloc(length);
    char ch;
    while ((n = read(0, &ch, sizeof(ch))) > 0)
    {
        if (ch != '\n')
        {
            *buf++ = ch;
        }
        else
        {
            *buf = '\0';
            return buf;
        }
    }
    return 0;
}

int main(int argc, char const *argv[])
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
        char *tmp = (char *)malloc(strlen(argv[i]) + 1);
        strcpy(tmp, argv[i]);
        *tmp = '\0';
        *cmd[i - 1] = *tmp;
    }

    int length = 512;
    int n;
    char *buf;
    buf = (char *)malloc(length);
    char ch;
    while ((n = read(0, &ch, 1)) > 0)
    {
        if (ch != '\n')
        {
            *buf++ = ch;
        }
        else
        {
            *buf = '\0';

            fprintf(2, "line: %s", *buf);
            int pid = fork();
            if (pid == -1)
            {
                *cmd[argc - 1] = *buf;
                fprintf(2, "fork fail");
                exit(-1);
            }

            if (pid == 0)
            {
                exec(cmd[0], cmd);
            }
            else
            {
                wait(0);
            }
        }
    }

    return 0;
}
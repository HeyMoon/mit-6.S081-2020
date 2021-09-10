#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int fork1(void);
void panic(char *);
void sieve(int *p);

int main(int argc, char const *argv[])
{
    int p[2];
    if (pipe(p) < 0)
    {
        panic("create pipe error");
    }

    sieve(p);

    for (int i = 2; i <= 35; i++)
    {
        if (write(p[1], &i, sizeof(i)) < sizeof(i))
        {
            panic("write number error");
        }
    }

    close(p[1]);
    wait(0);
    exit(0);
}

/* 筛选素数*/
void sieve(int *p)
{
    if (fork1() == 0)
    {
        int buf[1];
        read(p[0], buf, sizeof(buf));

        int prime = buf[0];
        fprintf(1, "prime %d\n", prime);

        int p1[2];
        if (pipe(p1) < 0)
        {
            panic("create pipe error");
        }

        sieve(p1);

        while (1)
        {
            int n = read(p[0], buf, sizeof(buf));
            if (n == 0)
            {
                close(p[0]);
                break;
            }

            if (n % prime != 0)
            {
                if (write(p1[1], &n, sizeof(n)) < sizeof(n))
                {
                    panic("write number error");
                }
            }
        }

        close(p1[1]);
        wait(0);
        exit(0);
    }
}

void panic(char *s)
{
    fprintf(2, "%s\n", s);
    exit(1);
}

int fork1(void)
{
    int pid;

    pid = fork();
    if (pid == -1)
        panic("fork");
    return pid;
}

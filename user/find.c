#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *
fmtpath(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}

char *getfilename(char *path)
{
    char ch = '/';
    char *fileanme = strchr(path, ch) + 1;
    return fileanme;
}

void dirwalk(char *path, char *searchname)
{
    int fd;
    char buf[512], *p;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        exit(1);
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        exit(1);
    }

    switch (st.type)
    {
    case T_FILE:
        printf("path:%s\n", fmtpath(path));
        char *filename = getfilename(path);
        if (strcmp(filename, searchname) == 0)
        {
            printf("%s\n", fmtpath(path));
        }
        break;

    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("ls: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (de.inum == 0)
                continue;

            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            {
                continue;
            }

            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if (stat(buf, &st) < 0)
            {
                printf("ls: cannot stat %s\n", buf);
                continue;
            }

            if (st.type == T_FILE)
            {
                char *filename = getfilename(buf);
                if (strcmp(filename, searchname) == 0)
                {
                    printf("%s\n", fmtpath(buf));
                }
            }
            else if (st.type == T_DIR)
            {
                dirwalk(buf, searchname);
            }
        }
        break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(2, "Usage: find path filename\n");
        exit(1);
    }

    char *path = argv[1];
    char *searchname = argv[2];

    dirwalk(path, searchname);

    exit(0);
}

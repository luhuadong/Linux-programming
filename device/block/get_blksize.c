#include <sys/stat.h>
#include <stdio.h>

int main(void)
{
    struct stat stats;

    if (!stat("/", &stats))
    {
        printf("%lu\n", stats.st_blksize);
    }
}
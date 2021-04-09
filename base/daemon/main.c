#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    daemon(0, 0);

    system("echo \"Hello, World\" >> /home/root/hello.txt");

    sleep(30);
    
    return 0;
}

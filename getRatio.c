#include <sys/vfs.h>
#include <stdio.h>



float getRatio()
{
    int blocks;
    int avail;
    float ratio;
    struct statfs lstatfs;

    statfs("/", &lstatfs);
    blocks = lstatfs.f_blocks * (lstatfs.f_bsize/1024); 
    avail  = lstatfs.f_bavail * (lstatfs.f_bsize/1024); 
    ratio  = (avail *100) / blocks;

    return ratio;
}

int main()
{
  
    float ratio = getRatio();

    printf("ratio: %f%\n", ratio);
                
    return 0;
}

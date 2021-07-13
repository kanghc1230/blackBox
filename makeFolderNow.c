#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>

char buf[BUFSIZ];

void getTime(void)
{
    time_t UTCtime;
    struct tm *tm;
 
    time(&UTCtime); // UTC 현재 시간 읽어오기
    printf("time : %u\n", (unsigned)UTCtime); // UTC 현재 시간 출력

    tm = localtime(&UTCtime);

    // 3rd : %a : 간단한 요일, %m :월, %e : 일, %H : 24시, %M :분, %S :초, %Y :년
    //strftime(buf,sizeof(buf),"%a %m %e %H:%M:%S %Y", tm); // 사용자 정의 문자열 지정
    strftime(buf,sizeof(buf),"%Y%m%d%H", tm); // 사용자 정의 문자열 지정
    printf("strftime: %s\n",buf);
}

int main(void)
{
    char dirname[40];
    DIR *dir_info;
    struct dirent *dir_entry;
    getTime();

    sprintf(dirname, "/home/pi/blackbox/%s",buf);
    printf("dirname=%s",dirname);
    mkdir(dirname, 0755);
    return 0;
}
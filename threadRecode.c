
#include <stdio.h>
#include <string.h>
#include <fcntl.h> //file
#include <unistd.h> 
#include <dirent.h> //mkdir
#include <sys/stat.h> 
#include <sys/vfs.h>  //getRatio().. statfs
#include <sys/types.h>
#include <pthread.h> //thread

#define FILENAME_TIME 0
#define FOLDERNAME_TIME 1
#define LOG_TIME 2

char BUF[30] = "전역변수출력테스트, "; //함수 내에 gettime
char dirname[30] = "2021302052";
long delFolderName = 2103041;
void getTime(int return_Type) //0file 1 folder 2log
{
    time_t UTCtime;
    struct tm *tm;

    time(&UTCtime); // UTC 현재 시간 읽어오기

    tm = localtime(&UTCtime);
    // 3rd : %a : 간단한 요일, %m :월, %e : 일, %H : 24시, %M :분, %S :초, %Y :년
    if (return_Type == FILENAME_TIME)                       //.avi
        strftime(BUF, sizeof(BUF), "%Y%m%d%H%M%S.avi", tm); // 사용자 정의 문자열 지정
    else if (return_Type == FOLDERNAME_TIME)                //foldername
        strftime(BUF, sizeof(BUF), "%Y%m%d%H", tm);
    else if (return_Type == LOG_TIME) // [xx:xx] ~.log
        strftime(BUF, sizeof(BUF), "[%Y-%m-%d, %H:%M:%S]", tm);
}



// 쓰레드부분 
void* log_t_fuc(void *data)
{
    int *input = (int *)data;
    char buff[100]; // 파일에 쓸 버퍼
    int fd; // 파일열기
    int WRByte;
    fd = open("/home/pi/blackBox/threadRecode_log.log", O_WRONLY | O_CREAT | O_APPEND, 0644); 
    getTime(LOG_TIME); //BUF에 [시간]
    if(*input == 1)
    {   
        sprintf(buff, "%s blackBox log write on... \n", BUF);
    }
    if(*input == 2)
    {
        sprintf(buff, "%s ERROR! Unable to open camera. \n", BUF);
    }
    if(*input == 3)
    {
        sprintf(buff, "%s %s 폴더가 생성되었습니다.\n", BUF, dirname);
    }
    if(*input == 4)
    {
        sprintf(buff, "%s %ld 폴더를 삭제합니다.\n", BUF, delFolderName);
    }
    if(*input == 5)
    {
        sprintf(buff, "%s ERROR! %ld 폴더 삭제를 시도하였으나 실패하였습니다.\n", BUF, delFolderName);
    }
    if(*input == 6) 
    {
        sprintf(buff, "%s", BUF); //로그시간 먼저 기입
        WRByte = write(fd, buff, strlen(buff));

        getTime(FILENAME_TIME); // 파일명.avi 기입
        sprintf(buff, " %s파일의 녹화를 시작합니다.\n", BUF);
        WRByte = write(fd, buff, strlen(buff));
    }
    if(*input == 7)
    {
        sprintf(buff, "%s Can't write video.\n", BUF);
    }
    if(*input == 8)
    {
        sprintf(buff, "%s EEROR black frame grabbed\n", BUF);
    }
    if(*input == 9)
    {
        sprintf(buff, "%s Stop Video Recording ##########\n", BUF);
    }

    WRByte = write(fd, buff, strlen(buff));
    printf("로그쓰레드 실행구간\n");
}

void* folder_t_fuc(void *data)
{
    printf("폴더쓰레드 실행구간\n");
}
void* codec_t_fuc(void *data)
{
    printf("코덱쓰레드 실행구간\n");
}

int main ()
{
    pthread_t p_thread[3];
    int status;
    int err;
    int TLOG_blackBox_ON = 1,TLOG_ERROR_openCamera = 2 ,TLOG_makeFolder= 3,
    TLOG_delteFolder = 4 , TLOG_EEROR_DelteFolder = 5, TLOG_recoding_ON = 6, 
    TLOG_EEROR_writeVideo = 7, TLOG_EEROR_frameGrabbed = 8, TLOG_stopRecoding = 9;

    err = pthread_create(&p_thread[0], NULL, log_t_fuc, (void *)&TLOG_blackBox_ON);
    err = pthread_create(&p_thread[0], NULL, log_t_fuc, (void *)&TLOG_ERROR_openCamera);
    err = pthread_create(&p_thread[0], NULL, log_t_fuc, (void *)&TLOG_makeFolder);
    
    err = pthread_create(&p_thread[1], NULL, folder_t_fuc, (void *)&TLOG_blackBox_ON);
    err = pthread_create(&p_thread[2], NULL, codec_t_fuc, (void *)&TLOG_ERROR_openCamera);

    pthread_join(p_thread[0], (void **) &status);
    pthread_join(p_thread[1], (void **) &status);
    pthread_join(p_thread[2], (void **) &status);

    return 0;
}

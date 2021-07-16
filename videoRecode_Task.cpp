#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>  //file
#include <unistd.h> 
#include <dirent.h> //mkdir
#include <sys/stat.h>
#include <sys/vfs.h> //getRatio().. statfs
#include <sys/types.h>
#include <pthread.h> //thread

using namespace cv;
using namespace std;

#define FILENAME_TIME 0
#define FOLDERNAME_TIME 1
#define LOG_TIME 2

// 파일 실행되는 창 이름
#define VIDEO_WINDOW_NAME "Recode"

// getTime buffer
char BUF[30];
// getTime log_t buffer
char log_BUF[30];
int log_print_BUF = 0;

// getTime fileName
char filename_BUF[30];
// 폴더 경로+이름 버퍼
char dirname[30];
// 삭제된 폴더이름 long
long delFolderName;

// 쓰레드 에러처리 변수
int err = 0; 

void getTime(int return_Type);
float getRatio();
int rmdirs(const char *path, int force);
static int filter(const struct dirent *dirent);
void deleteFolder(void);
void makeFolderNow(void);


// 쓰레드 처리부분 //
// 로그쓰레드함수
void* log_t_fuc(void *data)
{   
    char buff[100]; // 파일에 쓸 버퍼
    int fd; // 파일열기
    int WRByte;
    log_print_BUF = 0; //로그버퍼 초기화
    // O_APPEND 파일이 있으면 아래로 계속 문장추가
    fd = open("/home/pi/blackBox/blackBoxlog.log", O_WRONLY | O_CREAT | O_APPEND, 0644); 
    while(1)
    {
        // 로그출력할 문장이있으면
        if (log_print_BUF != 0)
        {
            getTime(LOG_TIME); //log_BUF에 [시간]
            //buff에 쓸 문장을 getTime()으로 받아온 log_BUF의 시간을 읽어들이는부분
            if(log_print_BUF == 1)
            {   
                sprintf(buff, "\n%s :: blackBox log write on... ::\n", log_BUF);
            }
            if(log_print_BUF == 2)
            {
                sprintf(buff, "%s ERROR! Unable to open camera. \n", log_BUF);
            }
            if(log_print_BUF == 3)
            {
                sprintf(buff, "%s %s 폴더가 생성되었습니다.\n", log_BUF, dirname);
            }
            if(log_print_BUF == 4)
            {

                sprintf(buff, "%s /home/pi/blackBox/%ld 폴더를 삭제합니다.\n", log_BUF, delFolderName);
            }
            if(log_print_BUF == 5)
            {
                sprintf(buff, "%s ERROR! %ld 폴더 삭제를 시도하였으나 실패하였습니다.\n", log_BUF, delFolderName);
            }
            if(log_print_BUF == 6) 
            {
                sprintf(buff, "%s", log_BUF); //로그시간 먼저 기입
                WRByte = write(fd, buff, strlen(buff));

                getTime(FILENAME_TIME); // 파일명.avi 기입
                sprintf(buff, " %s파일의 녹화를 시작합니다.\n", filename_BUF);
            }
            if(log_print_BUF == 7)
            {
                sprintf(buff, "%s ERROR Can't write video.\n", log_BUF);
            }
            if(log_print_BUF == 8)
            {
                sprintf(buff, "%s ERROR black frame grabbed\n", log_BUF);
            }
            if(log_print_BUF == 9)
            {
                sprintf(buff, "%s :: Stop Video Recording :: ##########\n", log_BUF);
            }
            if (log_print_BUF == 99)
            {
                close(fd);
                break;
            }
            WRByte = write(fd, buff, strlen(buff)); // 파일에 쓰기
            log_print_BUF = 0; // 출력후 초기화
        }
        //쓰레드가 에러가 발생하면
        if(err != 0)
        {
            getTime(LOG_TIME);
            sprintf(buff, "%s ERROR can't create thread\n", log_BUF);
            WRByte = write(fd, buff, strlen(buff)); // 파일에 쓰기
            err = 0; //출력 후 초기화
        }
        usleep(10000); //0.1sec
    }

    close(fd); // 99로 break빠져나오면 파일닫기
    return (void*)0;
}

//폴더처리 쓰레드 (루프)
void *folder_t_fuc(void * data)
{   
    float Volume;
    float Volume_LastMin = 10;
    char buff[100]; //전폴더이름
    
    //용량확인 함수
    Volume = getRatio();
    printf("현재남은 용량은 %.1f입니다.\n", Volume); 
    makeFolderNow(); //처음 폴더생성
    log_print_BUF = 3;
    sprintf(buff, "%s",BUF); // 처음 buff에 전 폴더이름 저장
    
    //시간읽어들여서 폴더이름변경,확인
    while (1)
    {   
        //용량읽기
        Volume = getRatio();
        if (Volume < Volume_LastMin)
        {
            //오래된 폴더삭제함수
            deleteFolder(); //delFolderName에 삭제된파일이름 long형으로 전달
            if (delFolderName){
                // 폴더삭제성공 로그파일 기록
                log_print_BUF = 4;
            }
            else if (delFolderName==0)
            {   // 폴더삭제실패 로그파일 기록
                log_print_BUF = 5;
            }
        }
        //폴더이름읽기
        getTime(FOLDERNAME_TIME);
        //찍어논 buff(전폴더이름)과 새로 겟타임한 BUF(새폴더이름)이 다르면 폴더생성  
        if (strcmp(BUF,buff)){  // strcmp은 같으면 0으로 작동안됨, BUF가 시간적으로 나중인문자열 상승시 1값(true)이나옴
            //로그파일에 시간기록, 폴더생성 기록
            makeFolderNow(); // 폴더생성후 이름을 dirname저장
            log_print_BUF = 3;
        }
        sprintf(buff, "%s",BUF); // buff에 BUF덮어쓰기
        usleep(500000); //0.5sec;  

        if (log_print_BUF == 99)
        {
                return (void*)0;
        }
    }
}

// 메인 함수 부분 //
int main(int, char **)
{
    // STEP 1. VideoCapture("경로") 동영상 파일을 가져올때
    //         VideoCapture(0) 카메라만 가져올때
    VideoCapture cap;
    VideoWriter writer;

    //쓰레드 3개 생성
    pthread_t p_thread[2]; //0로그쓰레드, 1폴더쓰레드
    int status;
    //로그 쓰레드 조건 #define선언용
    int TLOG_blackBox_ON = 1,TLOG_ERROR_openCamera = 2 ,TLOG_makeFolder= 3,
    TLOG_delteFolder = 4 , TLOG_EEROR_DelteFolder = 5, TLOG_recoding_ON = 6, 
    TLOG_EEROR_writeVideo = 7, TLOG_EEROR_frameGrabbed = 8, TLOG_stopRecoding = 9,
    TLOG_THREAD_END = 99;

    //종료플래그와 1800타이머 계산변수
    int exitFlag = 0;
    int MaxFrame = 1800; // 1분 타이머
    int countframe = 0;  // 30프레임에서 1분(1800프레임)을 체크할 변수

    //용량처리 변수
    float Volume; //저장 남은용량 비율%저장할변수
    float Volume_LastMin = 10; //최저 저장공간 10%미만이면 폴더삭제할것

    // 카메라 id변수
    int deviceID = 0;
    int apiID = cv::CAP_V4L2;

    Mat frame;

    //파일처리변수
    char buff[100];

    // 로그파일을 기록하기위해 파일열기
    err = pthread_create(&p_thread[0], NULL, log_t_fuc, (void *)0);
    sleep(1);

    log_print_BUF = 1;
    // STEP 1. cap.open() 카메라 장치열기
    cap.open(deviceID, apiID);
    if (!cap.isOpened())
    {
        printf("ERROR! Unable to open camera.\n");
        log_print_BUF = 2;
        return -1;
    }

    // 설정카메라 프레임 해상도 직접 변경방법
    cap.set(CAP_PROP_FPS, 30);
    cap.set(CAP_PROP_FRAME_WIDTH, 320);
    cap.set(CAP_PROP_FRAME_HEIGHT, 240);
    // 현재카메라가 초당 몇프레임으로 출력하는가 XX.get(CAP_PROP_FPS)
    float videoFPS = cap.get(CAP_PROP_FPS); //
    int videoWidth = cap.get(CAP_PROP_FRAME_WIDTH);
    int videoHeight = cap.get(CAP_PROP_FRAME_HEIGHT);
    printf("videoFPS = %f\n", videoFPS);
    printf("video width=%d, height=%d\n", videoWidth, videoHeight);

    //폴더생성,삭제 쓰레드, dirname값변경 
    err = pthread_create(&p_thread[1], NULL, folder_t_fuc, (void*)0);
    sleep(1);

    // STEP 3. 녹화시작
    while (1)
    {
        // 영상 촬영 시작부분 
        // 로그파일에 시간기록 파일명기록 
        log_print_BUF = 6;
        // 시간정보 읽어와 buff에 파일명을 생성 210712xxxxxx.avi
        // STEP 2. writer.open(저장하고자하는 경로+파일명,
        // 압축코덱지정 fourcc('x','x','x','x'), FPS, Size(가로,세로),
        // True == isColor? )
        getTime(FOLDERNAME_TIME);
        sprintf(dirname, "/home/pi/blackBox/%s", BUF); 
        getTime(FILENAME_TIME);
        sprintf(buff, "%s/%s",dirname,filename_BUF); //buff=파일경로,시간.avi 합침
        
        writer.open(buff, VideoWriter::fourcc('D', 'I', 'V', 'X'),
                    videoFPS, Size(videoWidth, videoHeight), true);
        printf("%s 파일의 녹화를 시작합니다.\n",buff);
        if (!writer.isOpened())
        {
            printf("Can't write video\n"); 
            log_print_BUF = 7;
            return -1;
        }
        // 창만들기
        namedWindow(VIDEO_WINDOW_NAME);
        countframe = 0; // 초기화

        while (countframe < MaxFrame)
        {
            countframe++;

            // 카메라에서 프레임 한장 읽어오기
            cap.read(frame);
            if (frame.empty())
            {
                perror("EEROR black frame grabbed\n");
                log_print_BUF = 8;
                break;
            }

            // 읽어온 프레임에 writer에 저장
            writer << frame;
            // STEP 4. 화면에 띄우기
            imshow(VIDEO_WINDOW_NAME, frame);

            // 27은 'ESC'키. 입력되면 종료
            if(waitKey(3)==27)
            {
                printf("Stop Video Recording\n");
                log_print_BUF = 9;
                exitFlag = 1;
                break;
            }
        }
        
        //1분 영상 하나 끝
        writer.release();
        if (exitFlag == 1)
            break;
    }
    // 로그 쓰레드, 폴더 쓰레드 종료 콜
    log_print_BUF = 99;
    sleep(1);

    // 쓰레드 반환
    pthread_join(p_thread[0], (void **) &status);
    pthread_join(p_thread[1], (void **) &status);

    // 카메라 off
    cap.release();
    destroyWindow(VIDEO_WINDOW_NAME);

    return 0;
}

// 시간 잡아와서 포맷변환
void getTime(int return_Type) //0file 1 folder 2log
{
    time_t UTCtime;
    struct tm *tm;

    time(&UTCtime); // UTC 현재 시간 읽어오기

    tm = localtime(&UTCtime);
    // 3rd : %a : 간단한 요일, %m :월, %e : 일, %H : 24시, %M :분, %S :초, %Y :년
    if (return_Type == FILENAME_TIME)                       //.avi
        strftime(filename_BUF, sizeof(filename_BUF), "%Y%m%d%H%M%S.avi", tm); // 사용자 정의 문자열 지정
    else if (return_Type == FOLDERNAME_TIME)                //foldername
        strftime(BUF, sizeof(BUF), "%Y%m%d%H", tm);
    else if (return_Type == LOG_TIME) // [xx:xx] ~.log
        strftime(log_BUF, sizeof(log_BUF), "[%Y-%m-%d, %H:%M:%S]", tm);
}

//파일 용량 읽어들이는 함수 , 반환값 float형 용량.0000
float getRatio()
{
    int blocks;
    int avail;
    float ratio;
    struct statfs lstatfs; //시스템구조체<sys/vfs.h>

    statfs("/", &lstatfs);
    blocks = lstatfs.f_blocks * (lstatfs.f_bsize/1024); 
    avail  = lstatfs.f_bavail * (lstatfs.f_bsize/1024); 
    ratio  = (avail *100) / blocks;

    return ratio;
}

//폴더강제삭제함수 (파일이 있는 디렉토리)
int rmdirs(const char *path, int force) 
{
    DIR * dir_ptr = NULL;
    struct dirent *file = NULL;
    struct stat buf;
    char filename[1024]; 
    // 목록을 읽을 디렉토리명으로 DIR *를 return 받습니다. 
    if((dir_ptr = opendir(path)) == NULL) 
    { // path가 디렉토리가 아니라면 삭제하고 종료합니다.  
        return unlink(path); 
    } // 디렉토리의 처음부터 파일 또는 디렉토리명을 순서대로 한개씩 읽습니다.  
    while((file = readdir(dir_ptr)) != NULL) 
    {
        // readdir 읽혀진 파일명 중에 현재 디렉토리를 나타네는 . 도 포함되어 있으므로 
        // 무한 반복에 빠지지 않으려면 파일명이 . 이면 skip 해야합니다 
        if(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) 
        { continue; } 
        sprintf(filename, "%s/%s", path, file->d_name);
        // 파일의 속성(파일의 유형, 크기, 생성/변경 시간 등을 얻기 위하여  
        if(lstat(filename, &buf) == -1) 
        { continue; } 
        if(S_ISDIR(buf.st_mode)) 
        { 
            // 검색된 이름의 속성이 디렉토리이면 
            // 검색된 파일이 directory이면 재귀호출로 하위 디렉토리를 다시 검색 
            if(rmdirs(filename, force) == -1 && !force) 
            {
                return -1; 
            } 
        } 
        else if(S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) 
        { // 일반파일 또는 symbolic link 이면 
            if(unlink(filename) == -1 && !force) 
            { 
                return -1; 
            } 
        } 
    } 
    // open된 directory 정보를 close 합니다.  
    closedir(dir_ptr); 
    return rmdir(path); 
}
/* ".", ".." 은 빼고 나머지 10자리 숫자(디렉토리)만 출력하는 필터 함수 */
static int filter(const struct dirent *dirent)
{   //폴더걸러내기
    if(strlen(dirent->d_name)!=10)
        return 0;
    else if(!(strcmp(dirent->d_name, ".")) ||
        !(strcmp(dirent->d_name, "..")) )
        return 0;
    for (int i=0; i < strlen(dirent->d_name); i++)
        if ( isdigit(dirent->d_name[i])==0 )
            return 0;
    // 정상적이면
    return 1;
}
// 오래된 폴더삭제함수
void deleteFolder(void)
{
    const char *path = "/home/pi/blackBox/";
    struct dirent **namelist; // #include <dirent.h>
    long min =0;
    int count; //폴더카운트
    int i;
    char delete_path[50];
    
    //scandir로읽어들이고
    //scandir(탐색위치 주소값, 폴더이름구조체, *filter함수 실행, alphasort 순으로 정렬) 
    if((count = scandir(path, &namelist, *filter, alphasort)) == -1) 
    { 
        fprintf(stderr, "%s Directory Scan Error: %s\n", path, strerror(errno)); //파일읽기실패
    } 
    for(i=0;i<count;i++)
    {
        printf("파일리스트 출력 : %s\n", namelist[i]->d_name);
        if (min == 0 || min > atol(namelist[i]->d_name)) // 초기값이거나 min보다 들어온값이작으면
            min = atol(namelist[i]->d_name);
    }
    printf("가장 오래된 폴더 %ld 를 삭제합니다.\n",min);

    sprintf(delete_path, "/home/pi/blackBox/%ld",min);
    //강제삭제함수실행(파일이 있는 디렉토리)
    i = rmdirs(delete_path,1);
    if (i) //삭제 에러가 반환시
    {
        printf("파일삭제 실패\n");
        delFolderName = 0;
    }
    //할당해제 //네임리스트 구조체는 힙
    for(i = 0; i < count; i++) 
    { 
        free(namelist[i]); 
    } 
    free(namelist); 

    delFolderName = min; //long형으로 전역변수에 가장작은값폴더 min반환
}

// 시간읽어서 새로운 폴더생성함수
void makeFolderNow(void)
{
    DIR *dir_info;
    getTime(FOLDERNAME_TIME);
    sprintf(dirname, "/home/pi/blackBox/%s", BUF); //dirname에 폴더경로가들어감
    printf("%s 폴더를 생성합니다.\n", dirname);
    mkdir(dirname, 0755);
}



// https://github.com/kanghc1230/AWSLinuxEx/blob/main/csv_readtest.c
// TASK 2 쓰레드2 로그파일 "blackbox.log" 생성 ////////////////
// fd=open("blaxbox.log","w");
// 로그메세지 앞에는 항상 시간정보를 적어준다.
// 조건1) 폴더 생성 할때 // 로그파일에 폴더명. sprintf(buf,"%s log ~\n" ,BUF)
// 조건2) 파일 생성 할때 // 로그파일에 영상파일명. sprintf(buf,"%s xxxx.avi ~\n",BUF)
// 조건3) 에러가 발생했을때 // 로그파일에 에러명. sprintf(buf,"%s error ~\n",BUF)

// 첫번째 할것 쓰레드 2개생성
// https://github.com/kanghc1230/AWSLinuxEx/blob/main/pthread_STtest.c

// 쓰레드가 전역변수 폴더이름 배열만들기
// TASK 1. 쓰레드1 1시간마다 시간폴더 "2021071217"생성 ////////////////
// 폴더명 연월일시

// 동영상 파일 폴더를 이동을 해서 저장. 16시폴더(1658 1659) 17시폴더(1700 1701)
// int mkdir( const char *dirname );
// char strFolderPath[] = { "D:\\CreateFolder" };
// int nResult = mkdir( strFolderPath );

// TASK 3. 스레드 파일용량 확인후 높으면 삭제
// 사용가능한용량 확인후 적으면 영상촬영 맨처음파일 지우기
// 생각한방법 popen 시스템어 df |grep /dev/root 읽어들이고 3번째인자사용
// 용량체크하기 사이트 https://www.joinc.co.kr/w/man/2/statfs#AEN8


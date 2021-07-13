
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>  //file
#include <unistd.h> //file

#include <sys/stat.h> //mkdir
#include <dirent.h>

using namespace cv;
using namespace std;

#define FILENAME_TIME 0
#define FOLDERNAME_TIME 1
#define LOG_TIME 2

// 출력되는 파일이름 설정방법
// define OUTPUT_VIDEO_NAME "test.avi" //test.avi
// 202107 121433.avi
// 파일 실행되는 창 이름
#define VIDEO_WINDOW_NAME "Recode"

// getTime buffer
char BUF[30];

// 폴더 이름 버퍼
char dirname[30];

// 시간 잡아와서 포맷변환
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

// 폴더생성함수
void makeFolderNow(void)
{
    DIR *dir_info;

    getTime(FOLDERNAME_TIME);
    sprintf(dirname, "/home/pi/blackBox/%s", BUF); //dirname에 폴더경로가들어감
    printf("dirname=%s 폴더를 생성합니다.\n", dirname);
    mkdir(dirname, 0755);
}

int main(int, char **)
{
    // STEP 1. VideoCapture("경로") 동영상 파일을 가져올때
    //         VideoCapture(0) 카메라만 가져올때
    VideoCapture cap;
    VideoWriter writer;

    //종료플래그와 1800타이머 계산변수
    int exitFlag = 0;
    int MaxFrame = 1800; // 타이머
    int FolderFrame = 108000; // 108000 108000
    int countframe = 108000;  // 30프레임 1분(1800프레임)을 체크할 변수

    int deviceID = 0;
    int apiID = cv::CAP_V4L2;

    Mat frame;

    //파일처리
    char buff[100];
    int WRByte;
    // 로그파일을 기록하기위해 파일열기
    int fd = open("/home/pi/blackBox/blackBoxlog.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    getTime(LOG_TIME);
    sprintf(buff, "%s blackBox log write on... #########\n", BUF);
    WRByte = write(fd, buff, strlen(buff));

    // STEP 1. cap.open() 카메라 장치열기
    cap.open(deviceID, apiID);
    if (!cap.isOpened())
    {
        printf("ERROR! Unable to open camera.\n");
        getTime(LOG_TIME);
        sprintf(buff, "%s ERROR! Unable to open camera.\n", BUF);
        WRByte = write(fd, buff, strlen(buff));
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

    // STEP 3. 녹화시작
    while (1)
    {
        // 폴더생성이름 전역변수 dirname[30] FolderFrame 108000(1시간)
        if (countframe == FolderFrame )
        {
            //로그파일에 시간기록, 폴더생성 기록
            getTime(LOG_TIME);
            sprintf(buff, "%s", BUF);
            WRByte = write(fd, buff, strlen(buff));

            makeFolderNow(); // 폴더생성후 이름을 dirname저장

            sprintf(buff, " %s 폴더가 생성되었습니다.\n", dirname);
            WRByte = write(fd, buff, strlen(buff));

            countframe = 0; // 처음생성후 0초기화, 녹화시작
        }
        // 로그파일에 시간기록 파일명기록
        getTime(LOG_TIME);
        sprintf(buff, "%s", BUF);
        WRByte = write(fd, buff, strlen(buff));

        getTime(FILENAME_TIME);
        sprintf(buff, " %s파일의 녹화를 시작합니다.\n", BUF);
        WRByte = write(fd, buff, strlen(buff));
        printf("%s파일의 녹화를 시작합니다.\n",BUF);
        // 시간정보 읽어와 buff에 파일명을 생성 210712xxxxxx.avi
        // STEP 2. writer.open(저장하고자하는 경로+파일명,
        // 압축코덱지정 fourcc('x','x','x','x'), FPS, Size(가로,세로),
        // True == isColor? )
        sprintf(buff, "%s/%s",dirname,BUF); //buff=파일경로,시간.avi 합침
        writer.open(buff, VideoWriter::fourcc('D', 'I', 'V', 'X'),
                    videoFPS, Size(videoWidth, videoHeight), true);
        if (!writer.isOpened())
        {
            printf("Can't write video\n");
            getTime(LOG_TIME);
            sprintf(buff, "%s Can't write video.\n", BUF);
            WRByte = write(fd, buff, strlen(buff));
            return -1;
        }
        // 창만들기
        namedWindow(VIDEO_WINDOW_NAME);

        countframe++;
        while (countframe % MaxFrame != 0)
        {
            countframe++;

            // 카메라에서 프레임 한장 읽어오기
            cap.read(frame);
            if (frame.empty())
            {
                perror("EEROR black frame grabbed\n");
                getTime(LOG_TIME);
                sprintf(buff, "%s EEROR black frame grabbed\n", BUF);
                WRByte = write(fd, buff, strlen(buff));
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
                getTime(LOG_TIME);
                sprintf(buff, "%s Stop Video Recording ##########\n", BUF);
                WRByte = write(fd, buff, strlen(buff));

                exitFlag = 1;
                break;
            }
        }
        writer.release();
        if (exitFlag == 1)
            break;
    }

    cap.release();
    destroyWindow(VIDEO_WINDOW_NAME);

    close(fd);
    return 0;
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

// TASK 3. 스레드 파일용량확인후 높으면 삭제
// 사용가능한용량 확인후 적으면 영상촬영 맨처음파일 지우기
// 생각한방법 popen df |grep /dev/root
// 용량체크하기 사이트 https://www.joinc.co.kr/w/man/2/statfs#AEN8

// 구조체 리턴함수만들기{
// sscanf(buf, "%s%s%s", mp->구조체변수1,구조체변수2,구조체변수3)
//
// return 구조체포인터}

/*
//로그파일끝
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>//file
#include <unistd.h>//file

using namespace cv;
using namespace std;
 
#define FILENAME_TIME 0
#define FOLDERNAME_TIME 1 
#define LOG_TIME 2

// 출력되는 파일이름 설정방법
// define OUTPUT_VIDEO_NAME "test.avi" //test.avi
// 202107 121433.avi 
// 파일 실행되는 창 이름
#define VIDEO_WINDOW_NAME "Recode"

// getTime buffer
char BUF[30];

// 시간 잡아와서 포맷변환
void getTime(int return_Type) //0file 1 folder 2log
{
    time_t UTCtime;
    struct tm *tm;
    
    time(&UTCtime); // UTC 현재 시간 읽어오기

    tm = localtime(&UTCtime);
    // 3rd : %a : 간단한 요일, %m :월, %e : 일, %H : 24시, %M :분, %S :초, %Y :년
    if(return_Type==FILENAME_TIME ) //.avi
        strftime(BUF,sizeof(BUF),"%Y%m%d%H%M%S.avi", tm); // 사용자 정의 문자열 지정
    else if (return_Type == FOLDERNAME_TIME) //foldername
        strftime(BUF,sizeof(BUF),"%Y%m%d%H", tm);
    else if (return_Type==LOG_TIME) // [xx:xx] ~.log
        strftime(BUF,sizeof(BUF),"[%Y-%m-%d, %H:%M:%S]", tm);
}

void makeFolderNow(void)
{
    char dirname[30];
    DIR *dir_info;
    struct dirent *dir_entry;

    //로그파일에 시간 쓰기
    getTime(LOG_TIME);
    sprintf(dirname ,"%s",BUF);
    WRByte = write(fd,dirname,strlen(dirname));

    getTime(FOLDERNAME_TIME);
    sprintf(dirname, "/home/pi/blackbox/%s",BUF);
    printf("dirname=%s 폴더를 생성합니다.",dirname);

    sprintf(dirname ,"%s",BUF);
    WRByte = write(fd,dirname,strlen(dirname));

    mkdir(dirname, 0755);
}////////////////////////

int main(int, char**)
{
    // STEP 1. VideoCapture("경로") 동영상 파일을 가져올때
    //         VideoCapture(0) 카메라만 가져올때
    VideoCapture cap;
    VideoWriter writer;

    //종료플래그와 1800타이머 계산변수
    int exitFlag = 0;
    int MaxFrame = 1800; // 타이머
    int countframe; // 30프레임 1분(1800프레임)을 체크할 변수

    int deviceID = 0;
    int apiID = cv::CAP_V4L2;

    Mat frame;
    
    //파일처리
    char buff[100];
    int WRByte; 
    // 로그파일을 기록하기위해 파일열기
    int fd = open("/home/pi/blackBox/blackBoxlog.log",O_WRONLY | O_CREAT | O_TRUNC, 0644);
    getTime(LOG_TIME);
    sprintf(buff ,"%s blackBox log write on... #########\n",BUF);
    WRByte = write(fd,buff,strlen(buff));

    // STEP 1. cap.open() 카메라 장치열기
    cap.open(deviceID, apiID);
    if (!cap.isOpened()) 
    {
        printf("ERROR! Unable to open camera.\n");
        getTime(LOG_TIME);
        sprintf(buff ,"%s ERROR! Unable to open camera.\n",BUF);
        WRByte = write(fd,buff,strlen(buff));
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
    
    // STEP 3. 녹화시작
    while(1)
    {
        getTime(LOG_TIME);
        sprintf(buff ,"%s",BUF);
        WRByte = write(fd,buff,strlen(buff));

        //시간정보 읽어와 fileName[]에 파일명을 생성 210712xxxxxx.avi 
        // STEP 2. writer.open(저장하고자하는 경로+파일명, 
        // 압축코덱지정 fourcc('x','x','x','x'), FPS, Size(가로,세로), 
        // True == isColor? )
        getTime(FILENAME_TIME);
        sprintf(buff ," %s파일의 녹화를 시작합니다.\n",BUF);
        WRByte = write(fd,buff,strlen(buff));

        writer.open(BUF, VideoWriter::fourcc('D','I','V','X'),
        videoFPS, Size(videoWidth, videoHeight), true);
        if (!writer.isOpened())
        {
            printf("Can't write video\n");
            getTime(LOG_TIME);
            sprintf(buff ,"%s Can't write video.\n",BUF);
            WRByte = write(fd,buff,strlen(buff));
            return -1;
        }
        // 창만들기
        namedWindow(VIDEO_WINDOW_NAME);
        countframe = 0; // 초기화

        while(countframe < MaxFrame)
        {
            countframe++;

            // 카메라에서 프레임 한장 읽어오기
            cap.read(frame);
            if(frame.empty())
            {
                perror("EEROR black frame grabbed\n");
                getTime(LOG_TIME);
                sprintf(buff ,"%s EEROR black frame grabbed\n",BUF);
                WRByte = write(fd,buff,strlen(buff));
                break;
            }

            // 읽어온 프레임에 writer에 저장
            writer << frame; 
            // STEP 4. 화면에 띄우기
            imshow(VIDEO_WINDOW_NAME, frame);
            
            // 27은 'ESC'키. 입력되면 종료
            if(waitKey(1)==27)
            {
                printf("Stop Video Recording\n");
                getTime(LOG_TIME);
                sprintf(buff ,"%s Stop Video Recording ##########\n",BUF);
                WRByte = write(fd,buff,strlen(buff));

                exitFlag = 1;
                break;
            }
        }
        writer.release();
        if(exitFlag==1)
            break;
    }
    cap.release();
    destroyWindow(VIDEO_WINDOW_NAME);

    close(fd);
    return 0;
}

*/
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;
 
// 출력되는 파일이름 설정방법
// define OUTPUT_VIDEO_NAME "test.avi" //test.avi
// 202107 121433.avi 
// 파일 실행되는 창 이름
#define VIDEO_WINDOW_NAME "Recode"

// 파일이름 전역변수로
char fileName[30];
// 시간 잡아와서 포맷변환
void makeFileName(void)
{
    time_t UTCtime;     // 틱 읽을 타임변수
    struct tm *tm;

    time(&UTCtime); // UTC 현재 시간 읽어오기
    tm = localtime(&UTCtime);
    // 포맷을 210712151620.avi
    strftime(fileName, sizeof(fileName), "%Y%m%d%H%M%S.avi" , tm);
    printf("strftime : %s\n",fileName);
}

int main(int, char**)
{
    // STEP 1. VideoCapture("경로") 동영상 파일을 가져올때
    //         VideoCapture(0) 카메라만 가져올때
    VideoCapture cap;
    VideoWriter writer;
    int exitFlag = 0;
    int MaxFrame = 1800; // 타이머
    int countframe; // 30프레임 1분(1800프레임)을 체크할 변수

    int deviceID = 0;
    int apiID = cv::CAP_V4L2;

    Mat frame;
    // STEP 1. cap.open() 카메라 장치열기
    cap.open(deviceID, apiID);
    if (!cap.isOpened()) {
        perror("ERROR! Unable to open camera\n");
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
        //시간정보 읽어와 fileName[]에 파일명을 생성 210712xxxxxx.avi
        makeFileName(); 

        // STEP 2. writer.open(저장하고자하는 파일명, 
        // 압축코덱지정 fourcc('x','x','x','x'), FPS, Size(가로,세로), 
        // True == isColor? )
        writer.open(fileName, VideoWriter::fourcc('D','I','V','X'),
        videoFPS, Size(videoWidth, videoHeight), true);
        if (!writer.isOpened())
        {
            printf("Can't write video\n");
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
                break;
            }

            // 읽어온 프레임에 writer에 저장
            writer << frame; 
            // STEP 4. 화면에 띄우기
            imshow(VIDEO_WINDOW_NAME, frame);
            
            // 27은 'ESC'키. 입력되면 종료
            if(waitKey(1)==27)
            {
                printf("Stop video record\n");
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

    return 0;
}

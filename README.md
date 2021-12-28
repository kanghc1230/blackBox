# blackBox

videoRecode_Task.cpp

리눅스 카메라 파일관리

## TASK 1 영상촬영
영상촬영 후 끊김이없도록함.

## TASK 2 쓰레드2 로그파일 "blackbox.log" 생성 ////////////////  
fd=open("blaxbox.log","w");  
로그메세지 앞에는 항상 시간정보를 적어준다.  
조건1) 폴더 생성 할때 // 로그파일에 폴더명. sprintf(buf,"%s log ~\n" ,BUF)  
조건2) 파일 생성 할때 // 로그파일에 영상파일명. sprintf(buf,"%s xxxx.avi ~\n",BUF)  
조건3) 에러가 발생했을때 // 로그파일에 에러명. sprintf(buf,"%s error ~\n",BUF)  
  
첫번째 할것 쓰레드 2개생성 *쓰레드 사용참고 자료   
https://github.com/kanghc1230/AWSLinuxEx/blob/main/pthread_STtest.c  
  
쓰레드가 전역변수 폴더이름 배열만들기  
  
  
## TASK 3. 쓰레드1 1시간마다 시간폴더 "2021071217"생성 ////////////////  
폴더명 연월일시  
  
동영상 파일 폴더를 이동을 해서 저장. 16시폴더(1658 1659) 17시폴더(1700 1701)  
int mkdir( const char *dirname );  
char strFolderPath[] = { "D:\\CreateFolder" };  
int nResult = mkdir( strFolderPath );  

## TASK 4. 스레드 파일용량 확인후 높으면 자동 삭제
사용가능한용량 확인후 적으면 영상촬영 맨처음파일 지우기  
생각한방법 popen 시스템어 df |grep /dev/root 읽어들이고 3번째인자사용   

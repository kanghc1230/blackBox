#include <dirent.h> 
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <ctype.h> //isdigit
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h>

int rmdirs(const char*, int); //폴더삭제함수 (경로,1)

#define MAX_LIST 50 

// 현재경로를 가리키는 path
// 
const char *path = "/home/pi/blackBox/"; 
/* ".", ".." 은 빼고 나머지 파일명 출력하는 필터 함수 */
static int filter(const struct dirent *dirent)
{  
    if(strlen(dirent->d_name)!=10)
        return 0;
    else if(!(strcmp(dirent->d_name, ".")) ||
        !(strcmp(dirent->d_name, "..")) )
        return 0;
    for (int i=0; i < strlen(dirent->d_name); i++)
        if ( isdigit(dirent->d_name[i])==0 )
            break;
}

long searchOldFolder(void) 
{ 
    const char *path = "/home/pi/blackBox/";
    char delete_path[100]; 
    struct dirent **namelist; // #include <dirent.h>
    long min =0;
    int count; //폴더카운트
    int i;

    //scandir로읽어들이고
    //scandir(탐색위치 주소값, 폴더이름구조체, *filter함수 실행, alphasort 순으로 정렬) 
    if((count = scandir(path, &namelist, *filter, alphasort)) == -1) 
    { 
        fprintf(stderr, "%s Directory Scan Error: %s\n", path, strerror(errno)); //파일읽기실패
    } 
    for(i=0;i<count;i++)
    {
        printf("%s\n", namelist[i]->d_name);
        if (min == 0 || min > atol(namelist[i]->d_name)) // 초기값이거나 min보다 들어온값이작으면
            min = atol(namelist[i]->d_name);
    }
    sprintf(delete_path, "/home/pi/blackBox/%ld",min);
    //printf("가장 작은 삭제할 폴더 : %s\n", delete_path);
    i = rmdirs(delete_path,1);

    for(i = 0; i < count; i++) 
    { 
        free(namelist[i]); 
    } 
    free(namelist); //네임리스트 구조체는 힙
   
    return min; 
    
}

int main(void)
{
    long result; 
    char folderName[30];
    result = searchOldFolder();
    sprintf(folderName,"%ld",result);
    printf("가장 오래된 폴더는 %s 이다.\n",folderName);
}


int rmdirs(const char *path, int force) 
{
    DIR * dir_ptr = NULL;
    struct dirent *file = NULL;
    struct stat buf;
    char filename[1024]; 
    /* 목록을 읽을 디렉토리명으로 DIR *를 return 받습니다. */
    if((dir_ptr = opendir(path)) == NULL) 
    { /* path가 디렉토리가 아니라면 삭제하고 종료합니다. */ 
        return unlink(path); 
    } /* 디렉토리의 처음부터 파일 또는 디렉토리명을 순서대로 한개씩 읽습니다. */ 
    while((file = readdir(dir_ptr)) != NULL) 
    {
        // readdir 읽혀진 파일명 중에 현재 디렉토리를 나타네는 . 도 포함되어 있으므로 
        // 무한 반복에 빠지지 않으려면 파일명이 . 이면 skip 해야합니다 
        if(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) 
        { continue; } 
        sprintf(filename, "%s/%s", path, file->d_name);
        /* 파일의 속성(파일의 유형, 크기, 생성/변경 시간 등을 얻기 위하여 */ 
        if(lstat(filename, &buf) == -1) 
        { continue; } 
        if(S_ISDIR(buf.st_mode)) 
        { 
            // 검색된 이름의 속성이 디렉토리이면 
            /* 검색된 파일이 directory이면 재귀호출로 하위 디렉토리를 다시 검색 */ 
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
    /* open된 directory 정보를 close 합니다. */ 
    closedir(dir_ptr); 
    return rmdir(path); 
}

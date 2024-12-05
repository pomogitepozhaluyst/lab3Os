#include <iostream>
#include <stdlib.h>
#include <errno.h>

#ifdef WIN32
 #include <tchar.h>
 #include <windows.h>
#else
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/wait.h>
 #include <spawn.h>
#endif


void createProgramm(std::string fileName){
 #ifdef WIN32
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  
  if (!CreateProcess(
    NULL,
    _T((char*)fileName.data()),
    NULL,
    NULL,
    FALSE,
    0,
    NULL,
    NULL,
    &si,
    &pi)
  ){
   printf("CreateProcess failed (%d). \n", GetLastError());
   return;
  }
  DWORD rv;
  WaitForSingleObject(pi.hProcess, INFINITE);
  GetExitCodeProcess(pi.hProcess, &rv);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  //printf("return value: %d\n", rv);
 #else
  pid_t pid;
  int rv = 0;
  switch(pid=fork()){
   case -1:
    perror("fork");
    exit(1);
   case 0:
    execlp("/bin/sh", "sh", "-c", fileName.data(), (char*)NULL);
   default:
    wait(&rv);
  }
  //printf("return value: %d \n", WEXITSTATUS(rv));

 #endif
}
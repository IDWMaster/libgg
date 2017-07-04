#include "GlobalGrid.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sstream>
#include <pthread.h>
#include <sys/stat.h>
#include <string>
#include <thread>
#include "cppext/cppext.h"



typedef struct {
  //Master control mutex
  pthread_mutex_t mtx;
  //Mastr control signal
  pthread_cond_t evt;
  pid_t pid; //Requesting process ID
} GLOBALGRID_CONTROL;

class ConnectionManager {
public:
  int fd;
  int remoteFd;
  GLOBALGRID_CONTROL* control;
  GLOBALGRID_CONTROL* remoteRPC; //Remote process RPC
  bool running = true;
  ConnectionManager() {
    const char* ggpath = getenv("GLOBALGRID_PATH");
    ggpath = ggpath ? ggpath : "/tmp/GlobalGrid";
    std::stringstream ss;
    ss<<ggpath<<"/control";
    fd = open(ss.str().data(),O_RDWR);
    if(fd <= 0) {
      throw "up";
    }
    pid_t process = getpid();
    struct stat us; //MAC == status symbol
    fstat(fd,&us);
    control = (GLOBALGRID_CONTROL*)mmap(0,us.st_size,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    while(1) {
    pthread_mutex_lock(&control->mtx);
    control->pid = process;
    pthread_cond_broadcast(&control->evt);
    pthread_mutex_unlock(&control->mtx);
    pthread_mutex_lock(&control->mtx);
    pid_t foundpid = control->pid;
    pthread_mutex_unlock(&control->mtx);
    if(foundpid == process) {
      std::string path = (const char*)(control+1);
      munmap(control,us.st_size);
      close(fd);
      fd = open(path.data(),O_RDWR);
      fstat(fd,&us);
      control = (GLOBALGRID_CONTROL*)mmap(0,us.st_size,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
      path = (const char*)(control+2);
      remoteFd = open(path.data(),O_RDWR);
      fstat(remoteFd,&us);
      remoteRPC = (GLOBALGRID_CONTROL*)mmap(0,us.st_size,PROT_READ | PROT_WRITE,MAP_SHARED,remoteFd,0);
      break;
    }
    }
    std::thread m([=](){
      while(running) {
	pthread_mutex_lock(&control->mtx);
	pthread_cond_wait(&control->evt);
	System::BStream str((const char*)(remoteRPC+1));
	unsigned char opcode;
	str.Read(opcode);
	switch(opcode) {
	  
	}
	pthread_mutex_unlock(&control->mtx);
      }
    });
    m.detach();
  }
  
};


static ConnectionManager manager;
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "ports.h"

int main(int argc, char *argv[])
{
  int s;
  ssize_t res;
  char m[32];
  char my_name[32];
  char ResourcesRequested [3];
  char ch;
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <id>\n", argv[0]);
    return -1;
  }
//Initialize how many resources
  strcpy(ResourcesRequested, argv[1]);

  sprintf(my_name, "/tmp/client%d\n", getpid());
  s = port_open(my_name);
  if (s < 0) {
    return -1;
  }

  res = port_send(s, ResourcesRequested, sizeof(ResourcesRequested), "/tmp/Allocate_resource");
  if (res < 0) {
    perror("Recv");
    return -1;
  }

  res = recv(s, m, 32, 0);
  if (res < 0) {
    return -1;
  }
  printf("Resource list of %s resources allocated for me: \n",ResourcesRequested );
//Now i can use them

  for (int i = 0; i<res; i++){
    if(m[i] >= '0' && m[i] <= '9'){
      printf("ID: %c\n", m[i] );
    }
  }

//Get Resource's id

  char deallocate[20];
  for (int i = 0; i<res; i++){
    if(m[i] >= '0' && m[i] <= '9'){
      printf("I'm using: %c\n",m[i] );
      sleep(4);
      printf("I finished!\n");
      printf("Deallocation for: %c\n",m[i] );
      sprintf(deallocate, "resource%c\n", m[i]);
      port_send(s, deallocate, strlen(deallocate) + 1, "/tmp/Deallocate_resource");
    }      
  }

  
printf("Press any key to exit\n");    
scanf("%c",&ch); 
  return 0;
}

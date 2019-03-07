#include <sys/un.h>
#include <sys/socket.h>
#include <poll.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ports.h"
#include <stdlib.h>

#define N 8 //Number of resources (From 0 to 7)
#define MaxQueue 100

struct QueueWait{
  struct sockaddr_un SocketWaiting;
  int NumberOfResource;
};

static bool allocated[N];
static unsigned int available = N;
struct QueueWait QW[MaxQueue];

int FirstFree(){
  unsigned int i;
  i = 0;
  while(allocated[i]) {
    i++;
  }
  return i;
}

int main()
{
//fd for communication
  int alloc_fd, dealloc_fd;
  int res;
  unsigned int i;
  int NumberWaitingClient = 0;
  int indexQueue = 0;
//setting struct
  for (i = 0; i < N; i++) {
    allocated[i] = false;
  }
  for (i = 0; i < MaxQueue; i++) {
    memset(&QW[i].SocketWaiting, 0, sizeof(QW[i].SocketWaiting));
    QW[i].NumberOfResource = -1;
  }

  alloc_fd   = port_open("/tmp/Allocate_resource");
  dealloc_fd = port_open("/tmp/Deallocate_resource");

  while(1) {
    char msg[32];
    struct pollfd guards[2];

    guards[0].fd     = alloc_fd; //fd for allocation
    guards[0].events = POLLIN;
    guards[1].fd     = dealloc_fd; //fd for deallocation
    guards[1].events = POLLIN;

    res = poll(guards, 2, -1);
    if (res <= 0) {
      fprintf(stderr, "Error: poll() returned %d\n", res);
    }

//****************************************ALLOCATION******************************//
      if ((available > 0) && (guards[0].revents == POLLIN)) {
        struct sockaddr_un sender;
        socklen_t len;

	  len = sizeof(struct sockaddr_un);
      res = recvfrom(alloc_fd, msg, 3, 0, (struct sockaddr *) &sender, &len);
      if (res < 0) {
          perror("RecvFrom");
          return -1;
        }
      printf("Client:%s ask for %d resources\n", sender.sun_path, (int)atoi(msg) );

/*If resources asked are available */
      if((unsigned int)atoi(msg) < available  && NumberWaitingClient == 0){ //only if there is enough resources and the queue is empty
        available-=atoi(msg);//reducing available
        int NumResource = atoi(msg);
        printf("%s can use %d resources\n", sender.sun_path, atoi(msg));

	  printf("Allocating %d resources\n", NumResource);
	  char ResourceList[32] = "";
	  strcat(ResourceList,"-");
        for(int i =0; i<NumResource; i++){
          printf("Allocated resource: %d\n", FirstFree());
          sprintf(msg, "%u", FirstFree());
          strcat(ResourceList,msg);
          strcat(ResourceList,"-");
          allocated[FirstFree()] = true;//allocation
          if (res < 0) { 
            perror("SendTo");
          }
        }
        strcat(ResourceList,"\n");
     printf("List sended %s\n",ResourceList );
     res = sendto(alloc_fd, ResourceList, strlen(ResourceList) + 1, 0, (struct sockaddr *) &sender, len);
      }
      else{/*resource not available */
        printf("Not enough resources for: %s can't use %d resources\n", sender.sun_path, atoi(msg));
//Save the WaitingClient
        QW[indexQueue].SocketWaiting = sender;
        QW[indexQueue].NumberOfResource = atoi(msg);  
        NumberWaitingClient ++; //queue not empty
        indexQueue = (indexQueue == 100)?0:indexQueue + 1;
      }
    }

      
//****************************************DEALLOCATION******************************//
      if (guards[1].revents == POLLIN) {
       res = recv(dealloc_fd, msg, 32, 0);
       if (res < 0) { 
        perror("SendTo");
      }

      unsigned int RisorsaDaDeallocare;
      sscanf(msg, "resource%u", &RisorsaDaDeallocare );
      allocated[RisorsaDaDeallocare] = false; //deallocation
      available++;
      printf("Deallocated resource: %d,\n now there are: %d resources available\n", RisorsaDaDeallocare, available);

//Using Queue to avoid starvation with FIFO.
      int i = 0;
      int len = sizeof(struct sockaddr_un);
// looking for the first one
      while(QW[i].NumberOfResource == -1 && i < MaxQueue){
        i++;
      }
// if the first one now has enough resources available
      if((unsigned int)QW[i].NumberOfResource < available && i < MaxQueue && QW[i].NumberOfResource != -1){
        available -= QW[i].NumberOfResource;
        printf("Waking up Client: %s asking for %d resources.\n",QW[i].SocketWaiting.sun_path, QW[i].NumberOfResource);
  		printf("Allocating %d resources....\n\n", QW[i].NumberOfResource);
  		char temp[4];
  		char ResourceListQueue[32] = "";
  		strcat(ResourceListQueue,"-");
	        for(int j =0; j<(int)QW[i].NumberOfResource; j++){
	          printf("Allocated resource: %d\n", FirstFree());
	          sprintf(temp, "%u", FirstFree());
	          strcat(ResourceListQueue,temp);
	          strcat(ResourceListQueue,"-");
	          allocated[FirstFree()] = true;
	        }
        printf("Done!\n List sended: %s\n",ResourceListQueue );

   		res = sendto(alloc_fd, ResourceListQueue, strlen(ResourceListQueue) + 1, 0, (struct sockaddr *) &QW[i].SocketWaiting, len);
   		   if (res < 0) { 
	            perror("SendTo");
	          }
        QW[i].NumberOfResource = -1;
        NumberWaitingClient--;
      }
    }
}
  return 0;
}

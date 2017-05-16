/* tcpszerver.c
 * 
 * Egyszerű TCP szerver példa. Képes egy kapcsolat fogadására és
 * kiírja a képernyőre a kapott adatokat.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <errno.h>

#define PORT "1122"
#define BUFFSIZE 1024
#define MAX_CONNECTION_NUM	2
#define DEBUG

int log_fd;
int ssock;
time_t t;
struct tm tm;

/* message format */
struct conn_info{
  int pid;          /* which working process */
  int csock;        /* which client socket */
  int status;       /* status of connection */
};

void resource_liberator(void)
{
  t = time(NULL);
  tm = *localtime(&t);
  #ifdef DEBUG
  printf("Session over at %d-%d-%d %d:%d:%d\n\n", tm.tm_year + 1900, 
        tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  #endif
  dprintf(log_fd, "Session over at %d-%d-%d %d:%d:%d\n\n", tm.tm_year + 1900, 
        tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  close(log_fd);
  close(ssock);
}

int main() 
{
  struct addrinfo hints;
  struct addrinfo* res;
  int err;
  struct sockaddr_in6 addr;
  socklen_t addrlen;
  char ips[NI_MAXHOST];
  char servs[NI_MAXSERV];
  int csock;
  int reuse;
  char arg_buf[16];
  int pid;
  int active_connection = 0;
  int status;
  key_t msgq_key;
  int msgq_id;
  struct pollfd pollset;

  /**************************** logfile init ****************************/
  log_fd = open("./logfile.log", O_APPEND|O_CREAT|O_WRONLY, 0644);
  if(log_fd < 0)
  {
    perror("Log file open");
    return 1;
  }

  t = time(NULL);
  tm = *localtime(&t);
  #ifdef DEBUG
  printf("Session started at %d-%d-%d %d:%d:%d\n\n", tm.tm_year + 1900, 
        tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  #endif
  dprintf(log_fd, "Session started at %d-%d-%d %d:%d:%d\n\n", tm.tm_year + 1900, 
        tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

  /***********************************************************************/

  /***************************** init msg queue **************************/
  //msgq_key = key(".", 's');
  //msgq_id = msgget(msgq_key, IPC_CREAT|IPC_EXEC, 0664);
  //if(msgq_id == -1)
  //{
  //  fprintf(stderr, "Message queue intialization error\n");
  //}

  /***********************************************************************/

  /*****************************socket init ******************************/  
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;

  err = getaddrinfo(NULL, PORT, &hints, &res);
  if(err != 0)
  {
    fprintf(stderr, "Getaddrinfo error: %s\n", gai_strerror(err));
    dprintf(log_fd, "Getaddrinfo error: %s\n", gai_strerror(err));
    resource_liberator();
    return -1;
  }
  if(res == NULL)
  {
    resource_liberator();
    return -1;
  }
  
  /* Létrehozzuk a server socketet getaddrinfo() válasza alapján */
  ssock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if(ssock < 0)
  {
    fprintf(stderr, "Server socket init error\n");
    dprintf(log_fd, "Server socket init error\n");
    resource_liberator(); 
    return 1;
  }
  
  /* socket reusable  */
  reuse = 1;
  setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  
  /* bind socket to */
  if(bind(ssock, res->ai_addr, res->ai_addrlen) < 0)
  {
    fprintf(stderr, "Server socket bind error.\n");    
    resource_liberator();
    return 1;
  }

  /********************************************************************************************************/

  /* Bekapcsoljuk a kapcsolodasra valo varakozast. */
  if(listen(ssock, MAX_CONNECTION_NUM) < 0)
  {
    fprintf(stderr, "Listen error.\n");
    dprintf(log_fd, "Listen error.\n");
    return 1;
  }

  /* free a getadrrinfo() */
  freeaddrinfo(res);

  /* address length */
  addrlen = sizeof(addr);

  //err = chroot(".");
  //if(err == -1)
  //{
  //  fprintf(stderr, "Error: chroot(), %d", errno);
  //
  //}

  while(1)
  {
  	if(active_connection < MAX_CONNECTION_NUM)
  	{

      /* accept incoming connection */
	  	if((csock = accept(ssock, (struct sockaddr*)&addr, &addrlen)) >= 0)
	  	{
	  		active_connection++;
	  		/* próbáljuk meg kideríteni a kapcsolódó nevét */
	  	  	if(getnameinfo((struct sockaddr*)&addr, addrlen, 
	  	    	ips, sizeof(ips), servs, sizeof(servs), 0) == 0)
	  	  	{
	  	  		t = time(NULL);
  				  tm = *localtime(&t);
	  	  		dprintf(log_fd, "Client connected: %s:%s\tat %d-%d-%d %d:%d:%d\n",ips, servs,
	  	  		 				tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, 
	  	  		 				tm.tm_min, tm.tm_sec);
	  	  		#ifdef DEBUG
	  	    	printf("Connected: %s:%s\n", ips, servs);
	  	    	#endif
	  	  	}
	  	  	else
	  	  	{
	  	  		t = time(NULL);
  				  tm = *localtime(&t);
	  	  		dprintf(log_fd, "Client connected: unknown client\tat %d-%d-%d %d:%d:%d\n",
	  	  		 	  		 	tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, 
	  	  		 				tm.tm_min, tm.tm_sec);
	  	  		#ifdef DEBUG
	  	    	printf("Connected: unknown client\n");
	  	    	#endif
	  	  	}


	  	  	pid = fork();
	  	  	if(pid < 0)
	  	  	{
	  	  		fprintf(stderr, "fork() error.\n");
            dprintf(log_fd, "fork() error.\n");
	  	  		return 1;
	  	  	}
	  	  	/* child process */
	  	  	if(pid == 0)
	  	  	{		
	  	  		sprintf(arg_buf, "%d", csock);
	  	  		execl("tcp_proc", "tcp_proc", "./tcp_proc.c", arg_buf, NULL);
	  	  	}
	  	}
	}
	else
	{

		dprintf(log_fd, "Maximal process number reached at %d-%d-%d %d:%d:%d\n\n", tm.tm_year + 1900, 
	  	  					tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		printf("Nincs szabad process\n");
		/* wait for any child */
		if( waitpid(0, &status, WNOHANG) == 0)
		{
			waitpid(0, &status, 0);
			active_connection--;
		}
		else
		{
			active_connection--;
		}
	}
  }

  dprintf(log_fd, "Session ended successfully\n");
  resource_liberator();
  return 0;
}



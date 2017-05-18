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
#include <stdbool.h>

#define PORT "1122"
#define BUFFSIZE 1024
#define MAX_CONNECTION_NUM	2
#define DEBUG

/* process registry */
struct proc_registry{
  int pid;          /* which working process */
  int csock;        /* which client socket is being processed by it */
  bool active;       /* status of connection */
};

int log_fd;
int ssock;
time_t t;
struct tm tm;
struct proc_registry *proc_list;

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

int proc_list_init(int conn_num)
{
  int i;
  proc_list = (struct proc_registry*) malloc(sizeof(struct proc_registry) * conn_num);
  if(proc_list == NULL)
  {
    return 1;
  }
  for(i=0; i<conn_num; i++)
  {
    proc_list[i].pid = -1;
    proc_list[i].csock = -1;
    proc_list[i].active = false;
  }
  return 0;
}

int proc_list_get_empty(int conn_num)
{
  int i;
  for(i=0; i<conn_num; i++)
  {
    if(proc_list[i].active == false)
    {
      return i;
    }
  }
  return -1;
}

int proc_list_add_proc(int pid, int csock, int conn_num)
{
  int i = proc_list_get_empty(conn_num);
  if(i >= 0)
  {
    proc_list[i].pid = pid;
    proc_list[i].csock = csock;
    proc_list[i].active = true;
    return 0;
  }
  return 1;
}

int proc_list_remove_process(int pid, int conn_num)
{
  int i;
  for(i=0; i<conn_num; i++)
  {
    if(proc_list[i].pid == pid)
    {
      proc_list[i].pid = -1;
      proc_list[i].csock = -1;
      proc_list[i].active = false;
      return 0;
    }
  }
  return 1;
}

int main() 
{
  int max_conn_num = MAX_CONNECTION_NUM;
  struct addrinfo hints;
  struct addrinfo* res;
  int err;
  struct sockaddr_in6 addr;
  socklen_t addrlen;
  char ips[NI_MAXHOST];
  char servs[NI_MAXSERV];
  int csock;
  int reuse;
  char csock_buf[16];
  char uid_buf[16];
  int pid;
  int active_connection = 0;
  int status;
  int incoming_ssock_req;
  struct pollfd pollset;
  char err_msg[] = "Server unavailable.\n";
  int uid = getuid();

  /**************************** logfile init ****************************/
  // TODO O_EXCL
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
  /************************ process list init ****************************/
  if(proc_list_init(max_conn_num))
  {
    fprintf(stderr, "Malloc error");
  }

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

  /* register ssock in pollset */
  pollset.fd = ssock;
  pollset.events = POLLIN;
  
  /* bind socket to */
  if(bind(ssock, res->ai_addr, res->ai_addrlen) < 0)
  {
    fprintf(stderr, "Server socket bind error.\n");    
    resource_liberator();
    return 1;
  }

  /********************************************************************************************************/

  /* Bekapcsoljuk a kapcsolodasra valo varakozast. */
  if(listen(ssock, max_conn_num) < 0)
  {
    fprintf(stderr, "Listen error.\n");
    dprintf(log_fd, "Listen error.\n");
    return 1;
  }

  /* free a getadrrinfo() */
  freeaddrinfo(res);

  /* address length */
  addrlen = sizeof(addr);

  while(1)
  {
    incoming_ssock_req = poll(&pollset, 1, 0);
    if(incoming_ssock_req < 0)
    {
        fprintf(stderr, "Poll error.\n");
        resource_liberator();
        return 1; 
    }
    /* incoming request on ssock */
    if((incoming_ssock_req == 1) && (pollset.revents & (POLLIN)))
    {
    	if(active_connection < max_conn_num)
    	{
        /* accept incoming connection */
  	  	if((csock = accept(ssock, (struct sockaddr*)&addr, &addrlen)) >= 0)
  	  	{
  	  		active_connection++;
  	  		/* get client data */
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
            resource_liberator();
  	  	  	return 1;
  	  	  }
  	  	  /* child process */
  	  	  if(pid == 0)
  	  	  {		
  	  	  	sprintf(csock_buf, "%d", csock);
            sprintf(uid_buf, "%d", uid);
  	  	  	execl("tcp_proc", "tcp_proc", "./tcp_proc.c", csock_buf, uid_buf, NULL);
  	  	  }
          /* register process in registry list */
          if(proc_list_add_proc(pid, csock, max_conn_num) == 1)
          {
            fprintf(stderr, "Failed to add process to list.\n");
            resource_liberator();
            return 1;
          }
  	  	}
  	  }
      else
  	  {
    		dprintf(log_fd, "Maximal process number reached at %d-%d-%d %d:%d:%d\n\n", tm.tm_year + 1900, 
    	  	  					tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    		printf("Maximal process number reached\n");
        if((csock = accept(ssock, (struct sockaddr*)&addr, &addrlen)) >= 0)
        {
          /* TODO send discard message */
          write(csock, err_msg, sizeof(err_msg));
          close(csock);
        }
  	  }
    }




    /* wait for any children */
    if((pid = waitpid(0, &status, WNOHANG)) > 0)
    {
      if(proc_list_remove_process(pid, max_conn_num) == 1)
      {
        fprintf(stderr, "Failed to remove process.\n");
        resource_liberator();
        return 1;
      }
      active_connection--;
    }
  }

  dprintf(log_fd, "Session ended successfully\n");
  resource_liberator();
  return 0;
}



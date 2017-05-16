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
#include <time.h>

#define PORT "1122"
#define BUFFSIZE 1024
#define MAX_CONNECTION_NUM	2
#define DEBUG
int main() 
{
  struct addrinfo hints;
  struct addrinfo* res;
  int err;
  struct sockaddr_in6 addr;
  socklen_t addrlen;
  char ips[NI_MAXHOST];
  char servs[NI_MAXSERV];
  int ssock, csock;
  int reuse;
  char arg_buf[16];
  int pid;
  int active_connection = 0;
  int status;
  int log_fd;
  time_t t;
  struct tm tm;

  log_fd = open("./logfile.log", O_APPEND|O_CREAT|O_WRONLY, 0644);
  if(log_fd < 0)
  {
  	perror("Log file open");
  	return 1;
  }

  t = time(NULL);
  tm = *localtime(&t);
  #ifdef DEBUG
  printf("Server started: Date: %d-%d-%d %d:%d:%d\n\n", tm.tm_year + 1900, 
	  	  tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  #endif
  dprintf(log_fd, "Server started: Date: %d-%d-%d %d:%d:%d\n\n", tm.tm_year + 1900, 
	  	  tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

  /*********************************************socket init *************************************/  
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;

  err = getaddrinfo(NULL, PORT, &hints, &res);
  if(err != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    return -1;
  }
  if(res == NULL)
  {
    return -1;
  }
  
  /* Létrehozzuk a server socketet getaddrinfo() válasza alapján */
  ssock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if(ssock < 0)
  {
    perror("socket"); 
    return 1;
  }
  
  /* Engedélyezzük a REUSE-t (SO_REUSEADDR). Socket level (SOL_SOCKET),  */
  reuse = 1;
  setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  
  /* Címhez kötjük a server socketet getaddrinfo() válasza alapján */
  if(bind(ssock, res->ai_addr, res->ai_addrlen) < 0)
  {
    perror("bind");    
    return 1;
  }

  /********************************************************************************************************/

  /* Bekapcsoljuk a kapcsolodasra valo varakozast. */
  if(listen(ssock, MAX_CONNECTION_NUM) < 0)
  {
    perror("listen");
    return 1;
  }

  /* Felszabadítjuk a getadrrinfo() által generált láncolt listát. A továbbiakban nem lesz rá szükségünk */
  freeaddrinfo(res);

  /* Cím hosszának beállítása sizeof()-fal */
  addrlen = sizeof(addr);

  /* Fogadjuk a kapcsolodasokat. */
  while(1)
  {
  	if(active_connection < MAX_CONNECTION_NUM)
  	{
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
	  	  		perror("fork() error\n");
	  	  		exit(-1);
	  	  	}
	  	  	/* child process */
	  	  	if(pid == 0)
	  	  	{		
	  	  		sprintf(arg_buf, "%d", csock);
	  	  		//args = {"tcp_proc", "./tcp_proc.c", arg_buf, NULL};
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

  /* lezárjuk a szerver socketet */
  close(log_fd);
  close(ssock);

  return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>

#define BUFFSIZE 1024

int main(int argc, char **argv)
{
	int csock = atoi(argv[2]);
	int uid = atoi(argv[3]);
	struct pollfd pollset[2];
  	int numpoll = 2;
  	int len, err;
  	char buf[BUFFSIZE];

	printf("Process létrehozva a %d. számú socketre\n", csock);
	pollset[0].fd = csock;
    pollset[0].events = POLLIN;
    pollset[1].fd = STDIN_FILENO;
    pollset[1].events = POLLIN;

    if(chroot("./htdocs") != 0)
    {
    	fprintf(stderr, "Error in chroot().\n");
    	return 1;
    }
    if(setuid(uid) != 0)
    {
    	fprintf(stderr, "Error in setuid()\n");
    	return 1;
    }
   	printf("%d\n", getuid());

    while(1)
  	{	
  		if( poll(pollset, numpoll, -1) < 0 )
   	 	{
   	   		perror("poll");
   	   		return -1;
   	 	}
	
   	 	/* hiba a socket-tel */
   	 	if(pollset[0].revents & (POLLERR|POLLNVAL))
   	 	{
   	   	printf("HIBA A SOCKET-TEL\n");
   	    	/* lezárjuk a szerver socketet */
   	     	close(csock);
   	     	return -1;
   	 	}
   	 	/* adat jött a socketen */
   	 	if(pollset[0].revents & (POLLIN))
   	 	{
   	   		len = read(csock, buf, BUFFSIZE);
   	   		buf[len] = 0;
   	   		if(len < 0)
   	   		{
   	    		printf("Socket olvasási hiba\n");
   	     		/* lezárjuk a szerver socketet */
   	     		close(csock);           
   	     		return -1;
   	   		}	
   	   		if(len == 0)
   	   		{
   	    		printf("Socket lezárult\n");
   	     		/* lezárjuk a szerver socketet */
   	     		close(csock);       
   	     		return 0;
   	   		}
   	   		if(len > 0)
   	   		{
   	    		printf("%s\n", buf);
   	   		}
   	 	}
   	 	/* adat a standard bemeneten */
   	 	if(pollset[1].revents & (POLLIN))
   	 	{
   	   		len = read(STDIN_FILENO, buf, BUFFSIZE);
   	   		buf[len] = 0;
   	   		if(len < 0)
   	   		{
   	     		printf("Standard bemeneti hiba\n");
   	     		/* lezárjuk a kliens socketet */
   	     		close(csock);        
   	     		return 1;
   	   		}
   	   		if(len>0)
   	   		{
   	     		err = write(csock, buf, len);
   	     		if(err < 0)
   	     		{
   	       			printf("Hiba socket írásakor\n");
   	       			/* lezárjuk a szerver socketet */
   	       			close(csock);
   	       			return 1;
   	     		}
   	   		}
     	}
	}

	return 0;
}
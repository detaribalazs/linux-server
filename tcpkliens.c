/* tcpkliens.c
 * 
 * Egyszerű TCP kliens példa. A program kapcsolódik a paraméterként
 * kapott szerverhez és elküldi a bemenetén kapott szöveget.
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>

#define PORT "1122"
#define BUFFSIZE 1024

int main(int argc, char* argv[])
{
  struct addrinfo hints;
  struct addrinfo* res;
  int err;
  int i;
  int csock;
  char buf[BUFFSIZE];
  int len;
  struct pollfd pollset[2];
  int numpoll = 2;
  
  /***************************************** socket init **********************************/
  if(argc != 2)
  {
    printf("Használat: %s <szerver>\n", argv[0]);
    return 1;
  }

  /* kitöltjük a hints struktúrát */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  /* végezzük el a névfeloldást */
  err = getaddrinfo(argv[1], PORT, &hints, &res);
  if(err != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    return -1;
  }
  if(res == NULL)
  {
    return -1;
  }
  
  /* létrehozzuk a kliens socketet */
  csock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if(csock < 0)
  {
    perror("socket");
    return -1;
  }
    /*************************************************************************************/
  pollset[0].fd = csock;
  pollset[0].events = POLLIN;
  pollset[1].fd = STDIN_FILENO;
  pollset[1].events = POLLIN;


  /* Kapcsolodunk a szerverhez. */
  if(connect(csock, res->ai_addr, res->ai_addrlen) < 0)
  {
    perror("connect");
    return -1;
  }

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
        /* szabadítsuk fel a láncolt listát */
        freeaddrinfo(res);
        return 1;
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
        /* szabadítsuk fel a láncolt listát */
        freeaddrinfo(res);
        return 1;
      }
      if(len == 0)
      {
        printf("Socket lezárult\n");
        /* lezárjuk a szerver socketet */
        close(csock);
        /* szabadítsuk fel a láncolt listát */
        freeaddrinfo(res);
        return 1;
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
        /* lezárjuk a szerver socketet */
        close(csock);
        /* szabadítsuk fel a láncolt listát */
        freeaddrinfo(res);
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
          /* szabadítsuk fel a láncolt listát */
          freeaddrinfo(res);
          return 1;
        }
      }
    }
  }



  //  for(i=0; i<numpoll; i++)
  //  {
  //    if(pollset[i].revents & (POLLERR|POLLNVAL))
  //    {
  //      printf("Hiba a muveletben a %d. leiroval", i);
  //      return 1;
  //    }
  //    if(pollset[i].revents & (POLLIN))
  //    {
  //      len = read(pollset[i].fd, buf, BUFFSIZE);
  //      if(len < 0)
  //      {
  //        perror("read");
  //        return 1;
  //      }
  //      err = write(pollset[(i+1)%2].fd, buf, len);
  //      if(err < 0)
  //      {
  //        perror("write");
  //        return 1;
  //      }
  //    }
  //  }
  //}

  return 0;
}

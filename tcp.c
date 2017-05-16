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

#define PORT "1122"

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
  char buf[256];
  int len;
  int reuse;

  char resp_buf[] = "HTTP/1.1 200 OK\n \
Host: localhost:1122\n \
Connection: keep-alive\n \
Cache-Control: max-age=0\n \
Upgrade-Insecure-Requests: 1\n \
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36\n \
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\n \
Accept-Encoding: gzip, deflate, sdch, br\n \
Accept-Language: hu,en-US;q=0.8,en;q=0.6,de;q=0.4\n \
\n \  
<html><body><h1>It works!</h1></body></html>\0";
  
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

  /* Bekapcsoljuk a kapcsolodasra valo varakozast. */
  if(listen(ssock, 5) < 0)
  {
    perror("listen");
    return 1;
  }

  /* Felszabadítjuk a getadrrinfo() által generált láncolt listát. A továbbiakban nem lesz rá szükségünk */
  freeaddrinfo(res);

  /* Cím hosszának beállítása sizeof()-fal */
  addrlen = sizeof(addr);
  /* Fogadjuk a kapcsolodasokat. */
  while((csock = accept(ssock, (struct sockaddr*)&addr, &addrlen)) >= 0)
  {
    /* próbáljuk meg kideríteni a kapcsolódó nevét */
    if(getnameinfo((struct sockaddr*)&addr, addrlen, 
      ips, sizeof(ips), servs, sizeof(servs), 0) == 0)
    {
      printf("Kapcsolódás: %s:%s\n", ips, servs);
    }
    write(ssock, resp_buf, sizeof(resp_buf));
    /* fogadjuk a beérkező csomagokat, és kiírjuk a tartalmát a képernyőre */
    while((len = read(csock, buf, sizeof(buf))) > 0)
    {
      write(STDOUT_FILENO, buf, len);
      if((write(ssock, resp_buf, sizeof(resp_buf)) > 0))
      {
        //write(STDOUT_FILENO, resp_buf, sizeof(resp_buf));
      }      
    }
    
    printf("Kapcsolat zárása.\n");
    /* lezárjuk a kliens socketet */
    close(csock);
  }

  /* lezárjuk a szerver socketet */
  close(ssock);

  return 0;
}



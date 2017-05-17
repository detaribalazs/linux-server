#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define MAX_LINE_LENGTH 	256
#define MAX_METHOD_LENGTH 	7
#define METHODS {"OPTIONS", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "CONNECT"}
#define METHOD_NUM			8
#define MAX_URI_LENGTH		64
#define PROTOCOLS {"HTTP/1.0", "HTTP/1.1"}
#define PROTOCOL_NUM		2
#define MAX_PROTOCOL_LENGTH 8
#define STATUS 	{"200 OK", "404 Not Found", "501 Not Implemented", "503 Service Unavailable"}
#define STATUS_NUM 			4

/* free pointers : line list, file buffer */

int get_line_num(const char *request)
{
	int i=0;
	char c;
	int request_length = strlen(request);
	int line_num = 1;

	while(i < request_length)
	{
		c = request[i];
		if(c == '\n')
		{
			line_num++;
		}
		i++;
	}
	return line_num;
}

void get_lines(const char *request, char ** line_list)
{
	int i=0;			/* actual line number */
	int j=0;			/* actual position in request */
	int k=0;			/* position in actual line */

	int line_num = get_line_num(request);

	while(i < line_num)
	{
		k = 0;
		while(request[j] != '\n' && request[j] != '\0')
		{
			line_list[i][k] = request[j];
			j++;
			k++;
		}
		line_list[i][k] = '\0';
		j++;
		i++;
	}
}

int get_method(char *request_line)
{
	char method[MAX_METHOD_LENGTH];
	char* methods[] = METHODS;
	int i = 0;
	int line_length = strlen(request_line);
	while((request_line[i] != ' ') && (i <line_length))
	{
		method[i] = request_line[i];
		i++;
	}
	method[i] = '\0';
	for(i=0; i<METHOD_NUM; i++)
	{
		if(strcmp(method, methods[i]) == 0)
		{
			return i;
		}
	}
	return -1;
}

int get_uri(char * request_line, char * uri)
{
	int i=0, j=0;
	char c = 0;
	while(c != ' ')
	{
		c = request_line[i];
		i++;
	}
	while(j<MAX_URI_LENGTH && request_line[i] != ' ')
	{
		uri[j] = request_line[i];
		j++;
		i++;
	}
	if((j == MAX_URI_LENGTH) && (request_line[i] != ' ') )
	{
		return 1;
	}
	uri[j] = '\0';
	return 0;
}

int get_protocol(char * request_line)
{
	char protocol[MAX_PROTOCOL_LENGTH];
	char* protocols[] = PROTOCOLS;
	int length = strlen(request_line);
	int i=0, j=0;
	char c = 0;
	while( (c != ' ') && (i < length) )
	{
		c = request_line[i];
		i++;
	}
	c = 0;
	while( (c != ' ') && (i < length) )
	{
		c = request_line[i];
		i++;
	}
	while((request_line[i] != '\0') && (i < length) )
	{
		protocol[j] = request_line[i];
		i++;
		j++;
	}
	protocol[j] ='\0';
	for(i=0; i<METHOD_NUM; i++)
	{
		if(strcmp(protocol, protocols[i]) == 0)
		{
			return i;
		}
	}
	return -1;
}

int create_status_line(int status, int protocol, int char* status_line)
{
	char protocols[] = PROTOCOLS;
	char status[] = "200 OK";
	status_line = (char*) malloc(sizeof(char) * (strlen(protocols[protocol]) + 
												 ))
}

int create_resp_header(char* header_buffer, int status, )
{
	time_t t;
	struct tm tm;
	char server[] = "Server: dbalazs server v1.0";

	t = time(NULL);
  	tm = *localtime(&t);

}

/* return temporal file descriptor */
int create_response(int method, char *uri, char *)
{
	struct stat req_inode;
	int req_fd;
	int req_file_size;
	int tmp_fd;
	char uri_buf[MAX_URI_LENGTH];
	time_t mtime;
	time_t curr_time;

	switch method{
		{"OPTIONS", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "CONNECT"}
		/* options */
		case 0:
		/* it's a php file */
		if(strstr(uri, ".php") != NULL)
		{
			sprintf(uri_buf, "./%s", uri);
			req_fd = open(uri_buf, O_RDONLY);
			if(req_fd == -1)
		}
		/* it's a html file */
		if(strstr(uri, ".html") != NULL)
		{
			sprintf(uri_buf, "./%s", uri);
			/* open requested file */
			req_fd = open(uri_buf, O_RDONLY);
			if(req_fd == -1)
			{
				fprintf(stderr, "Error in open file %s", uri_buf);
				return 1;
			}

			/* open requested file inode */
			if(lstat(uri_buf, &req_inode) < 0)
			{
				fprintf(stderr, "Error opening indode.\n");
				return 1;
			}
			mtime = req_inode.mtime;


			tmp_fd = open("./", O_TMPFILE|O_RDWR);
			if(req_fd == -1)
			{
				fprintf(stderr, "Error in open temporal file.\n");
				return 1;
			}
			create_resp_header()
		}


		break;
		/* get */
		case 1:


		break;
		/* head */
		case 2:

		break;
		/* post */
		case 3:

		break;
		/* put */
		case 4:

		break;
		/* delete */
		case 5:

		break;
		/* trace */
		case 6:

		break;
		/* connect */
		case 7:

		break;
		/* default */
		default:
			fprintf(stderr, "Unknown method in create response.\n");
			return 1;
		break;
	};


}



int main(void)
{
	int i, err;

	char **line_list;
	int line_num;

	char *methods[] = METHODS;
	int method;

	char uri[MAX_URI_LENGTH];

	char *protocols[] = PROTOCOLS;
	int protocol;

	const char request[] = "GET /index.html HTTP/1.1\n\
Host: localhost:1122\n\
Connection: keep-alive\n\
Upgrade-Insecure-Requests: 1\n\
User-Agent: Mozilla/5.0 (X11; Linux x86_64) Chrome/58.0.3029.110 Safari/537.36\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\n\
Accept-Encoding: gzip, deflate, sdch, br\n\
Accept-Language: hu,en-US;q=0.8,en;q=0.6,de;q=0.4\n\
\n\
bookId=12345&author=Karl+Marx";
	
	/*********************** init memory for header **************************/
	line_num = get_line_num(request);
	line_list =(char **)malloc(sizeof(char*) * line_num);
	for(i=0; i<line_num; i++)
	{
		line_list[i] = (char *)malloc(sizeof(char) * MAX_LINE_LENGTH);
	}

	if(line_list == NULL)
	{
		fprintf(stderr, "Error in malloc().\n");
		return 1;
	}
	/***************************************************************************/

	printf("%s\n", request);
	printf("Line number: %d\n", line_num);
	printf("\n\n\n");

	get_lines(request, line_list);
	for(i=0; i<line_num; i++)
	{
		printf("%s\t%d\n", line_list[i], (int)strlen(line_list[i]));
		printf("-----------------------------------------------------------\n");
	}
	method = get_method(line_list[0]);
	if( (method) < 0)
	{
		fprintf(stderr, "Unknown method\n");
		/* TODO send unimplemented method response */
		return 1;
	}
	err = get_uri(line_list[0], uri);
	if(err)
	{
		fprintf(stderr, "Unknown method\n");
		/* TODO send URI too long response */
		return 1;
	}
	printf("\n\n%s\n", uri);
	protocol = get_protocol(line_list[0]);
	if(protocol < 0)
	{
		fprintf(stderr, "Unknown protocol\n");
		/* TODO send URI too long response */
		return 1;
	}
	printf("%s\n", protocols[protocol]);

	return 0;
}
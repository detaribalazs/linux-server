#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/sendfile.h>

#define MAX_LINE_LENGTH 	256
#define MAX_METHOD_LENGTH 	7
#define METHODS {"OPTIONS", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "CONNECT"}
#define METHOD_NUM			8
#define MAX_URI_LENGTH		64
#define PROTOCOLS {"HTTP/1.0", "HTTP/1.1"}
#define PROTOCOL_NUM		2
#define MAX_PROTOCOL_LENGTH 8
#define STATUSES 	{"200 OK", "404 Not Found", "501 Not Implemented", "503 Service Unavailable", "500 Internal Server Error", "400 Bad Request"}
#define STATUS_NUM 			6
#define MAX_CON_NUM			3
#define SERVER_NAME			"Server: dbalazs server v1.0\r\n"

/* free pointers : line list, file buffer, status_line, resp_header_buffer */

// TODO implement general error handler */
void send_server_err_msg(void)
{
	char buf[] = "HTTP/1.0 500 Internal Server Error\r\n";
}

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
		while(request[j] != '\n' && request[j] != '\r' && request[j] != '\0')
		{
			line_list[i][k] = request[j];
			j++;
			k++;
		}
		j++;
		if(request[j] == '\r' || request[j] == '\n')
		{
			j++;
		}
		line_list[i][k] = '\0';
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

int create_status_line(int status, int protocol, char *status_line)
{
	char *protocols[] = PROTOCOLS;
	char *statuses[] = STATUSES;
	
	if(status_line == NULL)
	{
		fprintf(stderr, "Error in malloc() in create_status_line().\n");
		return 1;
	}
	if( (sprintf(status_line, "%s %s\r\n", protocols[protocol], statuses[status]) < 0) )
	{
		fprintf(stderr, "Error in creating status line.");
		return 1;
	}
	return 0;
}

int calc_resp_header_size(int status, int protocol, char *content, int content_len)
{
	char *protocols[] = PROTOCOLS;
	char *statuses[] = STATUSES;
	int status_line = strlen(protocols[protocol]) + strlen(statuses[status]) + 4;
	int date = 37;
	int server = strlen(SERVER_NAME);
	char tmp[50];
	int content_length;
	int content_type = strlen(content) + 16;
	int empty_line = 2;

	sprintf(tmp, "%d", content_len);
	content_length = 18 + strlen(tmp);
	return (status_line + date + server + content_length + content_type + empty_line);
}

int create_resp_header(int status, int protocol, char *content, 
					   int content_len, int fd)
{
	char *protocols[] = PROTOCOLS;
	char *statuses[] = STATUSES;
	int err;
	time_t t;
	struct tm tm;
	char server[] = SERVER_NAME;
	char date[70];
	char empty_line[2] ="\n";
	char content_length[40]; 
	char *content_type;
	char *status_line;

	/* status line */
	status_line = (char*) malloc(sizeof(char) * (strlen(protocols[protocol]) + 
												 strlen(statuses[status]) +
												  4) );
	err = create_status_line(status, protocol, status_line);
	if(err != 0)
	{
		return 1;
	}
	//printf("%s", status_line);
	/* content length */
	sprintf(content_length, "Content-Length: %d\r\n", content_len);

	/* content type */
	content_type = (char*)malloc(sizeof(char) * ( strlen(content) + 16) );	// its only 16 but no time for errors
	if(content_type == NULL)
	{
		fprintf(stderr, "Error in malloc() in create_response_header() content type.\n");
		return 1;
	}
	sprintf(content_type, "Content-Type: %s\r\n", content);
	printf("%s\n", content_type);

	/* date */
	t = time(NULL);
  	tm = *gmtime(&t);

  	strftime(date, 70, "Date: %a, %d %b %Y %T GMT\r\n", &tm);

	/* create response header */  	
	dprintf(fd, "%s%s%s%s%s%s", status_line, date, server, content_length,
												content_type, empty_line);
	free(content_type);
  	free(status_line);
	return 0;
}

/* return temporal file descriptor */
int create_response(const char *request)
{
	//{"200 OK", "404 Not Found", "501 Not Implemented", "503 Service Unavailable", "500 Internal Server Error", "400 Bad Request"}
	char *protocols[] = PROTOCOLS;
	int protocol;

	char *statuses[] = STATUSES;
	int status = 0;

	char *methods[] = METHODS;
	int method;

	char **line_list;
	int line_num;

	char uri[MAX_URI_LENGTH];


	struct stat req_inode;
	int req_fd;
	int req_file_size;
	int tmp_fd;
	bool tmp_flag = false;
	char uri_buf[MAX_URI_LENGTH];
	time_t mtime;
	time_t curr_time;
	char tmp_file_name[50];
	int req_size, err, i;

	/******************************* init line list *****************************/
	line_num = get_line_num(request);
	line_list =(char **)malloc(sizeof(char*) * line_num);
	for(i=0; i<line_num; i++)
	{
		line_list[i] = (char *)malloc(sizeof(char) * MAX_LINE_LENGTH);
	}

	if(line_list == NULL)
	{
		fprintf(stderr, "Error in malloc().\n");
		/* internal server error */
		status = 4;
	}
	get_lines(request, line_list);
	/******************************************************************************/
	/******************************* get paramters ********************************/
	method = get_method(line_list[0]);
	if( (method) < 0)
	{
		fprintf(stderr, "Unknown method\n");
		/* bad request */
		status = 5;			
	}
	err = get_uri(line_list[0], uri);
	if(err)
	{
		fprintf(stderr, "Unknown resource\n");
		/* bad request */
		status = 5;
		/* TODO send URI too long response */
	}
	
	protocol = get_protocol(line_list[0]);
	if(protocol < 0)
	{
		fprintf(stderr, "Unknown protocol\n");
		/* bad request */
		status = 5;
	}
	free(line_list);
	/******************************* open files ***************************************/
	//TODO find out, why O_TMPFILE doesnt work...
	while(!tmp_flag)
	{
		srand(time(NULL));
		sprintf(tmp_file_name, "./tmp%d", (int)(rand()*MAX_CON_NUM));
		tmp_fd = open(tmp_file_name, O_RDONLY);
		if(tmp_fd < 0)
		{
			tmp_fd = open(tmp_file_name, O_CREAT|O_RDWR|O_TRUNC, 0600);
			if(tmp_fd > 0)
			{
				tmp_flag = true;
			}
		}
		else
		{
			close(tmp_fd);
		}
	}

	if(status != 0)
	{
		if(status == 2)
		{
			//http_send_unimplemented(fp)
		}
	}

	switch(method){
		/* options */
		case 0:
		
		break;


		/* get */
		case 1:
			sprintf(uri_buf, ".%s", uri);


			/* it's a php file */
			if(strstr(uri, ".php") != NULL)
			{
				req_fd = open(uri_buf, O_RDONLY);
			}

			/* it's a html file */
			if(strstr(uri, ".html") != NULL)
			{
				/* open requested file */
				req_fd = open(uri_buf, O_RDONLY);
				if(req_fd == -1)
				{
					fprintf(stderr, "Error in open file %s", uri_buf);
					status = 1;
				}

				req_size = lseek(req_fd, 0, SEEK_END);
				lseek(req_fd, 0, SEEK_SET);
				
				create_resp_header(status, protocol, "text/html", req_size, tmp_fd);
				sendfile(tmp_fd, req_fd, NULL, req_size);
				printf("%s\n", strerror(errno));
				return tmp_fd;
			}
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
	int fd;
	const char request[] = "GET /index.html HTTP/1.1\r\n\
Host: localhost:1122\r\n\
Connection: keep-alive\r\n\
Upgrade-Insecure-Requests: 1\r\n\
User-Agent: Mozilla/5.0 (X11; Linux x86_64) Chrome/58.0.3029.110 Safari/537.36\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n\
Accept-Encoding: gzip, deflate, sdch, br\r\n\
Accept-Language: hu,en-US;q=0.8,en;q=0.6,de;q=0.4\r\n\
\r\n\
bookId=12345&author=Karl+Marx";
	
	fd = create_response(request);
	//close(fd);
	return 0;
}
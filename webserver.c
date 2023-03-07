#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>

#define ISspace(x) isspace((int)(x))
#define SERVER_STRING "Server: SongHao's http/0.1.0\r\n"

void error_handing(const char* str);
int startup(unsigned short int* port);
void* accept_request(void* from_client);
int get_line(int sock, char*buf, int size);
void unimplent(int client);
void not_found(int client);
void bad_request(int client);
void cannot_execute(int client);
void server_file(int client, char*path);
void execute_cgi( int client, const char*path, const char*method, const char* queue_string);//动态文件
void headers(int client, const char* type);
void cat(int client,FILE* resource);
const char* getHeadType(const char* filename);



void error_handing(const char* str) {
	perror(str);
	exit(1);
}
void not_found(int client) {
	//网页不存在
	//发送响应包的头信息,发送404
	char buf[1024];
	sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
	send(client, buf, strlen(buf),0);
	sprintf(buf, "Server: RockHttpd/0.1\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "your request because the resource specified\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "is unavailable or nonexistent.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);

}
void unimplent(int client) {
	// 向指定的套接字，发送一个提示还没实现的错误页面
	char buf[1024];
	//发送501说明相应方法没有实现
	sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</TITLE></HEAD>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}
void bad_request(int client)
{
	char buf[1024];
	//发送400
	sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "<P>Your browser sent a bad request, ");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "such as a POST without a Content-Length.\r\n");
	send(client, buf, sizeof(buf), 0);
}
void cannot_execute(int client)
{
	char buf[1024];
	//发送500
	sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
	send(client, buf, strlen(buf), 0);
}
const char* getHeadType(const char* filename) {//将文件类型解析出来
	const char* ret = "text/html";
	const char* p = strrchr(filename, '.');
	if (!p) {
		return ret;
	}
	if (!strcmp(p, "css")) ret = "text/css";
	else if (!strcmp(p, "jpg")) ret = "image/jpeg";
	else if (!strcmp(p, "png")) ret = "image/png";
	else if (!strcmp(p, "js")) ret = "application/x-javascript";
	return ret;

}

void headers(int client, const char* type) {//发送响应包的头信息
	char buf[1024];

	strcpy(buf, "HTTP/1.0 200 OK\r\n");//拷贝,状态行
	send(client, buf, strlen(buf), 0);

	strcpy(buf, "Server: RockHttpd/0.1\r\n");//服务器端的软件名称和它的版本号
	send(client, buf, strlen(buf), 0);

	char buf2[1024];
	sprintf(buf2, "Content-Type: %s\r\n", type);//返回消息的内容类型,
	send(client, buf2, strlen(buf2), 0);

	strcpy(buf, "\r\n");//空行
	send(client, buf, strlen(buf), 0);
}
void cat(int client, FILE* resource) {
	//发送文件内容
	// 
	//char buf[4096];//一次发4096,网络数据包4096，有人linux一次发一个，慢
	//int count = 0;
	//while (1) {//不停发送
	//	int ret = fread(buf, sizeof(char), sizeof(buf), resource);//一次读一个字节，读4096次, 从文件中读取若干字节数据到内存缓冲区中 ;
	////ret返回字节数
	//	if (ret <= 0) {
	//		break;
	//	}
	//	send(client, buf, ret, 0);
	//	count += ret;
	//}
	//printf("一共发送[%d]字节给浏览器\n", count);

	char buf[1024];
	fgets(buf, sizeof(buf), resource);
	while (!feof(resource))//检测流上的文件结束符的函数，如果文件结束，则返回非0值，否则返回0
	{

		send(client, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), resource);
	}


}
//如果不是CGI文件，也就是静态文件，直接读取文件返回给请求的http客户端即可
void server_file(int client,char* filename) {
	int numchar = 1;
	char buf[1024];
	while (numchar > 0 && strcmp(buf, "\n")) {
		numchar = get_line(client, buf, sizeof(buf));
		printf("剩余报文：%s\n", buf);
	}
	FILE* resourse = NULL;
	resourse = fopen(filename, "r");
	if (resourse == NULL) {
		not_found(client);
	}
	else {
		//正式发送资源给浏览器
		headers(client, getHeadType(filename));

		//发送请求的资源信息
		cat(client, resourse);
		printf("资源发送完毕\n");

		
	}
	fclose(resourse);
}
//执行cgi动态解析
void execute_cgi(int client, const char* path, const char* method, const char* query_string) {
	char buf[1024];
	int cgi_output[2];//创建管道，分别用于存储子进程的输出和输入。
	int cgi_input[2];

	pid_t pid; // 用于存储 fork 函数的返回值（子进程 ID）
	int status;

	int i;
	char c;

	int numchars = 1;
	int content_length = -1; // 用于存储请求体的长度，默认为 -1，表示还没有读取到 Content-Length 字段

	buf[0] = 'A';
	buf[1] = '0';
	if (strcasecmp(method, "GET") == 0) {
		while ((numchars > 0) && strcmp("\n", buf)) {//报文最后一行是\n
			numchars = get_line(client, buf, sizeof(buf));
		}
	}
	else {
		numchars = get_line(client, buf, sizeof(buf));
		while ((numchars > 0) && strcmp("\n", buf)) {
			buf[15] = '\0';
			if (strcasecmp(buf, "Content-Length:") == 0) {
				content_length = atoi(&(buf[16]));
			}
			numchars = get_line(client, buf, sizeof(buf));
		}
		// 如果 Content-Length 字段不存在，则认为请求不合法
		if (content_length == -1) {
			bad_request(client);
			return;
		}
	}
	// 发送状态行
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	send(client, buf, strlen(buf), 0);
	// 创建用于向 CGI 脚本写入数据和从 CGI 脚本读取数据的管道
	if (pipe(cgi_output) < 0) {//创建管道，将子进程的输出存储在cgi_output数组中，
		cannot_execute(client);
		return;
	}
	if (pipe(cgi_input) < 0) {
		cannot_execute(client);
		return;
	}
	// 创建子进程运行 CGI 脚本
	if ((pid = fork()) < 0) {
		cannot_execute(client);
		return;
	}
	if (pid == 0) {//子进程，想父进程传输数据
		char meth_env[255];
		char query_env[255];
		char length_env[255];

		// 关闭标准输入和标准输出,
		//dup2(1, 5);    //将“5”重定向到标准输出
		dup2(cgi_output[1], 1);//将 newfd 文件描述符重定向到 oldfd 所描述的文件
		dup2(cgi_input[0], 0);

		close(cgi_output[0]);//关闭了cgi_output中的读通道
		close(cgi_input[1]);//关闭了cgi_input中的写通道

		sprintf(meth_env, "REQUEST_METHOD=%s", method);//method==GET。。。环境变量
		putenv(meth_env);//用来改变或增加环境变量的内容.

		if (strcasecmp(method, "GET") == 0) {
			//存储QUERY_STRING
			sprintf(query_env, "QUERY_STRING=%s", query_string);
			putenv(query_env);
		}
		else {   /* POST */
			//存储CONTENT_LENGTH
			sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
			putenv(length_env);
		}
		execl(path, path, NULL);//执行脚本,execl() 是一个系统调用函数，它用于在当前进程中执行一个可执行文件
		//在函数调用中，我们传递了 path 作为第一个参数，并将其再次传递给 execl() 的第二个参数，表示要执行
	  //的可执行文件的名称。第三个参数为 NULL，表示可执行文件不接受任何其他参数。
		exit(0);
	}
	else {//如果当前进程是父进程
		close(cgi_output[1]);
		close(cgi_input[0]);
		 
		if (strcasecmp(method, "POST") == 0) {
			for (i = 0;i < content_length;i++) {
				recv(client, &c, 1, 0);//从客户端读取数据
				write(cgi_input[1], &c, 1);//写入 cgi_input[1] 
			}
		}
		//读取cgi脚本返回数据
		while (read(cgi_output[0], &c, 1) > 0)//父进程从 cgi_output[0] 读取 CGI 脚本返回的数据，并将其发送回客户端
			//发送给浏览器
		{
			send(client, &c, 1, 0);
		}

		//运行结束关闭
		close(cgi_output[0]);
		close(cgi_input[1]);


		waitpid(pid, &status, 0);//销毁僵尸进程


	}
}
void* accept_request(void* from_client) {
	int len = 0;
	char buf[1024];
	int client = *(int*)from_client;
	//实现请求行
	len = get_line(client, buf, sizeof(buf));
	printf("读到请求行： %s\n", buf);

	int i = 0;
	int j= 0;
	char method[255];
	while (!isspace(buf[i]) && i < sizeof(method) - 1) {//扫描buf,不是空白字符读取,目标读取get,post，也不越界
		method[i++] = buf[j++];
	}
	method[i] = '\0';
	printf("读到方法： %s\n", method);//解析后，method的值："GET"
	//检查请求方法，本服务器是否支持
	//if (strcmp(method, "GET") && strcmp(method, "POST")) {//只支持window

	//}
	if (strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
		unimplent(client);
		return NULL;
	}
	int cgi = 0;
	if (strcasecmp(method, "POST") == 0)  cgi = 1;

	char url[255];//存放请求的资源完整路径
	i = 0;
	while (isspace(buf[j]) && j < sizeof(method)) {//如果是空格,跳过去
		j++;
	}

	while (!isspace(buf[j]) && j < sizeof(buf) && i < sizeof(url) - 1) {
		url[i++] = buf[j++];

	}
	url[i] = '\0';
	printf("读到路径： %s\n", url);
	//定位到服务器本地html文件
	//处理url中？  html？kjdssaj
	
	//一种方法，截断？
	//{
	//	char* pos = strchr(url, '?');//定位到？出现的下标
	//	if (pos) {
	//		*pos = '\0';
	//		printf("read url: %s\n", url);
	//	}

	//}

	// 解析查询字符串
	// 如果浏览器的访问地址是： http://127.0.0.1:8000?name=rock 
	// 那么服务器端第一次收到的报文头就是：  buf = GET /?name=rock HTTP/1.1
	// 通过如果解析，query_string的值就是 "name=rock"
	char* queue_string = NULL;
	if (strcasecmp(method, "GET")) {
		queue_string = url;
		while ((*queue_string != '?') && (*queue_string != '\0')) {
			queue_string++;
		}
		//*如果有 ? 表明是动态请求, 开启cgi* /
		if (*queue_string == '?') {
			cgi = 1;
			*queue_string = '\0';
			queue_string++;
		}
	}
	char path[512];
	sprintf(path, "httpdocs%s", url);//格式化的数据写入字符串path中,./html_docs/url

	if (path[strlen(path) - 1] == '/') {
		strcat(path, "test.html");
	}
	printf("读到完整路径： %s\n", path);

	struct stat status;
	int numchar;
	if (stat(path, &status) == -1) {//读取失败
		while ((numchar > 0) && strcmp("\n", buf)) {
			numchar = get_line(client, buf, sizeof(buf));//把请求包的剩余报文全部读完
		}
		not_found(client);//网页不存在
	}
	else {
		if ((status.st_mode & S_IFMT) == S_IFDIR) {//未操作文件类型是目录
			strcat(path, "index.html");
		}
		if ((status.st_mode & S_IXUSR) ||
			(status.st_mode & S_IXGRP) ||
			(status.st_mode & S_IXOTH))
			//S_IXUSR:文件所有者具可执行权限
			//S_IXGRP:用户组具可执行权限
			//S_IXOTH:其他用户具可读取权限  
			cgi = 1;
	}

	if (!cgi) {
		server_file(client, path);//把静态文件发过去
	}
	else {  // 使用CGI来处理“动态请求”，例如在网页中，用户填写信息后点击提交按钮后，服务器端使用CGI来处理这个请求 
		execute_cgi(client, path, method, queue_string);//动态文件
	}
	close(client);
	//if (from_client) free(from_client);//释放动态分配的内存,不能有
	return NULL;



}

int get_line(int sock, char* buf, int size) {
	int i = 0;
	char c = '\0';
	int n;
	while ((i < size - 1) && (c != '\n')) {
		n = recv(sock, &c, 1,0);//返回读出字节的大小
		if (n > 0) {
			if (c == '\r') {
				n = recv(sock, &c, 1, MSG_PEEK);
				if (n > 0&&c=='\n') {
					recv(sock, &c, 1, 0);
				}
				else {
					c = '\n';//强制结束
				}
		  }
			buf[i++] = c;
		}
		else {
			c == '\n';
		}

	}
	buf[i] = 0;
	return i;
}
int startup(unsigned short int* port) {
	
	int Server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);//IPPROTO_TCP换成0也行
	if (Server_socket == -1) {
		error_handing("socket");
	}
	//绑定套接字
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(*port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//设置端口可复用
	int opt = 1;
	int r = setsockopt(Server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
	if (r == -1) {
		error_handing("setsockopt");
	}
	if (bind(Server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
		error_handing("bind");
	}
	//动态分配端口
	int len=sizeof(server_addr);
		if (*port == 0) {
			if (getsockname(Server_socket, (struct sockaddr*)&server_addr, &len)==-1) {
				error_handing("getsockname");

			}
			*port = ntohs(server_addr.sin_port);
	}
		//创建监听队列
		if (listen(Server_socket, SOMAXCONN) == -1) {
			error_handing("listen");
		}
		return (Server_socket);
}

int main() {
	unsigned short port = 8080;
	int server_socket = startup(&port);
	printf("http server_socket is %d\n", server_socket);
	printf("http running on port %d\n", port);
	while (1) {
		struct sockaddr_in client_name;
		int client_len = sizeof(client_name);
		//阻塞式等待用户通过浏览器访问
		int client_socket = accept(server_socket, (struct sockaddr*)&client_name, &client_len);
		printf("new connect... ip: %s,port: %d\n", inet_ntoa(client_name.sin_addr), ntohs(client_name.sin_port));
          if (client_socket == -1){
			  error_handing("accept");
		  }
				
		//使用client_sock对用户进行访问

		  //创建新的线程，进程包括多个线程,接待多个客户
		//DWORD threadid = 0;//线程id,window
		//CreateThread(NULL, 0, accept_request, (void*)client_socket, 0, &threadid);

		//linux
		pthread_t newthread;
		if (pthread_create(&newthread, NULL, accept_request, (void*)&client_socket)) {
			perror("pthread_create");
		}

	}
	close(server_socket);
	return 0;
}
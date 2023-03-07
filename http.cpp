
#define _CRT_SECURE_NO_WARNINGS 
#include<iostream>
#include<string>
#include<map>
#include<unordered_map>
#include<set>
#include< unordered_set>
#include<stddef.h>
#include<numeric>
#include<stdlib.h>
#include<fstream >
#include<iostream >
#include<algorithm>
#include<string>
#include<stack>
#include<utility>
#include<memory>
#include<typeinfo>
#include<regex>
#include<random>
#include <iomanip>
#include <sstream>
#include<list>
#include<deque>
#include<fstream>
#include<cctype>
#include<cassert>
#include<cstring>
#include<algorithm>
#include<vector>
#include<typeinfo>
#include<forward_list>
#include<functional>
#include<WinSock2.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#pragma comment(lib,"ws2_32.lib")

using namespace std;
#define PRINTF(str) printf("[%s - %d]"#str"=%s\n", __func__, __LINE__, str);//把函数定义成宏


void error_die(const char* str)
{
	perror(str);//打印错误原因,perror(s) 用来将上一个函数发生错误的原因输出到标准设备(stderr)。参数 s 所指的字符串会先打印出
	//，后面再加上错误原因字符串。此错误原因依照全局变量errno的值来决定要输出的字符串。
	exit(1);
}


//实现网络初始化
//返回值：套接字
//port 端口 为0自动分配
int startup(unsigned short int* port) {
	//1、网络通信初始化
	WSADATA data;
	int ret=WSAStartup(MAKEWORD(1, 1), &data);
	if (ret != 0)
	{
		//cout << "打开失败" << endl;;
		error_die("WSAStartup");
	
	}
	if (1 != HIBYTE(data.wVersion) || 1 != LOBYTE(data.wVersion))
	{
		//说明版本不对
		//清理网络库
		//std::cout << "版本号不对" << std::endl;
		WSACleanup();
		return 0;
	}

	int  server_socket= socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);//AF_INET,PF_INET一样
	if (server_socket == -1)
	{
		error_die("套接字");
	}
	/*if (server_socket == INVALID_SOCKET)
	{
		cout << "调用失败" << endl;
		int a = WSAGetLastError();
		WSACleanup();
		return 0;
	}*/



	//绑定套接字
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(*port);//char转成网络类型,short
	//server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");//客户端都可以来,将主机的unsigned long值转换成网络字节顺序（32位）
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//客户端都可以来,将主机的unsigned long值转换成网络字节顺序（32位）

		//设置端口可复用
	int opt = 1;
	int r = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
	if (r == -1) {
		error_die("setsockopt");
	}
	if (::bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		error_die("bind");
	}

	//动态分配端口
	int len = sizeof(server_addr);
	if (*port == 0) {
		if (getsockname(server_socket, (struct sockaddr*)&server_addr, &len) < 0)
		{
			error_die("getsockname");
		}
		//自动分配的端口放进去server_addr，再给port
		*port= ntohs(server_addr.sin_port);
	}
	//创建监听队列
	if (listen(server_socket, SOMAXCONN) == -1)
	{
		error_die("listen");
	}
	return(server_socket);
}

int get_line(int sock, char* buf, int size)//从指定的客户端套接字，读取一行数据，保存到buf
{
	char c = 0;//'\0'
	int i = 0;

	// \r\n,最后一个字符
	while (i < size - 1 && c != '\n') {//有没有越界，回车符
		int n = recv(sock, &c, 1, 0);//char 一个字节
		if (n > 0)
		{
			if (c == '\r')
			{
				n = recv(sock, &c, 1, MSG_PEEK);//MSG_PEEK 偷偷看一眼,看r后面是不是n
				if (n > 0 && c == '\n') {
					recv(sock, &c, 1, 0);//是n，正式读取	
				}
				else {
					c = '\n';//强制结束
				}
			}
			buf[i++] = c;//读到的的数据放到buf
		}
		else {
			c = '\n';
		}
	}
	buf[i] = 0;//'\0',字符串结束符
	return i;//返回多少字节
}
void unimplent(int client)
{
	//向指定的套接字，发送一个提示还没实现的错误页面
}
void headers(int client,const char* type) {
	//发送响应包的头信息
	char buf[1024];
	strcpy(buf, "HTTP/1.0 200 OK\r\n");//拷贝,状态行
	send(client, buf, strlen(buf), 0);

	strcpy(buf, "Server: RockHttpd/0.1\r\n");//服务器端的软件名称和它的版本号
	send(client, buf, strlen(buf), 0);



	//strcpy(buf, "Content-type:text/html\n");//返回消息的内容类型,图片应该是image/jpeg
	//send(client, buf, strlen(buf), 0);

	
	//strcpy(buf, "Content-type:image/jpeg\n");//返回消息的内容类型,图片应该是image/jpeg
	//send(client, buf, strlen(buf), 0);

	//strcpy(buf, "Content-type:text/css\n");//返回消息的内容类型,
	//send(client, buf, strlen(buf), 0);

	//strcpy(buf, "Content-type:image/png\n");//返回消息的内容类型,
	//send(client, buf, strlen(buf), 0);

	//strcpy(buf, "Content-type:application/x-javascript\n");//返回消息的内容类型,
	//send(client, buf, strlen(buf), 0);

	char buf2[1024];
		sprintf(buf2, "Content-Type: %s\r\n",type);//返回消息的内容类型,
	send(client, buf2, strlen(buf2), 0);

	//if (!strcmp(p, "css")) ret = "text/css";
	//else if (!strcmp(p, "jpg")) ret = "image/jpeg";
	//else if (!strcmp(p, "png")) ret = "image/png";
	//else if (!strcmp(p, "jjs")) ret = "application/x-javascript";
	//strcpy(buf, "Content-type: %s\r\n");//
	//send(client, buf, strlen(buf), 0);

	//还有长度。。。啥的

	strcpy(buf, "\r\n");//空行
	send(client, buf, strlen(buf), 0);

}
void not_found(int client) {
	//网页不存在
	//发送响应包的头信息,发送404
	char buf[1024];
	strcpy(buf, "HTTP/1.0 404 OK\r\n");//拷贝,状态行
	send(client, buf, strlen(buf), 0);

	strcpy(buf, "Server: RockHttpd/0.1\r\n");//服务器端的软件名称和它的版本号
	send(client, buf, strlen(buf), 0);

	strcpy(buf, "Content-type:text/html\n");//返回消息的内容类型,图片应该是image/jpeg
	//strcpy(buf, "Content-type:image/jpeg\n");//返回消息的内容类型,图片应该是image/jpeg
	send(client, buf, strlen(buf), 0);

	//还有长度。。。啥的

	strcpy(buf, "\r\n");//空行
	send(client, buf, strlen(buf), 0);

	//发送404网页内容
	sprintf(buf, "<HTML>                               \
	      <TITLE>Not Found</TITLE>        \
		  <BODY>                               \
		<H2> The resource is unavailable.</H2> \
		<Img src = \"404.gif\" / >                 \
		</BODY>                                   \
		</HTML>");  
	send(client, buf, strlen(buf), 0);
	

	

}
void cat(int client, FILE*resource) {
	//发送文件内容
	char buf[4096];//一次发4096,网络数据包4096，有人linux一次发一个，慢
	int count = 0;
	while (1) {//不停发送
		int ret = fread(buf, sizeof(char), sizeof(buf), resource);//一次读一个字节，读4096次, 从文件中读取若干字节数据到内存缓冲区中 ;
	//ret返回字节数
		if (ret <=0) {
			break;
		}
		send(client, buf, ret, 0);
		count += ret;
	}
	printf("一共发送[%d]字节给浏览器\n", count);
	



}
const char* getHeadType(const char* filename) {//将文件类型解析出来
	const char* ret = "text/html";
	const char* p = strrchr(filename, '.');
	if (!p) return ret;

	p++;
	if (!strcmp(p, "css")) ret = "text/css";
	else if(!strcmp(p, "jpg")) ret = "image/jpeg";
	else if(!strcmp(p, "png")) ret = "image/png";
	else if(!strcmp(p, "js")) ret = "application/x-javascript";
	return ret;
}
void server_file(int client, const char* filename) {
	//发送文件,正确也要把后续读完
	int numchars = 1;//int 大，不会越界
	char buf[1024];
	while (numchars > 0 && strcmp(buf, "\n"))//把请求包的剩余报文全部读完
	{
		numchars = get_line(client, buf, sizeof(buf));
		PRINTF(buf);
	}

	//FILE* resource = fopen(filename, "r");//文本文件打开
	FILE* resource = NULL;
	if (strcmp(filename, "htdocs/index.html") == 0) {//怎么判断路径结尾是html?????
		resource = fopen(filename, "r");//文本文件打开
	}
	else {
		resource = fopen(filename, "rb");//二进制打开
	}
	if (resource == NULL) {
		not_found(client);
	}
	else {
		//正式发送资源给浏览器
		
		headers(client,getHeadType(filename));//发送响应包的头信息

		//发送请求的资源信息
		cat(client, resource);

		printf("资源发送完毕\n");
	}
	fclose(resource);//记得关闭


}


DWORD WINAPI accept_request(LPVOID arg)//传递线程主函数信息
{
	//1、浏览器发起访问，向服务端发送请求：发送GET请求包
	//读取一行数据
	char buf[1024];
	int client = (SOCKET)arg;//arg客户端为套接字
	//GET / HTTP / 1.1\n
	int numchars = get_line(client, buf, sizeof(buf));
	//printf("读到： %s\n", buf);
	//printf("[%s - %d]%s", __func__, __LINE__, str);//__func__函数名，LINE行,在哪个函数的哪一行
	PRINTF(buf);//打印[函数-行数]buf="GET.."

	char method[255];
	int j = 0, i = 0;//j请求行下标
	while (!isspace(buf[j]) && i < sizeof(method) - 1) {//扫描buf,不是空白字符读取,目标读取get,post，也不越界
		method[i++] = buf[j++];
	}
	method[i] = 0;//加一个'\0'结束符，method里面全是字符，没有空白
	PRINTF(method); //解析后， method的值："GET"
	//检查请求方法，本服务器是否支持
	if (stricmp(method, "GET") && stricmp(method, "POST")) {//strcmp返回0,2个相等，只支持get，post这2种,GET和POST是HTTP请求的两种基本方法
		//向浏览器返回一个错误提示页面
		unimplent(client);
		return 0;
	}

	//解析资源文件的路径，www.rock.com/abc/test.html
	//GET /abc/test.html HTTP / 1.1\n
	char url[255];//存放请求的资源完整路径
	i = 0;
	while (isspace(buf[j]) && j < sizeof(method)) {//如果是空格,跳过去
		j++;
	}

	while (!isspace(buf[j]) && i < sizeof(url) - 1 && j < sizeof(method)) {
		url[i++] = buf[j++];//存放 /abc/test.html
	}
	url[i] = 0;
	PRINTF(url);

	//127.0.0.1/test.html
	//url /test.html 根目录
	//资源目录htdocs/，把url放在后面  htdocs/test.html

	//htdocs/index.html,默认有

	char path[512] = "";
	sprintf(path, "htdocs%s", url);//格式化的数据写入字符串path中

	// 如果浏览器的地址输入：http://127.0.0.1:8000/movies/
	// 那么url就是 /movies/ 
	// url的最后一个字符是路径分隔符/
	// 表示默认访问的是：/movies/index.html
	if (path[strlen(path) - 1] == '/')//path：htdocs/
	{
		strcat(path, "index.html");//拼接
	}
	PRINTF(path);
	// 检查访问的资源是否存在，文件下面是目录，还是文件，没有目录的话默认index.html文件
	
	//判断访问文件的属性，是文件，还是目录
	struct stat status;//通过文件名filename获取文件信息，并保存在buf所指的结构体stat中；status文件状态
	if (stat(path, &status) == -1) {//，保存在static 访问失败,stat第一个形参：指出文件（文件路径）； 第二个形参：出参数（函数对该参数操作，然后传出）
		// 如果不能访问它的属性信息，那么这个文件就不存在(没有index文件)
		// 此时，就需要把这个请求报文，读完！虽然已经没有用了，但是也要把这个报文读完
		while (numchars > 0 && strcmp(buf, "\n"))//把请求包的剩余报文全部读完
		{
			numchars = get_line(client, buf, sizeof(buf));
		}

		not_found(client);//网页不存在
	}
	else {//有index文件，直接发送
		// 如果浏览器的地址输入：http://127.0.0.1:8000/movies 
		// 如果movies是目录，就默认访问这个目录下的index.html
		if ((status.st_mode & S_IFMT) == S_IFDIR){//未操作文件类型是目录
			strcat(path, "index.html");//再拼接一个index
          }      
		server_file(client, path);//把文件发过去
	}
	closesocket(client);
	return 0;
}


int main()
{
	unsigned short port =8000;
	int server_socket = startup(&port);
	printf("http服务已经启动，正在监听 %d 端口...\n", port);

	

	while (1)
	{
		struct sockaddr_in client_addr;
		int client_addr_len = sizeof(client_addr);
		//阻塞式等待用户通过浏览器访问
		SOCKET client_sock=accept(server_socket,(struct sockaddr*)&client_addr,&client_addr_len);//分配生成新的server_socket(client_sock)，对新顾客接待
		if (client_sock == INVALID_SOCKET)
		{
			//cout << "客户端调用失败" << endl;
			int a = WSAGetLastError();
			error_die("accept");
			WSACleanup();
			return 0;
			
		}
		//使用client_sock对用户进行访问
		
		//创建新的线程，进程包括多个线程
	
		DWORD threadid = 0;//线程id
		CreateThread(NULL, 0, accept_request, (void*)client_sock, 0, &threadid);
	}

	closesocket(server_socket);
	return 0;

}

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
#define PRINTF(str) printf("[%s - %d]"#str"=%s\n", __func__, __LINE__, str);//�Ѻ�������ɺ�


void error_die(const char* str)
{
	perror(str);//��ӡ����ԭ��,perror(s) ��������һ���������������ԭ���������׼�豸(stderr)������ s ��ָ���ַ������ȴ�ӡ��
	//�������ټ��ϴ���ԭ���ַ������˴���ԭ������ȫ�ֱ���errno��ֵ������Ҫ������ַ�����
	exit(1);
}


//ʵ�������ʼ��
//����ֵ���׽���
//port �˿� Ϊ0�Զ�����
int startup(unsigned short int* port) {
	//1������ͨ�ų�ʼ��
	WSADATA data;
	int ret=WSAStartup(MAKEWORD(1, 1), &data);
	if (ret != 0)
	{
		//cout << "��ʧ��" << endl;;
		error_die("WSAStartup");
	
	}
	if (1 != HIBYTE(data.wVersion) || 1 != LOBYTE(data.wVersion))
	{
		//˵���汾����
		//���������
		//std::cout << "�汾�Ų���" << std::endl;
		WSACleanup();
		return 0;
	}

	int  server_socket= socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);//AF_INET,PF_INETһ��
	if (server_socket == -1)
	{
		error_die("�׽���");
	}
	/*if (server_socket == INVALID_SOCKET)
	{
		cout << "����ʧ��" << endl;
		int a = WSAGetLastError();
		WSACleanup();
		return 0;
	}*/



	//���׽���
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(*port);//charת����������,short
	//server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");//�ͻ��˶�������,��������unsigned longֵת���������ֽ�˳��32λ��
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//�ͻ��˶�������,��������unsigned longֵת���������ֽ�˳��32λ��

		//���ö˿ڿɸ���
	int opt = 1;
	int r = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
	if (r == -1) {
		error_die("setsockopt");
	}
	if (::bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		error_die("bind");
	}

	//��̬����˿�
	int len = sizeof(server_addr);
	if (*port == 0) {
		if (getsockname(server_socket, (struct sockaddr*)&server_addr, &len) < 0)
		{
			error_die("getsockname");
		}
		//�Զ�����Ķ˿ڷŽ�ȥserver_addr���ٸ�port
		*port= ntohs(server_addr.sin_port);
	}
	//������������
	if (listen(server_socket, SOMAXCONN) == -1)
	{
		error_die("listen");
	}
	return(server_socket);
}

int get_line(int sock, char* buf, int size)//��ָ���Ŀͻ����׽��֣���ȡһ�����ݣ����浽buf
{
	char c = 0;//'\0'
	int i = 0;

	// \r\n,���һ���ַ�
	while (i < size - 1 && c != '\n') {//��û��Խ�磬�س���
		int n = recv(sock, &c, 1, 0);//char һ���ֽ�
		if (n > 0)
		{
			if (c == '\r')
			{
				n = recv(sock, &c, 1, MSG_PEEK);//MSG_PEEK ͵͵��һ��,��r�����ǲ���n
				if (n > 0 && c == '\n') {
					recv(sock, &c, 1, 0);//��n����ʽ��ȡ	
				}
				else {
					c = '\n';//ǿ�ƽ���
				}
			}
			buf[i++] = c;//�����ĵ����ݷŵ�buf
		}
		else {
			c = '\n';
		}
	}
	buf[i] = 0;//'\0',�ַ���������
	return i;//���ض����ֽ�
}
void unimplent(int client)
{
	//��ָ�����׽��֣�����һ����ʾ��ûʵ�ֵĴ���ҳ��
}
void headers(int client,const char* type) {
	//������Ӧ����ͷ��Ϣ
	char buf[1024];
	strcpy(buf, "HTTP/1.0 200 OK\r\n");//����,״̬��
	send(client, buf, strlen(buf), 0);

	strcpy(buf, "Server: RockHttpd/0.1\r\n");//�������˵�������ƺ����İ汾��
	send(client, buf, strlen(buf), 0);



	//strcpy(buf, "Content-type:text/html\n");//������Ϣ����������,ͼƬӦ����image/jpeg
	//send(client, buf, strlen(buf), 0);

	
	//strcpy(buf, "Content-type:image/jpeg\n");//������Ϣ����������,ͼƬӦ����image/jpeg
	//send(client, buf, strlen(buf), 0);

	//strcpy(buf, "Content-type:text/css\n");//������Ϣ����������,
	//send(client, buf, strlen(buf), 0);

	//strcpy(buf, "Content-type:image/png\n");//������Ϣ����������,
	//send(client, buf, strlen(buf), 0);

	//strcpy(buf, "Content-type:application/x-javascript\n");//������Ϣ����������,
	//send(client, buf, strlen(buf), 0);

	char buf2[1024];
		sprintf(buf2, "Content-Type: %s\r\n",type);//������Ϣ����������,
	send(client, buf2, strlen(buf2), 0);

	//if (!strcmp(p, "css")) ret = "text/css";
	//else if (!strcmp(p, "jpg")) ret = "image/jpeg";
	//else if (!strcmp(p, "png")) ret = "image/png";
	//else if (!strcmp(p, "jjs")) ret = "application/x-javascript";
	//strcpy(buf, "Content-type: %s\r\n");//
	//send(client, buf, strlen(buf), 0);

	//���г��ȡ�����ɶ��

	strcpy(buf, "\r\n");//����
	send(client, buf, strlen(buf), 0);

}
void not_found(int client) {
	//��ҳ������
	//������Ӧ����ͷ��Ϣ,����404
	char buf[1024];
	strcpy(buf, "HTTP/1.0 404 OK\r\n");//����,״̬��
	send(client, buf, strlen(buf), 0);

	strcpy(buf, "Server: RockHttpd/0.1\r\n");//�������˵�������ƺ����İ汾��
	send(client, buf, strlen(buf), 0);

	strcpy(buf, "Content-type:text/html\n");//������Ϣ����������,ͼƬӦ����image/jpeg
	//strcpy(buf, "Content-type:image/jpeg\n");//������Ϣ����������,ͼƬӦ����image/jpeg
	send(client, buf, strlen(buf), 0);

	//���г��ȡ�����ɶ��

	strcpy(buf, "\r\n");//����
	send(client, buf, strlen(buf), 0);

	//����404��ҳ����
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
	//�����ļ�����
	char buf[4096];//һ�η�4096,�������ݰ�4096������linuxһ�η�һ������
	int count = 0;
	while (1) {//��ͣ����
		int ret = fread(buf, sizeof(char), sizeof(buf), resource);//һ�ζ�һ���ֽڣ���4096��, ���ļ��ж�ȡ�����ֽ����ݵ��ڴ滺������ ;
	//ret�����ֽ���
		if (ret <=0) {
			break;
		}
		send(client, buf, ret, 0);
		count += ret;
	}
	printf("һ������[%d]�ֽڸ������\n", count);
	



}
const char* getHeadType(const char* filename) {//���ļ����ͽ�������
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
	//�����ļ�,��ȷҲҪ�Ѻ�������
	int numchars = 1;//int �󣬲���Խ��
	char buf[1024];
	while (numchars > 0 && strcmp(buf, "\n"))//���������ʣ�౨��ȫ������
	{
		numchars = get_line(client, buf, sizeof(buf));
		PRINTF(buf);
	}

	//FILE* resource = fopen(filename, "r");//�ı��ļ���
	FILE* resource = NULL;
	if (strcmp(filename, "htdocs/index.html") == 0) {//��ô�ж�·����β��html?????
		resource = fopen(filename, "r");//�ı��ļ���
	}
	else {
		resource = fopen(filename, "rb");//�����ƴ�
	}
	if (resource == NULL) {
		not_found(client);
	}
	else {
		//��ʽ������Դ�������
		
		headers(client,getHeadType(filename));//������Ӧ����ͷ��Ϣ

		//�����������Դ��Ϣ
		cat(client, resource);

		printf("��Դ�������\n");
	}
	fclose(resource);//�ǵùر�


}


DWORD WINAPI accept_request(LPVOID arg)//�����߳���������Ϣ
{
	//1�������������ʣ������˷������󣺷���GET�����
	//��ȡһ������
	char buf[1024];
	int client = (SOCKET)arg;//arg�ͻ���Ϊ�׽���
	//GET / HTTP / 1.1\n
	int numchars = get_line(client, buf, sizeof(buf));
	//printf("������ %s\n", buf);
	//printf("[%s - %d]%s", __func__, __LINE__, str);//__func__��������LINE��,���ĸ���������һ��
	PRINTF(buf);//��ӡ[����-����]buf="GET.."

	char method[255];
	int j = 0, i = 0;//j�������±�
	while (!isspace(buf[j]) && i < sizeof(method) - 1) {//ɨ��buf,���ǿհ��ַ���ȡ,Ŀ���ȡget,post��Ҳ��Խ��
		method[i++] = buf[j++];
	}
	method[i] = 0;//��һ��'\0'��������method����ȫ���ַ���û�пհ�
	PRINTF(method); //������ method��ֵ��"GET"
	//������󷽷������������Ƿ�֧��
	if (stricmp(method, "GET") && stricmp(method, "POST")) {//strcmp����0,2����ȣ�ֻ֧��get��post��2��,GET��POST��HTTP��������ֻ�������
		//�����������һ��������ʾҳ��
		unimplent(client);
		return 0;
	}

	//������Դ�ļ���·����www.rock.com/abc/test.html
	//GET /abc/test.html HTTP / 1.1\n
	char url[255];//����������Դ����·��
	i = 0;
	while (isspace(buf[j]) && j < sizeof(method)) {//����ǿո�,����ȥ
		j++;
	}

	while (!isspace(buf[j]) && i < sizeof(url) - 1 && j < sizeof(method)) {
		url[i++] = buf[j++];//��� /abc/test.html
	}
	url[i] = 0;
	PRINTF(url);

	//127.0.0.1/test.html
	//url /test.html ��Ŀ¼
	//��ԴĿ¼htdocs/����url���ں���  htdocs/test.html

	//htdocs/index.html,Ĭ����

	char path[512] = "";
	sprintf(path, "htdocs%s", url);//��ʽ��������д���ַ���path��

	// ���������ĵ�ַ���룺http://127.0.0.1:8000/movies/
	// ��ôurl���� /movies/ 
	// url�����һ���ַ���·���ָ���/
	// ��ʾĬ�Ϸ��ʵ��ǣ�/movies/index.html
	if (path[strlen(path) - 1] == '/')//path��htdocs/
	{
		strcat(path, "index.html");//ƴ��
	}
	PRINTF(path);
	// �����ʵ���Դ�Ƿ���ڣ��ļ�������Ŀ¼�������ļ���û��Ŀ¼�Ļ�Ĭ��index.html�ļ�
	
	//�жϷ����ļ������ԣ����ļ�������Ŀ¼
	struct stat status;//ͨ���ļ���filename��ȡ�ļ���Ϣ����������buf��ָ�Ľṹ��stat�У�status�ļ�״̬
	if (stat(path, &status) == -1) {//��������static ����ʧ��,stat��һ���βΣ�ָ���ļ����ļ�·������ �ڶ����βΣ��������������Ըò���������Ȼ�󴫳���
		// ������ܷ�������������Ϣ����ô����ļ��Ͳ�����(û��index�ļ�)
		// ��ʱ������Ҫ����������ģ����꣡��Ȼ�Ѿ�û�����ˣ�����ҲҪ��������Ķ���
		while (numchars > 0 && strcmp(buf, "\n"))//���������ʣ�౨��ȫ������
		{
			numchars = get_line(client, buf, sizeof(buf));
		}

		not_found(client);//��ҳ������
	}
	else {//��index�ļ���ֱ�ӷ���
		// ���������ĵ�ַ���룺http://127.0.0.1:8000/movies 
		// ���movies��Ŀ¼����Ĭ�Ϸ������Ŀ¼�µ�index.html
		if ((status.st_mode & S_IFMT) == S_IFDIR){//δ�����ļ�������Ŀ¼
			strcat(path, "index.html");//��ƴ��һ��index
          }      
		server_file(client, path);//���ļ�����ȥ
	}
	closesocket(client);
	return 0;
}


int main()
{
	unsigned short port =8000;
	int server_socket = startup(&port);
	printf("http�����Ѿ����������ڼ��� %d �˿�...\n", port);

	

	while (1)
	{
		struct sockaddr_in client_addr;
		int client_addr_len = sizeof(client_addr);
		//����ʽ�ȴ��û�ͨ�����������
		SOCKET client_sock=accept(server_socket,(struct sockaddr*)&client_addr,&client_addr_len);//���������µ�server_socket(client_sock)�����¹˿ͽӴ�
		if (client_sock == INVALID_SOCKET)
		{
			//cout << "�ͻ��˵���ʧ��" << endl;
			int a = WSAGetLastError();
			error_die("accept");
			WSACleanup();
			return 0;
			
		}
		//ʹ��client_sock���û����з���
		
		//�����µ��̣߳����̰�������߳�
	
		DWORD threadid = 0;//�߳�id
		CreateThread(NULL, 0, accept_request, (void*)client_sock, 0, &threadid);
	}

	closesocket(server_socket);
	return 0;

}
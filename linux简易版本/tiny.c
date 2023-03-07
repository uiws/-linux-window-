#include <stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER_PORT 666
void error_handling(char* message);
static int debug=1;//想打应，不想打应改成0

void* do_http_request(void*clnt_sock);
int get_line(int clnt_sock,char*buf,int size);
void not_found(int clnt_sock);//404
void unimplemented(int clnt_sock);//501
 void bad_request(int clnt_sock);//400 
void iner_error(int clnt_sock);//500
void do_http_response(int clnt_sock,const char*path);

 //1.发送http头部
int header(int clnt_sock,FILE*resource);

void cat(int clnt_sock,FILE*resource);
	
	


int main(void)
{
    int serv_sock;
    int clnt_sock;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    char message[] = "Hello World!";

    // if (argc != 2)
    // {
        // printf("Usage : %s <port>\n", argv[0]);
        // exit(1);
    // }
    //调用 socket 函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //serv_addr.sin_port = htons(atoi(argv[1]));
	serv_addr.sin_port = htons(SERVER_PORT);
    //调用 bind 函数分配ip地址和端口号
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    //调用 listen 函数将套接字转为可接受连接状态
    if (listen(serv_sock, 128) == -1)
        error_handling("listen() error");
    printf("等待客户端的连接3223\n");

    while (1) {
        char client_ip[64];
        char buf[256];
        int len;
         int i;
		 pthread_t id;
		 int*pclient_sock=NULL;
		 
        clnt_addr_size = sizeof(clnt_addr);
        //调用 accept 函数受理连接请求。如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1)
            error_handling("accept() error");
	
         //打印客服端IP地址和端口号
         printf("client ip: %s\t port : %d\n",
             inet_ntop(AF_INET, &clnt_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
             ntohs(clnt_addr.sin_port));
			 
			//处理http请求，读取客户端发送的请求
		printf("读取客户端请求：\n");
		
		//do_http_request(clnt_sock);
		
		//启动线程
		pclient_sock=(int*)malloc(sizeof(int));
		*pclient_sock=clnt_sock;
		pthread_create(&id,NULL,do_http_request,(void*)pclient_sock);
		
			 
        // /*读取客户端发送的数据*/
        // len = read(clnt_sock, buf, sizeof(buf) - 1);
        // buf[len] = '\0';
        // printf("receive[%d]: %s\n", len, buf);
        // //转换成大写
        // for (i = 0; i < len; i++) {
            // /*if(buf[i]>='a' && buf[i]<='z'){
                // buf[i] = buf[i] - 32;
            // }*/
            // buf[i] = toupper(buf[i]);
        // }
        // //稍后要将介绍的 write 函数用于传输数据，若程序经过 accept 这一行执行到本行，则说明已经有了连接请求
        // len = write(clnt_sock, buf, len);
        // printf("finished. len: %d\n", len);

    }

    
    close(serv_sock);
    return 0;
}
void* do_http_request(void* pclient_sock){
	int len=0;
	char buf[1024];
	char method[64];
	char url[256];
	char path[256];
	int clnt_sock=*(int*)pclient_sock;
	
	struct stat st;
	
	// do{
		// len=get_line(clnt_sock,buf,sizeof(buf));
		// printf("read line:%s\n",buf);
	// }while(len>0);
	
	//1、读取请求行
	len=get_line(clnt_sock,buf,sizeof(buf));
	if(len>0){//读到了请求行
		int i=0,j=0;
		//扫描buf,isspace不是空白字符读取,
		while(!isspace(buf[j])&&(i<sizeof(method)-1)){
			method[i++]=buf[j++];
		}
		method[i]='\0';
		if(debug) printf("request method :%s\n",method);
		
		if(strncasecmp(method,"GET",i)==0){//strncasecmp比较相等
		if(debug) printf("method=GET\n");
		
		//获取url
		while(isspace(buf[j++]));//跳过空格
		i=0;
			while(!isspace(buf[j])&&(i<sizeof(url)-1)){
			url[i++]=buf[j++];
		}
		url[i]='\0';
		if(debug) printf("url:%s\n",url);
				//继续读取http头部
		do{
		    len=get_line(clnt_sock,buf,sizeof(buf));
		    printf("read line:%s\n",buf);
	    }while(len>0);
		
		//定位到服务器本地html文件
		//处理url中？  html？kjdssaj
		{
		char *pos=strchr(url,'?');//定位到？出现的下标
		if(pos){
			*pos='\0';
			printf("read url: %s\n",url);
		}
			
		}
		
		sprintf(path,"./html_docs/%s",url);//格式化的数据写入字符串path中,./html_docs/url
		if(debug) printf("path: %s\n",path);
		
		
		
		//执行http
		//判断文件是否存在，如果存在相应200 ok同时发送html文件，不存在，发送404
		if(stat(path,&st)==-1){//通过文件名filename获取文件信息，并保存在buf所指的结构体stat中；st文件状态
		    fprintf(stderr,"stat %s fail reason:%s\n",path,strerror(errno));
			not_found(clnt_sock);
		}else{
			
			if(S_ISDIR(st.st_mode)){//判断是不是目录，
				strcat(path,"/index.html");//追加index.html
			}
			do_http_response(clnt_sock,path);
		}
	}
	else{//非get请求,读取http头部，并响应客户端501，Method Not Implemented
		fprintf(stderr,"warning other request [%s]\n",method); 
			do{
		    len=get_line(clnt_sock,buf,sizeof(buf));
		    printf("read line:%s\n",buf);
	    }while(len>0);
				
		unimplemented(clnt_sock);  //请求未实现
		}
			
	}else{//请求格式有问题
	  bad_request(clnt_sock);  //在响应时再实现
		
	}
	close(clnt_sock);
	if(pclient_sock) free(pclient_sock);//释放动态分配的内存
	return NULL;
		
}

void do_http_response(int clnt_sock,const char*path){
	int ret=0;
	FILE*resource=NULL;
	resource=fopen(path,"r");
	
	if(resource==NULL){
		not_found(clnt_sock);
		return;
	}
	//1.发送http头部
	ret=header(clnt_sock,resource);

	//2.发送http_body
	if(!ret){
		cat(clnt_sock,resource);
	}
	
	fclose(resource);
}

 //1.发送http头部
int header(int clnt_sock,FILE*resource){
	struct stat st;
	int fileid=0;
	char buf[1024];
	char tem[64];
	strcpy(buf,"HTTP/1.0 200 OK\r\n");//拷贝
	strcat(buf,"Server: Martin Server\r\n");
	strcat(buf,"Content-Type: text/html\r\n");
	strcat(buf,"Connection: Close\r\n");
	
	fileid=fileno(resource);//返回指针
	if(fstat(fileid,&st)==-1){//内部服务器错误
	      iner_error(clnt_sock);
		  return -1;
	}
	
	snprintf(tem,64,"content_length：%d\r\n\r\n",st.st_size);
	//st.st_size;//拿到文件长度
	strcat(buf,tem);
	
	  if(debug) fprintf(stdout,"haeder: %s",buf);
	  if(send(clnt_sock,buf,strlen(buf),0)<0){
		  fprintf(stderr,"send fail.data:%s,reason:%s",buf,strerror(errno));
		  return -1;
	  }
	return 0;
}
//2.发送http_body,将html文件**按行**发给客户端
// void cat(int clnt_sock,FILE*resource){
	// char buf[1024];
	
	// fgets(buf,sizeof(buf),resource);
	
	// while(!feof(resource)){
		// int len=write(clnt_sock,buf,strlen(buf));
		
		// if(len<0){//出错，不重适l
			// fprintf(stderr,"send bodey error.reason:%s\n",strerror(errno));
			// break;
		// }
		// if(debug) fprintf(stdout,"%s",buf);
		
		// fgets(buf,sizeof(buf),resource);
	// }
	  
	
// }

//2.发送http_body,将html文件**按字节**发给客户端
void cat(int clnt_sock, FILE*resource) {
	//发送文件内容
	char buf[4096];//一次发4096,网络数据包4096，有人linux一次发一个，慢
	int count = 0;
	while (1) {//不停发送
		int ret = fread(buf, sizeof(char), sizeof(buf), resource);//一次读一个字节，读4096次, 从文件中读取若干字节数据到内存缓冲区中 ;
	//ret返回字节数
		if (ret <=0) {
			break;
		}
		send(clnt_sock, buf, ret, 0);
		count += ret;
	}
	if(debug) fprintf(stdout,"%s",buf);
	printf("一共发送[%d]字节给浏览器\n", count);
	

}
// void do_http_response(int clnt_sock){
	 // const char *main_header = "HTTP/1.0 200 OK\r\nServer: Martin Server\r\nContent-Type: text/html\r\nConnection: Close\r\n";


    // const char * welcome_content = "\
// <html lang=\"zh-CN\">\n\
// <head>\n\
// <meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\">\n\
// <title>This is a test</title>\n\
// </head>\n\
// <body>\n\
// <div align=center height=\"500px\" >\n\
// <br/><br/><br/>\n\
// <h2>大家好，欢迎来到奇牛学院VIP 试听课！</h2><br/><br/>\n\
// <form action=\"commit\" method=\"post\">\n\
// 尊姓大名: <input type=\"text\" name=\"name\" />\n\
// <br/>芳龄几何: <input type=\"password\" name=\"age\" />\n\
// <br/><br/><br/><input type=\"submit\" value=\"提交\" />\n\
// <input type=\"reset\" value=\"重置\" />\n\
// </form>\n\
// </div>\n\
// </body>\n\
// </html>";

	//1. 发送main_header
	// int len = write(clnt_sock, main_header, strlen(main_header));
	// if(debug) fprintf(stdout,"...do_http_respose...\n");
    // if(debug) fprintf(stdout,"write[%d]: %s",len,main_header);
	
	//2 生成content_length
	// char send_buf[64];
	// int wc_len=strlen(welcome_content);
	// len=snprintf(send_buf,64,"content_length：%d\r\n\r\n",wc_len);//将长度写入send——buf，限制64
	// len=write(clnt_sock,send_buf,len);
	    // if(debug) fprintf(stdout,"write[%d]: %s",len,send_buf);
		
			// len = write(clnt_sock, welcome_content, wc_len);
	// if(debug) fprintf(stdout, "write[%d]: %s", len, welcome_content);
	
// }



//-1代表出错，大于0成功读取一行， 等于0读到空行

int get_line(int sock, char *buf, int size){
	int count = 0;
	char ch = '\0';
	int len = 0;
	
	
	while( (count<size - 1) && ch!='\n'){
		len = read(sock, &ch, 1);
		
		if(len == 1){
			if(ch == '\r'){
				continue;
			}else if(ch == '\n'){
				//buf[count] = '\0';
				break;
			}
			
			//这里处理一般的字符
			buf[count] = ch;
			count++;
			
		}else if( len == -1 ){//读取出错
			perror("read failed");
			count = -1;
			break;
		}else {// read 返回0,客户端关闭sock 连接.
			fprintf(stderr, "client close.\n");
			count = -1;
			break;
		}
	}
	
	if(count >= 0) buf[count] = '\0';
	
	return count;
}

// int get_line(int clnt_sock,char *buf,int size){
	// int count=0;
	// char ch='\0';
	// int len=0;
	
	// while((count<size-1)&&ch!='\n'){
		// len=read(clnt_sock,&ch,1);
		// if(len==1){
			// if(ch=='\r'){
				// continue;
			// }
			// else if(ch=='\n'){
				// break;
			// }
			// //处理一般字符
			// buf[count]=ch;
			// count++;
			
		// }
		// else if(len==-1){//读取出错
		// perror("read failed");
		// count==-1;
		// break;
			
		// }
		// else{//read返回0，关闭客户端连接
			// fprintf(stderr,"client close.\n");
			// count==-1;
			// break;
		// }
	// }
	// if(count>=0) buf[count]='\0';//正常读完，溢出没读完，都加\0
	
	// return count;
	
	
	
	
// }
void not_found(int clnt_sock){
	
	const char * reply = "HTTP/1.0 404 NOT FOUND\r\n\
Content-Type: text/html\r\n\
\r\n\
<HTML lang=\"zh-CN\">\r\n\
<meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\">\r\n\
<HEAD>\r\n\
<TITLE>NOT FOUND</TITLE>\r\n\
</HEAD>\r\n\
<BODY>\r\n\
	<P>文件不存在！\r\n\
    <P>The server could not fulfill your request because the resource specified is unavailable or nonexistent.\r\n\
</BODY>\r\n\
</HTML>";

int len=write(clnt_sock,reply,strlen(reply));
if(debug) fprintf(stdout,reply);
  if(len<0){
	  fprintf(stderr,"send reply failed. reason: %s\n", strerror(errno));
  }
	
}
void unimplemented(int clnt_sock){//500
     const char * reply = "HTTP/1.0 501 Method Not Implemented\r\n\
Content-Type: text/html\r\n\
\r\n\
<HTML>\r\n\
<HEAD>\r\n\
<TITLE>Method Not Implemented</TITLE>\r\n\
</HEAD>\r\n\
<BODY>\r\n\
    <P>HTTP request method not supported.\r\n\
</BODY>\r\n\
</HTML>";
	int len=write(clnt_sock,reply,strlen(reply));
    if(debug) fprintf(stdout,reply);
    if(len<0){
	  fprintf(stderr,"send reply failed. reason: %s\n", strerror(errno));
  }
}

 void bad_request(int clnt_sock){//400
	  const char * reply = "HTTP/1.0 400 BAD REQUEST\r\n\
Content-Type: text/html\r\n\
\r\n\
<HTML>\r\n\
<HEAD>\r\n\
<TITLE>BAD REQUEST</TITLE>\r\n\
</HEAD>\r\n\
<BODY>\r\n\
    <P>Your browser sent a bad request！\r\n\
</BODY>\r\n\
</HTML>";

    int len = write(clnt_sock, reply, strlen(reply));
    if(len<=0){
        fprintf(stderr, "send reply failed. reason: %s\n", strerror(errno));
    }	
	 
 }
void iner_error(int clnt_sock){
	
	const char * reply = "HTTP/1.0 500 Internal Sever Error\r\n\
Content-Type: text/html\r\n\
\r\n\
<HTML lang=\"zh-CN\">\r\n\
<meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\">\r\n\
<HEAD>\r\n\
<TITLE>Inner Error</TITLE>\r\n\
</HEAD>\r\n\
<BODY>\r\n\
    <P>服务器内部出错.\r\n\
</BODY>\r\n\
</HTML>";

int len=write(clnt_sock,reply,strlen(reply));
if(debug) fprintf(stdout,reply);
  if(len<0){
	  fprintf(stderr,"send reply failed. reason: %s\n", strerror(errno));
  }
	
}
void error_handling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

//管道快速入门
#include<stdio.h>
#include<Windows.h>
#include<iostream>
using namespace std;
int main() {
	//创建管道
	HANDLE output[2];//管道二端句柄,读和写
	//管道属性
	SECURITY_ATTRIBUTES la;
	la.nLength = sizeof(la);
	la.bInheritHandle = true;
	la.lpSecurityDescriptor = 0;//安全描述符，0默认值


	bool bCreat = CreatePipe(&output[0], &output[1], &la, 0);
	if (bCreat == false) {
		MessageBox(0, "创建管道失败", "错误提示", MB_YESNOCANCEL);//错误提示
		return 1;
	}
	//c创建子进程
	char cmd[] = "ping www.baidu.com";
	//子进程启动属性
	STARTUPINFO si = { 0 };
	
	si.cb = sizeof(si);
	si.hStdOutput = output[1];//重定向到写端
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	PROCESS_INFORMATION pi = { 0 };//进程信息
	bCreat=CreateProcess(NULL,cmd,0,0,TRUE,0,0,0,&si,&pi);//类似fork,子进程搞完
	if (bCreat == false) {
		printf("子进程创建失败\n");
		return 1;
	}

	char buff[1024];
	DWORD size;//Double Word， 每个word为2个字节的长度，DWORD 双字即为4个字节，每个字节是8位，

	while (1) {//父进程开始读
		//printf("请输入： ");
		//gets_s(buff, sizeof(buff));

		//WriteFile(output[1], buff, strlen(buff) + 1, &size, NULL);//size真正写入的个数
		//printf("已经写入%d字节\n", size);

		ReadFile(output[0], buff, sizeof(buff), &size, NULL);
		buff[size] = '\0';//读到的东西加字符结束符
		printf("已经读到%d字节：[%s]\n", size, buff);
		cout << "已经读到" << size << "字节" << ":" << buff << endl;

	}
	return 0;
}


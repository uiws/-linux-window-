//�ܵ���������
#include<stdio.h>
#include<Windows.h>
#include<iostream>
using namespace std;
int main() {
	//�����ܵ�
	HANDLE output[2];//�ܵ����˾��,����д
	//�ܵ�����
	SECURITY_ATTRIBUTES la;
	la.nLength = sizeof(la);
	la.bInheritHandle = true;
	la.lpSecurityDescriptor = 0;//��ȫ��������0Ĭ��ֵ


	bool bCreat = CreatePipe(&output[0], &output[1], &la, 0);
	if (bCreat == false) {
		MessageBox(0, "�����ܵ�ʧ��", "������ʾ", MB_YESNOCANCEL);//������ʾ
		return 1;
	}
	//c�����ӽ���
	char cmd[] = "ping www.baidu.com";
	//�ӽ�����������
	STARTUPINFO si = { 0 };
	
	si.cb = sizeof(si);
	si.hStdOutput = output[1];//�ض���д��
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	PROCESS_INFORMATION pi = { 0 };//������Ϣ
	bCreat=CreateProcess(NULL,cmd,0,0,TRUE,0,0,0,&si,&pi);//����fork,�ӽ��̸���
	if (bCreat == false) {
		printf("�ӽ��̴���ʧ��\n");
		return 1;
	}

	char buff[1024];
	DWORD size;//Double Word�� ÿ��wordΪ2���ֽڵĳ��ȣ�DWORD ˫�ּ�Ϊ4���ֽڣ�ÿ���ֽ���8λ��

	while (1) {//�����̿�ʼ��
		//printf("�����룺 ");
		//gets_s(buff, sizeof(buff));

		//WriteFile(output[1], buff, strlen(buff) + 1, &size, NULL);//size����д��ĸ���
		//printf("�Ѿ�д��%d�ֽ�\n", size);

		ReadFile(output[0], buff, sizeof(buff), &size, NULL);
		buff[size] = '\0';//�����Ķ������ַ�������
		printf("�Ѿ�����%d�ֽڣ�[%s]\n", size, buff);
		cout << "�Ѿ�����" << size << "�ֽ�" << ":" << buff << endl;

	}
	return 0;
}


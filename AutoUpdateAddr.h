#pragma once
#include<Windows.h>
#include<iostream>
#include<string>
#include<vector>
#include<sstream>
using namespace std;
//ɨ������ڴ��жϻ�����
BOOL ScanProcess(HANDLE HandleProcess, DWORD BeginAddr, DWORD EndAddr, char* MachineCode, int MachineCodeLen);

//���ֽ�����ת��Ϊ�ַ���
BOOL ByteToChar(BYTE* ByteArray, char* CharArray, int CharArrayLen);

//�Ƚ���������ַ����Ļ������Ƿ����
BOOL CmpMachineStr(char* TempReadMachineCodeStr, char* MachineCodeStr, int MachineCodeStrLen);
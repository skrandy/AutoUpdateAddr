#pragma once
#include<Windows.h>
#include<iostream>
#include<string>
#include<vector>
#include<sstream>
using namespace std;
//扫描进行内存判断机器码
BOOL ScanProcess(HANDLE HandleProcess, DWORD BeginAddr, DWORD EndAddr, char* MachineCode, int MachineCodeLen);

//将字节数组转换为字符串
BOOL ByteToChar(BYTE* ByteArray, char* CharArray, int CharArrayLen);

//比较两个变成字符串的机器码是否相等
BOOL CmpMachineStr(char* TempReadMachineCodeStr, char* MachineCodeStr, int MachineCodeStrLen);
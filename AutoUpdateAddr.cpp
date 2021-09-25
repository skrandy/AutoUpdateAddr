#include"AutoUpdateAddr.h"
BOOL ReadCodeFile()
{
	FILE* fp;
	errno_t  errnoFile = fopen_s(&fp, "HardCode.txt", "r");
	if (errnoFile != 0)
		return FALSE;
	char* CodeBuff = new char[0x100]{ 0 };
	fgets(CodeBuff, 0x100, fp);


	fclose(fp);
	delete[]CodeBuff;
	return TRUE;
}
BOOL ReadStrFile(vector<string>& AllFileStr)
{
	FILE* fp;
	errno_t  errnoFile = fopen_s(&fp, "HardCode.txt", "r");
	if (errnoFile != 0)
		return FALSE;//判断文件是否打开成功


	char* TempStrBuff = new char[0x100]{ 0 };
	while (!feof(fp))
	{
		memset(TempStrBuff, 0, 0x100);
		fgets(TempStrBuff, 0x100, fp);
		for (int i = 0; i < 0x100; i++)
		{
			if (TempStrBuff[i] == '\n')
				TempStrBuff[i] = '\0';
		}
		string TempCodeStr = TempStrBuff;
		AllFileStr.push_back(TempCodeStr);
	}



	fclose(fp);
	delete[]TempStrBuff;
	return TRUE;
}
void split(vector<vector<string>>& vc,vector<string>CodeStr, const char Flag = ',')
{
	for (int i = 0; i < CodeStr.size(); i++)
	{
		istringstream is(CodeStr[i]);//把string变成istringstream的输入流
		string temp;
		vector<string> TempVC;
		while (getline(is, temp, Flag))
		{
			TempVC.push_back(temp);
		}
		vc.push_back(TempVC);
	}


}
BOOL CmpMachineStr(char* TempReadMachineCodeStr, char* MachineCodeStr, int MachineCodeStrLen)
{
	//	TempReadMachineCodeStr 读取的机器码字符串
	// MachineCodeStr	特征机器码字符串
	//MachineCodeStrLen特征机器码字符串长度
	for (int i = 0; i < MachineCodeStrLen; i++)
	{
		if (MachineCodeStr[i] == '?')
			continue;
		if (TempReadMachineCodeStr[i] != MachineCodeStr[i])
			return FALSE;
	}
	return TRUE;
}
BOOL ByteToChar(BYTE* ByteArray, char* CharArray, int ByteLen)
{
	//ByteArray是字节数组
	//CharArray是字符数组
	//ByteLen  是字节数组长度
	for (int i = 0; i < ByteLen; i++)
	{
		wsprintfA(&CharArray[i * 2], "%02X", ByteArray[i]);
	}


	return TRUE;
}
void SaveData(vector<DWORD> Data)
{
	FILE* fp;
	errno_t  errnoFile = fopen_s(&fp, "Data.txt", "w");
	if (errnoFile != 0)
	{
		cout << "保存文件失败" << endl;
		return;//判断文件是否打开成功
	}
	char temp[10];
	for (int i = 0; i < Data.size(); i++)
	{
		_itoa_s(Data[0], temp, 10, 16);
		fputs(temp,fp);
	}
	fclose(fp);
}
BOOL ScanProcess(HANDLE HandleProcess, DWORD BeginAddr, DWORD EndAddr, char* MachineCodeStr, int MachineCodeStrLen,DWORD& BaseAddr)
{
	//HandleProcess是进程的句柄，BeginAddr是起始内存地址，EndAddr是结束内存地址,MachineCode是机器码的字符串表达形式
	//MachineCodeLen是机器码字符串长度。
	int Flag = 0;

	//每次读取0x1000个机器码的内容进行比较。
	BYTE TempReadMachineCode[0x1000] = { 0 };
	for (DWORD TempBeginAddr = BeginAddr; TempBeginAddr < EndAddr - 0x1000; TempBeginAddr += (0x1000 - MachineCodeStrLen))
	{
		//将机器码缓冲区用0填充
		memset(TempReadMachineCode, 0x0, 0x1000);
		//读0x1000个机器码到byte缓冲数组里。
		BOOL RetReadProcessMemory = ReadProcessMemory(HandleProcess, (LPVOID)TempBeginAddr, TempReadMachineCode, 0x1000, NULL);
		if (RetReadProcessMemory == 0)
			continue;

		//把byte字节数组转换成字符串
		char TempReadMachineCodeStr[0x2001] = { 0 };
		ByteToChar(TempReadMachineCode, TempReadMachineCodeStr, 0x1000);

		//开始比较
		for (int i = 0; i < 0x2001 - MachineCodeStrLen; i++)
		{
			BOOL ret = CmpMachineStr(TempReadMachineCodeStr + i, MachineCodeStr, MachineCodeStrLen);
			if (ret == TRUE)
			{
				//cout << "找到了地址为";
				//printf("%X\n", TempBeginAddr + i / 2);
				BaseAddr = TempBeginAddr + i / 2;
				Flag = 1;
			}
		}
	}
	if (Flag == 0)
		cout << "未找到" << endl;

	return TRUE;
}
int main()
{
	HANDLE HandleProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, 1144);
	if (!HandleProcess)
	{
		cout << "打开进程失败" << endl;
		return 0;
	}

	vector<string> AllFileStr;
	//读取文件中的字符串
	ReadStrFile(AllFileStr);

	vector<DWORD> EndData;
	//分割文件中的字符串
	vector<vector<string>> AllFileStrToPartition;
	split(AllFileStrToPartition, AllFileStr);
	for (int i = 0; i < AllFileStrToPartition.size(); i++)
	{
		DWORD BaseAddr = 0;
		DWORD DataBaseAddr = 0;
		DWORD Context = 0;
		DWORD ReadData = 0;
		string TempCodeAddr = AllFileStrToPartition[i][0];
		int Num = atoi(AllFileStrToPartition[i][1].c_str());
		string Symbol = AllFileStrToPartition[i][2];
		ScanProcess(HandleProcess, 0x00000000, 0x7FFFFFFF, (char*)TempCodeAddr.c_str(), strlen(TempCodeAddr.c_str()), BaseAddr);
		//printf("%X:",BaseAddr);
		//cout << Symbol << " " << Num << endl;
		if (Symbol == "+")
		{
			DataBaseAddr = BaseAddr + Num;
		}
		else
		{
			DataBaseAddr = BaseAddr - Num;
		}
		//读取内容
		BOOL retReadRealData = ReadProcessMemory(HandleProcess, (LPVOID)DataBaseAddr, &ReadData, 4, NULL);
		if (retReadRealData == FALSE)
		{
			cout << "读取实际内容失败" << endl;
			return 0;
		}
		printf("%X\n", ReadData);
		EndData.push_back(ReadData);
	}
	SaveData(EndData);
	CloseHandle(HandleProcess);
	return 0;
}
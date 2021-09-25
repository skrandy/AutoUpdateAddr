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
		return FALSE;//�ж��ļ��Ƿ�򿪳ɹ�


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
		istringstream is(CodeStr[i]);//��string���istringstream��������
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
	//	TempReadMachineCodeStr ��ȡ�Ļ������ַ���
	// MachineCodeStr	�����������ַ���
	//MachineCodeStrLen�����������ַ�������
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
	//ByteArray���ֽ�����
	//CharArray���ַ�����
	//ByteLen  ���ֽ����鳤��
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
		cout << "�����ļ�ʧ��" << endl;
		return;//�ж��ļ��Ƿ�򿪳ɹ�
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
	//HandleProcess�ǽ��̵ľ����BeginAddr����ʼ�ڴ��ַ��EndAddr�ǽ����ڴ��ַ,MachineCode�ǻ�������ַ��������ʽ
	//MachineCodeLen�ǻ������ַ������ȡ�
	int Flag = 0;

	//ÿ�ζ�ȡ0x1000������������ݽ��бȽϡ�
	BYTE TempReadMachineCode[0x1000] = { 0 };
	for (DWORD TempBeginAddr = BeginAddr; TempBeginAddr < EndAddr - 0x1000; TempBeginAddr += (0x1000 - MachineCodeStrLen))
	{
		//�������뻺������0���
		memset(TempReadMachineCode, 0x0, 0x1000);
		//��0x1000�������뵽byte���������
		BOOL RetReadProcessMemory = ReadProcessMemory(HandleProcess, (LPVOID)TempBeginAddr, TempReadMachineCode, 0x1000, NULL);
		if (RetReadProcessMemory == 0)
			continue;

		//��byte�ֽ�����ת�����ַ���
		char TempReadMachineCodeStr[0x2001] = { 0 };
		ByteToChar(TempReadMachineCode, TempReadMachineCodeStr, 0x1000);

		//��ʼ�Ƚ�
		for (int i = 0; i < 0x2001 - MachineCodeStrLen; i++)
		{
			BOOL ret = CmpMachineStr(TempReadMachineCodeStr + i, MachineCodeStr, MachineCodeStrLen);
			if (ret == TRUE)
			{
				//cout << "�ҵ��˵�ַΪ";
				//printf("%X\n", TempBeginAddr + i / 2);
				BaseAddr = TempBeginAddr + i / 2;
				Flag = 1;
			}
		}
	}
	if (Flag == 0)
		cout << "δ�ҵ�" << endl;

	return TRUE;
}
int main()
{
	HANDLE HandleProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, 1144);
	if (!HandleProcess)
	{
		cout << "�򿪽���ʧ��" << endl;
		return 0;
	}

	vector<string> AllFileStr;
	//��ȡ�ļ��е��ַ���
	ReadStrFile(AllFileStr);

	vector<DWORD> EndData;
	//�ָ��ļ��е��ַ���
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
		//��ȡ����
		BOOL retReadRealData = ReadProcessMemory(HandleProcess, (LPVOID)DataBaseAddr, &ReadData, 4, NULL);
		if (retReadRealData == FALSE)
		{
			cout << "��ȡʵ������ʧ��" << endl;
			return 0;
		}
		printf("%X\n", ReadData);
		EndData.push_back(ReadData);
	}
	SaveData(EndData);
	CloseHandle(HandleProcess);
	return 0;
}
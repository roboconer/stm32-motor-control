//#include "stdafx.h"  
#include "SerialPort.h"  
#include <iostream>  

int main(int argc, CHAR* argv[])
{

	CSerialPort mySerialPort;

	if (!mySerialPort.InitPort(3))
	{
		std::cout << "initPort fail !" << std::endl;
	}
	else
	{
		std::cout << "initPort success !" << std::endl;
	}

	if (!mySerialPort.OpenListenThread())
	{
		std::cout << "OpenListenThread fail !" << std::endl;
	}
	else
	{
		std::cout << "OpenListenThread success !" << std::endl;
	}

	mySerialPort.WriteData((unsigned char*)"1600\r\n", strlen("1600\r\n"));
	int temp;
	std::cin >> temp;
	return 0;
}

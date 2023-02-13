#pragma once
#include <iostream>
#include <map>
namespace Error {

	enum ErrorType {
		InvalidPath, Unsafe_Read, Unsafe_Write, Graphics_Init_Fail, IncorrectMap, DeviceOverWrite
	};


	static std::map<ErrorType, std::string> Error_messages = 
	{
		{Graphics_Init_Fail, "Failed to initialize Graphics Device"},
		{IncorrectMap, "device made inpossible map request"},
		{DeviceOverWrite, "device was over written"}
	};
	static void Error(ErrorType error) {
		std::cout << Error_messages[error];
		exit(1);
	}

	static void Warning(std::string msg) {
		std::cout << msg << "\n";
	}

}

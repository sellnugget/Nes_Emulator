#pragma once
#include <iostream>

namespace Error {

	enum ErrorType {
		InvalidPath, Unsafe_Read, Unsafe_Write, Graphics_Init_Fail, 
	};

	static void Error(ErrorType error) {
		switch (error) {
			case Graphics_Init_Fail:
				std::cout << "Failed to initialize Graphics Device";
				exit(1);
		}
	}

	static void Warning() {

	}

}

#include "Execute.h"

#include <windows.h>
#include <ShellApi.h>
#include <iostream>

	void execute(char* name, char* dir)
	{
		std::cerr << name << std::endl;
		std::cerr << dir << std::endl;
		int err = (int)ShellExecuteA(NULL, "open", name, NULL, NULL, SW_SHOWNORMAL);
		if(err <= 32)
		{
			//SE_ERR_SHARE
			//err
		}
	}
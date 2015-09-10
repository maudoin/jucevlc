#include "Execute.h"

#include <windows.h>
#include <ShellApi.h>
#include <iostream>

	void execute(char* name, char* dir, char* params)
	{
		std::cerr << name << std::endl;
		std::cerr << dir << std::endl;
		/*HINSTANCE err = */ShellExecuteA(NULL, "open", name, params, NULL, SW_SHOWNORMAL);
		//if(err <= 32)
		{
			//SE_ERR_SHARE
			//err
		}
	}

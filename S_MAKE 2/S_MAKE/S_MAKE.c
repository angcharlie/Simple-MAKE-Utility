//Charlie Ang
//CSC 3350 Spring 2016
//May 9, 2016
//Lab 4 S_MAKE
//This program implements a simple version of a MAKE-like utility,
//which is similar to the old-style Microsoft MAKE utility.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include <WinBase.h>

//Function to spawn the cmdline and wait for the result
int ExecuteProgram(char *cmdline)
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcInfo;	//Filled in by CreateProcess
	//New Process Creation Flags:
	DWORD CreateFlags = 0;
	BOOL createStatus;
	DWORD waitStatus;
	DWORD exitStatus;

	GetStartupInfo(&StartupInfo);	//Use same as current process

	createStatus =
		CreateProcess(
		NULL,					//lpApplicationName
		cmdline,				//lpCommandLine. Must not have leading whitespace
		NULL,					//lpProcessAttributes Security 
		NULL,					//lpThreadAttributes Security
		TRUE,					//bInheritHandles
		CreateFlags,			//dwCreationFlags
		NULL,					//lpEnvironment
		NULL,					//lpCurrentDirectory
		&StartupInfo,			//lpStartupInfo
		&ProcInfo);				//lpProcessInformation

	if (createStatus == 0)
	{
		return (-1);			//early exit if CreateProcess fails
	}

	//Wait for the spawned process to finish
	waitStatus = WaitForSingleObject(ProcInfo.hProcess, INFINITE);

	if (waitStatus == WAIT_FAILED)
	{
		printf("Wait failed\n");
	}

	//Retrieve exit code from the cmdline process
	GetExitCodeProcess(ProcInfo.hProcess, &exitStatus);

	CloseHandle(ProcInfo.hProcess);
	CloseHandle(ProcInfo.hThread);

	return ((int)exitStatus);
}

///*
int main(int argc, char *argv[])
{
	FILE *filein;					//pointer to the input file

	int i;
	int exitCode = 0;				//if commandline successfully executed
	int commandlineSpaces = 0;		//keeps track of number of spaces in commandline
	int compareFileTime;			//comparing file time 
	char buff[256];					//buffer array for storing each line from file
	char target[256];				//stores target
	char dependent[256];			//stores dependents
	char commandLine[256];			//stores commandline arguments 
	char *tok;						//pointer to individual tokens 
	char *p;						//pointer grabbing next line 

	HANDLE targetFile;				//handle for target file
	HANDLE dependentFile;			//handle for dependent file

	FILETIME targetFileTime;		//filetime for target file
	FILETIME dependentFileTime;		//filetime for dependent file

	BOOL outOfDate = FALSE;			//bool to see if target/dependent is out of date 

	for (int i = 1; i < argc; i++)	//Find first filename on line
	{
		if (_stricmp(argv[i], "/?") == 0)	//Help message request switch 
		{
			printf("\nThis program implements a MAKE-like utility.\n");
			return 0;	//exit code 
		}
		else //try and open the file
		{
			if (freopen_s(&filein, argv[i], "r", stdin) == 0)	//r for reading in text mode...file opens 
			{
				//Script file successfully opens
			}
			else
			{
				printf("Error: Invalid filename %s\n", argv[i]);	//print out error message if file cannot be opened 	
			}
		}
	} //end of for loop


	if ((argc == 1))	//no script filename is given, so defaul to file named "PROJECT"
	{
		if (freopen_s(&filein, "PROJECT", "r", stdin) == 0) //if default PROJECT file can be opened 
		{
			fopen_s(&filein, "PROJECT", "r");	//open PROJECT file
		}
		else //"PROJECT can't be opened
		{
			printf("Script filename 'PROJECT' could not be found or opened.");
			exit(1);	//terminate program
		}	
	}
	else //open up script file 
	{
		//Script file successfully opens
		fopen_s(&filein, argv[1], "r");	//open script file

		while ((p = fgets(buff, 256, filein)) != NULL)	//while not EOF, keep grabbing lines
		{
			//Find the target/dependent line
			while ((buff[0] == ' ') || (buff[0] == '\t') || (buff[0] == '\n') || (buff[0] == '\r') || (buff[0] == "#"))	//if target does not start in column one 
			{
				(p = fgets(buff, 256, filein) != NULL);	//grab next line if not target/dependent line
			}

			tok = strtok(buff, ":");	// grabs first target...ends with colon
			strcpy_s(target, 256, tok);	//copies token into target 

			targetFile = CreateFile(target,		//lpFileName
				GENERIC_READ,					//dwDesiredAccess
				0,								//dwShareMode
				NULL,							//lpSecurityAttributes
				OPEN_EXISTING,					//dwCreatioDisposition
				FILE_ATTRIBUTE_NORMAL,			//dwFlagsAndAttributes
				NULL);

			if (targetFile == (HANDLE)0xFFFFFFFF)	//if target file does not exist...then it is out of date
			{
				outOfDate = TRUE;
			}

			//************after grabbing target***************************

			while (tok != NULL)	//while there are dependents
			{
				tok = strtok(NULL, " \n\r\t");	// grabs dependents
				if (tok != NULL)	//there is a dependent 
				{
					strcpy_s(dependent, 256, tok);	//copies the token into dependent string

					dependentFile = CreateFile(dependent,		//lpFileName
						GENERIC_READ,							//dwDesiredAccess
						0,										//dwShareMode
						NULL,									//lpSecurityAttributes
						OPEN_EXISTING,							//dwCreatioDisposition
						FILE_ATTRIBUTE_NORMAL,					//dwFlagsAndAttributes
						NULL);

					if (dependentFile == (HANDLE)0xFFFFFFFF)	//if dependent file does not exist...then terminate program 
					{
						printf("Invalid dependent file: %s\n", dependent);
						exit(1);	//terminate program since dependent file has to exist 
					}


					//GETS TARGET FILETIME
					GetFileTime(targetFile,
						NULL,						//Creation time
						NULL,						//Last Access time
						&targetFileTime);			//Last write time
					
					//GETS DEPENDENT FILETIME
					GetFileTime(dependent,
						NULL,						//Creation time
						NULL,						//Last Access time
						&dependentFileTime);		//Last write time

					
					compareFileTime = (CompareFileTime(&targetFileTime, &dependentFileTime)); //-1 first file time is earlier, 0 same, 1 first file time later
					if (compareFileTime == -1)	//if taget file is ealier 
					{
						outOfDate = TRUE;
					}

				} //at this point, there are no more dependents
				else  //tok == null...so there are no dependents ...so it is out of date 
				{
					outOfDate = TRUE;
				}
			} // if skips over while loop to here...then there are no dependents...or there are no more dependents  

			if (outOfDate == TRUE)	//execute all following command lines 
			{
				//grabs following line in file 
				p = fgets(commandLine, 256, filein);	//puts next line in commandline array
				//while not EOF and exitCode is still 0 and starts with at least one whitespace
				while ((p != NULL) && (exitCode == 0) && (((commandLine[0] == ' ') || (commandLine[0] == '\t') || (commandLine[0] == '\r'))))
				{
					commandlineSpaces = 0;	//starts off at 0
					for (int i = 0; i < strlen(commandLine); i++)	//iterate through commandline array until there isn't a space
					{
						if ((commandLine[i] == ' ') || (commandLine[i] == '\t') || (commandLine[i] == '\r'))
						{
							commandlineSpaces++;
						}
						else
						{
							break; //commandlineSpaces will keep keep track of index where commandline begins 
						}
					}
					//strcpy_s(commandLine, 256, &commandLine[commandlineSpaces]);
					exitCode = ExecuteProgram(&commandLine[commandlineSpaces]);	//execute commandline

					p = fgets(commandLine, 256, filein);	//grabs next line (could be command or could be something else)
				}
				outOfDate = FALSE;	//set back to false 
			}
			CloseHandle(targetFile);	//close handle 
			CloseHandle(dependentFile); //close handle

		} //after reaching here, EOF is reached 
	} //after going through script file

	fclose(filein);	//close script file 
	return 0;
}
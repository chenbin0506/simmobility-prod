#include "logger.h"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>

#define LOG_FRAME_LENGTH        (1024)
#define LOG_HEADER_LENGTH       (12)
#define LOG_DATA_LENGTH         ((LOG_FRAME_LENGTH)-(LOG_HEADER_LENGTH))

enum txtColor { DARKBLUE = 1, DARKGREEN, DARKTEAL, DARKRED, DARKPINK, DARKYELLOW, GRAY, DARKGRAY, BLUE, GREEN, TEAL, RED, PINK, YELLOW, WHITE };

void CpuFmtLog(int eLevel, const int eTaskID,
	const char *strFileName,
	int iLineNum,
	char *strFormat,
	...)
{
	char	buff[LOG_DATA_LENGTH];
	int		iLen;

	//char	szFileName[256];  // ƒtƒ@ƒCƒ‹–¼‚ª“ü‚é
	char	szDrive[8];
	char	szDir[256];
	char	szFname[256];
	char	szExt[8];

//	splitpath(strFileName, szDrive, szDir, szFname, szExt);
    char *szFileName = basename((char*)strFileName);
	//strcpy(szFileName, szFname);
	//strcat(szFileName, szExt);

	char str_level[64];

	switch (eLevel) {
		//	case 	fsr::logger::CLogManager::LEVEL_ANALYSIS:
	case 	LOG_ANALYSIS:
		strcpy(str_level, "ANALYSIS");
		break;
		//	case  fsr::logger::CLogManager::LEVEL_FATAL	:
	case  LOG_FATAL:
		strcpy(str_level, "FATAL");
		break;
		//	case fsr::logger::CLogManager::LEVEL_ERROR:
	case LOG_ERROR:
		strcpy(str_level, "ERROR");
		break;
		//	case fsr::logger::CLogManager::LEVEL_WARN:
	case LOG_WARN:
		strcpy(str_level, "WARN");
		break;
		//	case fsr::logger::CLogManager::LEVEL_INFO:
	case LOG_INFO:
		strcpy(str_level, "INFO");
		break;
		//	case fsr::logger::CLogManager::LEVEL_DEBUG:
	case LOG_DEBUG:
		strcpy(str_level, "DEBUG");
		break;
	default:
		strcpy(str_level, "UNKOWN");
		break;
	}

	iLen = sprintf(buff, "%s, %s, %d, ", str_level, szFileName, iLineNum);

	va_list argp;
	va_start(argp, strFormat);
	vsnprintf(&(buff[iLen]), LOG_DATA_LENGTH - iLen - 1, strFormat, argp);
	va_end(argp);

	std::cout << buff;
	std::cout << std::endl;

}

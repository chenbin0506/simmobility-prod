#ifndef _CPU_FMT_LOG_H_
#define _CPU_FMT_LOG_H_

#include <string.h>
enum {
	LOG_ANALYSIS	= 256,
	LOG_FATAL		= 16,
	LOG_ERROR		= 8,
	LOG_WARN		= 4,
	LOG_INFO		= 2,
	LOG_DEBUG		= 1,
};

void CpuFmtLog(const int eLevel, const int eTaskID,
				const char *strFileName,
				int iLineNum,
				char *strFormat,
				...);

#define LogDebug(args,...)    CpuFmtLog(LOG_DEBUG,0,__FILE__,__LINE__,args,##__VA_ARGS__)
#define LogInfo(args,...)     CpuFmtLog(LOG_INFO,0,__FILE__,__LINE__,args,##__VA_ARGS__)
#define LogError(args,...)    CpuFmtLog(LOG_ERROR,0,__FILE__,__LINE__,args,##__VA_ARGS__)
#define LogWarning(args,...)  CpuFmtLog(LOG_WARN,0,__FILE__,__LINE__,args,##__VA_ARGS__)

#endif //_CPU_FMT_LOG_H_
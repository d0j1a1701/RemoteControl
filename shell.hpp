#include "windows.h"
#include <cstring>
#include <string>

class Shell {
	public:
		Shell(void);
		~Shell(void);

		bool createProcess(const LPSTR process);
		bool terminateProcess(void);
		bool read(const std::string &endStr, int timeout, std::string &outstr );//获取输出字符串
		bool read(std::string &outstr );//获取输出字符串
		bool write(const std::string &cmd);//执行命令
	private:
		HANDLE m_hChildInputWrite;	//用于重定向子进程输入的句柄
		HANDLE m_hChildInputRead;
		HANDLE m_hChildOutputWrite;	//用于重定向子进程输出的句柄
		HANDLE m_hChildOutputRead;
		PROCESS_INFORMATION m_cmdPI;//cmd进程信息
};

Shell::Shell(void) {
	m_hChildInputWrite = NULL;
	m_hChildInputRead = NULL;
	m_hChildOutputWrite = NULL;
	m_hChildOutputRead = NULL;
	ZeroMemory(&m_cmdPI, sizeof(m_cmdPI));
}

Shell::~Shell(void) {
	terminateProcess();
}

inline bool Shell::createProcess(const LPSTR process) {
	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof(sa);

	//创建子进程输出匿名管道
	if(::CreatePipe(&m_hChildOutputRead, &m_hChildOutputWrite, &sa, 0) == FALSE)
		return true;

	//创建子进程输入匿名管道
	if(::CreatePipe(&m_hChildInputRead, &m_hChildInputWrite, &sa, 0) == FALSE) {
		::CloseHandle(m_hChildOutputWrite);
		::CloseHandle(m_hChildOutputRead);
		::CloseHandle(m_hChildOutputWrite);
		::CloseHandle(m_hChildOutputRead);
		return true;
	}

	ZeroMemory(&m_cmdPI, sizeof(m_cmdPI));
	STARTUPINFO si;
	GetStartupInfo(&si);

	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.hStdInput   = m_hChildInputRead;     //重定向子进程输入
	si.hStdOutput  = m_hChildOutputWrite;   //重定向子进程输入
	si.hStdError   = m_hChildOutputWrite;

	if(::CreateProcess(NULL, process, NULL, NULL, TRUE, 0, NULL, NULL, &si, &m_cmdPI) == FALSE) {
		::CloseHandle(m_hChildInputWrite);
		::CloseHandle(m_hChildInputRead);
		::CloseHandle(m_hChildOutputWrite);
		::CloseHandle(m_hChildOutputRead);
		m_hChildInputWrite = NULL;
		m_hChildInputRead  = NULL;
		m_hChildOutputWrite = NULL;
		m_hChildOutputRead = NULL;
		ZeroMemory(&m_cmdPI, sizeof(m_cmdPI));
		return true;
	}

	return false;
}

inline bool Shell::terminateProcess( void ) {
	::CloseHandle(m_hChildInputWrite);
	::CloseHandle(m_hChildInputRead);
	::CloseHandle(m_hChildOutputWrite);
	::CloseHandle(m_hChildOutputRead);
	m_hChildInputWrite = NULL;
	m_hChildInputRead  = NULL;
	m_hChildOutputWrite = NULL;
	m_hChildOutputRead = NULL;
	::TerminateProcess(m_cmdPI.hProcess, -1);
	::CloseHandle(m_cmdPI.hProcess);
	::CloseHandle(m_cmdPI.hThread);
	ZeroMemory(&m_cmdPI, sizeof(m_cmdPI));
	return false;
}

inline bool Shell::read(const std::string &endStr, int timeout, std::string &outstr) {
	if( NULL == m_hChildOutputRead )
		return false;

	outstr = "";
	char buffer[4096] = {0};
	DWORD readBytes = 0;
	while( timeout > 0 ) {
		//对管道数据进行读，但不会删除管道里的数据，如果没有数据，就立即返回
		if( FALSE == PeekNamedPipe( m_hChildOutputRead, buffer, sizeof(buffer) - 1, &readBytes, 0, NULL ) )
			return false;

		//检测是否读到数据，如果没有数据，继续等待
		if( 0 == readBytes ) {
			::Sleep(200);
			timeout -= 200;
			continue;
		}

		readBytes = 0;
		if( ::ReadFile( m_hChildOutputRead, buffer, sizeof(buffer) - 1, &readBytes, NULL) ) {
			outstr.insert( outstr.end(), buffer, buffer + readBytes );
			size_t pos = outstr.rfind(endStr);
			if(std::string::npos == pos )
				continue;
			if( pos == outstr.size() - endStr.size() ) {
				return true;//找到数据
			}
		} else
			return false;
	}

	return false;
}

inline bool Shell::read(std::string &outstr) {
	if( NULL == m_hChildOutputRead )
		return false;

	outstr = "";
	char buffer[4096] = {0};
	DWORD readBytes = 0;
	//对管道数据进行读，但不会删除管道里的数据，如果没有数据，就立即返回
	if( FALSE == PeekNamedPipe( m_hChildOutputRead, buffer, sizeof(buffer) - 1, &readBytes, 0, NULL ) )
		return false;

	//检测是否读到数据，如果没有数据，继续等待
	if( 0 == readBytes )
		return false;

	readBytes = 0;
	if( ::ReadFile( m_hChildOutputRead, buffer, sizeof(buffer) - 1, &readBytes, NULL) ) {
		outstr.insert( outstr.end(), buffer, buffer + readBytes );
		return true;
	}
	return false;
}

inline bool Shell::write( const std::string &cmd ) {
	if(m_hChildInputWrite == NULL)
		return "";
	std::string tmp = cmd + "\r\n";
	DWORD writeBytes = 0;
	if(::WriteFile( m_hChildInputWrite, tmp.c_str(), tmp.size(), &writeBytes, NULL ) == FALSE)
		return false;
	return true;
}
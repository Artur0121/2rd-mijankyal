#include "UDPSocket.h"
#include <iostream>
#include <stdio.h>

#include <Windows.h> 
typedef HANDLE THREADVAR;
typedef DWORD WINAPI THREADFUNCVAR;
typedef LPVOID THREADFUNCARGS;
typedef THREADFUNCVAR(*THREADFUNC)(THREADFUNCARGS);
typedef CRITICAL_SECTION THREAD_LOCK;



THREADVAR SpawnThread(THREADFUNC f, THREADFUNCARGS arg);
void StopThread(THREADVAR t);
void InitThreadLock(THREAD_LOCK& t);
void LockThread(THREAD_LOCK& t);
void UnlockThread(THREAD_LOCK& t);
void sleep(int ms);




THREADFUNCVAR MyAsyncThread(THREADFUNCARGS lpParam);


char buf[BUFLEN];
char message[BUFLEN];

unsigned short srv_port = 0;
char srv_ip_addr[40];


UDPSocket client_sock;




THREADVAR SpawnThread(THREADFUNC f, THREADFUNCARGS arg)
{
	DWORD thrId;
	THREADVAR out = CreateThread(
		NULL,          
		0,             
		(LPTHREAD_START_ROUTINE)f,    			
		arg,           
		0,        
		&thrId			 
	);
	return out;
}

void StopThread(THREADVAR t) {
#ifdef _COMPILE_LINUX
	pthread_exit((void*)t);
#endif
#ifdef _COMPILE_WINDOWS
	TerminateThread(t, 0);
	CloseHandle(t);
#endif
}


void InitThreadLock(THREAD_LOCK& t) {
#ifdef _COMPILE_LINUX
	t = PTHREAD_MUTEX_INITIALIZER;
#endif
#ifdef _COMPILE_WINDOWS
	InitializeCriticalSection(&t);
#endif
}

void LockThread(THREAD_LOCK& t) {
#ifdef _COMPILE_LINUX
	pthread_mutex_lock(&t);
#endif
#ifdef _COMPILE_WINDOWS
	EnterCriticalSection(&t);
#endif
}

void UnlockThread(THREAD_LOCK& t) {
#ifdef _COMPILE_LINUX
	pthread_mutex_unlock(&t);
#endif
#ifdef _COMPILE_WINDOWS
	LeaveCriticalSection(&t);
#endif
}


void sleep(int ms) {
#ifdef _COMPILE_LINUX
	usleep(ms * 1000);   
#endif
#ifdef _COMPILE_WINDOWS
	Sleep(ms);
#endif
}


int main(int argc, char* argv[])
{
	
	struct sockaddr_in si_other;
	int slen = sizeof(si_other);

	
	memset((char*)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;


	if (1 == argc)
	{
		si_other.sin_port = htons(PORT);
		si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);
		printf("1: Server - addr=%s , port=%d\n", SERVER, PORT);
	}
	else if (2 == argc)
	{
		si_other.sin_port = htons(atoi(argv[1]));
		si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);
		printf("2: argv[0]: Server - addr=%s , port=%d\n", SERVER, atoi(argv[1]));
	}
	else
	{
		si_other.sin_port = htons(atoi(argv[2]));
		si_other.sin_addr.S_un.S_addr = inet_addr(argv[1]);
		printf("3: Server - addr=%s , port=%d\n", argv[1], atoi(argv[2]));
	}

	

	THREAD_LOCK recv_lock;
	InitThreadLock(recv_lock);

	
	DWORD_PTR* svRecvThrArgs = new DWORD_PTR[1];
	
	svRecvThrArgs[0] = (DWORD_PTR)&recv_lock;

	
	THREADVAR recvThrHandle = SpawnThread(MyAsyncThread, (THREADFUNCARGS)svRecvThrArgs);
	while (1)
	{
		memset(buf, '\0', BUFLEN);
		LockThread(recv_lock); 
		client_sock.RecvDatagram(buf, BUFLEN, (struct sockaddr*)&si_other, &slen);
		puts(buf);
		UnlockThread(recv_lock); 
	}
	StopThread(recvThrHandle);
	return 0;
}

THREADFUNCVAR MyAsyncThread(THREADFUNCARGS lpParam) {
	struct sockaddr_in si_other;
	int slen = sizeof(si_other);
	memset((char*)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;

	si_other.sin_port = htons(PORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);

	
	DWORD_PTR* arg = (DWORD_PTR*)lpParam;
	THREAD_LOCK& ref_recv_lock = *((THREAD_LOCK*)arg[0]);

	
	while (1) {
		LockThread(ref_recv_lock);  
		printf("\nEnter message : ");
		gets_s(message, BUFLEN);
		client_sock.SendDatagram(message, (int)strlen(message), (struct sockaddr*)&si_other, slen);
		UnlockThread(ref_recv_lock); 
	}
	return NULL;
}

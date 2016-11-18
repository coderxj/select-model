#include "Client.h"
#include <iostream>
#include <process.h>
#include <vector>
FD_SET g_ClientSocket;   //client socket 集合

vector<Client> g_vClient;  //管理客户端信息向量 

/*
@function OpenTCPServer             打开TCP服务器
@param  _Out_ SOCKET* sServer       客户端套接字
@param _In_ unsigned short Port     服务器端口
@param  _Out_ DWORD* dwError               错误代码
@return  成功返回TRUE 失败返回FALSE
*/
BOOL OpenTCPServer(_Out_ SOCKET* sServer, _In_ unsigned short Port, _Out_ DWORD* dwError)
{
	BOOL bRet = FALSE;
	WSADATA wsaData = { 0 };
	SOCKADDR_IN ServerAddr = { 0 };
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	do
	{
		if (!WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			if (LOBYTE(wsaData.wVersion) == 2 || HIBYTE(wsaData.wVersion) == 2)
			{
				*sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (*sServer != INVALID_SOCKET)
				{
					if (SOCKET_ERROR != bind(*sServer, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)))
					{
						if (SOCKET_ERROR != listen(*sServer, SOMAXCONN))
						{
							bRet = TRUE;
							break;
						}
						*dwError = WSAGetLastError();
						closesocket(*sServer);
					}
					*dwError = WSAGetLastError();
					closesocket(*sServer);
				}
				*dwError = WSAGetLastError();
			}
			*dwError = WSAGetLastError();
		
		}
		*dwError = WSAGetLastError();
	} while (FALSE);
	return bRet;
}

//接受客户端请求
BOOL AcceptClient(SOCKET sServer)
{
	FD_ZERO(&g_ClientSocket);   //初始化FD_SET
	SOCKADDR_IN addrClient = { 0 };
	int iaddrLen = sizeof(addrClient);
	int iclientNum = 0;
	while (iclientNum++ < FD_SETSIZE) //这里FD_SETSIZE是64，如果想增大SIZE的话，可以定义一个FD_SET数组
	{
		SOCKET clientSocket = accept(sServer, (SOCKADDR*)&addrClient, &iaddrLen);
		if(clientSocket == INVALID_SOCKET)
			continue;
		FD_SET(clientSocket, &g_ClientSocket);  //把新的client添加到集合中
		
		pClientInfo pclientInfo = new ClientInfo;                //保存客户端信息
		pclientInfo->sClient = clientSocket;
		pclientInfo->usPort = addrClient.sin_port;
		memcpy(pclientInfo->szIP ,inet_ntoa(addrClient.sin_addr),sizeof(pclientInfo->szIP));
		Client client;                          //设置客户端信息  
		client.SetClientInfo(pclientInfo); 
		g_vClient.push_back(client);            //添加到向量表
	}
	return TRUE;
}

//根据socket查找用户
int FindClient(SOCKET s)
{
	for (int i =0;i<g_vClient.size();i++)
	{
		if (g_vClient[i].GetSocket() == s)
			return i;
	}
	return -1;
}

//根据name查找用户
SOCKET FindClient(string name)
{
	for (int i = 0;i < g_vClient.size();i++)
	{
		if (g_vClient[i].GetUserName() == name)
			return g_vClient[i].GetSocket();
	}
	return INVALID_SOCKET;
}

//接受客户端请求线程
unsigned int __stdcall ThreadAccept(void* lparam)
{
	AcceptClient(*(SOCKET*)lparam);   
	return 0;
}

//发送数据线程
unsigned int __stdcall ThreadSend(void* lparam)
{
	g_vClient[*(int*)lparam].SendPacket();
	return 0;
}

//接收数据线程
unsigned int __stdcall ThreadRecv(void* lparam)
{
	FD_SET fdRead;      //可读socket集合
	FD_ZERO(&fdRead);   //初始化可读socket集合
	int iRet = 0;
	char* buf = new char[PACKETSIZE];  //分配空间
	TIMEVAL tvl = { 0 };
	tvl.tv_sec = 2;  //只等待2秒  
	while (true)
	{
		fdRead = g_ClientSocket;     //如果这里fdRead中，fd_count=2,等select后，fd_count就等于1了，表示只有一个可读
		if (fdRead.fd_count == 0)
		{
			Sleep(500);
			continue;
		}	        //这里不能设为无限等待,因为一旦阻塞在原状态，如果有新的client加进来，就不能及时更新fdRead。
		iRet = select(0, &fdRead, NULL, NULL, &tvl);  //返回准备就绪的描述符数，若超时则返回0，若出错则返回-1
		if (iRet != SOCKET_ERROR)
		{
			for (int i = 0; i < g_ClientSocket.fd_count; i++) //遍历所有socket
			{
				if (FD_ISSET(g_ClientSocket.fd_array[i], &fdRead))  //如果socket在可读socket内，则可以进行读取
				{
					memset(buf, 0, PACKETSIZE);
					iRet = recv(g_ClientSocket.fd_array[i], buf, PACKETSIZE, 0);
					if (iRet == SOCKET_ERROR)
					{
						closesocket(g_ClientSocket.fd_array[i]);
						FD_CLR(g_ClientSocket.fd_array[i], &g_ClientSocket);
					}
					else  //有数据，处理
					{
						string str = buf;
						if (str.size() > 0)     //空buf忽略掉，不处理
						{
							int iClient = FindClient(g_ClientSocket.fd_array[i]);  //找到相应的用户
							if (iClient == -1)
								break;
							if (str[0] == '#')  //用户名
							{
								g_vClient[iClient].SetUserName(&buf[1]);
								cout << &buf[1] <<" is online.IP:"<< g_vClient[iClient] .GetIP()<<",Port:"<< g_vClient[iClient].GetPort()<< endl;
							}		
							else if(str[0] == '@') //chat name
								g_vClient[iClient].SetChatName(&buf[1]);
							else  //正常数据，打包，发送
							{
								Packet packet;
								packet.sClient = FindClient(g_vClient[iClient].GetChatName());
								memcpy(packet.szUserName, g_vClient[iClient].GetUserName().c_str(), sizeof(packet.szUserName));
								memcpy(packet.szChatName, g_vClient[iClient].GetChatName().c_str(), sizeof(packet.szChatName));
								memcpy(packet.szMessage, buf, MAXSIZE);
								g_vClient[iClient].SetPacket(packet);   //设置数据包
								_beginthreadex(NULL, 0, ThreadSend, &iClient, 0, NULL); //发送数据
							}
						}
					}
				}
			}
		}
	}

	if (buf)
		delete buf;
	return 0;
}

//client管理线程
unsigned int __stdcall ThreadManager(void* lparam)
{
	while (1)
	{
		vector<Client>::iterator iter = g_vClient.begin();
		while (iter != g_vClient.end())
		{
			if (SOCKET_ERROR == send(iter->GetSocket(), "", sizeof(""), 0))
			{
				cout << iter->GetUserName() << " is downline." << endl;
				FD_CLR(iter->GetSocket(), &g_ClientSocket);  //从集合中删除掉无效套接字
				g_vClient.erase(iter);
				break;
			}
			iter++;
		}
		Sleep(2000);
	}
	return 0;
}

int main()
{
	SOCKET sServer = INVALID_SOCKET;
	DWORD dwError = 0;
	USHORT uPort = 18000;
	if (OpenTCPServer(&sServer, uPort, &dwError)) //打开服务器
	{
		_beginthreadex(NULL, 0, ThreadAccept, &sServer, 0, NULL);  //开启三个线程
		_beginthreadex(NULL, 0, ThreadRecv, NULL, 0, NULL);
		_beginthreadex(NULL, 0, ThreadManager, NULL, 0, NULL);
	}
	Sleep(100000000); //主函数睡眠
	return 0;
}
#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <Winsock2.h>
#include <string>
#pragma comment(lib,"ws2_32.lib")
#define PACKETSIZE 1056 //数据包大小
#define MAXSIZE 1024    //最大发送数据量 
using namespace std;

//client信息结构
typedef struct _tagClientInfo
{
	char szIP[16];               //ip               
	char szUserName[16];         //username
	char szChatName[16];         //want to chat client name
	char szMessage[1024];        //send/recv message
	SOCKET sClient;              //client socket
	unsigned short usPort;       //port 
	bool bStatus;                //client online status
}ClientInfo, *pClientInfo;

//数据包格式
typedef struct _tagPacket  
{
	SOCKET sClient;              //the client who recv message
	char szUserName[16];         //username
	char szChatName[16];         //want to chat client name
	char szMessage[1024];        //send/recv message
}Packet,*pPacket;


class Client
{
public:
	Client();
	~Client();

public:
	void SetClientInfo(pClientInfo pclientInfo);                //set client info
	string GetIP() const;										//get client ip
	string GetUserName() const;									//get username
	string GetChatName() const;									//get chatname
	string GetMessage() const;									//get message
	SOCKET GetSocket() const;									//get client socket
	unsigned short GetPort() const;                             //get client port
	bool GetStatus() const;										//get client online status
	void SetIP(char* szIP);                                     //set client ip
	void SetUserName(char* szUserName);							//set username
	void SetChatName(char* szChatName);							//set chatname
	void SetMessage(char* szMessage);                           //set message
	void SetSocket(SOCKET s);									//set client socket
	void SetPort(unsigned short port);							//set client port
	void SetSatus(BOOL bStatus);								//set client online status
	BOOL SendPacket();											//send packet
	void SetPacket(Packet packet);                             //set packet
	string GetErrorMessage();                                   //get error message

private:
	string JointData(Packet packet);                             //合并包数据准备发送
private:
	string m_sIP;
	string m_sUserName;
	string m_sChatName;
	string m_sMessage;
	string m_sErrorMsg;
	SOCKET m_sClient;
	unsigned short m_usPort;
	bool m_bStatus;
	Packet m_packet;
};

#endif //_CLIENT_H_
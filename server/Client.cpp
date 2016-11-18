#include "Client.h"



Client::Client()
{
	m_bStatus = false;
}


Client::~Client()
{

}

//set client info
void Client::SetClientInfo(pClientInfo pclientInfo)
{
	m_sIP = pclientInfo->szIP;
	m_sUserName = pclientInfo->szUserName;
	m_sChatName = pclientInfo->szChatName;
	m_sMessage = pclientInfo->szMessage;
	m_sClient = pclientInfo->sClient;
	m_usPort = pclientInfo->usPort;
	m_bStatus = true;
}

//get client ip
string Client::GetIP() const
{
	return m_sIP;
}

//get username
string Client::GetUserName() const
{
	return m_sUserName;
}

//get chatname
string Client::GetChatName() const
{
	return m_sChatName;
}

//get message
string Client::GetMessage() const
{
	return m_sMessage;
}

//get client socket
SOCKET Client::GetSocket() const
{
	return m_sClient;
}

//get client port
unsigned short Client::GetPort() const
{
	return m_usPort;
}

//get client online status
bool Client::GetStatus() const
{
	return m_bStatus;
}

//set client ip
void Client::SetIP(char* szIP)
{
	m_sIP = szIP;
}

//set username
void Client::SetUserName(char* szUserName)
{
	m_sUserName = szUserName;
}

//set chatname
void Client::SetChatName(char* szChatName)
{
	m_sChatName = szChatName;
}

//set message
void Client::SetMessage(char* szMessage)
{
	m_sMessage = szMessage;
}

//set client socket
void Client::SetSocket(SOCKET s)
{
	m_sClient = s;
}

//set client port
void Client::SetPort(unsigned short port)
{
	m_usPort = port;
}

//set client online status
void Client::SetSatus(BOOL bStatus)
{
	m_bStatus = bStatus;
}

//send packet
BOOL Client::SendPacket()
{
	string data = JointData(m_packet);  //准备好要发送的数据
	int iTotal = data.size();         //总共要发送的字节数
	int iRemain = iTotal;             //剩余发送的字节
	int iCur = 0;                     //当前发送的字节
	while (iRemain)
	{
		int iRet = send(m_packet.sClient, &data[iCur], iTotal, 0);
		if (iRet == SOCKET_ERROR)
		{
			char buf[64] = { 0 };
			sprintf(buf, "send failed with error code: %d", WSAGetLastError());
			m_sErrorMsg = buf;
			return FALSE;
		}
		iRemain -= iRet;
		iCur = iRet;
	}
	return TRUE;
}

//set packet
void Client::SetPacket(Packet packet)
{
	m_packet = packet;
}
//合并包数据准备发送
string Client::JointData(Packet packet)
{
	string data;
	data = packet.szUserName;
	data += ": ";
	data += packet.szMessage;
	return data;
}

//get error message
string Client::GetErrorMessage()
{
	return m_sErrorMsg;
}
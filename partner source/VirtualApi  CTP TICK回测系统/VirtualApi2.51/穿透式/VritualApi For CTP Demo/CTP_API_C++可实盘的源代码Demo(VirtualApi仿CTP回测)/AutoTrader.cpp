// AutoTrader.cpp : ����Լ�汾,���뾭�͹�˾���룬ʵ���ʺţ����뼴���µ���
//�Զ����涩�ĺ�ԼTICK���ݵ�\Bin\TickData�£��ļ�����Լ����_����.txt	
 

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <time.h>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <stdlib.h>
using namespace std;

#include ".\ThostTraderApi\ThostFtdcTraderApi.h"
#include ".\ThostTraderApi\ThostFtdcMdApi.h"
#include "TraderSpi.h"
#include "MdSpi.h"
#include "Common.h"
#include "DataSniffer.h"
#include "MyTrader.h"

#pragma warning(disable : 4996)
// UserApi����
CThostFtdcTraderApi *pUserApi;
// MdApi����
CThostFtdcMdApi *pMdApi;

// ���ò���
char  FRONT_ADDR_1A[] = "tcp://180.168.146.187:10101";		// ǰ�õ�ַ1����:ʵ��
char  FRONT_ADDR_1B[] = "tcp://180.168.146.187:10111";		// ǰ�õ�ַ2����:ʵ��

char  FRONT_ADDR_2A[] = "tcp://180.168.146.187:10101";		// ǰ�õ�ַ1����:�̺�
char  FRONT_ADDR_2B[] = "tcp://180.168.146.187:10111";		// ǰ�õ�ַ2����:�̺�


TThostFtdcBrokerIDType	BROKER_ID = "9999";								// ʵ�̣����͹�˾
TThostFtdcInvestorIDType INVESTOR_ID = "038997";						// ʵ�̣�Ͷ���ߴ���
TThostFtdcPasswordType  PASSWORD = "000000wdg";		 					// ʵ�̣��û�����
TThostFtdcAuthCodeType	AuthCode="0000000000000000";                    //�ڻ���˾�ṩ����֤��,SINNOWΪ 0000000000000000
TThostFtdcAppIDType	AppID= "simnow_client_test";                        //appid

TThostFtdcInstrumentIDType INSTRUMENT_ID = "rb1810";					// ���׺�Լ����
TThostFtdcDirectionType	DIRECTION;										// ������������
TThostFtdcOffsetFlagType MARKETState;									// ��ƽ��
TThostFtdcPriceType	LIMIT_PRICE;										// ���׼۸�

//char *ppInstrumentID[] = {"IF1910", "rb1910","ag1910", "ru1910", "cu1910", "j1910", "SR1910", "m1910", "y1910", "p1910"};			// ���鶩���б�
//int iInstrumentID = 10;													// ���鶩������

char *ppInstrumentID[] = { "rb1810"};			// ���鶩���б�
int iInstrumentID = 1;							// ���鶩������
bool	ReceiveTick = false;

// ������
int iRequestID = 0;
// ����ʱ��
bool	JustRun = false;	//����������־

TThostFtdcDateExprType	TradingDay;

// User��������

extern	char	*InstrumentID_name;	//
extern	string	Q_BarTime_s;		//ʱ���ַ���
extern	int		Q_BarTime_1;		//ʱ��������
extern	double	Q_BarTime_2;		//ʱ���ʽ0.145100
extern	double	Q_UpperLimit;	//
extern	double	Q_LowerLimit;	//

extern	double	NewPrice;		//
extern	int		FirstVolume;	//ǰһ�γɽ�������

extern	double  Mn_open[3];		// 
extern	double  Mn_high[3];		// 
extern	double  Mn_low[3];		// 
extern	double  Mn_close[3];	// 

extern	double  BuyPrice;		//���ּ�
extern	double  SellPrice;		//���ּ�
extern	int		BNum;			//���ִ���
extern	int		SNum;			//���ִ���

extern	bool	BuySignal;		//
extern	bool	SellSignal;		//

extern	double	BSVolume;		//������

extern	int		TickABS;
extern	double  TickAPrice[4];		//
extern	int		TickBNum;
extern	double  TickBPrice[4];		//

extern	char    LogFilePaths[80];	//

// �Ự����
extern	TThostFtdcFrontIDType	FRONT_ID;	//ǰ�ñ��
extern	TThostFtdcSessionIDType	SESSION_ID;	//�Ự���
extern	TThostFtdcOrderRefType	ORDER_REF;	//��������



void main(void)
{
	//�Ǵ�͸ʽ����API��
	void Erasefiles();
	void Sniffer();
	void Trading();
	bool ReadConfiguration(char *filepaths);
	void WriteConfiguration(char *filepaths);
	
	Erasefiles();
	Sleep(2000);

	cerr << "--->>> " << "Welcom MyAutoTrader System!" << endl;
	cerr << "--->>> " << "Version 1.0.0!" << endl;

	// ��ʼ��MdApi
	pMdApi = CThostFtdcMdApi::CreateFtdcMdApi("./thostmduserapi.dll");
	CThostFtdcMdSpi* pMdSpi = new CMdSpi();
	pMdApi->RegisterSpi(pMdSpi);									// ע���¼���
	pMdApi->RegisterFront(FRONT_ADDR_1B);							// connect		���������ַ
	pMdApi->RegisterFront(FRONT_ADDR_2B);							// connect		���������ַ��1B�Ͽ����Զ�����2B��ַ
	cout << pMdApi->GetApiVersion() << endl;
	pMdApi->Init();
	cout << "--->>> " << "Initialing MdApi" << endl;


	// ��ʼ��UserApi
	pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi("./thosttraderapi.dll");
	CTraderSpi* pUserSpi = new CTraderSpi();
	pUserApi->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi);			// ע���¼���
	pUserApi->SubscribePublicTopic(THOST_TERT_RESTART);				// ע�ṫ����
	pUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);			// ע��˽����
	pUserApi->RegisterFront(FRONT_ADDR_1A);							// connect
	pUserApi->Init();
	cout << pUserApi->GetApiVersion() << endl;
	cout << "--->>> " << "Initialing TradeApi" << endl;




	
	Sleep(6000);
	ReadConfiguration("./AutoTrader.dat");			//�Զ������ݣ���ֲ����ݵȾ���
	cout << "--->>> " << "��ʼ�����!" << endl;
	

	while(1)
	{
			
		//ָ�����,����ֻ�Ǹ�������
		//���Խ����������и��Ӵ���  ��DataSniffer.h


		Sleep(5000);


	}

}




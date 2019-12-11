#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <time.h>
using namespace std;

#include "TraderSpi.h"
#include "MdSpi.h"
#pragma warning(disable : 4996)

// USER_API����
extern CThostFtdcMdApi* pMdApi;

// ���ò���
extern char FRONT_ADDR[];		
extern TThostFtdcBrokerIDType	BROKER_ID;
extern TThostFtdcInvestorIDType INVESTOR_ID;
extern TThostFtdcPasswordType	PASSWORD;
extern char* ppInstrumentID[];	
extern int iInstrumentID;
extern TThostFtdcInstrumentIDType INSTRUMENT_ID;

// ������
extern int iRequestID;

// User��������
extern	TThostFtdcDateExprType	TradingDay;

extern	bool	ReceiveTick;

bool	CloseAll= false;										//���̱�־

char	*InstrumentID_name;	//
double	Q_Dayopen=0;		//
double	Q_UpperLimit = 0;	//
double	Q_LowerLimit = 0;	//
string	Q_BarTime_s;		//
int		Q_BarTime_1;		//
double	Q_BarTime_2;		//
int		FirstVolume;		//ǰһ�γɽ�������
int		BNum = 0;			//���ִ���
int		SNum = 0;			//���ִ���

bool	MnKlinesig=false;		//
double  Mn_open[3]	= {0,0,0};			// 
double  Mn_high[3]	= {0,0,0};			// 
double  Mn_low[3]	= {0,0,0};			// 
double  Mn_close[3] = {0,0,0};			// 
double  NewPrice = 0;		//

double  Mn_bbreak = 0;		// 
double  Mn_sbreak = 0;		// 
double  Mn_UpperBand = 100000;	// 
double  Mn_LowerBand = 0;	// 

double  M4 = 1.0;			// 
double  M5 = 1.0;			// 

double  BuyPrice = 0;		//���ּ�
double  SellPrice = 0;		//���ּ�

double  BuyPrice1 = 0;		//��һ��
double  SellPrice1 = 0;		//��һ��
double  BuyVol1 = 0;		//��һ��
double  SellVol1 = 0;		//��һ��
double  BuyVol10 = 0;		//��һ����һ��
double  SellVol10 = 0;		//��һ����һ��

double	TodayVolume = 0;	//�ɽ���
double	OpenInt = 0;		//�ֲ���

double  BuyStopline = 0;	//����ֹ���
double  SellStopline = 0;	//����ֹ��� 
double	BSVolume = 1;		//������

bool	BuySignal = false;	//
bool	SellSignal = false;	//

int		TickABS = 0;
double  TickAPrice[4];		//
int		TickBNum = 0;
double  TickBPrice[4];		//
char    TickFileWritepaths[80]="";	//
char    LogFilePaths[80]="";	//

void CMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast)
{
	cerr << "--->>> "<< __FUNCTION__ << endl;
	IsErrorRspInfo(pRspInfo);
}

void CMdSpi::OnFrontDisconnected(int nReason)
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	cerr << "--->>> Reason = " << nReason << endl;
}
		
void CMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	cerr << "--->>> nTimerLapse = " << nTimeLapse << endl;
}

void CMdSpi::OnFrontConnected()
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	//cerr << "--->>> " << "CMdSpi::OnFrontConnected" << endl;
	///�û���¼����
	ReqUserLogin();

}


void CMdSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.UserID, INVESTOR_ID);
	strcpy(req.Password, PASSWORD);
	int iResult = pMdApi->ReqUserLogin(&req, ++iRequestID);
	cerr << "--->>> ���������û���¼����: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
}

void CMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << __FUNCTION__ << endl;

	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///��ȡ��ǰ������
		cerr << "--->>> ��ȡ��ǰ������ = " << pMdApi->GetTradingDay() << endl;
		// ����������
		SubscribeMarketData();	
	}
}

void CMdSpi::SubscribeMarketData()
{
	int iResult0 = pMdApi->UnSubscribeMarketData(ppInstrumentID, iInstrumentID);
	cerr << "--->>> ȡ�����鶩������: " << ((iResult0 == 0) ? "�ɹ�" : "ʧ��") << endl;
	Sleep(1000);
	int iResult = pMdApi->SubscribeMarketData(ppInstrumentID, iInstrumentID);
	cerr << "--->>> �������鶩������: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;

	//�ļ���������OrderInfo_date.txt��
	char perf[100];
	char tmp[20];
	int tmp1 = 20140424;
	strcpy(perf,"./AutoTrade/");
	strcpy(perf,"TraderInfo_");
	sprintf(tmp,"%d",tmp1);
	strcat(perf,tmp);
	strcat(perf,".txt");
	cerr << "--->>> " << perf << endl;
	
	//����ļ��Ƿ���ڣ��Ƿ���Ҫ�½��ı��ļ�
	ifstream inf;
	ofstream ouf;
	inf.open(perf,ios::out);

}

void CMdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> �ɹ����ĺ�Լ:" <<"_"<<pSpecificInstrument->InstrumentID<< endl;

}

void CMdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << __FUNCTION__ << endl;
}

extern void Sniffer();//����Tick�����Ѿ�ָ����� ʵ����

extern void Trading();	//�µ��Լ���������

bool openstate = false;
bool closestate = false;
extern void SendOrder(TThostFtdcInstrumentIDType FuturesId, int BuySell, int OpenClose);
extern 	bool tdloginstate;
int dealytick = 0;
 void CMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{

	 printf("%s %f\n",pDepthMarketData->InstrumentID, pDepthMarketData->LastPrice);
	if (tdloginstate)
	{
	
		if (!openstate)
		{
			openstate = true;
			NewPrice = pDepthMarketData->UpperLimitPrice;

			SendOrder(INSTRUMENT_ID, 0, 0);
			NewPrice = pDepthMarketData->LowerLimitPrice;

			SendOrder(INSTRUMENT_ID, 0, 0);

		}

		if (openstate)
		{
			dealytick++;
		}


		if (!closestate && dealytick>=8)
		{
			closestate = true;
			NewPrice = pDepthMarketData->LowerLimitPrice;

			SendOrder(INSTRUMENT_ID, 1, 3);

		}
	}
	
}


bool CMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// ���ErrorID != 0, ˵���յ��˴������Ӧ
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
	return bResult;
}



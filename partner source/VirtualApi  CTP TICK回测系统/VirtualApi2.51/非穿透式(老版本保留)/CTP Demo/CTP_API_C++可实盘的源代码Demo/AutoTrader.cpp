// AutoTrader.cpp : 单合约版本,输入经纪公司代码，实盘帐号，密码即可下单。
//自动保存订阅合约TICK数据到\Bin\TickData下，文件名合约名称_日期.txt	
//
//
//
//AutoTrader.cpp : 定义控制台应用程序的入口点。

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
// UserApi对象
CThostFtdcTraderApi *pUserApi;
// MdApi对象
CThostFtdcMdApi *pMdApi;

// 配置参数
char  FRONT_ADDR_1A[] = "tcp://180.168.146.187:10000";		// 前置地址1交易:实盘
char  FRONT_ADDR_1B[] = "tcp://180.168.146.187:10010";		// 前置地址2行情:实盘

char  FRONT_ADDR_2A[] = "tcp://180.168.146.187:10000";		// 前置地址1交易:盘后
char  FRONT_ADDR_2B[] = "tcp://180.168.146.187:10010";		// 前置地址2行情:盘后

char  FRONT_ADDR_3A[] = "tcp://180.168.146.187:10000";	    // 前置地址3交易:仿真 17:00开始
char  FRONT_ADDR_3B[] = "tcp://180.168.146.187:10010";		// 前置地址3行情:仿真 17:00开始

TThostFtdcBrokerIDType	BROKER_ID = "9999";								// 实盘：经纪公司代码 国泰君安=7090
TThostFtdcInvestorIDType INVESTOR_ID = "038997";						// 实盘：投资者代码
TThostFtdcPasswordType  PASSWORD = "000000wdg";							// 实盘：用户密码
//TThostFtdcBrokerIDType	BROKER_ID = "2030";							// 经纪公司代码:仿真
//TThostFtdcInvestorIDType INVESTOR_ID = "00092";						// 投资者代码:仿真
//TThostFtdcPasswordType  PASSWORD = "888888";							// 用户密码:仿真

TThostFtdcInstrumentIDType INSTRUMENT_ID = "rb1910";					// 交易合约代码
TThostFtdcDirectionType	DIRECTION;										// 交易买卖方向
TThostFtdcOffsetFlagType MARKETState;									// 开平仓
TThostFtdcPriceType	LIMIT_PRICE;										// 交易价格

//char *ppInstrumentID[] = { "IF1910", "rb1910","ag1910", "ru1910", "cu1910", "j1910", "SR1910", "m1910", "y1910", "p1910" };			// 行情订阅列表
//int iInstrumentID = 10;													// 行情订阅数量

char *ppInstrumentID[] = { "rb1910" };			// 行情订阅列表
int iInstrumentID = 1;													// 行情订阅数量


 bool	ReceiveTick = false;

// 请求编号
int iRequestID = 0;
// 交易时间
bool	JustRun = false;	//正在启动标志

TThostFtdcDateExprType	TradingDay;

// User行情数据

extern	char	*InstrumentID_name;	//
extern	string	Q_BarTime_s;		//时间字符串
extern	int		Q_BarTime_1;		//时间采用秒计
extern	double	Q_BarTime_2;		//时间格式0.145100
extern	double	Q_UpperLimit;	//
extern	double	Q_LowerLimit;	//

extern	double	NewPrice;		//
extern	int		FirstVolume;	//前一次成交量数据

extern	double  Mn_open[3];		// 
extern	double  Mn_high[3];		// 
extern	double  Mn_low[3];		// 
extern	double  Mn_close[3];	// 

extern	double  BuyPrice;		//开仓价
extern	double  SellPrice;		//开仓价
extern	int		BNum;			//开仓次数
extern	int		SNum;			//开仓次数

extern	bool	BuySignal;		//
extern	bool	SellSignal;		//

extern	double	BSVolume;		//开仓量

extern	int		TickABS;
extern	double  TickAPrice[4];		//
extern	int		TickBNum;
extern	double  TickBPrice[4];		//

extern	char    LogFilePaths[80];	//

// 会话参数
extern	TThostFtdcFrontIDType	FRONT_ID;	//前置编号
extern	TThostFtdcSessionIDType	SESSION_ID;	//会话编号
extern	TThostFtdcOrderRefType	ORDER_REF;	//报单引用



void main(void)
{
	void Erasefiles();
	void Sniffer();
	void Trading();
	bool ReadConfiguration(char *filepaths);
	void WriteConfiguration(char *filepaths);
	
	Erasefiles();
	Sleep(2000);

	cerr << "--->>> " << "Welcom MyAutoTrader System!" << endl;
	cerr << "--->>> " << "Version 1.0.0!" << endl;
	// 初始化UserApi
	pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi("./thosttraderapi.dll");			// 创建UserApi//"./thosttraderapi.dll"
	CTraderSpi* pUserSpi = new CTraderSpi();
	pUserApi->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi);			// 注册事件类
	pUserApi->SubscribePublicTopic(THOST_TERT_RESTART);				// 注册公有流
	pUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);			// 注册私有流
	pUserApi->RegisterFront(FRONT_ADDR_1A);							// connect
	pUserApi->Init();
	cout << pUserApi->GetApiVersion() << endl;
	cout << "--->>> " << "Initialing TradeApi" << endl;

	// 初始化MdApi
	pMdApi = CThostFtdcMdApi::CreateFtdcMdApi("./thostmduserapi.dll");					// 创建MdApi//"./thostmduserapi.dll"
	CThostFtdcMdSpi* pMdSpi = new CMdSpi();
	pMdApi->RegisterSpi(pMdSpi);									// 注册事件类
	pMdApi->RegisterFront(FRONT_ADDR_1B);							// connect		优先行情地址
	pMdApi->RegisterFront(FRONT_ADDR_2B);							// connect		备用行情地址，1B断开，自动连接2B地址
	pMdApi->Init();
	cout << pMdApi->GetApiVersion() << endl;
	cout << "--->>> " << "Initialing MdApi" << endl;
	//pMdApi->Join();
	//pMdApi->Release();
	
	Sleep(6000);
	ReadConfiguration("./AutoTrader.dat");			//自定义数据，如持仓数据等均可
	cerr << "--->>> " << "初始化完成!" << endl;
	

	while(1)
	{
			
		//指标计算,下面只是个简单例子
		//可自建函数，进行复杂处理  见DataSniffer.h
		Sniffer();
		
		//下单控制
		//可自建函数，单独复杂处理	见MyTrader.h
		Trading();

		Sleep(50);


	}

}




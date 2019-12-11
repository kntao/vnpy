// testTraderApi.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include ".\ThostTraderApi\ThostFtdcTraderApi.h"
#include "TraderSpi.h"
#include "inifile.h"
// UserApi����
CThostFtdcTraderApi* pUserApi;

// ���ò���
//char  FRONT_ADDR[] = "tcp://121.41.46.14:8886";		// ǰ�õ�ַ
char  FRONT_ADDR[60];// = "tcp://121.41.46.14:8886";		// ǰ�õ�ַ
TThostFtdcBrokerIDType	BROKER_ID;// = "1023";				// ���͹�˾����
TThostFtdcInvestorIDType INVESTOR_ID;// = "5501156";			// Ͷ���ߴ���
TThostFtdcPasswordType  PASSWORD;// = "369258";			// �û�����
TThostFtdcInstrumentIDType INSTRUMENT_ID;// = "cu1910";	// ��Լ����
TThostFtdcDirectionType	DIRECTION = THOST_FTDC_D_Sell;	// ��������
TThostFtdcPriceType	LIMIT_PRICE = 38850;				// �۸�

// ������
int iRequestID = 0;
#include <windows.h>
void main(void)
{
	IniFile file;
	if (!file.Init("setting.ini"))
	{
		printf("��ȡ�����ļ�setting.ini����");
		return;
	}
	//int Instrumentnum = file.GetIntFromSection("Instrument", "num", 1);

	string brokeidstr = file.GetValueFromSection("setting", "brokeid");
	string investorstr = file.GetValueFromSection("setting", "investor");
	string passwordstr = file.GetValueFromSection("setting", "password");
	string addressstr = file.GetValueFromSection("setting", "address");

	_snprintf_s(BROKER_ID, sizeof(BROKER_ID), sizeof(BROKER_ID)-1,"%s", brokeidstr.c_str());
	_snprintf_s(INVESTOR_ID, sizeof(INVESTOR_ID), sizeof(INVESTOR_ID) - 1, "%s", investorstr.c_str());
	_snprintf_s(PASSWORD, sizeof(PASSWORD), sizeof(PASSWORD) - 1, "%s", passwordstr.c_str());
	_snprintf_s(FRONT_ADDR, sizeof(FRONT_ADDR), sizeof(FRONT_ADDR) - 1, "%s", addressstr.c_str());

	printf("����֪�����Գ���������ļ�setting.ini���˻���Ϣ:\nBROKERID:%s\nINVESTOR:%s\nPASSWORD:%s\nADDRESS:%s\n",BROKER_ID, INVESTOR_ID, PASSWORD, FRONT_ADDR);

	// ��ʼ��UserApi
	pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();			// ����UserApi
	CTraderSpi* pUserSpi = new CTraderSpi();
	pUserApi->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi);			// ע���¼���
	//pUserApi->SubscribePublicTopic(TERT_RESTART);					// ע�ṫ����
	//pUserApi->SubscribePrivateTopic(TERT_RESTART);					// ע��˽����
	pUserApi->RegisterFront(FRONT_ADDR);							// connect
	pUserApi->Init();

	while (1)
	{
		pUserSpi->ReqQryInvestorPosition();	

		Sleep(3000);
		pUserSpi->ReqQryTradingAccount();

		Sleep(3000);
	}
	//pUserApi->Join();
//	pUserApi->Release();
}
// Simple OPC Client
//
// This is a modified version of the "Simple OPC Client" originally
// developed by Philippe Gras (CERN) for demonstrating the basic techniques
// involved in the development of an OPC DA client.
//
// The modifications are the introduction of two C++ classes to allow the
// the client to ask for callback notifications from the OPC server, and
// the corresponding introduction of a message comsumption loop in the
// main program to allow the client to process those notifications. The
// C++ classes implement the OPC DA 1.0 IAdviseSink and the OPC DA 2.0
// IOPCDataCallback client interfaces, and in turn were adapted from the
// KEPWARE´s  OPC client sample code. A few wrapper functions to initiate
// and to cancel the notifications were also developed.
//
// The original Simple OPC Client code can still be found (as of this date)
// in
//        http://pgras.home.cern.ch/pgras/OPCClientTutorial/
//
//
// Luiz T. S. Mendes - DELT/UFMG - 15 Sept 2011
// luizt at cpdee.ufmg.br
//

#include <atlbase.h>    // required for using the "_T" macro
#include <iostream>
#include <ObjIdl.h>
#include <string>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <algorithm>
#include <conio.h>

#pragma comment(lib, "WS2_32.lib")

#include "opcda.h"
#include "opcerror.h"
#include "SimpleOPCClient_v3.h"
#include "SOCAdviseSink.h"
#include "SOCDataCallback.h"
#include "SOCWrapperFunctions.h"
#include "Constants.h"
#include <thread>
#include <mutex>
#include <vector>
#include <sstream>

using namespace std;

#define OPC_SERVER_NAME L"Matrikon.OPC.Simulation.1"
#define VT VT_R4

// Global variables

// The OPC DA Spec requires that some constants be registered in order to use
// them. The one below refers to the OPC DA 1.0 IDataObject interface.
UINT OPC_DATA_TIME = RegisterClipboardFormat(_T("OPCSTMFORMATDATATIME"));

wchar_t ITEM_ID[] = L"Saw-toothed Waves.Real4";
wchar_t SAW_TOOTHED_WAVES_REAL[] = L"Saw-toothed Waves.Real4";
wchar_t TRIANGLE_WAVES_REAL[] = L"Triangle Waves.Real4";
wchar_t RANDOM_REAL_ID[] = L"Random.Real4";
wchar_t SQUARE_WAVES_REAL[] = L"Square Waves.Real4";
wchar_t BUCKET_BRIGADE_REAL8[] = L"Bucket Brigade.Real8";
wchar_t BUCKET_BRIGADE_REAL4[] = L"Bucket Brigade.Real4";
wchar_t BUCKET_BRIGADE_INT4[] = L"Bucket Brigade.Int4";


//////////////////////////////////////////////////////////////////////
// Read the value of an item on an OPC server. 
//
//shared_ptr<SOCDataCallback> local_instance = globalInstance;
IOPCServer* pIOPCServer = NULL;   //pointer to IOPServer interface
IOPCItemMgt* pIOPCItemMgt = NULL; //pointer to IOPCItemMgt interface

OPCHANDLE hServerGroup; // server handle to the group
OPCHANDLE hServerTempPreAque;  // server handle to the Temperatura da zona de pré aquecimento
OPCHANDLE hServerTempAque;  // server handle to the Temperatura da zona de pré aquecimento
OPCHANDLE hServerTempEnch;  // server handle to the Temperatura da zona de pré aquecimento
OPCHANDLE hServerFlowGas;  // server handle to the Temperatura da zona de pré aquecimento
OPCHANDLE hSPServerTempPreAque;  // server handle to the Temperatura da zona de pré aquecimento
OPCHANDLE hSPServerTempAque;  // server handle to the Temperatura da zona de pré aquecimento
OPCHANDLE hSPServerTempEnch;  // server handle to the Temperatura da zona de pré aquecimento

std::mutex mtxtempZAque;
std::mutex mtxtempZPreAque;
std::mutex mtxtempEnch;
std::mutex mtxflowGas;
std::mutex mtxtecla;

SOCKET sock;

void main()
{

	int i;
	char buf[100];

	// Have to be done before using microsoft COM library:
	printf("Inicializando o ambiente COM...\n");
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// Let's instantiante the IOPCServer interface and get a pointer of it:
	printf("Instanciando o MATRIKON OPC Server for Simulation...\n");
	pIOPCServer = InstantiateServer(OPC_SERVER_NAME);

	// Add the OPC group the OPC server and get an handle to the IOPCItemMgt
	//interface:
	printf("Adicionando grupo em estado INACTIVE...\n");
	AddTheGroup(pIOPCServer, pIOPCItemMgt, hServerGroup, GROUP_NAME);

	// Add the OPC item. First we have to convert from wchar_t* to char*
	// in order to print the item name in the console.
	printf("Adicionando itens para o grupo...\n");
	size_t m;

	wcstombs_s(&m, buf, 100, TEMP_Z_AQUE, _TRUNCATE);
	printf("Adicionando o item %s para o grupo...\n", buf);
	AddTheItem(pIOPCItemMgt, hServerTempAque, TEMP_Z_AQUE, SAW_TOOTHED_WAVES_REAL, TEMP_Z_AQUE_CLIENT);

	wcstombs_s(&m, buf, 100, TEMP_Z_PRE_AQUE, _TRUNCATE);
	printf("Adicionando o item %s para o grupo...\n", buf);
	AddTheItem(pIOPCItemMgt, hServerTempPreAque, TEMP_Z_PRE_AQUE, RANDOM_REAL_ID, TEMP_Z_PRE_AQUE_CLIENT);

	wcstombs_s(&m, buf, 100, TEMP_Z_ENCH, _TRUNCATE);
	printf("Adicionando o item %s para o grupo...\n", buf);
	AddTheItem(pIOPCItemMgt, hServerTempEnch, TEMP_Z_ENCH, TRIANGLE_WAVES_REAL, TEMP_Z_ENCH_CLIENT);

	wcstombs_s(&m, buf, 100, FLOW_TOTAL_GAS, _TRUNCATE);
	printf("Adicionando o item %s para o grupo...\n", buf);
	AddTheItem(pIOPCItemMgt, hServerFlowGas, FLOW_TOTAL_GAS, SQUARE_WAVES_REAL, FLOW_TOTAL_GAS_CLIENT);

	wcstombs_s(&m, buf, 100, SP_TEMP_Z_PRE_AQUE, _TRUNCATE);
	printf("Adicionando o item %s para o grupo...\n", buf);
	AddTheItem(pIOPCItemMgt, hSPServerTempPreAque, SP_TEMP_Z_PRE_AQUE, BUCKET_BRIGADE_REAL8, SP_TEMP_Z_PRE_AQUE_CLIENT);

	wcstombs_s(&m, buf, 100, SP_TEMP_Z_AQUE, _TRUNCATE);
	printf("Adicionando o item %s para o grupo...\n", buf);
	AddTheItem(pIOPCItemMgt, hSPServerTempAque, SP_TEMP_Z_AQUE, BUCKET_BRIGADE_REAL4, SP_TEMP_Z_AQUE_CLIENT);

	wcstombs_s(&m, buf, 100, SP_TEMP_Z_ENCH, _TRUNCATE);
	printf("Adicionando o item %s para o grupo...\n", buf);
	AddTheItem(pIOPCItemMgt, hSPServerTempEnch, SP_TEMP_Z_ENCH, BUCKET_BRIGADE_INT4, SP_TEMP_Z_ENCH_CLIENT);

	// Establish a callback asynchronous read by means of the IOPCDaraCallback
	// (OPC DA 2.0) method. We first instantiate a new SOCDataCallback object and
	// adjusts its reference count, and then call a wrapper function to
	// setup the callback.
	IConnectionPoint* pIConnectionPoint = NULL; //pointer to IConnectionPoint Interface
	DWORD dwCookie = 0;
	SOCDataCallback* pSOCDataCallback = new SOCDataCallback();
	pSOCDataCallback->mtxtecla = &mtxtecla;
	pSOCDataCallback->mtxflowGas = &mtxflowGas;
	pSOCDataCallback->mtxtempEnch = &mtxtempEnch;
	pSOCDataCallback->mtxtempZAque = &mtxtempZAque;
	pSOCDataCallback->mtxtempZPreAque = &mtxtempZPreAque;
	pSOCDataCallback->AddRef();
	printf("Setando o IConnectionPoint callback connection...\n");

	// Change the group to the ACTIVE state so that we can receive the
	// server´s callback notification
	printf("Mudando estado do grupo para ACTIVE...\n");

	SetGroupActive(pIOPCItemMgt);

	thread threadOPC(runClient, pSOCDataCallback, pIOPCItemMgt);

	SetDataCallback(pIOPCItemMgt, pSOCDataCallback, pIConnectionPoint, &dwCookie);

	threadOPC.join();

	// Cancel the callback and release its reference
	printf("Cancelando as notificacoes do IOPCDataCallback...\n");
	CancelDataCallback(pIConnectionPoint, dwCookie);
	pIConnectionPoint->Release();
	pSOCDataCallback->Release();

	// Remove the OPC item:
	wcstombs_s(&m, buf, 100, TEMP_Z_AQUE, _TRUNCATE);
	printf("Removendo o item OPC %s...\n", buf);
	RemoveItem(pIOPCItemMgt, hServerTempAque);

	wcstombs_s(&m, buf, 100, TEMP_Z_PRE_AQUE, _TRUNCATE);
	printf("Removendo o item OPC %s...\n", buf);
	RemoveItem(pIOPCItemMgt, hServerTempPreAque);

	wcstombs_s(&m, buf, 100, TEMP_Z_ENCH, _TRUNCATE);
	printf("Removendo o item OPC %s...\n", buf);
	RemoveItem(pIOPCItemMgt, hServerTempEnch);

	wcstombs_s(&m, buf, 100, FLOW_TOTAL_GAS, _TRUNCATE);
	printf("Removendo o item OPC %s...\n", buf);
	RemoveItem(pIOPCItemMgt, hServerFlowGas);

	wcstombs_s(&m, buf, 100, SP_TEMP_Z_PRE_AQUE, _TRUNCATE);
	printf("Removendo o item OPC %s...\n", buf);
	RemoveItem(pIOPCItemMgt, hServerTempPreAque);

	wcstombs_s(&m, buf, 100, SP_TEMP_Z_AQUE, _TRUNCATE);
	printf("Removendo o item OPC %s...\n", buf);
	RemoveItem(pIOPCItemMgt, hServerTempAque);

	wcstombs_s(&m, buf, 100, SP_TEMP_Z_ENCH, _TRUNCATE);
	printf("Removendo o item OPC %s...\n", buf);
	RemoveItem(pIOPCItemMgt, hServerTempEnch);

	// Remove the OPC group:
	printf("Removendo o grupo OPC...\n");
	pIOPCItemMgt->Release();
	RemoveGroup(pIOPCServer, hServerGroup);

	// release the interface references:
	printf("Removendo o server OPC...\n");
	pIOPCServer->Release();

	//close the COM library:
	printf("Liberando o ambiente COM...\n");
	CoUninitialize();
}

////////////////////////////////////////////////////////////////////
// Instantiate the IOPCServer interface of the OPCServer
// having the name ServerName. Return a pointer to this interface
//
IOPCServer* InstantiateServer(wchar_t ServerName[])
{
	CLSID CLSID_OPCServer;
	HRESULT hr;

	// get the CLSID from the OPC Server Name:
	hr = CLSIDFromString(ServerName, &CLSID_OPCServer);
	_ASSERT(!FAILED(hr));


	//queue of the class instances to create
	LONG cmq = 1; // nbr of class instance to create.
	MULTI_QI queue[1] =
	{ {&IID_IOPCServer,
	NULL,
	0} };

	//Server info:
	//COSERVERINFO CoServerInfo =
	//{
	//	/*dwReserved1*/ 0,
	//	/*pwszName*/ REMOTE_SERVER_NAME,
	//	/*COAUTHINFO*/  NULL,
	//	/*dwReserved2*/ 0
	//}; 

	// create an instance of the IOPCServer
	hr = CoCreateInstanceEx(CLSID_OPCServer, NULL, CLSCTX_SERVER,
		/*&CoServerInfo*/NULL, cmq, queue);
	_ASSERT(!hr);

	// return a pointer to the IOPCServer interface:
	return(IOPCServer*)queue[0].pItf;
}


/////////////////////////////////////////////////////////////////////
// Add group "Group1" to the Server whose IOPCServer interface
// is pointed by pIOPCServer. 
// Returns a pointer to the IOPCItemMgt interface of the added group
// and a server opc handle to the added group.
//
void AddTheGroup(IOPCServer* pIOPCServer, IOPCItemMgt*& pIOPCItemMgt,
	OPCHANDLE& hServerGroup, wchar_t groupName[])
{
	DWORD dwUpdateRate = 0;
	OPCHANDLE hClientGroup = 0;

	// Add an OPC group and get a pointer to the IUnknown I/F:
	HRESULT hr = pIOPCServer->AddGroup(/*szName*/ groupName,
		/*bActive*/ FALSE,
		/*dwRequestedUpdateRate*/ 1000,
		/*hClientGroup*/ hClientGroup,
		/*pTimeBias*/ 0,
		/*pPercentDeadband*/ 0,
		/*dwLCID*/0,
		/*phServerGroup*/&hServerGroup,
		&dwUpdateRate,
		/*riid*/ IID_IOPCItemMgt,
		/*ppUnk*/ (IUnknown**)&pIOPCItemMgt);
	_ASSERT(!FAILED(hr));
}



//////////////////////////////////////////////////////////////////
// Add the Item ITEM_ID to the group whose IOPCItemMgt interface
// is pointed by pIOPCItemMgt pointer. Return a server opc handle
// to the item.

void AddTheItem(IOPCItemMgt* pIOPCItemMgt, OPCHANDLE& hServerItem, wchar_t itemName[], wchar_t itemId[], int hClient)
{
	HRESULT hr;

	// Array of items to add:
	OPCITEMDEF ItemArray[1] =
	{ {
			/*szAccessPath*/ L"",
			/*szItemID*/ itemId,
			/*bActive*/ TRUE,
			/*hClient*/ hClient,
			/*dwBlobSize*/ 0,
			/*pBlob*/ NULL,
			/*vtRequestedDataType*/ VT,
			/*wReserved*/0
			} };

	//Add Result:
	OPCITEMRESULT* pAddResult = NULL;
	HRESULT* pErrors = NULL;

	// Add an Item to the previous Group:
	hr = pIOPCItemMgt->AddItems(1, ItemArray, &pAddResult, &pErrors);
	if (hr != S_OK) {
		printf("Failed call to AddItems function. Error code = %x\n", hr);
		exit(0);
	}

	// Server handle for the added item:
	hServerItem = pAddResult[0].hServer;

	// release memory allocated by the server:
	CoTaskMemFree(pAddResult->pBlob);

	CoTaskMemFree(pAddResult);
	pAddResult = NULL;

	CoTaskMemFree(pErrors);
	pErrors = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Read from device the value of the item having the "hServerItem" server 
// handle and belonging to the group whose one interface is pointed by
// pGroupIUnknown. The value is put in varValue. 
//
void ReadItem(IUnknown* pGroupIUnknown, OPCHANDLE hServerItem, VARIANT& varValue)
{
	// value of the item:
	OPCITEMSTATE* pValue = NULL;

	//get a pointer to the IOPCSyncIOInterface:
	IOPCSyncIO* pIOPCSyncIO;
	pGroupIUnknown->QueryInterface(__uuidof(pIOPCSyncIO), (void**)&pIOPCSyncIO);

	// read the item value from the device:
	HRESULT* pErrors = NULL; //to store error code(s)
	HRESULT hr = pIOPCSyncIO->Read(OPC_DS_DEVICE, 1, &hServerItem, &pValue, &pErrors);
	_ASSERT(!hr);
	_ASSERT(pValue != NULL);

	varValue = pValue[0].vDataValue;

	//Release memeory allocated by the OPC server:
	CoTaskMemFree(pErrors);
	pErrors = NULL;

	CoTaskMemFree(pValue);
	pValue = NULL;

	// release the reference to the IOPCSyncIO interface:
	pIOPCSyncIO->Release();
}

///////////////////////////////////////////////////////////////////////////////
// Write from device the value of the item having the "hServerItem" server 
// handle and belonging to the group whose one interface is pointed by
// pGroupIUnknown. The value is put in varValue. 
//
void WriteItem(IUnknown* pGroupIUnknown, OPCHANDLE hServerItem, VARIANT& varValue)
{
	//get a pointer to the IOPCSyncIOInterface:
	IOPCSyncIO* pIOPCSyncIO;
	pGroupIUnknown->QueryInterface(__uuidof(pIOPCSyncIO), (void**)&pIOPCSyncIO);
	/*VARIANT v[1];
	v[0].vt = VT_R8;
	v[0].dblVal = 1.0;*/
	// read the item value from the device:
	HRESULT* pErrors = NULL; //to store error code(s)
	HRESULT hr = pIOPCSyncIO->Write(1, &hServerItem, &varValue, &pErrors);
	_ASSERT(!hr);

	//Release memeory allocated by the OPC server:
	CoTaskMemFree(pErrors);
	pErrors = NULL;

	// release the reference to the IOPCSyncIO interface:
	pIOPCSyncIO->Release();
}

///////////////////////////////////////////////////////////////////////////
// Remove the item whose server handle is hServerItem from the group
// whose IOPCItemMgt interface is pointed by pIOPCItemMgt
//
void RemoveItem(IOPCItemMgt* pIOPCItemMgt, OPCHANDLE hServerItem)
{
	// server handle of items to remove:
	OPCHANDLE hServerArray[1];
	hServerArray[0] = hServerItem;

	//Remove the item:
	HRESULT* pErrors; // to store error code(s)
	HRESULT hr = pIOPCItemMgt->RemoveItems(1, hServerArray, &pErrors);
	_ASSERT(!hr);

	//release memory allocated by the server:
	CoTaskMemFree(pErrors);
	pErrors = NULL;
}

////////////////////////////////////////////////////////////////////////
// Remove the Group whose server handle is hServerGroup from the server
// whose IOPCServer interface is pointed by pIOPCServer
//
void RemoveGroup(IOPCServer* pIOPCServer, OPCHANDLE hServerGroup)
{
	// Remove the group:
	HRESULT hr = pIOPCServer->RemoveGroup(hServerGroup, FALSE);
	if (hr != S_OK) {
		if (hr == OPC_S_INUSE)
			printf("Failed to remove OPC group: object still has references to it.\n");
		else printf("Failed to remove OPC group. Error code = %x\n", hr);
		exit(0);
	}
}

void runClient(SOCDataCallback* socDataCallback, IOPCItemMgt* pIOPCItemMgt)
{
	

	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS


	#define BUFFER_SIZE  1024
	#define TAMMSGDADOS  43  // 6+4+7+7+7+7 caracteres + 5 separadores
	#define TAMMSGREQ    11  // 6+4 caracteres + 1 separador
	#define TAMMSGSP     33  // 6+4+7+7+5 caracteres + 4 separadores
	#define TAMMSGACK    11  // 6+4 caracteres + 1 separador
	#define TAMMSGACKSP  11  // 6+4 caracteres + 1 separador
	//#define ESC			 0x1B

	//WSADATA     wsaData;
	//SOCKET      sock;
	//SOCKADDR_IN ServerAddr;

	char msgdados[TAMMSGDADOS + 1];
	char msgreq[TAMMSGREQ + 1];
	char msgsp[TAMMSGSP + 1];
	char msgack[TAMMSGACK + 1];
	char msgacksp[TAMMSGACKSP + 1];
	char buf[100];

	//const char* ipaddr;
	int status;
	int sequencia = 1;
	const int msgTamI = 6;
	const int msgTamR = 7;

	sock = retryToSocket();

	char tecla[1];
	tecla[0] = '0';
	thread threadReq(runReq, tecla);

	while (true) {

		this_thread::sleep_for(std::chrono::milliseconds(2000));

		AllMutexLock();

		float tempZAque = socDataCallback->tempZAque.value.fltVal;
		float tempZPreAque = socDataCallback->tempZPreAque.value.fltVal;
		float tempEnch = socDataCallback->tempEnch.value.fltVal;
		float flowGas = socDataCallback->flowGas.value.fltVal;

		AllMutexUnlock();

		char strtzaque[msgTamR];
		sprintf(strtzaque, "%.1f", tempZAque);
		char strtzpreaque[msgTamR];
		sprintf(strtzpreaque, "%.1f", tempZPreAque);
		char tench[msgTamR];
		sprintf(tench, "%.1f", tempEnch);
		char strfgas[msgTamR];
		sprintf(strfgas, "%.1f", flowGas);
		
		std::string ftzaque = FormatZero(msgTamR, strtzaque);
		std::string ftzpreaque = FormatZero(msgTamR, strtzpreaque);
		std::string ftench = FormatZero(msgTamR, tench);
		std::string fflowgas = FormatZero(msgTamR, strfgas);
		std::string seq = FormatZero(msgTamI, to_string(sequencia));

		std::string strmsgdados = seq += ";0000;" + ftzaque + ";" += ftzpreaque + ";" += ftench + ";" += fflowgas;
		char* dados = &strmsgdados[0];
		
		AllMutexLock();
		strncpy(msgdados, dados, TAMMSGDADOS + 1);
		status = send(sock, msgdados, TAMMSGDADOS, 0);
		while (status < 0) {
			AllMutexUnlock();
			SOCKET okRetry = retryToSocket();
			if (!okRetry)
				exit(0);
			sequencia = 1;
			std::string seq = FormatZero(msgTamI, to_string(sequencia));
			std::string strmsgdados = seq += ";0000;" + ftzaque + ";" += ftzpreaque + ";" += ftench + ";" += fflowgas;
			status = send(sock, msgdados, TAMMSGDADOS, 0);
		}
		printf("[ENVIO] Dados da malha para o servidor:\n%s\n\n", msgdados);
		AllMutexUnlock();

		AllMutexLock();
		memset(buf, 0, sizeof(buf));
		status = recv(sock, buf, TAMMSGACK, 0);
		while (status < 0) {
			AllMutexUnlock();
			SOCKET okRetry = retryToSocket();
			if (!okRetry)
				exit(0);
			sequencia = 1;
			break;
		}
		strncpy(msgack, buf, TAMMSGACK + 1);
		printf("[RECEBIDO] ACK dados da malha:\n%s\n\n", msgack);
		AllMutexUnlock();

		sequencia = sequencia + 2;

		if (tecla[0] == 's') {

			seq = FormatZero(msgTamI, to_string(sequencia));
			std::string strmsgreq = seq += ";1100";
			char* req = &strmsgreq[0];

			AllMutexLock();
			strncpy(msgreq, req, TAMMSGREQ + 1);
			status = send(sock, msgreq, TAMMSGREQ, 0);
			while (status < 0) {
				AllMutexUnlock();
				SOCKET okRetry = retryToSocket();
				if (!okRetry)
					exit(0);
				sequencia = 1;
				seq = FormatZero(msgTamI, to_string(sequencia));
				std::string strmsgreq = seq += ";1100";
				char* req = &strmsgreq[0];
				status = send(sock, msgreq, TAMMSGREQ, 0);
			}
			printf("\n\n\n[ENVIO] Set-points das malhas:\n%s\n\n", msgreq);
			AllMutexUnlock();

			AllMutexUnlock();
			memset(buf, 0, sizeof(buf));
			status = recv(sock, buf, TAMMSGSP, 0);
			while (status < 0) {
				AllMutexUnlock();
				SOCKET okRetry = retryToSocket();
				if (!okRetry)
					exit(0);
				sequencia = 1;
				break;
			}
			strncpy(msgsp, buf, TAMMSGSP + 1);
			sequencia = sequencia + 2;
			printf("[RECEBIDO] Set-points das malhas:\n%s\n\n", msgsp);
			AllMutexUnlock();

			seq = FormatZero(msgTamI, to_string(sequencia));
			std::string stracksp = seq += ";0011";
			char* acksp = &stracksp[0];

			AllMutexUnlock();
			strncpy(msgacksp, acksp, TAMMSGACKSP + 1);
			status = send(sock, msgacksp, TAMMSGACKSP, 0);
			while (status < 0) {
				AllMutexUnlock();
				SOCKET okRetry = retryToSocket();
				if (!okRetry)
					exit(0);
				sequencia = 1;
				seq = FormatZero(msgTamI, to_string(sequencia));
				std::string stracksp = seq += ";0011";
				char* acksp = &stracksp[0];
				status = send(sock, msgacksp, TAMMSGACKSP, 0);
			}
			sequencia++;
			printf("Mensagem enviada ao servidor:\n%s\n\n", msgacksp);
			AllMutexUnlock();

			std::string strMessage(msgsp);
			std::stringstream strMessageStream(strMessage);
			std::vector<std::string> msgSplit;
			std::string segment;

			while (std::getline(strMessageStream, segment, ';'))
			{
				msgSplit.push_back(segment);
			}

			VARIANT preAque[1];
			preAque[0].vt = VT_R8;
			preAque[0].dblVal = atof(msgSplit[2].c_str());
			printf("Escrevendo Set-point para temperatura da zona de pre aquecimento (C): %f \n", preAque[0].dblVal);
			WriteItem(pIOPCItemMgt, hSPServerTempPreAque, *preAque);

			VARIANT aque[1];
			aque[0].vt = VT_R4;
			aque[0].fltVal = atof(msgSplit[3].c_str());
			printf("Escrevendo Set-point para temperatura da zona de aquecimento (C): %f \n", aque[0].fltVal);
			WriteItem(pIOPCItemMgt, hSPServerTempAque, *aque);

			VARIANT ench[1];
			ench[0].vt = VT_I4;
			ench[0].intVal = atoi(msgSplit[4].c_str());
			printf("Escrevendo Set-point para temperatura da zona de encharque (C): %d \n", ench[0].intVal);
			WriteItem(pIOPCItemMgt, hSPServerTempEnch, *ench);

		}
		else if (tecla[0] == '0') {
			continue;
		}
		tecla[0] = '0';
	}
	
	threadReq.join();

	printf("Encerrando a conexao ...\n");
	closesocket(sock);
	WSACleanup();
	printf("Conexao encerrada. Fim do programa.\n");

	exit(0);
}

double GetFloatPrecision(double value, double precision)
{
	double scale = 0.1; 
	return floor(value / scale + 0.5) * scale;
}

std::string FormatZero(size_t n, std::string str) 
{
	int precision = n - min(n, str.size());
	std::string s = std::string(precision, '0').append(str);
	return s;
}

void runReq(char* tecla) 
{
	while (true) {
		mtxtecla.lock();
		tecla[0] = getch();
		mtxtecla.unlock();
	}
	
}

SOCKET retryToSocket()
{
	int maxRetries = 0;
	closesocket(sock);
	for (;;) {
		if (maxRetries >= 100)
			break;
		printf("\n\n\nTentando conectar ao servidor...\n");
		sock = ConnectToSocket();
		if (sock) {
			printf("Conexão feita com sucesso!!!\n");
			return sock;
		}			
		this_thread::sleep_for(std::chrono::milliseconds(1000));
		maxRetries++;
	}	
	
	return NULL;
}

SOCKET ConnectToSocket()
{
	WSADATA     wsaData;
	//SOCKET      sock;
	SOCKADDR_IN ServerAddr;

	const char* ipaddr;
	int     port, status;

	ipaddr = "127.0.0.1";
	port = 3445;

	status = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (status != 0) {
		printf("Falha na inicializacao do Winsock 2! Erro  = %d\n", WSAGetLastError());
		WSACleanup();
		return NULL;
	}

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		status = WSAGetLastError();
		if (status == WSAENETDOWN)
			printf("Rede ou servidor de sockets inacessiveis!\n");
		else
			printf("Falha na rede: codigo de erro = %d\n", status);
		WSACleanup();
		return NULL;
	}

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(port);
	ServerAddr.sin_addr.s_addr = inet_addr(ipaddr);

	status = connect(sock, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
	if (status == SOCKET_ERROR) {
		printf("Falha na conexao ao servidor ! Erro  = %d\n", WSAGetLastError());
		WSACleanup();
		return NULL;
	}

	return sock;
}

void AllMutexLock()
{
	mtxtempZAque.lock();
	mtxtempZPreAque.lock();
	mtxtempEnch.lock();
	mtxflowGas.lock();
}

void AllMutexUnlock()
{
	mtxtempZAque.unlock();
	mtxtempZPreAque.unlock();
	mtxtempEnch.unlock();
	mtxflowGas.unlock();
}

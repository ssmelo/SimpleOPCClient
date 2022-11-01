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

#include "Data.h"
#include "SOCDataCallback.h"
#include <string>

#ifndef SIMPLE_OPC_CLIENT_H
#define SIMPLE_OPC_CLIENT_H

void RunOPCClient(DATA_PROCESS*& tempZAque, DATA_PROCESS*& tempZPreAque, DATA_PROCESS*& tempEnch, DATA_PROCESS*& flowGas, std::shared_ptr<SOCDataCallback> globalInstance);
IOPCServer *InstantiateServer(wchar_t ServerName[]);
void AddTheGroup(IOPCServer* pIOPCServer, IOPCItemMgt* &pIOPCItemMgt, 
				 OPCHANDLE& hServerGroup, wchar_t groupName[]);
void AddTheItem(IOPCItemMgt* pIOPCItemMgt, OPCHANDLE& hServerItem, wchar_t itemName[], wchar_t itemId[], int hClient);
void ReadItem(IUnknown* pGroupIUnknown, OPCHANDLE hServerItem, VARIANT& varValue);
void WriteItem(IUnknown* pGroupIUnknown, OPCHANDLE hServerItem, VARIANT& varValue);
void RemoveItem(IOPCItemMgt* pIOPCItemMgt, OPCHANDLE hServerItem);
void RemoveGroup(IOPCServer* pIOPCServer, OPCHANDLE hServerGroup);
void runClient(SOCDataCallback* socDataCallback, IOPCItemMgt* pIOPCItemMgt);
double GetFloatPrecision(double value, double precision);
std::string FormatZero(size_t n, std::string str);
void runReq(char* tecla);
SOCKET ConnectToSocket();
SOCKET retryToSocket();
void AllMutexUnlock();
void AllMutexLock();
#endif // SIMPLE_OPC_CLIENT_H not defined

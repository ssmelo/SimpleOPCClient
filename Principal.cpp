//#include <iostream>
//#include <thread>
//#include "opcda.h"
//#include "opcerror.h"
//#include "SOCWrapperFunctions.h"
//#include "SimpleOPCClient_v3.h"
//#include "Data.h"
//#include "SOCDataCallback.h"
//
//using namespace std;
//
//
//
//void main(void)
//{
//	shared_ptr<SOCDataCallback> globalInstance = make_shared<SOCDataCallback>();
//	SOCDataCallback** dataCallbackPtr = nullptr;
//	DATA_PROCESS* tempZAque;
//	DATA_PROCESS* tempZPreAque;
//	DATA_PROCESS* tempEnch;
//	DATA_PROCESS* flowGas;
//	TESTE tst;
//
//	printf("%p \n", &dataCallbackPtr);
//
//	thread threadOPC(RunOPCClient, ref(tempZAque), ref(tempZPreAque), ref(tempEnch), ref(flowGas), globalInstance);
//
//	threadOPC.join();
//	printf("saindo \n");
//
//	printf("%p \n", *dataCallbackPtr);
//}
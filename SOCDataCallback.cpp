//
// C++ class to implement the OPC DA 2.0 IOPCDataCallback interface.
//
// Note that only the ::OnDataChangeMethod() is currently implemented
// here. This code is largely based on the KEPWARE´s sample client code.
//
// Luiz T. S. Mendes - DELT/UFMG - 13 Sept 2011
//

#include <stdio.h>
#include "SOCDataCallback.h"
#include "SOCWrapperFunctions.h"
#include "Constants.h"

extern UINT OPC_DATA_TIME;

//	Constructor.  Reference count is initialized to zero.
SOCDataCallback::SOCDataCallback () : m_cnRef (0)
	{
	}

//	Destructor
SOCDataCallback::~SOCDataCallback ()
	{
	}

// IUnknown methods
HRESULT STDMETHODCALLTYPE SOCDataCallback::QueryInterface (REFIID riid, LPVOID *ppv)
{
	// Validate the pointer
	if (ppv == NULL)
		return E_POINTER;       // invalid pointer
	// Standard COM practice requires that we invalidate output arguments
	// if an error is encountered.  Let's assume an error for now and invalidate
	// ppInterface.  We will reset it to a valid interface pointer later if we
	// determine requested ID is valid:
	*ppv = NULL;
	if (riid == IID_IUnknown)
		*ppv = (IUnknown*) this;
	else if (riid == IID_IOPCDataCallback)
		*ppv = (IOPCDataCallback*) this;
	else
		return (E_NOINTERFACE); //unsupported interface

	// Success: increment the reference counter.
	AddRef ();                  
	return (S_OK);
}

ULONG STDMETHODCALLTYPE SOCDataCallback::AddRef (void)
{
    // Atomically increment the reference count and return
	// the value.
    return InterlockedIncrement((volatile LONG*)&m_cnRef);
}

ULONG STDMETHODCALLTYPE SOCDataCallback::Release (void)
{
	if (InterlockedDecrement((volatile LONG*)&m_cnRef) == 0){
		delete this;
		return 0;
	}
	else
	    return m_cnRef;
}

// OnDataChange method. This method is provided to handle notifications
// from an OPC Group for exception based (unsolicited) data changes and
// refreshes. Data for one or possibly more active items in the group
// will be provided.
//
// Parameters:
//	DWORD		dwTransID			Zero for normal OnDataChange events, 
//                                    non-zero for Refreshes.
//	OPCHANDLE	hGroup				Client group handle.
//	HRESULT		hrMasterQuality		S_OK if all qualities are GOOD, otherwise S_FALSE.
//	HRESULT		hrMasterError		S_OK if all errors are S_OK, otherwise S_FALSE.
//	DWORD		dwCount				Number of items in the lists that follow.
//	OPCHANDLE	*phClientItems		Item client handles.
//	VARIANT		*pvValues			Item data.
//	WORD		*pwQualities		Item qualities.
//	FILETIME	*pftTimeStamps		Item timestamps.
//	HRESULT		*pErrors			Item errors.
//
// Returns:
//	HRESULT - 
//		S_OK - Processing of advisement successful.
//		E_INVALIDARG - One of the arguments was invalid.
// **************************************************************************
HRESULT STDMETHODCALLTYPE SOCDataCallback::OnDataChange(
	DWORD dwTransID,
	OPCHANDLE hGroup,
	HRESULT hrMasterQuality,
	HRESULT hrMasterError,
	DWORD dwCount,
	OPCHANDLE *phClientItems,
	VARIANT *pvValues,
	WORD *pwQualities,
	FILETIME *pftTimeStamps,
	HRESULT *pErrors)
{
	FILETIME lft;
	SYSTEMTIME st;
    char szLocalDate[255], szLocalTime[255];
	bool status;
	char buffer[100];
	WORD quality;

	// Validate arguments.  Return with "invalid argument" error code 
	// if any are invalid. KEPWARE´s original code checks also if the
	// "hgroup" parameter (the client´s handle for the group) was also
	// NULL, but we dropped this check since the Simple OPC Client
	// sets the client handle to 0 ...
	if (dwCount					== 0	||
		phClientItems			== NULL	||
		pvValues				== NULL	||
		pwQualities				== NULL	||
		pftTimeStamps			== NULL	||
		pErrors					== NULL){
		printf("IOPCDataCallback::ONDataChange: invalid arguments.\n");
		return (E_INVALIDARG);
	}
	
	mtxtempZAque->lock();
	mtxtempZPreAque->lock();
	mtxtempEnch->lock();
	mtxflowGas->lock();
	// Loop over items:
	for (DWORD dwItem = 0; dwItem < dwCount; dwItem++)
	{
		// Print the item value, quality and time stamp. In this example, only
		// a few OPC data types are supported.
		
		status = VarToStr(pvValues[dwItem], buffer);
		if (status){
			DATA_PROCESS* item = GetItemFactory(phClientItems[dwItem]);

			item->value = pvValues[dwItem];
			item->quality = pwQualities[dwItem] & OPC_QUALITY_MASK;
			printf(GetItemName(phClientItems[dwItem]).c_str());
			printf(": %s", buffer);
			quality = pwQualities [dwItem] & OPC_QUALITY_MASK;
			if (quality == OPC_QUALITY_GOOD)
				printf(" Qualidade: boa");
			else
			    printf(" Qualidade: ruim");
			// Code below extracted from the Microsoft KB:
			//     http://support.microsoft.com/kb/188768
			// Note that in order for it to work, the Visual Studio C++ must
			// be configured so that the "character set" property is "not set"
			// (Project->Project Properties->Configuration Properties->General).
			// Otherwise, if defined e.g. as "use Unicode" (as it seems to be
			// the default when a new project is created), there will be
			// compilation errors.
			FileTimeToLocalFileTime(&pftTimeStamps [dwItem],&lft);
			FileTimeToSystemTime(&lft, &st);
			item->timestamp = st;
			GetDateFormat(LOCALE_SYSTEM_DEFAULT, DATE_SHORTDATE, &st, NULL, szLocalDate, 255);
			GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &st, NULL, szLocalTime, 255);
			printf(" Tempo: %s %s\n", szLocalDate, szLocalTime);
		}
		else printf ("IOPCDataCallback: tipo de item nao suportado\n");
	}

	mtxtempZAque->unlock();
	mtxtempZPreAque->unlock();
	mtxtempEnch->unlock();
	mtxflowGas->unlock();

	// Return "success" code.  Note this does not mean that there were no 
	// errors reported by the OPC Server, only that we successfully processed
	// the callback.
	return (S_OK);
}
// The remaining methods of IOPCDataCallback are not implemented here, so
// we just use dummy functions that simply return S_OK.
HRESULT STDMETHODCALLTYPE SOCDataCallback::OnReadComplete(
	DWORD dwTransID,
	OPCHANDLE hGroup,
	HRESULT hrMasterQuality,
	HRESULT hrMasterError,
	DWORD dwCount,
	OPCHANDLE *phClientItems,
	VARIANT *pvValues,
	WORD *pwQualities,
	FILETIME *pftTimeStamps,
	HRESULT *pErrors)
{
	return (S_OK);
}

HRESULT STDMETHODCALLTYPE SOCDataCallback::OnWriteComplete(
	DWORD dwTransID,
	OPCHANDLE hGroup,
	HRESULT hrMasterError,
	DWORD dwCount,
	OPCHANDLE *phClientItems,
	HRESULT *pErrors)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE SOCDataCallback::OnCancelComplete(
	DWORD dwTransID,
	OPCHANDLE hGroup)
{
	return(S_OK);
}

DATA_PROCESS* SOCDataCallback::GetItemFactory(int hClient)
{
	switch (hClient)
	{
		case TEMP_Z_PRE_AQUE_CLIENT:
			return &tempZAque;
		case TEMP_Z_AQUE_CLIENT:
			return &tempZPreAque;
		case TEMP_Z_ENCH_CLIENT:
			return &tempEnch;
		case FLOW_TOTAL_GAS_CLIENT:
			return &flowGas;
		case SP_TEMP_Z_PRE_AQUE_CLIENT:
			return &spTempZAque;
		case SP_TEMP_Z_AQUE_CLIENT:
			return &spTempZPreAque;
		case SP_TEMP_Z_ENCH_CLIENT:
			return &spTempEnch;
		default:
			break;
	}
}

std::string SOCDataCallback::GetItemName(int hClient)
{
	switch (hClient)
	{
	case TEMP_Z_PRE_AQUE_CLIENT:
		return "Temperatura da zona de pre aquecimento (C)";
	case TEMP_Z_AQUE_CLIENT:
		return "Temperatura da zona de aquecimento (C)";
	case TEMP_Z_ENCH_CLIENT:
		return "Temperatura da zona de encharque (C)";
	case FLOW_TOTAL_GAS_CLIENT:
		return "Vazao total de gas (m3/s)";
	case SP_TEMP_Z_PRE_AQUE_CLIENT:
		return "SP da temperatura da zona de pre aquecimento (C)";
	case SP_TEMP_Z_AQUE_CLIENT:
		return "SP da temperatura da zona de aquecimento (C)";
	case SP_TEMP_Z_ENCH_CLIENT:
		return "SP da temperatura da zona de encharque (C)";
	default:
		break;
	}
}
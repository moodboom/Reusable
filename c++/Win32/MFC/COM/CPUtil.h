// CPUtil.h : Declaration of the ContextAccessor class
//
// Copyright (c) 2001 Tim Ewald
// All rights reserved.
// NO WARRANTIES ARE EXTENDED. USE AT YOUR OWN RISK.
// This code may be used without payment in your own projects, but may not
// be redistributed in source code form without express written permission of
// the author. You may contact the author using tjewald@develop.com.
//
//
// The inline functions defined in the ComPlusUtils namespace provide simplified
// access to COM+ object and call context. The functions are divided into five
// logical groups:
// 
//   Object functions: CoGetObjectContext, CoGetCallContext, GetAspObject,
//            GetDeclarativeTransaction
//   Service detection functions: IsInActivity, IsInTransaction, IsInJitaContext
//   Id functions: GetContextId, GetActivityId, GetTransactionId
//   Happy/done bit functions: SetTransactionVote, GetTransactionVote,
//                             SetDeactivate, GetDeactivate,
//                             SetComplete, SetAbort, EnableCommit, DisableCommit
//   Security functions: IsSecurityEnabled, GetCallersName, IsCallerInRole
//
// Usage:
//
//   Here are some examples that show how these functions are used.
//
//   HRESULT SomeClass::SomeMethod()
//   {
//     CComPtr<IObjectContextInfo> spCtxInfo;
//     HRESULT hr = CoGetObjectContext(&spCtxInfo); // acquire reference to object context
//     if (FAILED(hr)) return hr;
//     ...
//
//   HRESULT SomeClass::SomeMethod()
//   {
//     CComPtr<Response> spResponse;           // type defined in asptlb.h
//     HRESULT hr = GetAspObject(&spResponse); // acquire reference to ASP object
//     if (FAILED(hr)) return hr;
//     hr = spResponse->Write(CComBSTR("bathysphere");
//     ...
//
//  HRESULT SomeClass::SomeMethod()
//  {
//     HRESULT hr = SetDeactivate(true);          // turn done bit on
//     if (FAILED(hr)) return hr;
//     SetTransactionVote(TxAbort);               // turn happy bit off
//     if (FAILED(hr)) return hr;
//
//     ... // do work with declarative transaction
//
//     SetTransactionVote(TxCommit);              // turn happy bit back on
//     if (FAILED(hr)) return hr;
//     return S_OK;
//  }
//
//  HRESULT SomeClass::SomeMethod()
//  {
//    bool bSec = false;
//    bool bInRole = false;
//   
//    HRESULT hr = IsSecurityEnabled(&bSec);      // check for access checks
//    if (FAILED(hr)) return hr;
//    else if (!bSec) return E_ACCESSDENIED;
//
//    hr = IsCallerInRole("SpiceGirl", &bInRole); // check for role membership
//    if (FAILED(hr)) return hr;
//    else if (!bInRole) return E_ACCESSDENIED;
// 
//    ...
//
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef __CPUTIL_H_
#define __CPUTIL_H_

#include <atlbase.h>
#include <comsvcs.h>
#include <asptlb.h>
#include <transact.h>

namespace ComPlusUtils
{

// retrieve reference to object context
template <class T>
inline HRESULT CoGetObjectContext(T** pp)
{
	return ::CoGetObjectContext(__uuidof(T), (void**)pp);
}

// retrieve reference to call context
template <class T>
inline HRESULT CoGetCallContext(T** pp)
{
	return ::CoGetCallContext(__uuidof(T), (void**)pp);
}

// retrieve reference to ASP intrinsic object from object context
template <class T>
inline HRESULT GetAspObject(LPOLESTR wszObj, T** pp)
{
	// retrieve object context reference
	CComPtr<IGetContextProperties> spCtxProp;
	HRESULT hr = CoGetObjectContext(&spCtxProp);
	if (FAILED(hr)) return hr;

	// retrieve ASP object reference
	CComVariant v;
	hr = spCtxProp->GetProperty(CComBSTR(wszObj), &v);
	if (FAILED(hr)) return hr;
	if (v.vt != VT_UNKNOWN && v.vt != VT_DISPATCH) return E_FAIL;

	// return requested pointer type
	return v.punkVal->QueryInterface(__uuidof(T), (void**)pp);
}

// retrieve reference to ASP intrinsic object from object context,
// inferring object from argument type
template <class T> inline HRESULT GetAspObject(T** pp);
template<> inline HRESULT GetAspObject(IResponse** pp) { return GetAspObject(L"Response", pp); }
template<> inline HRESULT GetAspObject(IRequest** pp) { return GetAspObject(L"Request", pp); }
template<> inline HRESULT GetAspObject(IServer** pp) { return GetAspObject(L"Server", pp); }
template<> inline HRESULT GetAspObject(ISessionObject** pp) { return GetAspObject(L"Session", pp); }
template<> inline HRESULT GetAspObject(IApplicationObject** pp) { return GetAspObject(L"Application", pp); }

// retrieve reference to transaction stream's distributed transaction
template <class T>
inline HRESULT GetDeclarativeTransaction(T** pp)
{
	// retrieve object context reference
	CComPtr<IObjectContextInfo> spCtxInfo;
	HRESULT hr = CoGetObjectContext(&spCtxInfo);
	if (FAILED(hr)) return hr;

	// retrieve transaction object reference
	CComPtr<IUnknown> spTx;
	hr = spCtxInfo->GetTransaction(&spTx);
	if (FAILED(hr)) return hr;

	// return requested pointer type
	return spTx->QueryInterface(__uuidof(T), (void**)pp);
}

// test for presence of an activity
inline bool IsInActivity()
{
	CComPtr<IObjectContextInfo> spCtxInfo;
	HRESULT hr = CoGetObjectContext(&spCtxInfo);
	if (FAILED(hr)) return false;
	GUID actId;
	hr = spCtxInfo->GetActivityId(&actId);
	if (FAILED(hr)) return false;
	return actId == GUID_NULL ? true : false;
}

// test for presence of a declarative transaction
inline bool IsInTransaction()
{
	CComPtr<IObjectContextInfo> spCtxInfo;
	HRESULT hr = CoGetObjectContext(&spCtxInfo);
	if (FAILED(hr)) return false;
	return spCtxInfo->IsInTransaction() ? true : false;
}

// test for presence of JITA
inline bool IsInJitaContext()
{
	CComPtr<IContextState> spCtxState;
	HRESULT hr = CoGetObjectContext(&spCtxState);
	if (FAILED(hr)) return false;
	VARIANT_BOOL vb;
	hr = spCtxState->GetDeactivateOnReturn(&vb);
	return hr == CONTEXT_E_NOJIT ? true : false;
}

// retrieve context id as GUID
inline HRESULT GetContextId(GUID *pId)
{
	CComPtr<IObjectContextInfo> spCtxInfo;
	HRESULT hr = CoGetObjectContext(&spCtxInfo);
	if (FAILED(hr)) return hr;
	return spCtxInfo->GetContextId(pId);
}

// retrieve context id as BSTR
inline HRESULT GetContextId(BSTR *bstrId)
{
	GUID ctxId;
	HRESULT hr = GetContextId(&ctxId);
	if (FAILED(hr)) return hr;
	return CComBSTR(ctxId).CopyTo(bstrId);
}

// retrieve activity id as GUID
inline HRESULT GetActivityId(GUID *pId)
{
	CComPtr<IObjectContextInfo> spCtxInfo;
	HRESULT hr = CoGetObjectContext(&spCtxInfo);
	if (FAILED(hr)) return hr;
	return spCtxInfo->GetActivityId(pId);
}

// retrieve activity id as BSTR
inline HRESULT GetActivityId(BSTR *bstrId)
{
	GUID actId;
	HRESULT hr = GetActivityId(&actId);
	if (FAILED(hr)) return hr;
	return CComBSTR(actId).CopyTo(bstrId);
}

// retrieve distributed transaction id as GUID
inline HRESULT GetDeclarativeTransactionId(GUID *pId)
{
	CComPtr<ITransaction> spTx;
	HRESULT hr = GetDeclarativeTransaction(&spTx);
	if (FAILED(hr)) return hr;
	XACTTRANSINFO info = {0};
	hr = spTx->GetTransactionInfo(&info);
	if (FAILED(hr)) return hr;
	*pId = *(GUID*)&info.uow;
	return S_OK;
}

// retrieve distributed transaction id as BSTR
inline HRESULT GetDeclarativeTransactionId(BSTR *bstrId)
{
	GUID ctxId;
	HRESULT hr = GetDeclarativeTransactionId(&ctxId);
	if (FAILED(hr)) return hr;
	return CComBSTR(ctxId).CopyTo(bstrId);
}


// turn happy bit on or off
inline HRESULT SetTransactionVote(TransactionVote vote)
{
	CComPtr<IContextState> spCtxState;
	HRESULT hr = CoGetObjectContext(&spCtxState);
	if (FAILED(hr)) return hr;
	return spCtxState->SetMyTransactionVote(vote);
}

// retrieve happy bit setting
inline HRESULT GetTransactionVote(TransactionVote *pvote)
{
	CComPtr<IContextState> spCtxState;
	HRESULT hr = CoGetObjectContext(&spCtxState);
	if (FAILED(hr)) return hr;
	return spCtxState->GetMyTransactionVote(pvote);
}

// turn done bit on or off
inline HRESULT SetDeactivate(bool b)
{
	CComPtr<IContextState> spCtxState;
	HRESULT hr = CoGetObjectContext(&spCtxState);
	if (FAILED(hr)) return hr;
	return spCtxState->SetDeactivateOnReturn(b ? VARIANT_TRUE : VARIANT_FALSE);
}

// retrieve done bit setting
inline HRESULT GetDeactivate(bool *pb)
{
	CComPtr<IContextState> spCtxState;
	HRESULT hr = CoGetObjectContext(&spCtxState);
	if (FAILED(hr)) return hr;
	VARIANT_BOOL vb = VARIANT_FALSE;
	hr = spCtxState->GetDeactivateOnReturn(&vb);
	*pb = (vb == VARIANT_TRUE);
	return hr;
}

// turn happy bit on and done bit on
inline HRESULT SetComplete()
{
	CComPtr<IObjectContext> spCtx;
	HRESULT hr = CoGetObjectContext(&spCtx);
	if (FAILED(hr)) return hr;
	return spCtx->SetComplete();
}

// turn happy bit off and done bit on
inline HRESULT SetAbort()
{
	CComPtr<IObjectContext> spCtx;
	HRESULT hr = CoGetObjectContext(&spCtx);
	if (FAILED(hr)) return hr;
	return spCtx->SetAbort();
}

// turn happy bit on and done bit off
inline HRESULT EnableCommit()
{
	CComPtr<IObjectContext> spCtx;
	HRESULT hr = CoGetObjectContext(&spCtx);
	if (FAILED(hr)) return hr;
	return spCtx->EnableCommit();
}

// turn happy bit off and done bit off
inline HRESULT DisableCommit()
{
	CComPtr<IObjectContext> spCtx;
	HRESULT hr = CoGetObjectContext(&spCtx);
	if (FAILED(hr)) return hr;
	return spCtx->DisableCommit();
}

// test for presence of access checks
inline HRESULT IsSecurityEnabled(bool *pb)
{
	CComPtr<ISecurityCallContext> spCallCtx;
	HRESULT hr = CoGetCallContext(&spCallCtx);
	if (FAILED(hr)) return hr;
	VARIANT_BOOL vb = VARIANT_FALSE;
	hr = spCallCtx->IsSecurityEnabled(&vb);
	*pb = (vb == VARIANT_TRUE);
	return hr;
}

// test for presence of caller in role
inline HRESULT GetCallersName(BSTR *pbstr)
{
    CComPtr<ISecurityCallContext> spCallCtx;
	HRESULT hr = CoGetCallContext(&spCallCtx);
    if (FAILED(hr)) return hr;

    CComVariant v;
    hr = spCallCtx->get_Item(CComBSTR("DirectCaller"), &v);
    if (FAILED(hr)) return hr;
    if (v.vt != VT_UNKNOWN && v.vt != VT_DISPATCH) return E_FAIL;

    CComPtr<ISecurityIdentityColl> spSecId;
    hr = v.punkVal->QueryInterface(__uuidof(spSecId), (void**)&spSecId);
    if (FAILED(hr)) return hr;

    v.Clear();
    hr = spSecId->get_Item(CComBSTR("AccountName"), &v);
    if (FAILED(hr)) return hr;
    if (v.vt != VT_BSTR) return E_FAIL;

	*pbstr = v.bstrVal;
	v.vt = VT_EMPTY;

	return S_OK;
}

// retrieve callers account name
inline HRESULT IsCallerInRole(BSTR bstrRole, bool *pb)
{
	CComPtr<ISecurityCallContext> spCallCtx;
	HRESULT hr = CoGetCallContext(&spCallCtx);
	if (FAILED(hr)) return hr;
	VARIANT_BOOL vb = VARIANT_FALSE;
	hr = spCallCtx->IsCallerInRole(bstrRole, &vb);
	*pb = (vb == VARIANT_TRUE);
	return hr;
}

} // end namespace

#ifndef __NO_USING_COMPLUSUTILS
using namespace ComPlusUtils;
#endif

#endif //__CPUTIL_H_


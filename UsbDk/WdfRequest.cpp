/**********************************************************************
* Copyright (c) 2013-2014  Red Hat, Inc.
*
* Developed by Daynix Computing LTD.
*
* Authors:
*     Dmitry Fleytman <dmitry@daynix.com>
*     Pavel Gurvich <pavel@daynix.com>
*
* This work is licensed under the terms of the BSD license. See
* the LICENSE file in the top-level directory.
*
**********************************************************************/

#include <ntddk.h>
#include "WdfRequest.h"

NTSTATUS CWdfRequest::SendAndForget(WDFIOTARGET Target)
{
    WDF_REQUEST_SEND_OPTIONS options;
    WDF_REQUEST_SEND_OPTIONS_INIT(&options, WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);

    NTSTATUS status = STATUS_SUCCESS;

    if (WdfRequestSend(m_Request, Target, &options))
    {
        Detach();
    }
    else
    {
        status = WdfRequestGetStatus(m_Request);
        SetStatus(status);
    }

    return status;
}

NTSTATUS CWdfRequest::SendWithCompletion(WDFIOTARGET Target, PFN_WDF_REQUEST_COMPLETION_ROUTINE CompletionFunc)
{
    auto status = STATUS_SUCCESS;

    WdfRequestSetCompletionRoutine(m_Request, CompletionFunc, nullptr);
    if (WdfRequestSend(m_Request, Target, WDF_NO_SEND_OPTIONS))
    {
        Detach();
    }
    else
    {
        status = WdfRequestGetStatus(m_Request);
        SetStatus(status);
    }

    return status;
}

NTSTATUS CWdfRequest::FetchSafeInputBuffer(WDFMEMORY &Buffer) const
{
    PVOID Ptr;
    size_t Length;

    auto status = WdfRequestRetrieveUnsafeUserInputBuffer(m_Request, 0, &Ptr, &Length);
    if (NT_SUCCESS(status))
    {
        status = WdfRequestProbeAndLockUserBufferForRead(m_Request, Ptr, Length, &Buffer);
    }

    return status;
}

NTSTATUS CWdfRequest::FetchSafeOutputBuffer(WDFMEMORY &Buffer) const
{
    PVOID Ptr;
    size_t Length;

    auto status = WdfRequestRetrieveUnsafeUserOutputBuffer(m_Request, 0, &Ptr, &Length);
    if (NT_SUCCESS(status))
    {
        status = WdfRequestProbeAndLockUserBufferForWrite(m_Request, Ptr, Length, &Buffer);
    }

    return status;
}

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

#include "FilterStrategy.h"
#include "trace.h"
#include "FilterStrategy.tmh"
#include "FilterDevice.h"
#include "ControlDevice.h"
#include "WdfRequest.h"

NTSTATUS CUsbDkFilterStrategy::PNPPreProcess(PIRP Irp)
{
    IoSkipCurrentIrpStackLocation(Irp);
    return WdfDeviceWdmDispatchPreprocessedIrp(m_Owner->WdfObject(), Irp);
}

void CUsbDkFilterStrategy::ForwardRequest(WDFREQUEST Request)
{
    CWdfRequest WdfRequest(Request);

    auto status = WdfRequest.SendAndForget(m_Owner->IOTarget());
    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_FILTERSTRATEGY, "%!FUNC! failed %!STATUS!", status);
    }
}

void CUsbDkFilterStrategy::IoDeviceControl(WDFREQUEST Request,
                                           size_t OutputBufferLength, size_t InputBufferLength,
                                           ULONG IoControlCode)
{
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(IoControlCode);
    ForwardRequest(Request);
}

void CUsbDkFilterStrategy::WritePipe(WDFREQUEST Request, size_t Length)
{
    UNREFERENCED_PARAMETER(Length);
    ForwardRequest(Request);
}

void CUsbDkFilterStrategy::ReadPipe(WDFREQUEST Request, size_t Length)
{
    UNREFERENCED_PARAMETER(Length);
    ForwardRequest(Request);
}

void CUsbDkFilterStrategy::IoInCallerContext(WDFDEVICE Device, WDFREQUEST Request)
{
    auto status = WdfDeviceEnqueueRequest(Device, Request);
    if (!NT_SUCCESS(status))
    {
        WdfRequestComplete(Request, status);
    }
}

NTSTATUS CUsbDkFilterStrategy::Create(CUsbDkFilterDevice *Owner)
{
    m_Owner = Owner;

    m_ControlDevice = CUsbDkControlDevice::Reference(Owner->GetDriverHandle());
    if (m_ControlDevice == nullptr)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

void CUsbDkFilterStrategy::Delete()
{
    if (m_ControlDevice != nullptr)
    {
        CUsbDkControlDevice::Release();
    }
}

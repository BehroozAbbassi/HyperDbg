/**
 * @file MsrHandlers.h
 * @author Sina Karvandi (sina@rayanfam.com)
 * @brief Headers to Handle for MSR-related tasks in VMX-root
 * 
 * @version 0.1
 * @date 2021-12-24
 * 
 * @copyright This project is released under the GNU Public License v3.
 * 
 */
#pragma once

//////////////////////////////////////////////////
//				    Functions					//
//////////////////////////////////////////////////

VOID
MsrHandleRdmsrVmexit(PGUEST_REGS GuestRegs);

VOID
MsrHandleWrmsrVmexit(PGUEST_REGS GuestRegs);

BOOLEAN
MsrHandleSetMsrBitmap(ULONG64 Msr, INT ProcessorID, BOOLEAN ReadDetection, BOOLEAN WriteDetection);

BOOLEAN
MsrHandleUnSetMsrBitmap(ULONG64 Msr, INT ProcessorID, BOOLEAN ReadDetection, BOOLEAN WriteDetection);

VOID
MsrHandlePerformMsrBitmapReadChange(UINT64 MsrMask);

VOID
MsrHandlePerformMsrBitmapReadReset();

VOID
MsrHandlePerformMsrBitmapWriteChange(UINT64 MsrMask);

VOID
MsrHandlePerformMsrBitmapWriteReset();

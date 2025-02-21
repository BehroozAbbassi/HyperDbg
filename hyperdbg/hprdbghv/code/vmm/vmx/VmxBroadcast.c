/**
 * @file VmxBroadcast.c
 * @author Sina Karvandi (sina@rayanfam.com)
 * @brief Broadcast mechanism in vmx-root
 * @details
 * @version 0.1
 * @date 2021-12-31
 * 
 * @copyright This project is released under the GNU Public License v3.
 * 
 */
#include "..\hprdbghv\pch.h"

/**
 * @brief Broadcast NMI in vmx-root mode
 * @details caller to this function should take actions to the current core
 * the NMI won't be triggered for the current core
 * 
 * @param CurrentCoreIndex
 * @param VmxBroadcastAction
 * @return VOID 
 */
VOID
VmxBroadcastNmi(UINT32 CurrentCoreIndex, NMI_BROADCAST_ACTION_TYPE VmxBroadcastAction)
{
    ULONG CoreCount;

    CoreCount = KeQueryActiveProcessorCount(0);

    //
    // Indicate that we're waiting for NMI
    //
    for (size_t i = 0; i < CoreCount; i++)
    {
        if (i != CurrentCoreIndex)
        {
            g_GuestState[i].DebuggingState.WaitingForNmi      = TRUE;
            g_GuestState[i].DebuggingState.NmiBroadcastAction = VmxBroadcastAction;
        }
    }

    //
    // make sure, nobody is in the middle of sending anything
    //
    SpinlockLock(&DebuggerResponseLock);

    //
    // Broadcast NMI through APIC (xAPIC or x2APIC)
    //
    ApicTriggerGenericNmi();

    SpinlockUnlock(&DebuggerResponseLock);
}

/**
 * @brief Handle broadcast NMIs for halting cores in vmx-root mode
 * 
 * @param CurrentCoreIndex
 * @param GuestRegs
 * @param IsOnVmxNmiHandler
 *
 * @return VOID 
 */
VOID
VmxBroadcastHandleKdDebugBreaks(UINT32 CurrentCoreIndex, PGUEST_REGS GuestRegs, BOOLEAN IsOnVmxNmiHandler)
{
    if (IsOnVmxNmiHandler)
    {
        //
        // Indicate that it's called from NMI handle, and it relates to
        // halting the debuggee
        //
        g_GuestState[CurrentCoreIndex].DebuggingState.NmiCalledInVmxRootRelatedToHaltDebuggee = TRUE;

        //
        // If the core was in the middle of spinning on the spinlock
        // of getting the debug lock, this mechansim is not needed,
        // but if the core is not spinning there or the core is processing
        // a random vm-exit, then we inject an immediate vm-exit after vm-entry
        // or inject a DPC
        // this is used for two reasons.
        //
        //      1. first, we will get the registers (context) to halt the core
        //      2. second, it guarantees that if the NMI arrives within any
        //         instruction in vmx-root mode, then we injected an immediate
        //         vm-exit and we won't miss any cpu cycle in the guest
        //
        // KdFireDpc(KdHaltCoreInTheCaseOfHaltedFromNmiInVmxRoot, NULL);
        // VmxMechanismCreateImmediateVmexit(CurrentCoreIndex);
        HvSetMonitorTrapFlag(TRUE);
    }
    else
    {
        //
        // Handle core break
        //
        KdHandleNmi(CurrentCoreIndex, GuestRegs);
    }
}

/**
 * @brief Handle broadcast NMIs in vmx-root mode
 * 
 * @param CurrentCoreIndex
 * @param GuestRegisters
 * @param IsOnVmxNmiHandler
 *
 * @return VOID 
 */
VOID
VmxBroadcastNmiHandler(UINT32 CurrentCoreIndex, PGUEST_REGS GuestRegs, BOOLEAN IsOnVmxNmiHandler)
{
    switch (g_GuestState[CurrentCoreIndex].DebuggingState.NmiBroadcastAction)
    {
    case NMI_BROADCAST_ACTION_KD_HALT_CORE:

        //
        // Handle NMI of halt the other cores
        //
        VmxBroadcastHandleKdDebugBreaks(CurrentCoreIndex, GuestRegs, IsOnVmxNmiHandler);

        break;

    default:

        // LogError("Err, invalid NMI reason received");

        break;
    }

    //
    // Set NMI broadcasting action to none
    //
    g_GuestState[CurrentCoreIndex].DebuggingState.NmiBroadcastAction = NMI_BROADCAST_ACTION_NONE;
}

/**
 * @file VmxRegions.c
 * @author Sina Karvandi (sina@rayanfam.com)
 * @brief Implement allocations for VMX Regions (VMXON Region, VMCS, MSR Bitmap and etc.)
 * @details
 * @version 0.1
 * @date 2020-04-11
 * 
 * @copyright This project is released under the GNU Public License v3.
 * 
 */
#include "..\hprdbghv\pch.h"

/**
 * @brief Allocates Vmxon region and set the Revision ID based on IA32_VMX_BASIC_MSR
 * 
 * @param CurrentGuestState 
 * @return BOOLEAN Returns true if allocation was successfull and vmxon executed without error
 * otherwise returns false
 */
BOOLEAN
VmxAllocateVmxonRegion(VIRTUAL_MACHINE_STATE * CurrentGuestState)
{
    PHYSICAL_ADDRESS   PhysicalMax = {0};
    IA32_VMX_BASIC_MSR VmxBasicMsr = {0};
    int                VmxonSize;
    int                VmxonStatus;
    BYTE *             VmxonRegion;
    UINT64             VmxonRegionPhysicalAddr;
    UINT64             AlignedVmxonRegion;
    UINT64             AlignedVmxonRegionPhysicalAddr;

    //
    // at IRQL > DISPATCH_LEVEL memory allocation routines don't work
    //
    if (KeGetCurrentIrql() > DISPATCH_LEVEL)
        KeRaiseIrqlToDpcLevel();

    PhysicalMax.QuadPart = MAXULONG64;

    VmxonSize = 2 * VMXON_SIZE;

    //
    // Allocating a 4-KByte Contigous Memory region
    //
    VmxonRegion = MmAllocateContiguousMemory(VmxonSize + ALIGNMENT_PAGE_SIZE, PhysicalMax);

    if (VmxonRegion == NULL)
    {
        LogError("Err, couldn't allocate buffer for VMXON region");
        return FALSE;
    }

    VmxonRegionPhysicalAddr = VirtualAddressToPhysicalAddress(VmxonRegion);

    //
    // zero-out memory
    //
    RtlSecureZeroMemory(VmxonRegion, VmxonSize + ALIGNMENT_PAGE_SIZE);

    AlignedVmxonRegion = (BYTE *)((ULONG_PTR)(VmxonRegion + ALIGNMENT_PAGE_SIZE - 1) & ~(ALIGNMENT_PAGE_SIZE - 1));
    LogDebugInfo("VMXON Region Address : %llx", AlignedVmxonRegion);

    //
    // 4 kb >= buffers are aligned, just a double check to ensure if it's aligned
    //
    AlignedVmxonRegionPhysicalAddr = (BYTE *)((ULONG_PTR)(VmxonRegionPhysicalAddr + ALIGNMENT_PAGE_SIZE - 1) & ~(ALIGNMENT_PAGE_SIZE - 1));
    LogDebugInfo("VMXON Region Physical Address : %llx", AlignedVmxonRegionPhysicalAddr);

    //
    // get IA32_VMX_BASIC_MSR RevisionId
    //
    VmxBasicMsr.All = __readmsr(IA32_VMX_BASIC);
    LogDebugInfo("Revision Identifier (IA32_VMX_BASIC - MSR 0x480) : 0x%x", VmxBasicMsr.Fields.RevisionIdentifier);

    //
    //Changing Revision Identifier
    //
    *(UINT64 *)AlignedVmxonRegion = VmxBasicMsr.Fields.RevisionIdentifier;

    //
    // Execute Vmxon instruction
    //
    VmxonStatus = __vmx_on(&AlignedVmxonRegionPhysicalAddr);
    if (VmxonStatus)
    {
        LogError("Err, executing vmxon instruction failed with status : %d", VmxonStatus);
        return FALSE;
    }

    CurrentGuestState->VmxonRegionPhysicalAddress = AlignedVmxonRegionPhysicalAddr;

    //
    // We save the allocated buffer (not the aligned buffer) because we want to free it in vmx termination
    //
    CurrentGuestState->VmxonRegionVirtualAddress = VmxonRegion;

    return TRUE;
}

/**
 * @brief Allocate Vmcs region and set the Revision ID based on IA32_VMX_BASIC_MSR
 * 
 * @param CurrentGuestState 
 * @return BOOLEAN Returns true if allocation was successfull and vmptrld executed without error
 * otherwise returns false
 */
BOOLEAN
VmxAllocateVmcsRegion(VIRTUAL_MACHINE_STATE * CurrentGuestState)
{
    PHYSICAL_ADDRESS   PhysicalMax = {0};
    int                VmcsSize;
    BYTE *             VmcsRegion;
    UINT64             VmcsPhysicalAddr;
    UINT64             AlignedVmcsRegion;
    UINT64             AlignedVmcsRegionPhysicalAddr;
    IA32_VMX_BASIC_MSR VmxBasicMsr = {0};

    //
    // at IRQL > DISPATCH_LEVEL memory allocation routines don't work
    //
    if (KeGetCurrentIrql() > DISPATCH_LEVEL)
        KeRaiseIrqlToDpcLevel();

    PhysicalMax.QuadPart = MAXULONG64;

    VmcsSize   = 2 * VMCS_SIZE;
    VmcsRegion = MmAllocateContiguousMemory(VmcsSize + ALIGNMENT_PAGE_SIZE, PhysicalMax); // Allocating a 4-KByte Contigous Memory region

    if (VmcsRegion == NULL)
    {
        LogError("Err, couldn't allocate Buffer for VMCS region");
        return FALSE;
    }
    RtlSecureZeroMemory(VmcsRegion, VmcsSize + ALIGNMENT_PAGE_SIZE);

    VmcsPhysicalAddr = VirtualAddressToPhysicalAddress(VmcsRegion);

    AlignedVmcsRegion = (BYTE *)((ULONG_PTR)(VmcsRegion + ALIGNMENT_PAGE_SIZE - 1) & ~(ALIGNMENT_PAGE_SIZE - 1));
    LogDebugInfo("VMCS region address : %llx", AlignedVmcsRegion);

    AlignedVmcsRegionPhysicalAddr = (BYTE *)((ULONG_PTR)(VmcsPhysicalAddr + ALIGNMENT_PAGE_SIZE - 1) & ~(ALIGNMENT_PAGE_SIZE - 1));
    LogDebugInfo("VMCS region physical address : %llx", AlignedVmcsRegionPhysicalAddr);

    //
    // get IA32_VMX_BASIC_MSR RevisionId
    //
    VmxBasicMsr.All = __readmsr(IA32_VMX_BASIC);
    LogDebugInfo("Revision Identifier (IA32_VMX_BASIC - MSR 0x480) : 0x%x", VmxBasicMsr.Fields.RevisionIdentifier);

    //
    //Changing Revision Identifier
    //
    *(UINT64 *)AlignedVmcsRegion = VmxBasicMsr.Fields.RevisionIdentifier;

    CurrentGuestState->VmcsRegionPhysicalAddress = AlignedVmcsRegionPhysicalAddr;
    //
    // We save the allocated buffer (not the aligned buffer)
    // because we want to free it in vmx termination
    //
    CurrentGuestState->VmcsRegionVirtualAddress = VmcsRegion;

    return TRUE;
}

/**
 * @brief Allocate VMM Stack
 * 
 * @param ProcessorID Logical Core Id
 * @return BOOLEAN Returns true if allocation was successfull otherwise returns false
 */
BOOLEAN
VmxAllocateVmmStack(INT ProcessorID)
{
    //
    // Allocate stack for the VM Exit Handler
    //
    g_GuestState[ProcessorID].VmmStack = ExAllocatePoolWithTag(NonPagedPool, VMM_STACK_SIZE, POOLTAG);

    if (g_GuestState[ProcessorID].VmmStack == NULL ||
        g_GuestState[ProcessorID].VmmStack == NULL)
    {
        LogError("Err, insufficient memory in allocationg vmm stack");
        return FALSE;
    }
    RtlZeroMemory(g_GuestState[ProcessorID].VmmStack, VMM_STACK_SIZE);

    LogDebugInfo("VMM Stack for logical processor : 0x%llx", g_GuestState[ProcessorID].VmmStack);

    return TRUE;
}

/**
 * @brief Allocate a buffer forr Msr Bitmap
 * 
 * @param ProcessorID 
 * @return BOOLEAN Returns true if allocation was successfull otherwise returns false
 */
BOOLEAN
VmxAllocateMsrBitmap(INT ProcessorID)
{
    //
    // Allocate memory for MSR Bitmap
    // Should be aligned
    //
    g_GuestState[ProcessorID].MsrBitmapVirtualAddress = ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, POOLTAG);

    if (g_GuestState[ProcessorID].MsrBitmapVirtualAddress == NULL)
    {
        LogError("Err, insufficient memory in allocationg MSR Bitmaps");
        return FALSE;
    }
    RtlZeroMemory(g_GuestState[ProcessorID].MsrBitmapVirtualAddress, PAGE_SIZE);

    g_GuestState[ProcessorID].MsrBitmapPhysicalAddress = VirtualAddressToPhysicalAddress(g_GuestState[ProcessorID].MsrBitmapVirtualAddress);

    LogDebugInfo("MSR Bitmap virtual address : 0x%llx", g_GuestState[ProcessorID].MsrBitmapVirtualAddress);
    LogDebugInfo("MSR Bitmap physical address : 0x%llx", g_GuestState[ProcessorID].MsrBitmapPhysicalAddress);

    return TRUE;
}

/**
 * @brief Allocate a buffer forr I/O Bitmap
 * 
 * @param ProcessorID 
 * @return BOOLEAN Returns true if allocation was successfull otherwise returns false
 */
BOOLEAN
VmxAllocateIoBitmaps(INT ProcessorID)
{
    //
    // Allocate memory for I/O Bitmap (A)
    //
    g_GuestState[ProcessorID].IoBitmapVirtualAddressA = ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, POOLTAG); // should be aligned

    if (g_GuestState[ProcessorID].IoBitmapVirtualAddressA == NULL)
    {
        LogError("Err, insufficient memory in allocationg I/O Bitmaps A");
        return FALSE;
    }
    RtlZeroMemory(g_GuestState[ProcessorID].IoBitmapVirtualAddressA, PAGE_SIZE);

    g_GuestState[ProcessorID].IoBitmapPhysicalAddressA = VirtualAddressToPhysicalAddress(g_GuestState[ProcessorID].IoBitmapVirtualAddressA);

    LogDebugInfo("I/O Bitmap A Virtual Address : 0x%llx", g_GuestState[ProcessorID].IoBitmapVirtualAddressA);
    LogDebugInfo("I/O Bitmap A Physical Address : 0x%llx", g_GuestState[ProcessorID].IoBitmapPhysicalAddressA);

    //
    // Allocate memory for I/O Bitmap (B)
    //
    g_GuestState[ProcessorID].IoBitmapVirtualAddressB = ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, POOLTAG); // should be aligned

    if (g_GuestState[ProcessorID].IoBitmapVirtualAddressB == NULL)
    {
        LogError("Err, insufficient memory in allocationg I/O Bitmaps B");
        return FALSE;
    }
    RtlZeroMemory(g_GuestState[ProcessorID].IoBitmapVirtualAddressB, PAGE_SIZE);

    g_GuestState[ProcessorID].IoBitmapPhysicalAddressB = VirtualAddressToPhysicalAddress(g_GuestState[ProcessorID].IoBitmapVirtualAddressB);

    LogDebugInfo("I/O Bitmap B virtual address : 0x%llx", g_GuestState[ProcessorID].IoBitmapVirtualAddressB);
    LogDebugInfo("I/O Bitmap B physical address : 0x%llx", g_GuestState[ProcessorID].IoBitmapPhysicalAddressB);

    return TRUE;
}

/**
 * @file pch.h
 * @author Sina Karvandi (sina@rayanfam.com)
 * @author Alee Amini (aleeaminiz@gmail.com)
 * @brief Pre-compiled headers 
 * @details 
 *
 * @version 0.1
 * @date 2020-07-13
 * 
 * @copyright This project is released under the GNU Public License v3.
 * 
 */
#pragma once

#define _NO_CRT_STDIO_INLINE

//
// Windows defined functions
//
#include <ntifs.h>
#include <ntddk.h>
#include <wdf.h>
#include <wdm.h>
#include <ntstrsafe.h>
#include <Windef.h>

//
// HyperDbg Kernel-mode headers
//
#include "Definition.h"
#include "Configuration.h"
#include "..\hprdbghv\header\common\Dpc.h"
#include "..\hprdbghv\header\common\LengthDisassemblerEngine.h"
#include "..\hprdbghv\header\common\Logging.h"
#include "..\hprdbghv\header\memory\MemoryMapper.h"
#include "..\hprdbghv\header\common\Msr.h"
#include "..\hprdbghv\header\debugger\tests\KernelTests.h"
#include "..\hprdbghv\header\memory\PoolManager.h"
#include "..\hprdbghv\header\common\Trace.h"
#include "..\hprdbghv\header\debugger\broadcast\DpcRoutines.h"
#include "..\hprdbghv\header\misc\InlineAsm.h"
#include "..\hprdbghv\header\vmm\ept\Vpid.h"
#include "..\hprdbghv\header\vmm\ept\Ept.h"
#include "..\hprdbghv\header\vmm\vmx\Events.h"
#include "..\hprdbghv\header\common\Common.h"
#include "..\hprdbghv\header\common\List.h"
#include "..\hprdbghv\header\debugger\core\Debugger.h"
#include "..\hprdbghv\header\devices\Apic.h"
#include "..\hprdbghv\header\debugger\core\Kd.h"
#include "..\hprdbghv\header\vmm\vmx\Mtf.h"
#include "..\hprdbghv\header\debugger\communication\GdbStub.h"
#include "..\hprdbghv\header\debugger\core\DebuggerEvents.h"
#include "..\hprdbghv\header\debugger\features\Hooks.h"
#include "..\hprdbghv\header\vmm\vmx\Counters.h"
#include "..\hprdbghv\header\debugger\transparency\Transparency.h"
#include "..\hprdbghv\header\vmm\vmx\IdtEmulation.h"
#include "..\hprdbghv\header\vmm\ept\Invept.h"
#include "..\hprdbghv\header\debugger\broadcast\Broadcast.h"
#include "..\hprdbghv\header\vmm\vmx\Vmcall.h"
#include "..\hprdbghv\header\vmm\vmx\ManageRegs.h"
#include "..\hprdbghv\header\vmm\vmx\Vmx.h"
#include "..\hprdbghv\header\debugger\commands\BreakpointCommands.h"
#include "..\hprdbghv\header\debugger\commands\DebuggerCommands.h"
#include "..\hprdbghv\header\debugger\commands\ExtensionCommands.h"
#include "..\hprdbghv\header\debugger\communication\SerialConnection.h"
#include "..\hprdbghv\header\debugger\objects\Process.h"
#include "..\hprdbghv\header\debugger\objects\Thread.h"
#include "..\hprdbghv\header\components\registers\DebugRegisters.h"
#include "..\hprdbghv\header\vmm\vmx\HypervisorRoutines.h"
#include "..\hprdbghv\header\vmm\vmx\ProtectedHvRoutines.h"
#include "..\hprdbghv\header\vmm\vmx\IoHandler.h"
#include "..\hprdbghv\header\debugger\core\Steppings.h"
#include "..\hprdbghv\header\debugger\core\Termination.h"
#include "ScriptEngineCommonDefinitions.h"

//
// Global Variables should be the last header to include
//
#include "..\hprdbghv\header\misc\GlobalVariables.h"

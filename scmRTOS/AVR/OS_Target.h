//******************************************************************************
//*
//*     FULLNAME:  Single-Chip Microcontroller Real-Time Operating System
//*
//*     NICKNAME:  scmRTOS
//*
//*     PROCESSOR: AVR (Atmel)
//*
//*     TOOLKIT:   avr-gcc (GNU)
//*
//*     PURPOSE:   Target Dependent Stuff Header. Declarations And Definitions
//*
//*     Version: 4.00
//*
//*     $Revision: 528 $
//*     $Date:: 2012-04-04 #$
//*
//*     Copyright (c) 2003-2012, Harry E. Zhurov
//*
//*     Permission is hereby granted, free of charge, to any person
//*     obtaining  a copy of this software and associated documentation
//*     files (the "Software"), to deal in the Software without restriction,
//*     including without limitation the rights to use, copy, modify, merge,
//*     publish, distribute, sublicense, and/or sell copies of the Software,
//*     and to permit persons to whom the Software is furnished to do so,
//*     subject to the following conditions:
//*
//*     The above copyright notice and this permission notice shall be included
//*     in all copies or substantial portions of the Software.
//*
//*     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//*     EXPRESS  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//*     MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//*     IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//*     CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//*     TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
//*     THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//*
//*     =================================================================
//*     See http://scmrtos.sourceforge.net for documentation, latest
//*     information, license and contact details.
//*     =================================================================
//*
//******************************************************************************
//*     avr-gcc port by Oleksandr O. Redchuk, Copyright (c) 2007-2012

#ifndef scmRTOS_AVR_H
#define scmRTOS_AVR_H

#include <avr/io.h>
#include <avr/interrupt.h>

//------------------------------------------------------------------------------
//
//    Compiler and Target checks
//
//
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#ifndef __GNUC__
#error "This file should only be compiled with GCC C++ Compiler"
#endif // __GNUC__

#ifndef __AVR_ARCH__
#error "This file must be compiled for AVR processor only."
#endif

#ifndef SPH
#error "AVRs with 1-byte stack pointer (no more than 160 bytes of RAM) not suported by scmRTOS"
#endif

//
#if (__GNUC__ < 3)
#error "This file should be compiled by AVR-GCC C/C++ Compiler v3.0 or higher."
#endif

// EIND is defined in some h-files of devices with no more than 128K flash
// (for example atmega32u4, atmega1281) but it is not needed for this devices
// EIND is only needed for devices with 3-byte PC and __AVR_3_BYTE_PC__
// is checked below.
// Strictly, __AVR_HAVE_EIJMP_EICALL__ must be used for EIND handling, but
// the macro is introduced since avr-gcc 4.2.2 while atmega256x support and
// __AVR_3_BYTE_PC__ macro since avr-gcc 4.1.2

#ifdef __AVR_3_BYTE_PC__
# define SAVE_EIND 1
#endif

// RAMPZ is defined in h-files for some devices with no more than 64K flash (like atmega32u4)

#if GCC_VERSION > 40300
# ifdef __AVR_HAVE_RAMPZ__
#   define SAVE_RAMPZ 1
# endif
#else
# ifdef RAMPZ
#   define SAVE_RAMPZ 1
# endif
#endif


#ifndef __ASSEMBLER__

//------------------------------------------------------------------------------
//
//    Compiler specific attributes
//
//
#ifndef INLINE
#define INLINE      __attribute__((__always_inline__)) inline
#endif

#ifndef NOINLINE
#define NOINLINE    __attribute__((__noinline__))
#endif

#ifndef NORETURN
#define NORETURN    __attribute__((__noreturn__))
#endif


//------------------------------------------------------------------------------
//
//    Target specific types
//
//
typedef uint8_t stack_item_t;
typedef uint8_t status_reg_t;

//-----------------------------------------------------------------------------
//
//      Target specific macros
//

#define SEPARATE_RETURN_STACK   0

#if GCC_VERSION >= 40202
#define OS_PROCESS              __attribute__((OS_task))
#else
#define OS_PROCESS              __attribute__((__noreturn__))
#endif

#define INLINE_PROCESS_CTOR   INLINE

#ifdef __INTR_ATTRS /* default ISR attributes from avr/interrupt.h */
# define    OS_INTERRUPT    extern "C" __attribute__((__signal__,__INTR_ATTRS))
#else
# define    OS_INTERRUPT    extern "C" __attribute__((__signal__))
#endif

#define DUMMY_INSTR()   __asm__ __volatile__ ("nop" : : )

//--------------------------------------------------
//
//   Uncomment macro value below for SystemTimer() run in critical section
//
//   This is useful (and necessary) when target processor has hardware
//   enabled nested interrups. AVR does not have such interrupts.
//
#define SYS_TIMER_CRIT_SECT()           // TCritSect cs
#define CONTEXT_SWITCH_HOOK_CRIT_SECT() // TCritSect cs


//-----------------------------------------------------------------------------
//
//     Include project-level configurations
//    !!! The order of includes is important !!!
//
#include "scmRTOS_CONFIG.h"
#include "scmRTOS_TARGET_CFG.h"
#include "scmRTOS_defs.h"

//-----------------------------------------------------------------------------
//
//    Target-specific configuration macros
//
#ifdef scmRTOS_USER_DEFINED_STACK_PATTERN
#define scmRTOS_STACK_PATTERN scmRTOS_USER_DEFINED_STACK_PATTERN
#else
#define scmRTOS_STACK_PATTERN 0x5AU
#endif

#define scmRTOS_TARGET_IDLE_HOOK_ENABLE 0

//-----------------------------------------------------------------------------
//      allow some optimization
//extern "C" NORETURN void os_start(stack_item_t* sp);


//-----------------------------------------------------------------------------
//
//     The Critital Section Wrapper
//
//
class TCritSect
{
public:
    TCritSect () : StatusReg( SREG ) { cli(); }
    ~TCritSect() { SREG = StatusReg; }
    //~TCritSect() { if( StatusReg & (1 << SREG_I)) sei(); }

private:
    status_reg_t StatusReg;

};


//-----------------------------------------------------------------------------
//
//     Priority stuff
//
//
namespace OS
{
struct TPrioMaskTable
{
    TPrioMaskTable()
    {
        TProcessMap pm = 0x01;
        for(uint8_t i = 0; i < sizeof(Table)/sizeof(Table[0]); i++)
        {
            Table[i] = pm;
            pm <<= 1;
        }
    }

    TProcessMap Table[scmRTOS_PROCESS_COUNT+1];
};
extern TPrioMaskTable PrioMaskTable;

INLINE OS::TProcessMap get_prio_tag(const uint_fast8_t pr) { return PrioMaskTable.Table[pr]; }

#if scmRTOS_PRIORITY_ORDER == 0
    INLINE uint8_t highest_priority(TProcessMap pm)
    {
        uint8_t pr = 0;

        while( !(pm & 0x0001) )
        {
            pr++;
            pm >>= 1;
        }
        return pr;
    }
#else
    INLINE uint8_t highest_priority(TProcessMap pm)
    {
        uint8_t pr = scmRTOS_PROCESS_COUNT;

        while( !(pm & (1 << scmRTOS_PROCESS_COUNT) ) )
        {
            pr--;
            pm <<= 1;
        }
        return pr;
    }
#endif // scmRTOS_PRIORITY_ORDER
}


//-----------------------------------------------------------------------------
//
//     Interrupt and Interrupt Service Routines support
//


INLINE status_reg_t get_interrupt_state()                { return SREG; }
INLINE void         set_interrupt_state(status_reg_t sr) { SREG = sr; }

INLINE void enable_interrupts()  { sei(); }
INLINE void disable_interrupts() { cli(); }

INLINE stack_item_t* get_stack_pointer() { return reinterpret_cast<stack_item_t*>(SP); }
INLINE void  set_stack_pointer(stack_item_t* sp) { SP = reinterpret_cast<uint16_t>(sp); }

INLINE void set_isr_stack_pointer()
{
    // OS::run() never returns and main stack can be used as ISR stack 
    // symbol from gcrt1.S, initial stack value, set to RAMEND by default
    extern uint8_t __stack;
    SP = reinterpret_cast<uint16_t>(&__stack);
}

namespace OS
{
    INLINE void enable_context_switch()  { enable_interrupts();  }
    INLINE void disable_context_switch() { disable_interrupts(); }
}

#if scmRTOS_OBSOLETE_NAMES == 1

INLINE status_reg_t GetInterruptState( )               { return get_interrupt_state(); }
INLINE void         SetInterruptState(status_reg_t sr) { set_interrupt_state(sr);      }

INLINE void EnableInterrupts()  { enable_interrupts();  }
INLINE void DisableInterrupts() { disable_interrupts(); }

INLINE stack_item_t* GetStackPointer()                 { return get_stack_pointer(); }
INLINE void          SetStackPointer(stack_item_t* sp) { set_stack_pointer(sp);      }
INLINE void          SetISRStackPointer()              { set_isr_stack_pointer();    }

namespace OS
{
    INLINE void EnableContextSwitch()  { enable_context_switch();  }
    INLINE void DisableContextSwitch() { disable_context_switch(); }
}

#endif // scmRTOS_OBSOLETE_NAMES

//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#include <OS_Kernel.h>

///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
namespace OS
{

    //--------------------------------------------------------------------------
    //
    //      NAME       :   OS ISR support
    //
    //      PURPOSE    :   Implements common actions on interrupt enter and exit
    //                     under the OS
    //
    //      DESCRIPTION:
    //
    //
    class TISRW
    {
    public:
        INLINE  TISRW()  { ISR_Enter(); }
        INLINE  ~TISRW() { ISR_Exit();  }

    private:
        //-----------------------------------------------------
        INLINE void ISR_Enter() // volatile
        {
            Kernel.ISR_NestCount++;
        }
        //-----------------------------------------------------
        INLINE void ISR_Exit()
        {
            disable_interrupts();
            if(--Kernel.ISR_NestCount) return;
            Kernel.sched_isr();
        }
        //-----------------------------------------------------
    };

    class TISRW_SS
    {
    public:
        INLINE TISRW_SS()  { ISR_Enter(); }
        INLINE ~TISRW_SS() { ISR_Exit();  }

    private:
        //-----------------------------------------------------
        INLINE void ISR_Enter() // volatile
        {
            if(Kernel.ISR_NestCount++ == 0)
            {
                Kernel.ProcessTable[Kernel.CurProcPriority]->StackPointer = get_stack_pointer();
                set_isr_stack_pointer();
            }
        }
        //-----------------------------------------------------
        INLINE void ISR_Exit()
        {
            disable_interrupts();
            if(--Kernel.ISR_NestCount) return;
            set_stack_pointer(Kernel.ProcessTable[Kernel.CurProcPriority]->StackPointer);
            Kernel.sched_isr();
        }
        //-----------------------------------------------------
    };
}
//-----------------------------------------------------------------------------

#endif // __ASSEMBLER__

#endif // scmRTOS_AVR_H


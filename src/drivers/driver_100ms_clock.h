

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __DRIVER_100MS_CLOCK__
#define	__DRIVER_100MS_CLOCK__

#include <xc.h> // include processor files - each processor file is guarded.


// Assign the function pointer to where the timer tick should call out to
// WARNING: Is in the context of the interrupt, be careful
// void func()
typedef void (*_100ms_timer_callback_func_t) ();

extern void Driver100msClock_Initialization(_100ms_timer_callback_func_t _100ms_callback_func);

extern void Driver100msClock_pause_100ms_timer();

extern void Driver100msClock_resume_100ms_timer();


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __DRIVER_100MS_CLOCK__ */


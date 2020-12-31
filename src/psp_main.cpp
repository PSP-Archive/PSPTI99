#include <stdio.h>
#include <zlib.h>
#include "SDL.h"

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <pspthreadman.h>
#include <stdlib.h>
#include <stdio.h>
#include "psp_irkeyb.h"

#define STDOUT_FILE	"stdout.txt"
#define STDERR_FILE	"stderr.txt"

/* If application's main() is redefined as SDL_main, and libSDLmain is
   linked, then this file will create the standard exit callback,
   define the PSP_* macros, add an exception handler, nullify device 
   checks and exit back to the browser when the program is finished. */

extern int a_main(int argc, char *argv[]);

static void cleanup_output(void);

#ifndef LINUX_MODE
# ifndef PSPFW30X
PSP_MODULE_INFO("PSPTI99", 0x1000, 1, 1);
# else
PSP_MODULE_INFO("PSPTI99", 0x0, 1, 1);
PSP_HEAP_SIZE_KB(12*1024);
# endif
PSP_MAIN_THREAD_ATTR(0);
PSP_MAIN_THREAD_STACK_SIZE_KB(64);
#endif

#ifndef LINUX_MODE
int 
sdl_psp_exit_callback(int arg1, int arg2, void *common)
{
  cleanup_output();
  sceKernelExitGame();
	return 0;
}

int sdl_psp_callback_thread(SceSize args, void *argp)
{
	int cbid;
	cbid = sceKernelCreateCallback("Exit Callback", 
				       sdl_psp_exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();
	return 0;
}

int sdl_psp_setup_callbacks(void)
{
	int thid = 0;

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

	thid = sceKernelCreateThread("update_thread", 
				     sdl_psp_callback_thread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
		sceKernelStartThread(thid, 0, 0);
	return thid;
}
# endif

/* Remove the output files if there was no output written */
static void cleanup_output(void)
{
#ifndef NO_STDIO_REDIRECT
	FILE *file;
	int empty;
#endif

	/* Flush the output in case anything is queued */
	fclose(stdout);
	fclose(stderr);

#ifndef NO_STDIO_REDIRECT
	/* See if the files have any output in them */
	file = fopen(STDOUT_FILE, "rb");
	if ( file ) {
		empty = (fgetc(file) == EOF) ? 1 : 0;
		fclose(file);
		if ( empty ) {
			remove(STDOUT_FILE);
		}
	}
	file = fopen(STDERR_FILE, "rb");
	if ( file ) {
		empty = (fgetc(file) == EOF) ? 1 : 0;
		fclose(file);
		if ( empty ) {
			remove(STDERR_FILE);
		}
	}
#endif
}

extern void _fini(void);

#ifdef main
#undef main
#endif

void
user_thread(SceSize args, void *argp)
{
#ifndef LINUX_MODE
  sdl_psp_setup_callbacks();
# endif
  char *a_arg_v[1] = { "pspti99" };
  a_main(1, a_arg_v);
}

int 
main(int argc, char *argv[])
{
	pspDebugScreenInit();

#ifndef NO_STDIO_REDIRECT
	/* Redirect standard output and standard error. */
	/* TODO: Error checking. */
	freopen(STDOUT_FILE, "w", stdout);
	freopen(STDERR_FILE, "w", stderr);
	setvbuf(stdout, NULL, _IOLBF, BUFSIZ);	/* Line buffered */
	setbuf(stderr, NULL);					/* No buffering */
#endif /* NO_STDIO_REDIRECT */

	/* Functions registered with atexit() are called in reverse order, so make
	   sure that we register sceKernelExitGame() first, so that it's called last. */
#ifndef LINUX_MODE
	atexit(sceKernelExitGame);
# endif
# if 0
	/* Make sure that _fini() is called before returning to the OS. */
	atexit(_fini);
# endif
	atexit(cleanup_output);

#ifndef LINUX_MODE
# ifdef USE_PSP_IRKEYB
  if (psp_irkeyb_init()) {
    fprintf(stdout, "failed to initialize IR keyboard !\n");
  }
# endif

  int user_thid = sceKernelCreateThread( "user_thread", 
     (SceKernelThreadEntry)user_thread, 0x16, 256*1024, PSP_THREAD_ATTR_USER, 0 );
  if(user_thid >= 0) {
    sceKernelStartThread(user_thid, 0, 0);
    sceKernelWaitThreadEnd(user_thid, NULL);
  }

	/* Delay 2.5 seconds before returning to the OS. */
	sceKernelDelayThread(2500000);
# else
  user_thread(argc, argv);
# endif

	return 0;
}

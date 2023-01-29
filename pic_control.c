#include <cyg/kernel/kapi.h>

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* now declare (and allocate space for) some kernel objects,
   like the two threads we will use */
cyg_thread thread_s[3];		/* space for two thread objects */

char stack[3][4096];		/* space for two 4K stacks */

/* now the handles for the threads */
cyg_handle_t ui_thread, comms_thread, proc_thread;

/* and now variables for the procedure which is the thread */
cyg_thread_entry_t monitor;
cyg_mutex_t stdout_m;

extern void cmd_ini (int, char** );
extern void monitor(cyg_addrword_t);
extern void communications(cyg_addrword_t);
extern void processing(cyg_addrword_t);
/* we install our own startup routine which sets up threads */
void cyg_user_start(void)
{
  printf("Entering twothreads' cyg_user_start() function\n");

  cmd_ini(0, NULL);
  cyg_mutex_init(&stdout_m);
  cyg_thread_create(4, monitor, (cyg_addrword_t) &stdout_m,
		    "User Interface", (void *) stack[0], 4096,
		    &ui_thread, &thread_s[0]);
  cyg_thread_create(5, communications, (cyg_addrword_t) 0,
		    "Communications Interface", (void *) stack[1], 4096,
		    &comms_thread, &thread_s[1]);

  cyg_thread_create(3, processing, (cyg_addrword_t) &stdout_m,
		    "Communications Interface", (void *) stack[2], 4096,
		    &proc_thread, &thread_s[2]);
  cyg_thread_resume(ui_thread);
  cyg_thread_resume(comms_thread);
  cyg_thread_resume(proc_thread);

}


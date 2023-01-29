/***************************************************************************
| File: monitor.c
|
| Autor: Carlos Almeida (IST), from work by Jose Rufino (IST/INESC), 
|        from an original by Leendert Van Doorn
| Data:  Nov 2002
***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <cyg/kernel/kapi.h>

/*-------------------------------------------------------------------------+
| Headers of command functions
+--------------------------------------------------------------------------*/ 
extern void cmd_ini (int, char** );
       void cmd_sos  (int, char** );

extern void cmd_rc(int , char**);
extern void cmd_sc(int , char**);
extern void cmd_rtl(int, char**);
extern void cmd_rp(int, char**);
extern void cmd_mmp(int, char**);
extern void cmd_mta(int, char**);
extern void cmd_ra(int, char**);
extern void cmd_dac(int, char**);
extern void cmd_dtl(int, char**);
extern void cmd_aa(int, char**);
extern void cmd_ir(int, char**);
extern void cmd_trc(int, char**);
extern void cmd_tri(int, char**);
extern void cmd_irl(int, char**);
extern void cmd_lr(int, char**);
extern void cmd_dr(int, char**);

extern void cmd_cpt(int, char**);
extern void cmd_mpt(int, char**);
extern void cmd_cttl(int, char**);
extern void cmd_dttl(int, char**);
extern void cmd_pr(int, char**);

/*------------------------------------------------------------------------n
| Variable and constants definition
+--------------------------------------------------------------------------*/ 
const char TitleMsg[] = "\n Application Control Monitor (tst)\n";
const char InvalMsg[] = "\nInvalid command!";
struct 	command_d {
  void  (*cmd_fnct)(int, char**);
  char*	cmd_name;
  char*	cmd_help;
} const commands[] = {
  {cmd_sos,  "sos","                 help"},
  {cmd_ini,  "ini","<d>              inicializar dispositivo (0/1) ser0/ser1"},
  {cmd_rc,   "rc","		     le relogio"},
  {cmd_sc,   "sc","<h><m><s>	     set clock"},
  {cmd_rtl,  "rtl","                 read temperature and luminosity"},
  {cmd_rp,   "rp","		     read Parameters (PMON/TALA)"},
  {cmd_mmp,  "mmp","<p>              modify monitoring period (seconds - 0 deactive)"},
  {cmd_mta,  "mta","<t>		     modify time alarm"},
  {cmd_ra,   "ra", "		     read alarms"},
  {cmd_dac,  "dac","<h><n><s>	     define alarm clock"},
  {cmd_dtl,  "dtl","<t><l>	     define alarm temperature and luminosity"},
  {cmd_aa,   "aa", "<a>		     activate/deactivate alarms(1/0)"},
  {cmd_ir,   "ir", "		     information about registers (NREG, nr, iread, iwrite)"},
  {cmd_trc, "trc", "<n>		     transfer n registers from the current iread position"},
  {cmd_tri, "tri", "<n><i>	     transfer n registers from index i (0-oldest)"},
  {cmd_irl, "irl", "	             information about local registers(NRBUF, nr, iread, iwrite)"},
  {cmd_lr, "lr", "<n><i>	     list n registers (local memory) from index i (0-oldest)"},
  {cmd_dr, "dr", "		     delete registers(local memory)"},
  {cmd_cpt, "cpt", "                 check period of transference"},
  {cmd_mpt, "mpt", "<p>              modify period of transference (minutes - 0 deactivate)"},
  {cmd_cttl, "cttl", "                check threshold temperature and luminosity for processing"},
  {cmd_dttl, "dttl", "<t><l>          define threshold temperature and luminosity for processing"},
  {cmd_pr, "pr", "<h1 m1 s1><h2 m2 s2> process registers (max, min, mean) between instants t1 and t2"}
};

#define NCOMMANDS  (sizeof(commands)/sizeof(struct command_d))
#define ARGVECSIZE 10
#define MAX_LINE   50

/*-------------------------------------------------------------------------+
| Function: cmd_sos - provides a rudimentary help
+--------------------------------------------------------------------------*/ 
void cmd_sos (int argc, char **argv)
{
  int i;

  printf("%s\n", TitleMsg);
  for (i=0; i<NCOMMANDS; i++)
    printf("%s %s\n", commands[i].cmd_name, commands[i].cmd_help);
}

/*-------------------------------------------------------------------------+
| Function: getline        (called from monitor) 
+--------------------------------------------------------------------------*/ 
int my_getline (char** argv, int argvsize)
{
  static char line[MAX_LINE];
  char *p;
  int argc;

  fgets(line, MAX_LINE, stdin);

  /* Break command line into an o.s. like argument vector,
     i.e. compliant with the (int argc, char **argv) specification --------*/

  for (argc=0,p=line; (*line != '\0') && (argc < argvsize); p=NULL,argc++) {
    p = strtok(p, " \t\n");
    argv[argc] = p;
    if (p == NULL) return argc;
  }
  argv[argc] = p;
  return argc;
}

/*-------------------------------------------------------------------------+
| Function: monitor        (called from main) 
+--------------------------------------------------------------------------*/ 
void monitor (cyg_addrword_t data)
{
  static char *argv[ARGVECSIZE+1], *p;
  int argc, i;
  cyg_mutex_t *to_write;

  to_write = (cyg_mutex_t *)data;
  printf("%s Type sos for help\n", TitleMsg);
  for (;;) {
    cyg_mutex_lock(to_write);
    printf("\nCmd> ");
    /* Reading and parsing command line  ----------------------------------*/
    if ((argc = my_getline(argv, ARGVECSIZE)) > 0) {
      for (p=argv[0]; *p != '\0'; *p=tolower(*p), p++);
      for (i = 0; i < NCOMMANDS; i++) 
	if (strcmp(argv[0], commands[i].cmd_name) == 0) 
	  break;
      /* Executing commands -----------------------------------------------*/
      if (i < NCOMMANDS)
	commands[i].cmd_fnct (argc, argv);
      else  
	printf("%s", InvalMsg);
     cyg_mutex_unlock(to_write);
    } /* if my_getline */
  } /* forever */
}

/***************************************************************************
| File: comando.c  -  Concretizacao de comandos (exemplo)
|
| Autor: Carlos Almeida (IST)
| Data:  Maio 2008
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <cyg/io/io.h>
#include <ctype.h>
#include <cyg/kernel/kapi.h>
#include "reg_buf.h"

#define INS_BUF_SIZE 100
#define MSG_BUF_SIZE 50
#define NREG 100

Cyg_ErrNo err;
cyg_io_handle_t serH;

/* and now a mutex to protect calls to the C library */
cyg_mutex_t ins_msg;
cyg_mutex_t wakeproc;
cyg_cond_t resp_msg;
cyg_cond_t condproc;
cyg_handle_t clock;

unsigned char ins_buff[INS_BUF_SIZE];
unsigned char msg_buf[MSG_BUF_SIZE];
unsigned char msg_rpos,	msg_wpos, ins_rpos, ins_wpos;


unsigned int transf_time;
unsigned int temp_proc;
unsigned int lum_proc;

unsigned long tick_sec;
/*-------------------------------------------------------------------------+
| Function: cmd_ini - inicializar dispositivo
+--------------------------------------------------------------------------*/ 
void cmd_ini(int argc, char **argv)
{
  cyg_resolution_t resolution;
  msg_rpos = 0;
  msg_wpos = 0;
  ins_rpos = 0;
  ins_wpos = 0;
  transf_time = 1;
  temp_proc = 0;
  lum_proc = 0;
  initialize_regs();
  printf("io_lookup\n");
  if ((argc > 1) && (argv[1][0] = '1'))
    err = cyg_io_lookup("/dev/ser1", &serH);
  else err = cyg_io_lookup("/dev/ser0", &serH);
  cyg_mutex_init(&ins_msg);
  cyg_cond_init(&resp_msg, &ins_msg);
  cyg_mutex_init(&wakeproc);
  cyg_cond_init(&condproc, &wakeproc);
  
  clock = cyg_real_time_clock();
  resolution = cyg_clock_get_resolution(clock);
  tick_sec = 1000000000 / resolution.dividend;
  tick_sec = tick_sec * resolution.divisor;
  printf("lookup err=%x\n", err);
}
unsigned char ring_index(int size, unsigned char pos){
	if(pos > size -1)
		pos = 0;
	else
		pos++;
	return pos;
}

void make_request(unsigned char req[], unsigned int size)
{

	unsigned int i;
	for(i = 0; i < size; i++)
	{
		ins_buff[ins_wpos] = req[i];
		ins_wpos = ring_index(INS_BUF_SIZE, ins_wpos);
	}


}

void make_arg_request(char **argv, unsigned char req[], unsigned int size, unsigned char prog)
{
	
	unsigned int i;

	req[0] = 0xfd;
    	req[1] = prog;
	for (i=2; i<size-1; i++){	
		unsigned int x; 
		sscanf(argv[i-1], "%d", &x); 
		req[i]=(unsigned char)x;
	}
	req[size-1] = 0xfe;
	make_request(req, size);	


}

char read_msg(unsigned char response[], unsigned int size)
{ 
	
	unsigned int i_res = 0, i_read = 0;
	do
	{
		if(msg_buf[msg_rpos] != 0xfd && msg_buf[msg_rpos] != 0xfe){
			if(i_res < size)
				response[i_res++] = msg_buf[msg_rpos];
			i_read++;
		}
		msg_rpos = ring_index(MSG_BUF_SIZE, msg_rpos);
	}while(msg_buf[msg_rpos] != 0xfe);
	if(i_read != size)
		return 0;
	else
		return 1;
}

/*-----cmd_rc - reads clock from PIC-----------*/
void cmd_rc(int argc, char **argv)
{
	
	unsigned char request[3] = {0xfd, 0xc0, 0xfe};
	unsigned char res[4];
	char correct;

	cyg_mutex_lock(&ins_msg);
	make_request(request, 3);
	cyg_cond_wait(&resp_msg);
	correct = read_msg(res, 4);
	printf("%d\n", correct);
	if(correct && res[0] == request[1])
		printf("%.2d:%.2d:%.2d\n", res[1], res[2], res[3]);
	else
		printf("Error in Pic Response\n");
	cyg_mutex_unlock(&ins_msg);
}

void cmd_sc(int argc, char **argv)
{
	
	unsigned char request[6];
	unsigned char res[2];
	char correct;

	cyg_mutex_lock(&ins_msg);
	
	if(argc != 4)
	{
		printf("Invalid Number of arguments\n");
		cyg_mutex_unlock(&ins_msg);
		return ;
	}
	make_arg_request(argv, request, 6, 0xc1);
	cyg_cond_wait(&resp_msg);
	correct = read_msg(res, 2);
	if(correct == 1 && res[1] == 0)
		printf("Success\n");
	else
		printf("Error\n");
	cyg_mutex_unlock(&ins_msg);
}


void cmd_rtl(int argc, char **argv)
{
	
	unsigned char request[3] = {0xfd, 0xc2, 0xfe};
	unsigned char res[3];
	char correct;

	cyg_mutex_lock(&ins_msg);
	make_request(request, 3);
	cyg_cond_wait(&resp_msg);
	correct = read_msg(res, 3);
	if(correct && res[0] == request[1])
		printf("Lum:%d\tTemp:%d\n", res[1], res[2]);
	else
		printf("Error in Pic Response\n");
	cyg_mutex_unlock(&ins_msg);
}


void cmd_rp(int argc, char **argv)
{
	
	unsigned char request[3] = {0xfd, 0xc3, 0xfe};
	unsigned char res[3];
	char correct;

	cyg_mutex_lock(&ins_msg);
	make_request(request, 3);
	cyg_cond_wait(&resp_msg);
	correct = read_msg(res, 3);
	if(correct && res[0] == request[1])
		printf("PMON:%d\tTALA:%d\n", res[1], res[2]);
	else
		printf("Error in Pic Response\n");
	cyg_mutex_unlock(&ins_msg);
}


void cmd_mmp(int argc, char **argv)
{

	unsigned char request[4];
	unsigned char res[2];
	char correct;		

	cyg_mutex_lock(&ins_msg);
	if(argc != 2)
	{
		printf("Invalid Number of arguments\n");
		cyg_mutex_unlock(&ins_msg);
		return ;
	}
	make_arg_request(argv, request, 4, 0xc4);
	cyg_cond_wait(&resp_msg);
	correct = read_msg(res, 2);
	if(correct == 1 && res[1] == 0)
		printf("Success\n");
	else
		printf("Error\n");
	cyg_mutex_unlock(&ins_msg);

}

void cmd_mta(int argc, char **argv)
{
	unsigned char request[4];
	unsigned char res[2];
	char correct;

	cyg_mutex_lock(&ins_msg);
	if(argc != 2)
	{
		printf("Invalid Number of arguments\n");
		cyg_mutex_unlock(&ins_msg);
		return ;
	}
	make_arg_request(argv, request, 4, 0xc5);
	cyg_cond_wait(&resp_msg);
	correct = read_msg(res, 2);
	if(correct == 1 && res[1] == 0)
		printf("Success\n");
	else
		printf("Error\n");
	cyg_mutex_unlock(&ins_msg);
}


void cmd_ra(int argc, char **argv)
{
	
	unsigned char request[3] = {0xfd, 0xc6, 0xfe};
	unsigned char res[7];
	char correct;

	cyg_mutex_lock(&ins_msg);
	make_request(request, 3);
	cyg_cond_wait(&resp_msg);
	correct = read_msg(res, 7);
	if(correct && res[0] == request[1])
		printf("Alarms\n\tClock: %.2d:%.2d:%.2d\n\tTemperature: %d\n\tLuminosity:%d\n\tEnable: %d\n", res[1], res[2], res[3], res[4], res[5], res[6]);
	else
		printf("Error in Pic Response\n");
	cyg_mutex_unlock(&ins_msg);
}

void cmd_dac(int argc, char **argv)
{
	unsigned char request[6];
	unsigned char res[2];
	char correct;

	cyg_mutex_lock(&ins_msg);
	if(argc != 4)
	{
		printf("Invalid Number of arguments\n");
		cyg_mutex_unlock(&ins_msg);
		return ;
	}
	make_arg_request(argv, request, 6, 0xc7);
	cyg_cond_wait(&resp_msg);
	correct = read_msg(res, 2);
	if(correct == 1 && res[1] == 0)
		printf("Success\n");
	else
		printf("Error\n");
	cyg_mutex_unlock(&ins_msg);
}


void cmd_dtl(int argc, char **argv)
{
	unsigned char request[5];
	unsigned char res[2];
	char correct;

	cyg_mutex_lock(&ins_msg);
	if(argc != 3)
	{
		printf("Invalid Number of arguments\n");
		cyg_mutex_unlock(&ins_msg);
		return ;
	}
	make_arg_request(argv, request, 5, 0xc8);
	cyg_cond_wait(&resp_msg);
	correct = read_msg(res, 2);
	if(correct == 1 && res[1] == 0)
		printf("Success\n");
	else
		printf("Error\n");
	cyg_mutex_unlock(&ins_msg);
}

void cmd_aa(int argc, char **argv)
{
	unsigned char request[4];
	unsigned char res[2];
	char correct;

	cyg_mutex_lock(&ins_msg);
	if(argc != 2)
	{
		printf("Invalid Number of arguments\n");
		cyg_mutex_unlock(&ins_msg);
		return ;
	}
	make_arg_request(argv, request, 4, 0xc9);
	cyg_cond_wait(&resp_msg);
	correct = read_msg(res, 2);
	if(correct == 1 && res[1] == 0)
		printf("Success\n");
	else
		printf("Error\n");
	cyg_mutex_unlock(&ins_msg);
}

void cmd_ir(int argc, char **argv)
{
	
	unsigned char request[3] = {0xfd, 0xca, 0xfe};
	unsigned char res[5];
	char correct;

	cyg_mutex_lock(&ins_msg);
	make_request(request, 3);
	cyg_cond_wait(&resp_msg);
	correct = read_msg(res, 5);
	if(correct && res[0] == request[1])
		printf("Pic Register Info:\n\tNREG: %d\n\tN: %d\n\tiread: %d\n\tiwrite: %d\n", res[1], res[2], res[3], res[4]);
	else
		printf("Error in Pic Response\n");
	cyg_mutex_unlock(&ins_msg);
}

void cmd_trc(int argc, char **argv)
{

	unsigned char request[4];
	unsigned int i;
	register_t *r;

	cyg_mutex_lock(&ins_msg);
	if(argc != 2)
	{
		printf("Invalid Number of arguments\n");
		cyg_mutex_unlock(&ins_msg);
		return ;
	}
	make_arg_request(argv, request, 4, 0xcb);
	cyg_cond_wait(&resp_msg);
	for(i = 0; i < request[2]; i++)
	{
		if((r = get_next_reg()) == NULL)
			break;
		print_reg(r);
	}
	cyg_mutex_unlock(&ins_msg);
}

void cmd_tri(int argc, char **argv)
{

	unsigned char request[5];
	unsigned int i;
	register_t *r;

	cyg_mutex_lock(&ins_msg);
	if(argc != 3)
	{
		printf("Invalid Number of arguments\n");
		cyg_mutex_unlock(&ins_msg);
		return ;
	}
	make_arg_request(argv, request, 5, 0xcc);
	cyg_cond_wait(&resp_msg);
	for(i = request[3]; i < request[2]+request[3]; i++)
	{
		if(i>=get_nr())
			break;
		if((r = get_next_reg()) == NULL)
			break;
		print_reg(r);
	}
	cyg_mutex_unlock(&ins_msg);
}

//operations in local memory

void cmd_irl(int argc, char **argv)
{
	cyg_mutex_lock(&ins_msg);
	printf("Information about local registers:\n\tNRBUF: %d\n\tN: %d\n\tiread: %d\n\tiwrite: %d\n", NREG, get_nr(), get_iread(), get_iwrite());
	cyg_mutex_unlock(&ins_msg);
}

void cmd_lr(int argc, char **argv)
{
	int  n, index;
	unsigned int i;
	register_t *r;

	cyg_mutex_lock(&ins_msg);
	if(argc != 3)
	{
		printf("Invalid Number of arguments\n");
		cyg_mutex_unlock(&ins_msg);
		return ;
	}
	
	sscanf(argv[1], "%d", &n); 

	sscanf(argv[2], "%d", &index); 
	if(n > NREG || n < 0 || index < 0){
		printf("Invalid arguments\n");
		cyg_mutex_unlock(&ins_msg);
		return ;
	}
	for(i = index; i < n+index; i++)
	{	
		if(i>=get_nr())
			break;
		if((r = get_index_reg(i)) == NULL)
			break;
		print_reg(r);
	}
	cyg_mutex_unlock(&ins_msg);
}

void cmd_dr(int argc, char **argv)
{
	cyg_mutex_lock(&ins_msg);
  	initialize_regs();
	printf("All registers deleted\n");
	cyg_mutex_unlock(&ins_msg);
}

void cmd_cpt(int argc, char **argv)
{
	
	printf("Transference Period: %d\n", transf_time);
}

void cmd_mpt(int argc, char **argv)
{

	int aux;
	int prev = transf_time;
	if(argc != 2)
	{
		return;
	}
	aux = atoi(argv[1]);
	if(aux>=0)
		transf_time=aux;
	
	cyg_mutex_lock(&wakeproc);
	if(prev == 0 && transf_time != 0)
		cyg_cond_signal(&condproc);
	printf("Modified into: %d\n", transf_time);
	cyg_mutex_unlock(&wakeproc);
}


void cmd_cttl(int argc, char **argv)
{

	printf("Temperature for Processing: %d\nLuminosity for Processing: %d\n",temp_proc, lum_proc);
}

void cmd_dttl(int argc, char **argv)
{

	int aux;
	if(argc!=3)
	{
		return;
	}
	aux = atoi(argv[1]);
	if(aux >= 0 && aux <= 50)
		temp_proc = aux;

	aux = atoi(argv[2]);
	if(aux >= 0 && aux <= 7)
		lum_proc = aux;
	printf("Modified Processing: temp=%d, lum=%d\n", temp_proc, lum_proc);
}

void cmd_pr(int argc, char **argv)
{
/*
	int  n, index;
	unsigned int i;
	register_t *r;
	unsigned char buf[3];
	unsigned int h1, h2, m1, m2, s1, s2;
	cyg_mutex_lock(&ins_msg);
	if(argc != 7)
	{
		printf("Invalid Number of arguments\n");
		cyg_mutex_unlock(&ins_msg);
		return ;
	}
	
	sscanf(argv, "%s %d %d %d %d %d %d", buf, &h1, &m1, &s1, &h2, &m2, &s2); 

	for(i = 0; i < get_nr(); i++)
	{	
	// nao se fez esta task	
		
		if((r = get_index_reg(i)) == NULL)
			break;
		print_reg(r);
	}
	cyg_mutex_unlock(&ins_msg);*/
	return;
}










void comms_read_regs(void)
{
	unsigned char c, reg_buff[4];
	unsigned int n = 4, n2 = 1;
	unsigned char msg_flag = 0;
	
	while(1)
	{
		err = cyg_io_read(serH, &c, &n2);
		if(c == 0xfe)
			break;
		if(c == 0xfd && msg_flag == 0){
			err = cyg_io_read(serH, &c, &n2);
			msg_flag = 1;
		}
		else if(msg_flag == 1){
			err = cyg_io_read(serH, reg_buff, &n);
			add_reg(c, reg_buff);
		}
	}
}

void communications(cyg_addrword_t data)
{
	unsigned char store_data[20], c;
	char msg_flag = 0;
	unsigned int i = 0, n =1;
	unsigned char cmd = 0;
	while(1)
	{	
		cyg_mutex_lock(&ins_msg);
		//read instruction
		i = 0;
		msg_flag = 0;
		cmd = 0;
		while(ins_rpos != ins_wpos){
			if(ins_buff[ins_rpos] == 0xfd && msg_flag == 0){
				store_data[i] = 0xfd;
				i++;
				msg_flag = 1;
			}
			else if(msg_flag == 1){
				if(cmd == 0)
					cmd = ins_buff[ins_rpos];
				store_data[i] = ins_buff[ins_rpos];
				i++;
				if(ins_buff[ins_rpos] == 0xfe){
					msg_flag = 2;
					break ;
				}
			}
			ins_rpos = ring_index(INS_BUF_SIZE, ins_rpos);
		}
		
		err = cyg_io_write(serH, store_data, &i);
		i = 0;
		msg_flag = 0;
		if(cmd == 0xcb || cmd == 0xcc)
			comms_read_regs();
		else{
		//read result from pic
			do{
				err = cyg_io_read(serH, &c, &n);
				if(c == 0xfd && msg_flag == 0){
					msg_buf[msg_wpos] = 0xfd;
					msg_flag = 1;
					msg_wpos = ring_index(MSG_BUF_SIZE, msg_wpos);
				}
				else if(msg_flag == 1){
					msg_buf[msg_wpos] = c;
					msg_wpos = ring_index(MSG_BUF_SIZE, msg_wpos);
				}
			}while(c != 0xfe);
		//signal	
		}
		cyg_cond_signal(&resp_msg);
		cyg_mutex_unlock(&ins_msg);
	}
}






void processing(cyg_addrword_t data)
{
	int delay;
	cyg_mutex_t *to_print;

	to_print = (cyg_mutex_t*)data;
	
	printf("Start thread one\n");
	unsigned char request[4];
	unsigned int i;
	char *buf[2], arg1[] = "trc", arg2[1];
	register_t *r;
	sprintf(arg2, "%d", NREG);
	buf[0] = arg1;
	buf[1] = arg2;
	
	while(1)
	{
		cyg_mutex_lock(to_print);
		cyg_mutex_lock(&ins_msg);
		delay = transf_time*60*tick_sec;
		if(delay != 0){
			make_arg_request(buf, request, 4, 0xcb);
			cyg_cond_wait(&resp_msg);
			for(i = 0; i < request[2]; i++)
			{
				if((r = get_next_reg()) == NULL)
					break;
				printf("Processed Reg\n");
				processInfo(r, temp_proc, lum_proc);	
			}
		}
		cyg_mutex_unlock(&ins_msg);
		cyg_mutex_unlock(to_print);
			
		if(delay != 0)
		{
			cyg_thread_delay(delay);
		}
		else{
			cyg_mutex_lock(&wakeproc);
			cyg_cond_wait(&condproc);
			cyg_mutex_unlock(&wakeproc);
		}
	}
}

#include <stdlib.h>
#include <stdio.h>
#include "reg_buf.h"

#define NREG 100

struct register_s{
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
	unsigned char temp;
	unsigned char lum;
};




register_t regs[NREG];
unsigned char iread, iwrite, nr;

void initialize_regs(void)
{
	unsigned int i;

	for(i = 0; i < NREG; i++)
	{
		regs[iwrite].hour = 60;
		regs[iwrite].min = 60;
		regs[iwrite].sec = 60;
		regs[iwrite].temp = 0;
		regs[iwrite].lum =  0;
	}
	nr = 0;
	iread = 0;
	iwrite = 0;
}

void add_reg(unsigned c, unsigned char buf[])
{
	regs[iwrite].hour = c;
	regs[iwrite].min = buf[0];
	regs[iwrite].sec = buf[1];
	regs[iwrite].temp = buf[2];
	regs[iwrite].lum = buf[3];
	iwrite = (iwrite+1) % NREG;
	if(nr <  NREG)
		nr++;
}

void print_reg(register_t *r)
{
	printf("================REGISTER================\n");
	printf("Time: %d:%d:%d\n", r->hour, r->min, r->sec);
	printf("Temperature: %d\n", r->temp);
	printf("Luminosity: %d\n", r->lum);

}

register_t *get_next_reg(void)
{
	register_t *r;
	
	if (iread == iwrite)
		return NULL;
	r = &regs[iread];
	iread = (iread+1) % 50;
	return(r);
}


register_t *get_index_reg(int index)
{
	register_t *r;
	int old, i;

	if(index >= nr)
		return(NULL);
	old = (nr == NREG) ? (iwrite + 1)%NREG : 0;
	i = (old + index) % NREG;
	if(i == iwrite)
		return(NULL);
	r = &regs[i];
	return (r);
}

unsigned char get_iwrite(void)
{
	return (iwrite);
}
unsigned char get_iread(void)
{
	return (iread);
}
unsigned char get_nr(void)
{
	return(nr);
}

//processamento de informacao
void processInfo(register_t *r, int temperatura, int luminosidade)
{
	if(temperatura < r->temp || luminosidade < r->lum)
		print_reg(r);
}

unsigned char compareHour(unsigned char h1, unsigned char m1, unsigned char s1, unsigned char h2, unsigned char m2, unsigned char s2)
{

	if(h1 > h2) return 1;
	
	if(h1 == h2 && m1 > m2) return 1;

	if(h1 == h2 && m1 == m2 && s1 >= s2) return 1;
	
	return 0;
}

void printLastTask(register_t *r, unsigned char h1, unsigned char m1, unsigned char s1, unsigned char h2, unsigned char m2, unsigned char s2){


return;



}


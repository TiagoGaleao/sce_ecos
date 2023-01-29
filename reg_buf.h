#ifndef _REG_BUF_H
#define _REG_BUF_H

typedef struct register_s register_t;


void initialize_regs(void);


void add_reg(unsigned c, unsigned char buf[]);


void print_reg(register_t *r);


register_t *get_next_reg(void);
register_t *get_index_reg(int i);

unsigned char get_iwrite(void);
unsigned char get_iread(void);
unsigned char get_nr(void);
void processInfo(register_t *r, int temperatura, int luminosidade);

#endif

#ifndef INCLUDE_IDT_H
#define INCLUDE_IDT_H

struct idt_ptr {
    unsigned short size;
    unsigned int address;
} __attribute__((packed));

struct idt_entry {
    unsigned short offset_low;
    unsigned short segment_selector;
    unsigned char reserved;
    unsigned char type_attributes;
    unsigned short offset_high;
} __attribute__((packed));

struct cpu_state {
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int esi;
    unsigned int edi;
    unsigned int ebp;
} __attribute__((packed));

struct stack_state {
    unsigned int int_num;
    unsigned int error_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
} __attribute__((packed));

typedef void (*interrupt_handler) (struct cpu_state*);

void load_idt(struct idt_ptr* table);
void register_interrupt_handler(unsigned int interrupt, interrupt_handler handler);
void idt_init();
void handle_interrupt(struct cpu_state *cpu);
void pic_init();
void pic_ack(unsigned int interrupt);

#endif

#ifndef INCLUDE_IDT_H
#define INCLUDE_IDT_H

#include "types.h"

#define INT_SYSCALL     0x80

struct idt_ptr {
    uint16_t size;
    uint32_t address;
} __attribute__((packed));

struct idt_entry {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t type_attributes;
    uint16_t offset_high;
} __attribute__((packed));

struct cpu_state {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
} __attribute__((packed));

struct stack_state {
    uint32_t int_num;
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} __attribute__((packed));

typedef void (*interrupt_handler) (struct cpu_state*, struct stack_state*);

void load_idt(struct idt_ptr* table);
void register_interrupt_handler(uint32_t interrupt, interrupt_handler handler);
void idt_init();
void handle_interrupt(struct cpu_state *cpu);
void pic_init();
void pic_ack(uint32_t interrupt);

#endif

#include "idt.h"
#include "io.h"

static struct idt_entry idt[256];
static struct idt_ptr idt_p;

#define PIC1_PORT_A 0x20
#define PIC2_PORT_A 0xA0
#define PIC1_PORT_DATA (PIC1_PORT_A + 1)
#define PIC2_PORT_DATA (PIC2_PORT_A + 1)

#define PIC1_START_INT 0x20
#define PIC2_START_INT 0x28
#define PIC2_END_INT (PIC2_START_INT + 7)

#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

#define PIC_ACK 0x20

#define KBD_DATA_PORT 0x60

extern void* isr_stub_table[];

interrupt_handler handlers[256];

/**
 * Sets an entry within the idt table.
*/
static void set_entry(
    int i, void *isr, unsigned short segment_selector,
    unsigned char type_attributes
)
{
    idt[i].offset_low = (unsigned int) isr & 0xffff;
    idt[i].offset_high = ((unsigned int) isr >> 16) & 0xffff;
    idt[i].reserved = 0x0;
    idt[i].segment_selector = segment_selector;
    idt[i].type_attributes = type_attributes;
}


void idt_init()
{
    idt_p.address = (unsigned int) &idt;
    idt_p.size = (sizeof(struct idt_entry) * 256) - 1;

    // Initialise all handlers to default handler
    for (int i = 0; i < 48; i++)
    {
        set_entry(i, isr_stub_table[i], 0x08, 0x8E);
    }

    set_entry(0x80, isr_stub_table[48], 0x08, 0xEE);

    pic_init();
    load_idt(&idt_p);
}

void handle_interrupt(struct cpu_state *cpu)
{
    struct stack_state *stack = (struct stack_state *)((unsigned char *)cpu + sizeof(struct cpu_state));

    int int_num = stack->int_num;

    if (handlers[int_num] != 0) {
        interrupt_handler handler = handlers[int_num];
        handler(cpu);
    }

    pic_ack(stack->int_num);
}

/**
 * Registers a new handler for the specified interrupt number
 */
void register_interrupt_handler(unsigned int interrupt, interrupt_handler handler)
{
    handlers[interrupt] = handler;
}


void pic_init()
{
    // Start init sequence
    outb(PIC1_PORT_A, ICW1_INIT);
    outb(PIC2_PORT_A, ICW1_INIT);

    // remap vectors
    outb(PIC1_PORT_DATA, PIC1_START_INT);
    outb(PIC2_PORT_DATA, PIC2_START_INT);

    // tell pics about each other
    outb(PIC1_PORT_DATA, 0x04); // master - slave is on irq2
    outb(PIC2_PORT_DATA, 0x02); // slave - confirm identity is 2

    // set 8086 mode
    outb(PIC1_PORT_DATA, ICW4_8086);
    outb(PIC2_PORT_DATA, ICW4_8086);

    // mask all interrupts for now - should unmask as we implement handlers
    outb(PIC1_PORT_DATA, 0xfd);
    outb(PIC2_PORT_DATA, 0xff);
}

void pic_ack(unsigned int interrupt)
{
    if (interrupt < PIC1_START_INT || interrupt > PIC2_END_INT)
        return;

    if (interrupt < PIC2_START_INT) {
        outb(PIC1_PORT_A, PIC_ACK);
    } else {
        outb(PIC2_PORT_A, PIC_ACK);
    }
}

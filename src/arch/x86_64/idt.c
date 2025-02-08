#include <kernel/kernel.h>
#include <kernel/interrupt.h>
#include <protect.h>

// 中断描述符
typedef
struct idt_entry_t {
        u16 base_lo;        // 中断处理函数地址 15～0 位
        u16 sel;            // 目标代码段描述符选择子
        u8  ist;            // legacy 置 0 段, used for x86_64 TSS
        u8  flags;          // 一些标志，文档有解释
        u16 base_mid;        // 中断处理函数地址 31～16 位
        u32 bash_hi;
        u32 __zero;
} __packed__ idt_entry_t;

// IDTR
typedef
struct idt_ptr_t {
        u16 limit;        // 限长
        addr_t base;         // 基址
} __packed__ idt_ptr_t;


// 中断描述符表
idt_entry_t idt_entries[INTERRUPT_MAX] __attribute__((aligned(16)));

// IDTR
static idt_ptr_t idt_ptr;




// 中断处理函数指针类型
typedef void (*intr_func_t)();

// 中断处理函数指针数组
extern intr_func_t isr_func[INTERRUPT_MAX];


// 设置中断描述符
static void idt_set_gate(u8 num, addr_t base, u8 flags)
{
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_mid = (base >> 16) & 0xFFFF;
    idt_entries[num].bash_hi = (base >> 32) & 0xFFFFFFFF;

    idt_entries[num].sel = GD_SEL(SEG_KTEXT);
    idt_entries[num].ist = IST_NONE;

    idt_entries[num].flags = flags;
}

void idt_load(void)
{
    // 更新设置中断描述符表
    asm volatile(
        "lidt (%0)"::"r"(&idt_ptr)
    );
}

// 初始化中断描述符表
void idt_init(void)
{

    idt_ptr.limit = sizeof(idt_entry_t) * INTERRUPT_MAX - 1;
    idt_ptr.base = (addr_t)idt_entries;

    u32 vec = 0;
    for(;vec < 256; vec++)
    {
        idt_set_gate(vec, (addr_t)isr_func[vec], DA_386IGate);
    }
    idt_entries[INT_DOUBLE_FAULT].ist = IST_DOUBLE_FAULT;
    idt_entries[INT_NMI].ist = IST_NMI;
    idt_load();
}
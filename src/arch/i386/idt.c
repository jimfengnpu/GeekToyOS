#include <kernel/kernel.h>
#include <kernel/interrupt.h>
#include <protect.h>

// 中断描述符
typedef
struct idt_entry_t {
        u16 base_lo;        // 中断处理函数地址 15～0 位
        u16 sel;            // 目标代码段描述符选择子
        u8  always0;        // 置 0 段
        u8  flags;          // 一些标志，文档有解释
        u16 base_hi;        // 中断处理函数地址 31～16 位
}__packed__ idt_entry_t;

// IDTR
typedef
struct idt_ptr_t {
        u16 limit;        // 限长
        u32 base;         // 基址
} __packed__ idt_ptr_t;

// 中断描述符表
static idt_entry_t idt_entries[INTERRUPT_MAX] __attribute__((aligned(16)));

// IDTR
static idt_ptr_t idt_ptr;


// 设置中断描述符
static void idt_set_gate(u8 num, u32 base, u8 flags);

// 声明加载 IDTR 的函数
extern void idt_flush(u32);

// 中断处理函数指针类型
typedef void (*intr_func_t)();

// 中断处理函数指针数组
extern intr_func_t isr_func[INTERRUPT_MAX];

// 设置中断描述符
static void idt_set_gate(u8 num, u32 base, u8 flags)
{
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

    idt_entries[num].sel = GD_KTEXT;
    idt_entries[num].always0 = 0;

    idt_entries[num].flags = flags;
}

void idt_init()
{
    idt_ptr.limit = sizeof(idt_entry_t) * INTERRUPT_MAX - 1;
    idt_ptr.base = (u32)&idt_entries;

    // 0~31:  用于 CPU 的中断处理
    // 32~47: Intel 保留
    u32 vec = 0;
    for(;vec < 256; vec++)
    {
        u8 dpl = 0;
        if(vec == INT_SYSCALL){
            dpl = DA_DPL3;
        }
        idt_set_gate(vec, (addr_t)isr_func[vec], DA_386IGate | dpl);
    }
    // 更新设置中断描述符表
    asm volatile(
        "lidt (%0)"::"r"(&idt_ptr)
    );
}
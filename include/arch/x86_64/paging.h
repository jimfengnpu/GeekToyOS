#ifndef X64_PAGING_H
#define X64_PAGING_H
#include <lib/types.h>


typedef addr_t pte_t;
typedef addr_t pde_t;
typedef addr_t pgd_t;
// A linear address 'la' in x86-64 has a  structure as follows:
//
// +---16----+--------9-------+-------9--------+-------9--------+-------9--------+---------12----------+
// |sign ext |     P G D      |   P  U   D     |    Page Dir    |  Page Table    | Offset within Page  |
// +---------+----------------+----------------+----------------+----------------+---------------------+
//            \--- PGX(la) --/ \--- PUX(la) --/ \--- PDX(la) --/ \--- PTX(la) --/ \---- PGOFF(la) ----/
//

// offset in page
#define PGOFF(la)	(((addr_t) (la)) & 0xFFF)
#define PTADDR(pde) ((addr_t)(pde) & (~0xFFF))

// construct linear address from indexes and offset
// #define PGADDR(d, t, o)	((void*) ((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))

// Page directory and page table constants.
#define NPDENTRIES	512		// page directory entries per page directory
#define NPTENTRIES	512		// page table entries per page table

#define PGSIZE      4096
#define PGSHIFT		12		// log2(PGSIZE)

#define PTSIZE		(PGSIZE*NPTENTRIES) // bytes mapped by a page directory entry
#define PTSHIFT		21		// log2(PTSIZE)

#define PTXSHIFT	12		// offset of PTX in a linear address
#define PDXSHIFT	21		// offset of PDX in a linear address
#define PUXSHIFT	30		// offset of PUX in a linear address
#define PGXSHIFT	39		// offset of PUX in a linear address

#define DEFINE_PD_INDEXER(part) \
static inline addr_t P##part##X(addr_t la){   \
    return ((la) >> P##part##XSHIFT) & 0x1FF;\
}
DEFINE_PD_INDEXER(G)
DEFINE_PD_INDEXER(U)
DEFINE_PD_INDEXER(D)
DEFINE_PD_INDEXER(T)

// Page table/directory entry flags.
#define PTE_P		0x001	// Present
#define PTE_W		0x002	// Writeable
#define PTE_U		0x004	// User
#define PTE_PWT		0x008	// Write-Through
#define PTE_PCD		0x010	// Cache-Disable
#define PTE_A		0x020	// Accessed
#define PTE_D		0x040	// Dirty
#define PTE_PS		0x080	// Page Size

// Control Register flags
#define CR0_PE		0x00000001	// Protection Enable
#define CR0_MP		0x00000002	// Monitor coProcessor
#define CR0_EM		0x00000004	// Emulation
#define CR0_TS		0x00000008	// Task Switched
#define CR0_ET		0x00000010	// Extension Type
#define CR0_NE		0x00000020	// Numeric Errror
#define CR0_WP		0x00010000	// Write Protect
#define CR0_AM		0x00040000	// Alignment Mask
#define CR0_NW		0x20000000	// Not Writethrough
#define CR0_CD		0x40000000	// Cache Disable
#define CR0_PG		0x80000000	// Paging

#define CR4_PCE		0x00000100	// Performance counter enable
#define CR4_MCE		0x00000040	// Machine Check Enable
#define CR4_PSE		0x00000010	// Page Size Extensions
#define CR4_DE		0x00000008	// Debugging Extensions
#define CR4_TSD		0x00000004	// Time Stamp Disable
#define CR4_PVI		0x00000002	// Protected-Mode Virtual Interrupts
#define CR4_VME		0x00000001	// V86 Mode Extensions


// Page fault error codes
#define FEC_PR		0x1	// Page fault caused by protection violation
#define FEC_WR		0x2	// Page fault caused by a write
#define FEC_U		0x4	// Page fault occured while in user mode

#endif
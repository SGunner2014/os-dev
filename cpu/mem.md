## Key Responsibilities:

- Tracking free frames
- Allocating new frames, pages
- Checking if a vaddr is already mapped (kernel and process)
- Allocating a frame and mapping to a specific vaddr
- Initialising paging:
  - Calculate end of kernel
  - Create new directory for kernel above 3GB
  - Load directory
  - Fill in frame bitmap
- Creating new page directories
- Convert physical <-> virtual kernel/process addresses
- Convert addr to page-aligned
- Allocating a specific virtual range
- Freeing pages/ranges
- Keeping track of free/used frames

## Interface:

```c

/*
 * Utility Functions
 */
static inline bool is_page_aligned(uint32_t *addr);
static inline uint32_t *get_page_aligned(uint32_t *addr);

/*
 * Kernel Functions
 */

// Generic
void k_init_paging(
  uint32_t kernel_start,
  uint32_t kernel_physical_start,
  uint32_t kernel_end,
  uint32_t kernel_physical_end,
  multiboot_info_t *multiboot
);
void k_copy_kernel_mappings(pde_t *pd);

// Frames
uint32_t k_find_free_frame();
void k_mark_frame_occupied(uint32_t frame_no);
void k_free_frame(uint32_t frame_no)
bool k_is_frame_occupied(uint32_t frame_no);

// Pages
// kalloc - always returns virtual addresses, physical should be obtained with k_virt_to_phys
uint32_t *k_alloc_page(uint32_t *vaddr); // Returns the virtual address allocated, can pass NULL as arg
void k_free_page(uint32_t *vaddr);
uint32_t *k_virt_to_phys(uint32_t *vaddr);
uint32_t *k_phys_to_virt(uint32_t *paddr);
bool k_is_virt_allocated(uint32_t *vaddr);

/*
 * Process Functions
 */

// Pages
// alloc - always returns virtual addresses, physical should be obtained with virt_to_phys
uint32_t *alloc_page(pde_t *pd, uint32_t *vaddr); // Returns virt addr, can pass NULL as arg
void free_page(pde_t *pd, uint32_t *vaddr);
uint32_t *virt_to_phys(pde_t *pd, uint32_t *vaddr);
// uint32_t *phys_to_virt ?
bool is_virt_allocated(pde_t *pd, uint32_t *vaddr);
```

#ifndef __BACKPORT_MM_H
#define __BACKPORT_MM_H
#include_next <linux/mm.h>
#include <linux/page_ref.h>
#include <linux/sched.h>
#include <linux/overflow.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

#ifndef VM_NODUMP
/*
 * defined here to allow things to compile but technically
 * using this for memory regions will yield in a no-op on newer
 * kernels but on older kernels (v3.3 and older) this bit was used
 * for VM_ALWAYSDUMP. The goal was to remove this bit moving forward
 * and since we can't skip the core dump on old kernels we just make
 * this bit name now a no-op.
 *
 * For details see commits: 909af7 accb61fe cdaaa7003
 */
#define VM_NODUMP      0x0
#endif

#ifndef VM_DONTDUMP
#define VM_DONTDUMP    VM_NODUMP
#endif

#if LINUX_VERSION_IS_LESS(3,15,0)
#define kvfree LINUX_BACKPORT(kvfree)
void kvfree(const void *addr);
#endif /* < 3.15 */

#if LINUX_VERSION_IS_LESS(3,20,0)
#define get_user_pages_locked LINUX_BACKPORT(get_user_pages_locked)
long get_user_pages_locked(unsigned long start, unsigned long nr_pages,
		    int write, int force, struct page **pages, int *locked);
#define get_user_pages_unlocked LINUX_BACKPORT(get_user_pages_unlocked)
long get_user_pages_unlocked(unsigned long start, unsigned long nr_pages,
		    int write, int force, struct page **pages);
#elif LINUX_VERSION_IS_LESS(4,6,0)
static inline
long backport_get_user_pages_locked(unsigned long start, unsigned long nr_pages,
		    int write, int force, struct page **pages, int *locked)
{
	return get_user_pages_locked(current, current->mm, start, nr_pages,
		    write, force, pages, locked);
}
#define get_user_pages_locked LINUX_BACKPORT(get_user_pages_locked)

static inline
long backport_get_user_pages_unlocked(unsigned long start, unsigned long nr_pages,
				      int write, int force, struct page **pages)
{
	return get_user_pages_unlocked(current, current->mm, start,  nr_pages,
		    write, force, pages);
}
#define get_user_pages_unlocked LINUX_BACKPORT(get_user_pages_unlocked)
#endif

#if LINUX_VERSION_IS_LESS(4,6,0)
static inline
long backport_get_user_pages(unsigned long start, unsigned long nr_pages,
			    int write, int force, struct page **pages,
			    struct vm_area_struct **vmas)
{
	return get_user_pages(current, current->mm, start,  nr_pages,
		    write, force, pages, vmas);
}
#define get_user_pages LINUX_BACKPORT(get_user_pages)
#endif

#ifndef FOLL_TRIED
#define FOLL_TRIED	0x800	/* a retry, previous pass started an IO */
#endif

#if LINUX_VERSION_IS_LESS(4,1,9) && \
     LINUX_VERSION_IS_GEQ(3,6,0)
#define page_is_pfmemalloc LINUX_BACKPORT(page_is_pfmemalloc)
static inline bool page_is_pfmemalloc(struct page *page)
{
	return page->pfmemalloc;
}
#endif /* < 4.2 */

#if LINUX_VERSION_IS_LESS(4,12,0)
#define kvmalloc LINUX_BACKPORT(kvmalloc)
static inline void *kvmalloc(size_t size, gfp_t flags)
{
	gfp_t kmalloc_flags = flags;
	void *ret;

	if ((flags & GFP_KERNEL) != GFP_KERNEL)
		return kmalloc(size, flags);

	if (size > PAGE_SIZE)
		kmalloc_flags |= __GFP_NOWARN | __GFP_NORETRY;

	ret = kmalloc(size, flags);
	if (ret || size < PAGE_SIZE)
		return ret;

	return vmalloc(size);
}

#define kvmalloc_array LINUX_BACKPORT(kvmalloc_array)
static inline void *kvmalloc_array(size_t n, size_t size, gfp_t flags)
{
	size_t bytes;

	if (unlikely(check_mul_overflow(n, size, &bytes)))
		return NULL;

	return kvmalloc(bytes, flags);
}

#define kvzalloc LINUX_BACKPORT(kvzalloc)
static inline void *kvzalloc(size_t size, gfp_t flags)
{
	return kvmalloc(size, flags | __GFP_ZERO);
}
#endif

#endif /* __BACKPORT_MM_H */

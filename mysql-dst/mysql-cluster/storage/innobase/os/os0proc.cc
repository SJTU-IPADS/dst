/*****************************************************************************

Copyright (c) 1995, 2014, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA

*****************************************************************************/

/**************************************************//**
@file os/os0proc.cc
The interface to the operating system
process control primitives

Created 9/30/1995 Heikki Tuuri
*******************************************************/

#include "ha_prototypes.h"

#include "os0proc.h"
#ifdef UNIV_NONINL
#include "os0proc.ic"
#endif

#include "srv0srv.h"
#include "ut0mem.h"
#include "ut0byte.h"

/* FreeBSD for example has only MAP_ANON, Linux has MAP_ANONYMOUS and
MAP_ANON but MAP_ANON is marked as deprecated */
#if defined(MAP_ANONYMOUS)
#define OS_MAP_ANON	MAP_ANONYMOUS
#elif defined(MAP_ANON)
#define OS_MAP_ANON	MAP_ANON
#endif

/** The total amount of memory currently allocated from the operating
system with os_mem_alloc_large(). */
ulint	os_total_large_mem_allocated = 0;

/** Whether to use large pages in the buffer pool */
my_bool	os_use_large_pages;

/** Large page size. This may be a boot-time option on some platforms */
uint	os_large_page_size;

/** Converts the current process id to a number.
@return process id as a number */
ulint
os_proc_get_number(void)
/*====================*/
{
#ifdef _WIN32
	return(static_cast<ulint>(GetCurrentProcessId()));
#else
	return(static_cast<ulint>(getpid()));
#endif
}

/** Allocates large pages memory.
@param[in,out]	n	Number of bytes to allocate
@return allocated memory */
void*
os_mem_alloc_large(
	ulint*	n)
{
	void*	ptr;
	ulint	size;
#if defined HAVE_LINUX_LARGE_PAGES && defined UNIV_LINUX
	int shmid;
	struct shmid_ds buf;

	if (!os_use_large_pages || !os_large_page_size) {
		goto skip;
	}

	/* Align block size to os_large_page_size */
	ut_ad(ut_is_2pow(os_large_page_size));
	size = ut_2pow_round(*n + (os_large_page_size - 1),
			     os_large_page_size);

	shmid = shmget(IPC_PRIVATE, (size_t) size, SHM_HUGETLB | SHM_R | SHM_W);
	if (shmid < 0) {
		ib::warn() << "Failed to allocate " << size
			<< " bytes. errno " << errno;
		ptr = NULL;
	} else {
		ptr = shmat(shmid, NULL, 0);
		if (ptr == (void*)-1) {
			ib::warn() << "Failed to attach shared memory segment,"
				" errno " << errno;
			ptr = NULL;
		}

		/* Remove the shared memory segment so that it will be
		automatically freed after memory is detached or
		process exits */
		shmctl(shmid, IPC_RMID, &buf);
	}

	if (ptr) {
		*n = size;
		os_atomic_increment_ulint(
			&os_total_large_mem_allocated, size);

		UNIV_MEM_ALLOC(ptr, size);
		return(ptr);
	}

	ib::warn() << "Using conventional memory pool";
skip:
#endif /* HAVE_LINUX_LARGE_PAGES && UNIV_LINUX */

#ifdef _WIN32
	SYSTEM_INFO	system_info;
	GetSystemInfo(&system_info);

	/* Align block size to system page size */
	ut_ad(ut_is_2pow(system_info.dwPageSize));
	/* system_info.dwPageSize is only 32-bit. Casting to ulint is required
	on 64-bit Windows. */
	size = *n = ut_2pow_round(*n + (system_info.dwPageSize - 1),
				  (ulint) system_info.dwPageSize);
	ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE,
			   PAGE_READWRITE);
	if (!ptr) {
		ib::info() << "VirtualAlloc(" << size << " bytes) failed;"
			" Windows error " << GetLastError();
	} else {
		os_atomic_increment_ulint(
			&os_total_large_mem_allocated, size);
		UNIV_MEM_ALLOC(ptr, size);
	}
#else
	size = getpagesize();
	/* Align block size to system page size */
	ut_ad(ut_is_2pow(size));
	size = *n = ut_2pow_round(*n + (size - 1), size);
	ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | OS_MAP_ANON, -1, 0);
	if (UNIV_UNLIKELY(ptr == (void*) -1)) {
		ib::error() << "mmap(" << size << " bytes) failed;"
			" errno " << errno;
		ptr = NULL;
	} else {
		os_atomic_increment_ulint(
			&os_total_large_mem_allocated, size);
		UNIV_MEM_ALLOC(ptr, size);
	}
#endif
	return(ptr);
}

/** Frees large pages memory.
@param[in]	ptr	pointer returned by os_mem_alloc_large()
@param[in]	size	size returned by os_mem_alloc_large() */
void
os_mem_free_large(
	void	*ptr,
	ulint	size)
{
	ut_a(os_total_large_mem_allocated >= size);

#if defined HAVE_LINUX_LARGE_PAGES && defined UNIV_LINUX
	if (os_use_large_pages && os_large_page_size && !shmdt(ptr)) {
		os_atomic_decrement_ulint(
			&os_total_large_mem_allocated, size);
		UNIV_MEM_FREE(ptr, size);
		return;
	}
#endif /* HAVE_LINUX_LARGE_PAGES && UNIV_LINUX */
#ifdef _WIN32
	/* When RELEASE memory, the size parameter must be 0.
	Do not use MEM_RELEASE with MEM_DECOMMIT. */
	if (!VirtualFree(ptr, 0, MEM_RELEASE)) {
		ib::error() << "VirtualFree(" << ptr << ", " << size
			<< ") failed; Windows error " << GetLastError();
	} else {
		os_atomic_decrement_ulint(
			&os_total_large_mem_allocated, size);
		UNIV_MEM_FREE(ptr, size);
	}
#elif !defined OS_MAP_ANON
	ut_free(ptr);
#else
# if defined(UNIV_SOLARIS)
	if (munmap(static_cast<caddr_t>(ptr), size)) {
# else
	if (munmap(ptr, size)) {
# endif /* UNIV_SOLARIS */
		ib::error() << "munmap(" << ptr << ", " << size << ") failed;"
			" errno " << errno;
	} else {
		os_atomic_decrement_ulint(
			&os_total_large_mem_allocated, size);
		UNIV_MEM_FREE(ptr, size);
	}
#endif
}
 
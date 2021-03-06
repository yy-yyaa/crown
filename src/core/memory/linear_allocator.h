/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "allocator.h"

namespace crown
{

/// Allocates memory linearly from a predefined chunk
/// and frees all the allocations with a single call to clear()
///
/// @ingroup Memory
class LinearAllocator : public Allocator
{
public:

	LinearAllocator(Allocator& backing, uint32_t size);
	LinearAllocator(void* start, uint32_t size);
	~LinearAllocator();

	/// @copydoc Allocator::allocate()
	void* allocate(uint32_t size, uint32_t align = Allocator::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	/// @note
	/// The linear allocator does not support deallocating
	/// individual allocations, rather you have to call
	/// clear() to free all allocated memory at once.
	void deallocate(void* data);

	/// Frees all the allocations made by allocate()
	void clear();

	/// @copydoc Allocator::allocated_size()
	uint32_t allocated_size(const void* /*ptr*/) { return SIZE_NOT_TRACKED; }

	/// @copydoc Allocator::total_allocated()
	uint32_t total_allocated() { return _offset; }

private:

	Allocator* _backing;

	void* _physical_start;
	uint32_t _total_size;
	uint32_t _offset;
};

} // namespace crown

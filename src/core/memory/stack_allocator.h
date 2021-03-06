/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "allocator.h"

namespace crown
{

/// Allocates memory linearly in a stack-like fashion from a
/// predefined chunk. All deallocations must occur in LIFO
/// order.
///
/// @ingroup Memory
class StackAllocator : public Allocator
{
public:

	StackAllocator(void* start, uint32_t size);
	~StackAllocator();

	/// @copydoc Allocator::allocate()
	void* allocate(uint32_t size, uint32_t align = Allocator::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	/// @note
	/// Deallocations must occur in LIFO order i.e. the
	/// last allocation must be freed for first.
	void deallocate(void* data);

	/// @copydoc Allocator::allocated_size()
	uint32_t allocated_size(const void* /*ptr*/) { return SIZE_NOT_TRACKED; }

	/// @copydoc Allocator::total_allocated()
	uint32_t total_allocated();

private:

	struct Header
	{
		uint32_t offset;
		uint32_t alloc_id;
	};

	void* _physical_start;
	void* _top;
	uint32_t _total_size;
	uint32_t _allocation_count;
};

} // namespace crown

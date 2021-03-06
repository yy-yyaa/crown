/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "error.h"
#include "mutex.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#include "win_headers.h"
	#include <limits.h>
#endif

namespace crown
{

struct Semaphore
{
	Semaphore()
#if CROWN_PLATFORM_POSIX
		: _count(0)
#elif CROWN_PLATFORM_WINDOWS
		: _handle(INVALID_HANDLE_VALUE)
#endif
	{
#if CROWN_PLATFORM_POSIX
		int result = pthread_cond_init(&m_cond, NULL);
		CE_ASSERT(result == 0, "pthread_cond_init: errno = %d", result);
		CE_UNUSED(result);
#elif CROWN_PLATFORM_WINDOWS
		_handle = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
		CE_ASSERT(_handle != NULL, "CreateSemaphore: GetLastError = %d", GetLastError());
		CE_UNUSED(_handle);
#endif
	}

	~Semaphore()
	{
#if CROWN_PLATFORM_POSIX
		int result = pthread_cond_destroy(&m_cond);
		CE_ASSERT(result == 0, "pthread_cond_destroy: errno = %d", result);
		CE_UNUSED(result);
#elif CROWN_PLATFORM_WINDOWS
		BOOL err = CloseHandle(_handle);
		CE_ASSERT(err != 0, "CloseHandle: GetLastError = %d", GetLastError());
		CE_UNUSED(err);
#endif
	}

	void post(uint32_t count = 1)
	{
#if CROWN_PLATFORM_POSIX
		ScopedMutex sm(_mutex);

		for (uint32_t i = 0; i < count; ++i)
		{
			int result = pthread_cond_signal(&m_cond);
			CE_ASSERT(result == 0, "pthread_cond_signal: errno = %d", result);
			CE_UNUSED(result);
		}

		_count += count;
#elif CROWN_PLATFORM_WINDOWS
		BOOL err = ReleaseSemaphore(_handle, count, NULL);
		CE_ASSERT(err != 0, "ReleaseSemaphore: GetLastError = %d", GetLastError());
		CE_UNUSED(err);
#endif
	}

	void wait()
	{
#if CROWN_PLATFORM_POSIX
		ScopedMutex sm(_mutex);

		while (_count <= 0)
		{
			int result = pthread_cond_wait(&m_cond, &(_mutex._mutex));
			CE_ASSERT(result == 0, "pthread_cond_wait: errno = %d", result);
			CE_UNUSED(result);
		}

		_count--;
#elif CROWN_PLATFORM_WINDOWS
		DWORD result = WaitForSingleObject(_handle, INFINITE);
		CE_ASSERT(result == WAIT_OBJECT_0, "WaitForSingleObject: GetLastError = %d", GetLastError());
		CE_UNUSED(result);
#endif
	}

private:

#if CROWN_PLATFORM_POSIX
	Mutex _mutex;
	pthread_cond_t m_cond;
	int32_t _count;
#elif CROWN_PLATFORM_WINDOWS
	HANDLE _handle;
#endif

private:

	// Disable copying
	Semaphore(const Semaphore& s);
	Semaphore& operator=(const Semaphore& s);
};

} // namespace crown

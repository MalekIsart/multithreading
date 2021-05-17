#pragma once

#include <vector>

#include <thread>
#include <mutex>
#include <atomic>
#include <future>

#if defined(_MSC_VER)
#define PAUSE() _mm_pause()
#elif defined(__arm64__)
#define PAUSE() __asm__ __volatile__ ("yield")
#elif defined(__GNUC__) || defined(__clang__)
#define PAUSE() __asm__ __volatile__ ("pause")
#endif

#define USE_ATOMIC_EVENT 1
#define USE_ATOMIC_JOBCOUNT 0
#define USE_ALIGNED_ATOMICS 1
#if USE_ATOMIC_EVENT == 1
#define USE_ATOMIC_SPIN 0
#endif
#define USE_YIELD 1
#define ATOMIC_TYPE uint32_t

struct Event
{
#if USE_ATOMIC_EVENT
	std::atomic<bool> m_Event = {true};
#else
	std::mutex mutex;
	std::condition_variable cond;
#endif

	void reset() {
	}

	void wait() {
#if USE_ATOMIC_EVENT
		for (;;)
		{
	#if USE_ATOMIC_SPIN
			if (!m_Event.load(std::memory_order_acquire)) {
				return;
			}
#if 0//USE_YIELD
			std::this_thread::yield();
			PAUSE();
#endif
	#else
			// si l'ancienne valeur (retournee par exchange()) vaut false, c'est qu'on a ete signale 
			if (!m_Event.exchange(true, std::memory_order_acquire))
				return;

			// autrement, cela signifie que l'on vient de mettre la variable a vrai, on attend qu'elle
			// passe a false, mais en etant sympa avec le systeme
			while (m_Event.load(std::memory_order_relaxed)) {
		#if USE_YIELD
				std::this_thread::yield();
				PAUSE();
		#endif
			}
	#endif
		}
#else
		std::unique_lock<std::mutex> lock(mutex);
		cond.wait(lock);
#endif
		reset();
	}

	void signal() {
#if USE_ATOMIC_EVENT
		m_Event.store(false, std::memory_order_release);
#else
		std::unique_lock<std::mutex> lock(mutex);
		cond.notify_one();
#endif
	}
};

#if USE_ALIGNED_ATOMICS
#define CACHE_ALIGN alignas(64)
#else
#define CACHE_ALIGN 
#endif

struct ThreadPool
{
	std::vector<std::thread> m_Threads;
	Event m_Signal;

#if USE_ATOMIC_JOBCOUNT
	CACHE_ALIGN std::atomic<size_t> m_JobCount = {};
#else
	size_t m_JobCount = 0;
#endif
	CACHE_ALIGN std::atomic<ATOMIC_TYPE> m_BatchCounter = {};
	CACHE_ALIGN std::atomic<ATOMIC_TYPE> m_SyncCounter = {};
	CACHE_ALIGN std::atomic<ATOMIC_TYPE> m_FinishedCount = {};
	// padding
	CACHE_ALIGN bool m_Quit = false;
	int m_GroupSize = 1;

	std::function<void(ATOMIC_TYPE)> m_Job;

	std::mutex m_DebugMutex;

	void executeBatch(const std::function<void(ATOMIC_TYPE)>& job, ATOMIC_TYPE count = 1);

	void dispatch(const std::function<void(ATOMIC_TYPE)>& job, ATOMIC_TYPE count, ATOMIC_TYPE groupSize);

	inline void setJobCount(ATOMIC_TYPE count) {
#if USE_ATOMIC_JOBCOUNT
		m_JobCount.store(count); 
#else
		m_JobCount = count;
#endif
		m_FinishedCount.store(0);
	}

	inline ATOMIC_TYPE getJobCount() const {
#if USE_ATOMIC_JOBCOUNT
		return m_JobCount.load(std::memory_order_relaxed);
#else
		return m_JobCount;
#endif
	}

	void exit();

	//void notify_all();
	void notify_one();

	void wait_ready();
	void wait_idle();
	void wait_finish();

	void start(size_t count = 1);
	void clean();


};


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
			// si l'ancienne valeur (retournee par exchange()) vaut false, c'est qu'on a ete signale 
			if (!m_Event.exchange(true, std::memory_order_acquire))
				return;

			// autrement, cela signifie que l'on vient de mettre la variable a vrai, on attend qu'elle
			// passe a false, mais en etant sympa avec le systeme
			while (m_Event.load(std::memory_order_relaxed)) {
				std::this_thread::yield();
				PAUSE();
			}
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

#if 1
#define CACHE_ALIGN alignas(64)
#else
#define CACHE_ALIGN 
#endif

struct ThreadPool
{
	std::vector<std::thread> m_Threads;
	Event m_Signal;

	CACHE_ALIGN std::atomic<size_t> m_SyncCounter = {};
	CACHE_ALIGN std::atomic<size_t> m_JobCount = {};
	CACHE_ALIGN std::atomic<size_t> m_FinishedCount = {};	
	CACHE_ALIGN std::atomic<size_t> m_BatchCounter = {};
	CACHE_ALIGN bool m_Quit = false;

	std::function<void(int)> m_Job;

	std::mutex m_DebugMutex;

	void executeBatch(const std::function<void(int)>& job, int count = 1);

	inline void setJobCount(int i) { 
		m_JobCount.store(i); 
		m_FinishedCount.store(0);
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


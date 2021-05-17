#include "ThreadPool.h"

#include <iostream>

void ThreadPool::exit() 
{ 
	m_Quit = true; 
	// defini le nombre de threads a attendre
	m_SyncCounter.store(m_Threads.size());
}


void ThreadPool::executeBatch(const std::function<void(ATOMIC_TYPE)>& job, ATOMIC_TYPE count)
{
	m_BatchCounter.store(0);
	m_Job = job;
	setJobCount(count);

	m_Signal.signal();
}

void ThreadPool::dispatch(const std::function<void(ATOMIC_TYPE)>& job, ATOMIC_TYPE count, ATOMIC_TYPE groupSize)
{
	int workGroupSize = count / m_Threads.size();

	m_BatchCounter.store(0);
	m_GroupSize = workGroupSize;
	setJobCount(m_Threads.size());
	
	m_Job = job;

	m_Signal.signal();

	// le boss travaille un peu pour finir le reste si necessaire

	ATOMIC_TYPE remainder = count - workGroupSize * m_Threads.size();
	ATOMIC_TYPE startIndex = count - remainder;
	for (ATOMIC_TYPE i = startIndex; i < count; i++) {
		job(i);
	}
}


void ThreadPool::notify_one()
{
	m_Signal.signal();
}

void ThreadPool::wait_idle()
{
	while (getJobCount() > m_FinishedCount.load(std::memory_order_acquire))
	{
		m_Signal.signal();
		// dans notre cas le yield fait perdre de precieuses millisecondes
#if USE_YIELD
	#if USE_ATOMIC_SPIN
		std::this_thread::yield();
	#endif
		PAUSE();
#endif
	}
}

void ThreadPool::wait_ready()
{
	// on attend que tout le monde soit initialise
	while (m_SyncCounter.load() > 0) {
		std::this_thread::yield();
	}
}

void ThreadPool::wait_finish()
{
	// il est possible qu'un (ou plusieurs) thread soit en etat wait
	// il faut donc le reveiller pour qu'il puisse quitter
	while (m_SyncCounter.load() > 0) {
		m_Signal.signal();
		std::this_thread::yield();
	}

	// on peut se passer du code suivant si les thread sont detaches
	for (auto& thread : m_Threads)
	{
		thread.join();
	}
}

void ThreadPool::clean()
{
	m_Threads.clear();
	m_Threads.shrink_to_fit();
}

void ThreadPool::start(size_t count)
{
	std::cout << "le thread boss est " << std::this_thread::get_id() << std::endl;

	m_Threads.resize(count);

	m_Signal.reset();
	
	// initialize le compteur de preparation
	m_SyncCounter.store(count);
	
	while (count--)
	{
		m_Threads[count] = std::thread(
			[this, count]
			{
				const size_t index = count;
				
				{
					std::lock_guard<std::mutex> lock(m_DebugMutex);
					std::cout << "Le worker [" << index << "] est " << std::this_thread::get_id() << std::endl;
				}

				// compteur de preparation
				m_SyncCounter--;

				size_t workCount = 0;

				for (;;) 
				{
					m_Signal.wait();

					if (!m_Quit)
					{
						ATOMIC_TYPE id = m_BatchCounter.load(std::memory_order_acquire);
						if (id < m_Threads.size())
						{
							m_BatchCounter++;
							ATOMIC_TYPE start = id * m_GroupSize;
							ATOMIC_TYPE end = start + m_GroupSize;
							for (ATOMIC_TYPE i = start; i < end; i++)
							{
								m_Job(i);
								workCount++;
							}


							/*{
								std::lock_guard<std::mutex> lock(m_DebugMutex);
								std::cout << "[" << index << "] travail #" << workCount << std::endl;
							}*/

							m_FinishedCount.fetch_add(1, std::memory_order_acquire);
						}
					}
					else {
						break;
					}
				}

				{
					std::lock_guard<std::mutex> lock(m_DebugMutex);
					std::cout << "[" << index << "] travail total " << workCount << std::endl;
					std::cout << "[" << index << "] " << std::this_thread::get_id() << " vient de quitter" << std::endl;
				}

				// compteur de fin lorsque l'on quitte la boucle
				m_SyncCounter--;
			}
		);
	}
}

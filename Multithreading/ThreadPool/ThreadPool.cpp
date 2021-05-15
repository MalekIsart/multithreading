#include "ThreadPool.h"

#include <iostream>

void ThreadPool::exit() 
{ 
	m_Quit = true; 
	// defini le nombre de threads a attendre
	m_SyncCounter.store(m_Threads.size());
}


void ThreadPool::executeBatch(const std::function<void(int)>& job, int count) 
{
	m_BatchCounter.store(0);
	m_Job = job;
	setJobCount(count);

	m_Signal.signal();
}

void ThreadPool::dispatch(const std::function<void(int)>& job, int count, int groupSize)
{
	int workGroupSize = count / m_Threads.size();

	m_BatchCounter.store(0);
	m_GroupSize = workGroupSize;
	setJobCount(m_Threads.size());
	
	m_Job = job;

	m_Signal.signal();

	// le boss travaille un peu pour finir le reste si necessaire

	int remainder = count - workGroupSize * m_Threads.size();
	int startIndex = count - remainder;
	for (int i = startIndex; i < count; i++) {
		job(i);
	}
}


void ThreadPool::notify_one()
{
	m_Signal.signal();
}

void ThreadPool::wait_idle()
{
	while (m_JobCount.load(std::memory_order_relaxed) > m_FinishedCount.load(std::memory_order_relaxed))
	{
		m_Signal.signal();
		// dans notre cas le yield fait perdre de precieuses millisecondes
		//std::this_thread::yield();
		PAUSE();
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
						if (m_BatchCounter < m_Threads.size())
						{
							int id = m_BatchCounter++;
							int start = id * m_GroupSize;
							int end = start + m_GroupSize;
							for (int i = start; i < end; i++)
							{
								m_Job(i);
								workCount++;
							}


							/*{
								std::lock_guard<std::mutex> lock(m_DebugMutex);
								std::cout << "[" << index << "] travail #" << workCount << std::endl;
							}*/

							m_FinishedCount++;
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

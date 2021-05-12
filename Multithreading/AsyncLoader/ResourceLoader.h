#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>

struct ResourceLoader
{
	std::thread m_Thread;
	std::future<void> m_Task;

	std::atomic<bool> m_Event = {};

	bool m_Quit = false;
	bool m_Ready = false;
	bool m_GotWork = false;

	void exit();

	void reset();
	void wait();
	void signal();
	
	void waitForWork();
	void notifyWork();

	// le main du thread
	void main();

	// cree et execute le thread
	void run();
	// similaire a run mais en utilisant une tache async
	void runTask();
};
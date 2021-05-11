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

	bool m_Quit = false;
	bool m_Ready = false;

	void exit();

	void reset();
	void wait();
	void signal();

	// le main du thread
	void main();

	// cree et execute le thread
	void run();

	// similaire a run mais en utilisant une tache async
	void runTask();
};
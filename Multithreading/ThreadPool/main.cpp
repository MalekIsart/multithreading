

// pour quitter la boucle : 
// a ne pas utiliser dans une vraie application
#include <conio.h>
#include <iostream>

#include "ThreadPool.h"

struct Application
{
	struct Data
	{
		float m[16];
		void Compute(uint32_t value)
		{
			for (int i = 0; i < 16; ++i)
			{
				m[i] += float(value + i);
			}
		}
	};
	static const uint32_t dataCount = 1000000;

	bool shouldQuit()
	{
		if (_kbhit())
		{
			int ch = _getch();
			if (ch == 27)			// ESC 
				return true;
		}
		return false;
	}

	void Spin(float milliseconds)
	{
		milliseconds /= 1000.0f;
		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
		double ms = 0;
		while (ms < milliseconds)
		{
			std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
			ms = time_span.count();
		}
	}

	struct timer
	{
		std::string name;
		std::chrono::high_resolution_clock::time_point start;

		timer(const std::string& name) : name(name), start(std::chrono::high_resolution_clock::now()) {}
		~timer()
		{
			auto end = std::chrono::high_resolution_clock::now();
			std::cout << name << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " milliseconds" << std::endl;
		}
	};

	void run()
	{
		size_t numCores = std::thread::hardware_concurrency();		

		// Serial test
		{
			Data* dataSet = new Data[dataCount];
			{
				auto t = timer("loop test: ");

				for (uint32_t i = 0; i < dataCount; ++i)
				{
					dataSet[i].Compute(i);
				}
			}
			delete[] dataSet;
		}

		ThreadPool pool;
		{			
			pool.start(numCores);
			
			pool.wait_ready();

			Data* dataSet = new Data[dataCount];
			for (int i = 0; i < 10; i++)
			{
				auto t = timer("Dispatch test ");
				
				pool.dispatch(
					[&dataSet](int j) {
						dataSet[j].Compute(j);
					}
					, dataCount, 10000);
				
				pool.wait_idle();
			}
			delete[] dataSet;
		}

		std::cout << "quitting" << std::endl;		
		pool.exit();

		pool.wait_finish();
		pool.clean();

		std::cout << "bye" << std::endl;
	}
};

int main(void)
{
	Application app;

	app.run();

	return 1;
}

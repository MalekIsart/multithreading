

// pour quitter la boucle : 
// a ne pas utiliser dans une vraie application
#include <conio.h>
#include <iostream>

#include "ResourceLoader.h"

struct Application
{
	ResourceLoader resourceLoader;

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

	void run()
	{
		resourceLoader.run();

		// attente du signal de demarrage
		resourceLoader.wait();

		std::cout << "starting" << std::endl;

		// reinitialisation pour usage
		// il est possible que le reset() ait lieu avant le wait()
		// il faut donc une forme de barriere memoire dans reset()
		resourceLoader.reset();

		// ATTENTION: quittez avec la touche ECHAP/ESC

		while (!shouldQuit())
		{
			std::cout << "new frame" << std::endl;

			// emule un traitement (une synchro verticale par ex.)
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		std::cout << "user wants to quit" << std::endl;

		resourceLoader.exit();

		// attente du signal de fin
		resourceLoader.wait();

		//resourceLoader.join();

		std::cout << "quitting" << std::endl;
	}
};

int main(void)
{
	Application app;
	
	app.run();

	return 1;
}
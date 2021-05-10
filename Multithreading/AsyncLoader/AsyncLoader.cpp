

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

		// todo: attente du signal de demarrage

		// ATTENTION: quittez avec la touche ECHAP/ESC

		while (!shouldQuit())
		{
			std::cout << "new frame" << std::endl;

			// emule un traitement (une synchro verticale par ex.)
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		resourceLoader.exit();
	}
};

int main(void)
{
	Application app;
	
	app.run();

	return 1;
}
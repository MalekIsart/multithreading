
#include "ResourceLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb/stb_image.h"

#include <iostream>

bool loadImage(const char* path)
{
	int w, h, c;
	uint8_t* data = stbi_load(path, &w, &h, &c, STBI_rgb_alpha);
	if (data == nullptr)
	{
		std::cout << "erreur au chargement de " << path;
		return false;
	}

	// pour le moment on ne fait que liberer la memoire
	stbi_image_free(data);

	// todo: push une structure Bitmap ou Texture dans une Queue
}

// ---

void ResourceLoader::exit()
{
	m_Quit = true;
}

void ResourceLoader::main()
{
	// code de preparation

	// todo: signal de confirmation demarrage

	while (!m_Quit)
	{
		// routine principale
		loadImage("../data/ironman.dff.png");
		std::cout << "donnees chargees" << std::endl;
	}

	std::cout << "exit!" << std::endl;

	// todo: signal de confirmation de fin

	// code de nettoyage
}

void ResourceLoader::run()
{
	m_Thread = std::thread(&ResourceLoader::main, this);
	// essayez avec et sans detach pour voir la difference au niveau destructeur
	//m_Thread.detach();
}

void ResourceLoader::runTask()
{
	m_Task = std::async(std::launch::async, &ResourceLoader::main, this);
	//std::packaged_task<void(void)>()
}

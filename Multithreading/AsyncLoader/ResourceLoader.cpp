
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

void ResourceLoader::reset()
{
	// si on a besoin d'un ordonnancement, mais que l'on ne souhaite pas
	// pour autant faire de Ready une variable atomique, on peut utiliser une fence
	// qui va agir comme une barriere memoire
	// ici empeche le reordonnancement des read avant le write 
	std::atomic_thread_fence(std::memory_order_seq_cst);

	m_Ready = false;
}

void ResourceLoader::signal()
{
	// si on a besoin d'un ordonnancement, mais que l'on ne souhaite pas
	// pour autant faire de Ready une variable atomique, on peut utiliser une fence
	// qui va agir comme une barriere memoire
	// ici empeche le reordonnancement des read avant le write 
	std::atomic_thread_fence(std::memory_order_seq_cst);

	m_Ready = true;
}

void ResourceLoader::wait()
{
	while (!m_Ready) {
		std::this_thread::yield();
		_mm_pause();
	};
}

void ResourceLoader::exit()
{
	m_Quit = true;
}

void ResourceLoader::main()
{
	// todo: code de preparation

	reset();	

	// signal de confirmation demarrage
	signal();

	while (!m_Quit)
	{
		// routine principale
		loadImage("../data/ironman.dff.png");
		std::cout << "donnees chargees" << std::endl;
	}

	// on a bien quit la boucle
	std::cout << "exit!" << std::endl;

	// signal de confirmation de fin (doit etre reset au prealable)
	signal();	

	// todo: code de nettoyage
}

void ResourceLoader::run()
{
	m_Thread = std::thread(&ResourceLoader::main, this);
	// essayez avec et sans detach pour voir la difference au niveau destructeur
	m_Thread.detach();
}

void ResourceLoader::runTask()
{
	m_Task = std::async(std::launch::async, &ResourceLoader::main, this);
	//std::packaged_task<void(void)>()
}

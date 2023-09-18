#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <ctime>
#include <set>
#include <thread>
#include "fetchhtml.h"
#include "pagehandler.h"
#include <vector>
#include <mutex>


std::mutex mutexThread;
typedef std::set<std::string> ListeString;
ListeString listeUrl;
ListeString listeUrlTraité;
int _count;

struct HandlerInfo
{
	PageHandler* handler;
	HMODULE dll;
};

std::vector<HandlerInfo> handlers;


HandlerInfo LoadPlugin(const std::string& dllName)
{
	HandlerInfo hi;
	hi.dll = LoadLibrary(dllName.c_str());

	if (!hi.dll)
	{
		return hi;
	}

	typedef PageHandler* (*GetHandlerPtr)();
	GetHandlerPtr GetHandler = (GetHandlerPtr)GetProcAddress(hi.dll, "GetHandler");
	hi.handler = GetHandler();

	return hi;
}

void UnloadPlugin(HandlerInfo hi)
{
	FreeLibrary(hi.dll);
}

void ExtraireUrl()
{
	std::string html;
	std::string avant;
	_count = listeUrl.size();

	while (_count > 0)
	{
		// Prendre le premier lien
		if (!listeUrl.empty())
		{
			mutexThread.lock();
			std::string url = *listeUrl.begin();
			listeUrl.erase(listeUrl.begin());
			mutexThread.unlock();

			html = FetchHtml(url);
			avant = url;
		}

		std::cout << "." << std::flush;

		mutexThread.lock();
		for (HandlerInfo hi : handlers)
		{
			hi.handler->HandlePage(html);
		}
		mutexThread.unlock();

		std::string::size_type pos1 = 0;
		while (pos1 != std::string::npos)
		{
			pos1 = html.find("<a ", pos1);

			if (pos1 != std::string::npos)
			{
				std::string::size_type pos2 = html.find("href=", pos1);

				if (pos2 != std::string::npos)
				{
					std::string::size_type pos3 = html.find("\"", pos2 + 6);
					std::string url2 = html.substr(pos2 + 6, pos3 - pos2 - 6);

					if (!url2.empty() && !(url2.find("mailto:") != std::string::npos) && (listeUrlTraité.find(url2) != listeUrlTraité.end()) == false && avant.back() == '/')
					{
						mutexThread.lock();
						listeUrl.insert(avant + url2);
						listeUrlTraité.insert(avant + url2);
						mutexThread.unlock();
					}

					pos1 = pos3;
				}
				else
				{
					pos1++;
				}
			}
		}
		_count = listeUrl.size();
	}
	std::cout << "test" << std::endl;
}

int main()
{
	std::ifstream courriels("urls.txt");
	if (!courriels.is_open())
	{
		std::cout << "Erreur lors de l'ouverture du fichier de courriels (courriels.txt)" << std::endl;
		system("pause");
		return 1;
	}

	std::string url;
	while (std::getline(courriels, url))
	{
		listeUrl.insert(url);
	}
	courriels.close();

	_count = listeUrl.size();

	std::cout << "La liste initiale contient " << listeUrl.size() << " urls..." << std::endl;

	time_t debut = time(0);

	ListeString listeCourriel;

	HMODULE exe = GetModuleHandle(0);
	char exePathBuffer[MAX_PATH];
	GetModuleFileName(exe, exePathBuffer, MAX_PATH);
	std::string exePath(exePathBuffer);
	exePath = exePath.substr(0, exePath.rfind("\\") + 1);

	// Rechercher les fichiers .dll
	WIN32_FIND_DATA data;
	HANDLE h = FindFirstFile((exePath + "*.dll").c_str(), &data);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			std::cout << "Chargement de " << data.cFileName << std::endl;
			handlers.push_back(LoadPlugin(data.cFileName)); // Doit etre le premier dans la liste..
		} while (FindNextFile(h, &data));
		FindClose(h);
	}

	// Déclaration des threads
	const int threadCount = 20;
	std::thread t[threadCount];

	// Appeler les threads
	for (int i = 0; i < threadCount; i++)
		t[i] = std::thread(ExtraireUrl);

	for (int i = 0; i < threadCount; i++)
		t[i].join();

	// Afficher les résultats
	for (HandlerInfo hi : handlers)
	{
		hi.handler->AfficherResultat();
	}

	std::cout << std::endl;
	std::cout << "Temps requis: " << time(0) - debut << " secondes." << std::endl;

	for (HandlerInfo hi : handlers)
		UnloadPlugin(hi);

	system("pause");
}
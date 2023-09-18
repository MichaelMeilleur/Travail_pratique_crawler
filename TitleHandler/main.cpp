#include "pch.h"
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include "../crawler/pagehandler.h"

typedef std::vector<std::string> ListeString;

class TitleHandler : public PageHandler
{
public:
	TitleHandler() {}

	~TitleHandler() {}

	virtual void HandlePage(std::string& html)
	{
		// Cette boucle localise tout titre h1 présents dans la page
		std::string::size_type pos1 = 0;
		while (pos1 != std::string::npos)
		{
			// Chercher pour les balises h1
			pos1 = html.find("<h1>", pos1);

			if (pos1 != std::string::npos)
			{
				// Incrémenter la position de la longueur de "<h1>"
				pos1 += 4;

				// Chercher la fin de h1
				std::string::size_type pos2 = html.find("</h1>", pos1);

				// Si non trouvé, le document HTML est mal formé, skipper
				// ce titre h1
				if (pos2 == std::string::npos)
					continue;

				// Extraire le titre avec les positions trouvées
				std::string title = html.substr(pos1, pos2 - pos1);

				if (title.length() != 0)
				{
					// Insérer le titre trouvé dans la liste
					listeTitresH1.push_back(title);
				}
			}
		}
	}

	virtual void AfficherResultat()
	{
		for (std::string title : listeTitresH1)
		{
			std::cout << title << std::endl;
		}
		std::cout << "Nombre de titres: " << listeTitresH1.size() << std::endl;
	}

private:
	ListeString listeTitresH1;
};

TitleHandler handler;

extern "C"
{
	__declspec (dllexport) PageHandler* GetHandler()
	{
		return &handler;
	}
}
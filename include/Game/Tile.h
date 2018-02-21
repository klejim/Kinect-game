#pragma once
#include "SFML\Graphics.hpp"
#include <random>
#include <ctime>
#include <iostream>



//definition des variables globales
const int largeur_fenetre(28), hauteur_fenetre(17);
const int largeur_tile(70), hauteur_tile(70);



class TileMap : public sf::Drawable, public sf::Transformable
{
public:

	int level[largeur_fenetre + 1][hauteur_fenetre];

	TileMap(void) 
	{

		chargement();//initialisation des textures
		for (int hauteur(0); hauteur < hauteur_fenetre; hauteur++) //test pour initialiser le tableau au debut (ne foncitonne pas pour le moment)
		{
			for (int largeur(0); largeur < largeur_fenetre; largeur++)
			{
				//placement du sol 
				if (hauteur % 5 == 3)
				{
					level[largeur][hauteur] = sol_base;
				}

				//placement du 2eme tile du sol
				else if (hauteur % 5 == 4)
				{
					level[largeur][hauteur] = sous_sol;
				}
				else
				{
					level[largeur][hauteur] = arriere_plan;
				}
			}
		}
	}

	bool chargement() 
	{

		if (!m_tileset.loadFromFile("tileset.png"))
		{
			return false;
		}
	
		
		return 1;
	}

    bool refresh( void)
    {
		// on redimensionne le tableau de vertex pour qu'il puisse contenir tout le niveau (4 points par forme, donc largeur_tille*hauteur_tile*4)
        m_vertices.setPrimitiveType(sf::Quads);
        m_vertices.resize(largeur_fenetre * hauteur_fenetre * 4);

        // on remplit le tableau de vertex, avec un quad par tile 
        for (unsigned int i = 0; i < largeur_fenetre; ++i)
            for (unsigned int j = 0; j < hauteur_fenetre; ++j)
            {
                // on récupère le numéro de tuile courant
                int tileNumber = level[i][j];

                // on en déduit sa position dans la texture du tileset
                int tu = tileNumber % (m_tileset.getSize().x / tileSize.x);
                int tv = tileNumber / (m_tileset.getSize().x / tileSize.x);
				
                // on récupère un pointeur vers le quad à définir dans le tableau de vertex
                sf::Vertex* quad = &m_vertices[(i + j * largeur_fenetre) * 4];

                // on définit ses quatre coins
                quad[0].position = sf::Vector2f((i* tileSize.x), j * tileSize.y);
                quad[1].position = sf::Vector2f((i + 1) * tileSize.x, j * tileSize.y);
                quad[2].position = sf::Vector2f((i + 1) * tileSize.x, (j + 1) * tileSize.y);
                quad[3].position = sf::Vector2f(i * tileSize.x, (j + 1) * tileSize.y);

                // on définit ses quatre coordonnées de texture
				quad[0].texCoords = sf::Vector2f(tu * tileSize.x, tv * tileSize.y);
				quad[1].texCoords = sf::Vector2f((tu + 1) * tileSize.x, tv * tileSize.y);
				quad[2].texCoords = sf::Vector2f((tu + 1) * tileSize.x, (tv + 1) * tileSize.y);
				quad[3].texCoords = sf::Vector2f(tu * tileSize.x, (tv + 1) * tileSize.y);
			
            }
		 return true;
    }


	int gene_level(int mode) 
	{
		
		switch (mode) 
		{//switch case pour choisir le mode de gene level
		//case 0 : foncitonnement normal, a uttiliser pour rafraichir le tableau lorsque n=70
		case 0 :
			// on décale  toutes  les collonnes de 1 tile vers la gauche
			for (int largeur(0); largeur < largeur_fenetre; largeur++) 
			{

				for (int hauteur(1); hauteur < hauteur_fenetre; hauteur++) 
				{//animation des objets et decalage
					level[largeur - 1][hauteur] = level[largeur][hauteur];//decalage

					if (level[largeur - 1][hauteur] == scie_sol) //animation de la scie en 2 tiles 
					{
						level[largeur - 1][hauteur] = scie_sol2;
					}

					else if (level[largeur - 1][hauteur] == scie_sol2) 
					{
						level[largeur - 1][hauteur] = scie_sol;
					}

				}
			}

			//On génere la derniere collonne du tableau
			for (int hauteur(0); hauteur < hauteur_fenetre; hauteur++) 
			{
				//placement du sol 
				if (hauteur % 5 == 3) 
				{
					level[largeur_fenetre - 1][hauteur] = sol_base;
				}

				//placement du 2eme tile du sol
				else if (hauteur % 5 == 4) 
				{
					level[largeur_fenetre - 1][hauteur] = sous_sol;
				}

				//4 pour cent de chances de dessiner une scie
				else if (rand() % 100 <= 4 && hauteur % 5 == 2)
				{
					level[largeur_fenetre - 1][hauteur] = scie_sol;
				}

				else 
				{
					level[largeur_fenetre - 1][hauteur] = arriere_plan;
				}
				
			}

		break;

		case 1 :
			//cse 1 : foncitonnement pour initialiser le tableau

			for (int largeur(0); largeur < largeur_fenetre; largeur++) 
			{
				for (int hauteur(0); hauteur < hauteur_fenetre; hauteur++) 
				{
					level[largeur][hauteur] = arriere_plan;
					
				}
			}
		break;

		}
			
		
	return 1;
	}

private:

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		// on applique la transformation
		states.transform *= getTransform();

		// on applique la texture du tileset
		states.texture = &m_tileset;

		// et on dessine enfin le tableau de vertex
		target.draw(m_vertices, states);
	}

	
	
	sf::VertexArray m_vertices;
	sf::VertexArray ladder;
	sf::Texture m_tileset;

	sf::Vector2u tileSize = sf::Vector2u(largeur_tile, hauteur_tile);

	enum tiletype  {sol_base=2,sous_sol=4,arriere_plan=107,scie_sol=67,scie_sol2=68, echelle=46};

	
	
};

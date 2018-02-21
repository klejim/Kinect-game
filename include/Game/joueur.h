#pragma once
#include "SFML\Graphics.hpp"


class joueur : public sf::Drawable, public sf::Transformable
{
public:
	int position_x_finale = 140;
	int position_y_finale=7*70;
	int vie;
	const int largeur_joueur = 75;
	const int hauteur_joueur = 96;
	

	joueur (void)
	{
		vie = 140;
		chargement();
		//position_x_finale = 1;//ligne pour eviter que la condition de sortie du jeu soit activée des le debut 
	}
	
	bool chargement() 
	{

		if (!m_tileset.loadFromFile("walk.png"))
			return false;
		return 1;
	}
	
	bool mouvement(int texture,int i,float vitesse) {// n variable definissant la texture courante, mouvement variable definissant la vittesse du personnage,
		//i variable definissant le retour a la ligne (horloge du programme) 

		if (i == 70) 
		{
			position_x = position_x-70;
		}
		else 
		{
			position_x=position_x+vitesse;
		}

		position_x_finale = position_x;
		position_y_finale = position_y;

		// on redimensionne le tableau de vertex pour qu'il puisse contenir tout le niveau (4 points par forme, donc largeur_tile*hauteur_tile*4)
		m_vertices.setPrimitiveType(sf::Quads);
		m_vertices.resize(largeur_fenetre * hauteur_fenetre * 4);

		// on récupère le numéro de tuile courant
		int tileNumber = texture;

		// on en déduit sa position dans la texture du tileset
		int tu = tileNumber % (m_tileset.getSize().x / largeur_tile);
		int tv = tileNumber / (m_tileset.getSize().x / largeur_tile);

		// on récupère un pointeur vers le quad à définir dans le tableau de vertex
		sf::Vertex* quad = &m_vertices[largeur_fenetre * 4];


		// on définit ses quatre coins
		quad[0].position = sf::Vector2f(position_x_finale, position_y);
		quad[1].position = sf::Vector2f(position_x_finale + largeur_tile, position_y);
		quad[2].position = sf::Vector2f(position_x_finale + largeur_tile, position_y + hauteur_tile);
		quad[3].position = sf::Vector2f(position_x_finale, position_y + hauteur_tile);

		// on définit ses quatre coordonnées de texture
		quad[0].texCoords = sf::Vector2f(tu * largeur_joueur, tv * hauteur_joueur);
		quad[1].texCoords = sf::Vector2f((tu + 1) * largeur_joueur, tv * hauteur_joueur);
		quad[2].texCoords = sf::Vector2f((tu + 1) * largeur_joueur, (tv + 1) * hauteur_joueur);
		quad[3].texCoords = sf::Vector2f(tu * largeur_joueur, (tv + 1) * hauteur_joueur);

		return (1);
	}

	void deplacement_y(bool choix) 
	{//déplacement du joueur par le clavier
		if (choix==1 && position_y < 13*70) //descendre
		{
			position_y = position_y + 5 * 70;
		}
		else if (choix == 0)
		{//monter
			position_y = position_y - 5 * 70;
		}
	}

	int collision(int tile_actuelle_largeur, int tile_actuelle_hauteur, TileMap tile)
	{	
		int type_tile_rencontre;

		type_tile_rencontre = tile.level[tile_actuelle_largeur][tile_actuelle_hauteur];
		//switch case type_tile_rencontre
		switch (type_tile_rencontre) 
		{
		case (67) :// si le joueur rencontre une scie
			vie = vie - 60;//on diminue sa marge de 60
			
			break;
		
		case (68) :// si le joueur rencontre une scie
			vie = vie - 60;//on diminue sa marge de 60
		
			break;

		case (107) : //si le joueur ne rencontre pas d'obstacles
			if (vie <= 140) 
			{
				vie = vie + 1;
			}
			break;

		case (46) ://échelle
			if (tile.level[tile_actuelle_largeur][tile_actuelle_hauteur+1]==46) //descendre
			{
				position_y = position_y + 5 * 70;
			}
			else if (tile.level[tile_actuelle_largeur][tile_actuelle_hauteur-1]==46)
			{//monter
				position_y = position_y - 5 * 70;
			}
			break;


		default :

			break;
		
		}
		//si obstacle 1
		//si obstacle ...

	return vie;
	}


private :

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
	sf::Texture m_tileset;
	
	float position_x=140; 
	float position_y=7*70;

	

};
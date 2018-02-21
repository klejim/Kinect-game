#pragma once
#include "SFML\Graphics.hpp"
#include "SFML\Window.hpp"
#include "Tile.h"

class objects : public sf::Drawable, public sf::Transformable //la classe objets ne sera uttilisée que dans des interruptions
{
public:
	objects() { chargement(); }
	void center_objects(int curseur_x, int curseur_y)
	{// on place la position de l'echelle a 

		position_x = curseur_x - (hauteur_fenetre*hauteur_tile) / 2;
		position_y = curseur_y - (largeur_fenetre*largeur_tile) / 2;

	}
	bool chargement()
	{
		if (!m_tileset.loadFromFile("tileset.png"))
		{
			return false;
		}
		return 1;
	}
	void move_object(int Cposition_x, int Cposition_y) //deplace l'objet d'une position x,y a une position x',y'
	{
		position_x = Cposition_x;
		position_y = Cposition_y;

		return;
	}

	void set_object(int curseur_x, int curseur_y, TileMap &tile) // "fixe" l'objet dans le tableau level 
	{
		int tile_actuelle_hauteur, tile_actuelle_largeur;
		//detection de la position de l'echelle
		tile_actuelle_hauteur = (curseur_y) / hauteur_tile;
		tile_actuelle_largeur = (curseur_x) / largeur_tile;
		int i = 0;
		if (tile_actuelle_hauteur < hauteur_fenetre / 2)
		{
			i = 7;
		}
		else 
		{
			i = 12;
		}
		for (int k=i; k > i-6; k--)
		{
			tile.level[tile_actuelle_largeur][k] = 46;
		}
	}

	void refresh() {

		// on redimensionne le tableau de vertex pour qu'il puisse contenir tout le niveau (4 points par forme, donc largeur_tile*hauteur_tile*4)
		m_vertices.setPrimitiveType(sf::Quads);
		m_vertices.resize(largeur_fenetre * hauteur_fenetre * 4);

		// on récupère le numéro de tuile courant
		int tileNumber = 46;//numeros correspondant a l'echelle

		// on en déduit sa position dans la texture du tileset
		int tu = tileNumber % (m_tileset.getSize().x / largeur_tile);
		int tv = tileNumber / (m_tileset.getSize().x / largeur_tile);

		// on récupère un pointeur vers le quad à définir dans le tableau de vertex
		sf::Vertex* quad = &m_vertices[largeur_fenetre * 4];


		// on définit ses quatre coins
		quad[0].position = sf::Vector2f(position_x, position_y);
		quad[1].position = sf::Vector2f(position_x + largeur_tile, position_y);
		quad[2].position = sf::Vector2f(position_x + largeur_tile, position_y + hauteur_tile);
		quad[3].position = sf::Vector2f(position_x, position_y + hauteur_tile);

		// on définit ses quatre coordonnées de texture
		quad[0].texCoords = sf::Vector2f(tu * largeur_tile, tv * hauteur_tile);
		quad[1].texCoords = sf::Vector2f((tu + 1) * largeur_tile, tv * hauteur_tile);
		quad[2].texCoords = sf::Vector2f((tu + 1) * largeur_tile, (tv + 1) * hauteur_tile);
		quad[3].texCoords = sf::Vector2f(tu * largeur_tile, (tv + 1) * hauteur_tile);
	}

private:

	float position_x, position_y;

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
};

	
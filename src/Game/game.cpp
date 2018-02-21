#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "KinectInterface/Kinect.hpp"

#include "Game/App.hpp"
#include "Game/Tile.h"
#include "Game/joueur.h"
#include "Game/objects.h"

#include <stdio.h>
#include <string> //to_string
#include <random>

unsigned int App::game(void)
{
	srand(time(0));
	int n(0);
	int vie, tenue;
	int tile_actuelle_hauteur, tile_actuelle_largeur, type_tile_rencontre, memoire_type_tile_rencontre;
	vie = 140;
	memoire_type_tile_rencontre = 0;
	//horloge du compteur, va de 0 a 70 (largeur d'une tile) et se reset une fois arrivée.
	//elle permet de savoir quand retravailler le tablleau a affficher
	int tile_joueur(0);//variable donnant la texture actuelle du joueur, elle va de 0 a 10
	//texte du score
	sf::Vector2f winsize = window.getDefaultView().getSize();
	sf::Font font;
	font.loadFromFile("F25_Executive.otf");
	sf::Text textScore;
	textScore.setFont(font);
	textScore.setString("Score : 0");
	textScore.setFillColor(sf::Color(255,169,99));//ORANGE!
	textScore.setPosition(winsize.x/20- textScore.getGlobalBounds().width/2, 10);
	unsigned int score(0);

	TileMap map;//création de l'objet map
	TileMap background;
	joueur player;
	objects echelle;

	sf::View view;
	view.reset(sf::FloatRect(0, 0, largeur_fenetre*largeur_tile - 100, hauteur_fenetre*hauteur_tile));
	window.setView(view);
	// adapte les coordonnées contenues dans les évènements Kinect aux nouvelles coordonnées de la fenêtre
	KInterface.setTargetImageSize(largeur_fenetre*largeur_tile - 100, hauteur_fenetre*hauteur_tile);

	// on fait tourner la boucle principale
	bool continuer(true);
	while (window.isOpen() && continuer)
	{
		KtI::KinectEvent kinect_event;
		sf::Event event;

		while (window.pollEvent(event));
		while (KInterface.pollEvent(kinect_event))
		{
			if (kinect_event.type==KtI::KinectEvent::HandGrip)//si reconnaissance mouvement creation echelle et pas deja une echelle tenue
			{
				//
				cursor.handClosed = true;
				echelle.center_objects(kinect_event.hand.position.x, kinect_event.hand.position.y);
				tenue = 1;

			}

			else if (kinect_event.type == KtI::KinectEvent::HandRelease)//si  l'echelle n'est plus tenue on l'integre dans les tiles
			{
				cursor.handClosed = false;
				echelle.set_object(kinect_event.hand.position.x, kinect_event.hand.position.y,map);
			}

			else if (kinect_event.type == KtI::KinectEvent::HandCursorMoved)//si le curseur se déplace, on applique la même transformation a l'echelle (elle le suit)
			{
				sf::Vector2f move;
				move.x = kinect_event.hand.position.x;
				move.y = kinect_event.hand.position.y;
				cursor.setPosition(move);
				echelle.move_object(move.x, move.y);
			}
			else if (kinect_event.type == KtI::KinectEvent::PositionChanged)
			{
				if (kinect_event.position.type == KtI::PositionType::STOP)
					window.close();
			}

		}

		//ecriture du tableau qui devra etre affiché par la methode refresh.
		//On actualise le niveau seulement losrque la fenetre view atteint la fin du tableau
		if (n == 70) {
			map.gene_level(0);
		}


		background.gene_level(1);

		//dessin du tableau d'entiers level definissant les tiles a dessiner
		map.refresh();



		// dessin du niveau en fonction du tableau d'entiers level
		//mouvement de la vue, et du joueur lié a celle ci
		if (n == 70)
		{
			view.reset(sf::FloatRect(0, 0, largeur_fenetre*largeur_tile - largeur_tile, hauteur_fenetre*hauteur_tile));
			//déplace le curseur
			sf::Vector2f Cpos(cursor.getPosition());
			Cpos.x += largeur_tile;
			cursor.setPosition(Cpos);
			//puis le joueur
			player.mouvement(tile_joueur, n, 1);
			//et le score
			sf::Vector2f tPos(textScore.getPosition());
			tPos.x -= 70;
			textScore.setPosition(tPos);
			n = 0;
		}
		else if (n == 1)
		{
			///////////////////Gestion des Collisions/////////////////////////////

			//on déclare les trois variables dont on aura besoin
			//tile_actuelle_X nous donne la tile actuelle rencontrée par le joueur
			//type_tile_rencontre nous permet de choisir l'action a effectuer en de contact  si = 0 alors pas d'obstacles rencontrés
			//mémoire tile_rencontre nous ppermet de memoriser la derniere tile rencontrée, pour ne modifier la vittesse qu'une fois


			//definition de la position du joueur en tile
			//on réduit aussi la tialle du joueur pour la placer dans un tile de la map
			//on ne recupere que la partie "basse" celle juste au dessus du sol
			tile_actuelle_hauteur = (player.position_y_finale) / hauteur_tile;
			tile_actuelle_largeur = (player.position_x_finale + 70 - n) / largeur_tile;

			//on regarde quelle type de tile le personnage rencontre
			//memoire_type_tile_rencontre = type_tile_rencontre;

			///////////////Fin pré-gestion Collisions/////////////////////////////////////////////////////////
			player.collision(tile_actuelle_largeur, tile_actuelle_hauteur, map);
		}
		else {

			
			if (player.vie - player.position_x_finale <= 0)
			{//blocage du joueur autour de la valeur 140 en x, on le fait accélerer si il pase en dessous du seuil

				player.mouvement(tile_joueur, n, 1);
			}

			else if (player.vie - player.position_x_finale > 0)
			{
				player.mouvement(tile_joueur, n, 2);
				
			}
			view.move(1, 0);//on décale la vue de 1 pixel vers la droite
		}


		if (n % 10 == 0) //incrémentation de la texture du joueur, pour afficher l'image d'apres lors de l'animation marche
		{//ralentissement de l'animation du joueur

			if (tile_joueur == 10)
			{//si on est arrivé a la derniere texture de l'animation, on reaffiche au début des textures
				tile_joueur = 0;//
			}

			else
			{//si la texture suivante peut être chargée, on incrémente la variable
				tile_joueur++;//
			}//
		}
		// gestion du score (calcul et mise à jour de l'affichage
		if (n % 10 == 0)
		{
			score++;
			std::string tmp = textScore.getString().substring(0, 8) + std::to_string(score);
			textScore.setString(tmp);
		}
		int xScore = textScore.getPosition().x;
		if (xScore < winsize.x / 20)
		{
			xScore += 2;
		}
		else xScore++;
		textScore.setPosition(xScore, 10);

		// test de sortie du personnage
		if (player.vie <= -70)
		{
			continuer = false;
		}

		echelle.refresh();
		//gestion de l'affichage
		window.setView(view);
		window.clear();
		window.draw(background);//affichage fond (ne marche pas encore)
		window.draw(map);//affichage du décor + obstacles
		window.draw(textScore);
		window.draw(player);//affichage du joueur
		window.draw(echelle);//affichage de l'echelle
		window.draw(cursor);
		window.display();

		//sf::sleep(sf::milliseconds(2));
		n++;

	}
	// restore les coordonnées de bases (évènements Kinect)
	sf::Vector2f size(window.getDefaultView().getSize());
	KInterface.setTargetImageSize(size.x, size.y);
	return score;
}
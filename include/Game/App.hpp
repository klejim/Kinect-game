#pragma once
#ifndef APP_HPP
#define APP_HPP
#include "SFML\Graphics.hpp"

#include "KinectInterface\Kinect.hpp"
// Ce fichier définit la classe App mais aussi et d'abord les classes SoloTracker et Cursor, utilisées respectivement pour
//  choisir un utilisateur à traquer et définir un curseur remplaçant la souris 


// classe utilisée pour implanter une méthode de sélection des utilisateurs à traquer. Comme elle utilise directement la NUI, on la
// met dans un espace de nom à part.
namespace tracker {
    using namespace KtI::NUI;
    class SoloTracker : public KtI::SkeletonTracker
    {
    public:
        DWORD* choseTrackedSkeletons(NUI_SKELETON_FRAME &newSkeletonFrame)
        {
            // prend le premier utilisateur détecté (les indices sont cohérents entre les trames, donc ce sera toujours le même)
            DWORD *ids = new DWORD[NUI_SKELETON_MAX_TRACKED_COUNT];
            float min(10);
            for (int i = 0; i < NUI_SKELETON_COUNT; i++)
            {
                auto data = newSkeletonFrame.SkeletonData[i];
                if (data.eTrackingState != NUI_SKELETON_NOT_TRACKED)
                {
                    ids[0] = data.dwTrackingID;
                    break;
                }
            }
            ids[1] = 0;
            return ids;
        }
    };
}

// classe représentant un curseur remplaçant la souris
class Cursor : public sf::Drawable
{
public:
    bool handClosed;				 ///> Si la main est fermée ou non (équivalent à un click)

    Cursor() : handClosed(false) {};
    void setTextures(sf::Texture &text1, sf::Texture &text2)
    {
        handCursor.setTexture(text1);
        handCursorClosed.setTexture(text2);
        handCursor.setScale(0.25, 0.25);
        handCursorClosed.setScale(0.25, 0.25);
        return;
    }
    sf::Vector2f getPosition(void)
    {
        return handCursor.getPosition();
    }
    void setPosition(sf::Vector2f pos) 
    {
        handCursor.setPosition(pos.x, pos.y);
        handCursorClosed.setPosition(pos.x, pos.y);
        return;
    }
    void draw(sf::RenderTarget &target, sf::RenderStates states)const
    {
        target.draw(handClosed ? handCursorClosed : handCursor);
        return;
    };
private:
    
    sf::Sprite handCursor;			 ///> Sprite d'une main ouverte, pour remplacer le curseur de la souris.
    sf::Sprite handCursorClosed;     ///> Sprite d'une main fermée, remplace la souris lorsque la main du joueur est fermée.
};

// Classe regroupant les fonctions décrivant les différents écrans (jeu et menus divers)
// Elle possède deux membres : la fenêtre sf::RenderWindow et l'objet KinectInterface dont elle se sert (pour faciliter l'accès
// à ces variables, comme toutes les fonctions en ont besoin).
class App
{
public:	
    sf::RenderWindow window;		 ///> La fenêtre utilisée par l'application.
    KtI::KinectInterface KInterface; ///> L'objet KinectInterface dont elle se sert.
    Cursor cursor;					 ///> Le curseur remplaçant la souris.
    // les méthodes décrivant les différents "écrans"
    void run(void);				     ///> Affiche le menu principal.
    ~App() {
        int a=1;
        a++;
    }
private:
    void startGame(void);			 ///> Lance le jeu et enregistre le nom de l'utilisateur si son score est parmi les 5 meilleurs
    unsigned int game(void);		 ///> Le jeu.
    void optionMenu(void);			 ///> Affiche un menu de démonstration.
    void quit(void);				 ///> Quitte l'application.
};
#endif
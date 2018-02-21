#pragma once
#ifndef VIRTUAL_KEYBOARD_HPP
#define VIRTUAL_KEYBOARD_HPP
#include "SFML\Graphics.hpp"
#include "App.hpp"
#include "Button.hpp"

class VKeyboard;

/// \brief Sp�cialisation de la classe Button utilis�e par la classe VKeyboard.
///
/// Les diff�rences sont:
///		- la fonction li�e accepte un param�tre
///		- la fonction li�e est une m�thode de la classe VKeyboard.
template <class T>
class KeyButton : public Button
{
public:
    // on red�finit les fonctions utilisant le nouveau m_task
    void bind(void((VKeyboard::*task))(T&))
    {
        m_task = task;
        return;
    }

    void doTask(T& arg)
    {
        if (m_task != nullptr)
        {
            (m_keyboard->*m_task)(arg);
        }
        return;
    }
    void setKeyboard(VKeyboard *keyboard)
    {
        m_keyboard = keyboard;
    }
private:
    // red�finition de m_task et ajout d'un pointeur vers le clavier auquel le bouton appartient.
    VKeyboard *m_keyboard;
    void (VKeyboard::*m_task)(T&);
};

/// \brief Classe repr�sentant un clavier num�rique
///
/// Ne g�re que les lettres "simples" : il n'y a pas d'accents.
class VKeyboard : public sf::Drawable
{
public:
    /// \brief Constructeur par d�faut. Ne fait rien.
    VKeyboard();

    /// \brief Construit l'objet VKeyboard. 
    /// \param font La police � utiliser pour le texte.
    /// \param x L'abscisse du point.
    /// \param y L'ordonn�e du point.
    /// \param app l'adresse de l'instance de la classe App utilis�e.
    VKeyboard(sf::Font &font, unsigned int charSize, float x, float y,App *app);

    /// \brief R�cup�re la taille du rectangle englobant le clavier.
    sf::Vector2f getSize(void);

    /// \brief Modifie la position du clavier
    void setPosition(int x, int y);

    /// \brief R�cup�re la cha�ne entr�e par l'utilisateur.
    ///
    /// Cette fonction peut �tre appel�e � n'importe quel moment. Cependant, le clavier dispose d'une m�thode stringReady() pour
    /// signifier que la r�cup�ration de la cha�ne est termin�e (entr�e a �t� appuy�). 
    /// Si getString est appel�e alors que stringReady renvoie true, la cha�ne est automatiquement d�truite pour laisser place � 
    /// la prochaine.
    /// \return string Une copie de la cha�ne entr�e par l'utilisateur.
    sf::String getString(void);

    /// \brief Renvoie true si le bouton entr�e a �t� activ�.
    bool stringReady(void);

    /// \brief Met � jour l'�tat du clavier.
    ///
    /// Cette fonction prend en argument la position du curseur et met � jour l'�tat du clavier et de ses boutons en fonction de
    /// cette position. Un appel � cette fonction est �quivalent � un appel � la m�thode Button::update() de chacun des boutons
    /// du clavier (et met � jour leur focus).
    /// Cette fonction ne fait rien si le clavier n'est pas actif.
    void update(sf::Vector2f cursorPos);

    /// \brief V�rifie si un des boutons doit effectuer sa fonction.
    /// 
    void VKeyboard::checkForTask(void);

    void draw(sf::RenderTarget &target, sf::RenderStates states)const;
    void setActive(bool active);
    bool isActive(void);
    bool onScreen(void);
    void setOnScreen(bool onScreen);


private:
    sf::RectangleShape m_rectangle;		///> Le rectangle englobant le clavier.
    sf::String m_string;				///> La cha�ne recup�rant les lettres entr�es par l'utilisateur.
    KeyButton<char> m_letters[27];		///> Les lettres de l'alphabet.
    KeyButton<int> m_back;				///> Effacer le dernier caract�re entr�.
    KeyButton<bool> m_enter;			///> Valide la cha�ne courante.
    bool m_done;						///> Indique que la cha�ne est pr�te � �tre r�cup�r�e (entr�e a �t� appuy�).
    bool m_active;						///> Si le clavier est actif (interactions voulues).
    bool m_onScreen;					///> Si le clavier et ses boutons doivent �tre dessin�s sur l'�cran.
    
    // utilis�es par les boutons.
    void addCharacter(char &c);			///> Ajoute un caract�re � la cha�ne du clavier.
    void eraseCharacter(int &n);		///> Supprime les n derniers caract�res de la cha�ne.			
    void setDone(bool &done);			/// Modifie la propri�t� m_done (indiquant que la touche entr�e a �t� appuy�e).
};
#endif
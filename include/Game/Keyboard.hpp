#pragma once
#ifndef VIRTUAL_KEYBOARD_HPP
#define VIRTUAL_KEYBOARD_HPP
#include "SFML\Graphics.hpp"
#include "App.hpp"
#include "Button.hpp"

class VKeyboard;

/// \brief Spécialisation de la classe Button utilisée par la classe VKeyboard.
///
/// Les différences sont:
///		- la fonction liée accepte un paramètre
///		- la fonction liée est une méthode de la classe VKeyboard.
template <class T>
class KeyButton : public Button
{
public:
    // on redéfinit les fonctions utilisant le nouveau m_task
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
    // redéfinition de m_task et ajout d'un pointeur vers le clavier auquel le bouton appartient.
    VKeyboard *m_keyboard;
    void (VKeyboard::*m_task)(T&);
};

/// \brief Classe représentant un clavier numérique
///
/// Ne gère que les lettres "simples" : il n'y a pas d'accents.
class VKeyboard : public sf::Drawable
{
public:
    /// \brief Constructeur par défaut. Ne fait rien.
    VKeyboard();

    /// \brief Construit l'objet VKeyboard. 
    /// \param font La police à utiliser pour le texte.
    /// \param x L'abscisse du point.
    /// \param y L'ordonnée du point.
    /// \param app l'adresse de l'instance de la classe App utilisée.
    VKeyboard(sf::Font &font, unsigned int charSize, float x, float y,App *app);

    /// \brief Récupère la taille du rectangle englobant le clavier.
    sf::Vector2f getSize(void);

    /// \brief Modifie la position du clavier
    void setPosition(int x, int y);

    /// \brief Récupère la chaîne entrée par l'utilisateur.
    ///
    /// Cette fonction peut être appelée à n'importe quel moment. Cependant, le clavier dispose d'une méthode stringReady() pour
    /// signifier que la récupération de la chaîne est terminée (entrée a été appuyé). 
    /// Si getString est appelée alors que stringReady renvoie true, la chaîne est automatiquement détruite pour laisser place à 
    /// la prochaine.
    /// \return string Une copie de la chaîne entrée par l'utilisateur.
    sf::String getString(void);

    /// \brief Renvoie true si le bouton entrée a été activé.
    bool stringReady(void);

    /// \brief Met à jour l'état du clavier.
    ///
    /// Cette fonction prend en argument la position du curseur et met à jour l'état du clavier et de ses boutons en fonction de
    /// cette position. Un appel à cette fonction est équivalent à un appel à la méthode Button::update() de chacun des boutons
    /// du clavier (et met à jour leur focus).
    /// Cette fonction ne fait rien si le clavier n'est pas actif.
    void update(sf::Vector2f cursorPos);

    /// \brief Vérifie si un des boutons doit effectuer sa fonction.
    /// 
    void VKeyboard::checkForTask(void);

    void draw(sf::RenderTarget &target, sf::RenderStates states)const;
    void setActive(bool active);
    bool isActive(void);
    bool onScreen(void);
    void setOnScreen(bool onScreen);


private:
    sf::RectangleShape m_rectangle;		///> Le rectangle englobant le clavier.
    sf::String m_string;				///> La chaîne recupèrant les lettres entrées par l'utilisateur.
    KeyButton<char> m_letters[27];		///> Les lettres de l'alphabet.
    KeyButton<int> m_back;				///> Effacer le dernier caractère entré.
    KeyButton<bool> m_enter;			///> Valide la chaîne courante.
    bool m_done;						///> Indique que la chaîne est prête à être récupérée (entrée a été appuyé).
    bool m_active;						///> Si le clavier est actif (interactions voulues).
    bool m_onScreen;					///> Si le clavier et ses boutons doivent être dessinés sur l'écran.
    
    // utilisées par les boutons.
    void addCharacter(char &c);			///> Ajoute un caractère à la chaîne du clavier.
    void eraseCharacter(int &n);		///> Supprime les n derniers caractères de la chaîne.			
    void setDone(bool &done);			/// Modifie la propriété m_done (indiquant que la touche entrée a été appuyée).
};
#endif
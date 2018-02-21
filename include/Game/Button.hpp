#pragma once
#ifndef BUTTON_HPP
#define BUTTON_HPP
#include <string> // std::string
#include <vector> // std::vector
#include "SFML\Graphics.hpp"
#include "App.hpp"

/// \brief Classe repr�sentant un �l�ment d'interface auquel est li� une fonction.
///
/// Un bouton est d�fini pour sa partie visible par l'association d'un sf::Text et d'un sf::RectangularShape repr�sentant
/// respectivement le texte du bouton et le rectangle l'englobant. Un bouton poss�de diverses propri�t�s, dont:
///		- un pointeur vers une fonction qui est appel�e lorsque ce dernier est s�lectionn�
///		- le "focus", signifiant que le bouton a l'attention du curseur (concr�tement, la sourie est pos�e dessus)
///		- un �tat d'activit� indiquant si le bouton doit interagir avec l'interface ou non
/// 
/// Un bouton inactif est consid�r� comme interdit d'agir sur la fen�tre ou de recevoir des informations de celle-ci. Cela signifie
/// que les fonctions update(), draw() et doTask() n'effectueront rien dans cet �tat (les appeller ne g�n�rera aucune erreur, mais
/// rien ne sera fait). Il est toujours possible de changer certaines propri�t�s propres au bouton, comme par exemple sa position
/// ou la fonction � laquelle il est li�.
///
/// Les fonctions effectu�s par un bouton doivent �tre des m�thodes de la classe App de prototype:
/// \code void App::fonction(void);
/// \endcode
/// La classe KeyButton est un exemple de bouton dont les fonctions acceptent un param�tre.
class Button : public sf::Drawable
{
    // m�thodes
public:
    /// \brief Constructeur par d�faut. L'objet construit est incomplet et doit �tre termin� avant d'�tre utilis�.
    Button();
    
    /// \brief Construit un objet Button.
    /// \param text Un sf::Text repr�sentant le texte affich� par le bouton.
    /// \param x L'abscisse du bouton.
    /// \param y L'ordonn�e du bouton.
    /// \param task Un pointeur vers la t�che effectu� par le bouton.
    /// \param app Un pointeur vers l'application � laquelle le bouton appartient.
    /// \param focusColor La couleur du surlignage lorsque le bouton a le focus.
    Button(sf::Text text, float x, float y, void(App::*task)(void) = nullptr, App *app = nullptr, sf::Color focusColor = sf::Color::Blue);
    
    /// \brief Construit un objet Button.
    /// \param text Un sf::Text repr�sentant le texte affich� par le bouton.
    /// \param pos La position du bouton.
    /// \param task Un pointeur vers la t�che effectu� par le bouton.
    /// \param app Un pointeur vers l'application � laquelle le bouton appartient.
    /// \param focusColor La couleur du surlignage lorsque le bouton a le focus.
    Button(sf::Text text, sf::Vector2f pos, void(App::*task)(void) = nullptr, App *app = nullptr, sf::Color focusColor = sf::Color::Blue);

    /// \brief Destructeur.
    ~Button();

    /// \brief Met � jour l'�tat du bouton � partir de la position du curseur.
    ///
    /// Si le bouton est actif et que le curseur est sur son rectangle, il est marqu� comme poss�dant le focus.
    /// \param cursorPos La position du curseur.
    void update(sf::Vector2f cursorPos);

    /// \brief Dessine les �l�ments du bouton.
    ///
    /// S'il poss�de le focus, son rectangle prend une couleur d�cid�e par focusColor (modifi�e pour �tre un peu transparente). 
    /// Sinon, il est transparent et seul le texte est dessin�.
    void draw(sf::RenderTarget &target, sf::RenderStates states)const;

    /// \brief Modifie le texte affich� par le bouton.
    /// \param text Une std::string repr�sentant le nouveau texte.
    void setText(std::string text);

    /// \brief Modifie la police utilis�e par le bouton.
    /// \param font La police � utiliser.
    void setFont(sf::Font &font);

    /// \brief R�cup�re une r�f�rence vers l'objet sf::Text utilis� par le bouton.
    /// \return text R�f�rence vers le sf::Text du bouton.
    sf::Text& getText(void);

    /// \brief Modifie la couleur du texte affich� par le bouton.
    /// \param color La nouvelle couleur.
    void setTextColor(sf::Color color);

    /// Modifie la taille des caract�res.
    void setCharacterSize(unsigned int size);

    /// \brief Renvoie la couleur du texte du bouton.
    /// \return color Une copie de la couleur du texte du bouton.
    sf::Color getTextColor(void);

    /// \brief Modifie la propri�t� "focus" du bouton.
    /// \param focus Le nouvel �tat du bouton.
    void setFocus(bool focus);

    /// \brief Donne l'�tat du focus du bouton.
    /// \return True si le bouton a le focus et false sinon.
    bool hasFocus(void);

    /// \brief Modifie la couleur du rectangle s'affichant si le bouton a le focus.
    /// \param color La nouvelle couleur.
    void setFocusColor(sf::Color color);

    /// \brief Modifie la position du bouton.
    /// \param pos La nouvelle position.
    void setPosition(sf::Vector2f pos);

    /// \brief Retourne la position actuelle du bouton.
    sf::Vector2f getPosition(void);

    /// \brief R�cup�re la taille du retangle englobant le bouton.
    sf::Vector2f getSize(void);

    /// \brief D�finition de l'op�rateur d'�galit�
    ///
    /// Cet op�rateur est utilis� dans le fonctionnement interne de la classe.
    bool operator==(Button &right);
    
    /// \brief Modifie la fonction li�e au bouton
    /// \param task Un pointeur vers une fonction de prototype void fonction(void);.
    void bind(void(App::*task)(void));
    
    /// \brief Ex�cute la fonction � laquelle le bouton est li�.
    ///
    /// Si le bouton n'est li� � aucune fonction ou n'est pas actif, cette fonction ne fait rien.
    void doTask(void);

    /// \brief Modifie l'application � laquelle le bouton appartient.
    /// \param app Un pointeur vers une instance de la classe App.
    void setApp(App *app);

    /// \brief Retourne l'�tat d'activit� du bouton.
    /// \return True si le bouton est actif et false sinon.
    bool isActive(void);

    /// \brief Modifie l'�tat d'activit� du bouton.
    void setActive(bool active);

    /// \brief Indique si le bouton est actuellement dessin� sur l'�cran ou non (c'est-�-dire s'il est autoris� � l'�tre).
    bool onScreen(void);

    /// \brief Indique si le bouton doit �tre dessin� ou non.
    void setOnScreen(bool onScreen);

    // membres de la classe
    static std::vector<Button*> buttonList;	///> Un tableau de pointeurs vers tous les boutons cr��s.
private:							
    sf::Text m_text;						///> Le sf::Text utilis� par le bouton.		
    sf::RectangleShape m_shape;			    ///> Le sf::RectangleShape utilis� par le bouton.
    sf::Color m_focusColor;					///> La couleur prise par le rectangle du bouton si celui-ci a le focus.
    App *m_app;								///> Un pointeur vers l'application � laquelle appartient le bouton.
    bool m_hasFocus;						///> bool�en stockant l'�tat focus du bouton.
    bool m_active;							///> bool�en stokant l'�tat d'activit� du bouton.
    bool m_onScreen;						///> Bool�en indiquant que le bouton doit �tre dessin�.
    void(App::*m_task)(void);				///> Un pointeur vers la fonction � appeler lorsque le bouton est d�clench�.

};

#endif
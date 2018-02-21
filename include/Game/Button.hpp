#pragma once
#ifndef BUTTON_HPP
#define BUTTON_HPP
#include <string> // std::string
#include <vector> // std::vector
#include "SFML\Graphics.hpp"
#include "App.hpp"

/// \brief Classe représentant un élément d'interface auquel est lié une fonction.
///
/// Un bouton est défini pour sa partie visible par l'association d'un sf::Text et d'un sf::RectangularShape représentant
/// respectivement le texte du bouton et le rectangle l'englobant. Un bouton possède diverses propriétés, dont:
///		- un pointeur vers une fonction qui est appelée lorsque ce dernier est sélectionné
///		- le "focus", signifiant que le bouton a l'attention du curseur (concrètement, la sourie est posée dessus)
///		- un état d'activité indiquant si le bouton doit interagir avec l'interface ou non
/// 
/// Un bouton inactif est considéré comme interdit d'agir sur la fenêtre ou de recevoir des informations de celle-ci. Cela signifie
/// que les fonctions update(), draw() et doTask() n'effectueront rien dans cet état (les appeller ne génèrera aucune erreur, mais
/// rien ne sera fait). Il est toujours possible de changer certaines propriétés propres au bouton, comme par exemple sa position
/// ou la fonction à laquelle il est lié.
///
/// Les fonctions effectués par un bouton doivent être des méthodes de la classe App de prototype:
/// \code void App::fonction(void);
/// \endcode
/// La classe KeyButton est un exemple de bouton dont les fonctions acceptent un paramètre.
class Button : public sf::Drawable
{
    // méthodes
public:
    /// \brief Constructeur par défaut. L'objet construit est incomplet et doit être terminé avant d'être utilisé.
    Button();
    
    /// \brief Construit un objet Button.
    /// \param text Un sf::Text représentant le texte affiché par le bouton.
    /// \param x L'abscisse du bouton.
    /// \param y L'ordonnée du bouton.
    /// \param task Un pointeur vers la tâche effectué par le bouton.
    /// \param app Un pointeur vers l'application à laquelle le bouton appartient.
    /// \param focusColor La couleur du surlignage lorsque le bouton a le focus.
    Button(sf::Text text, float x, float y, void(App::*task)(void) = nullptr, App *app = nullptr, sf::Color focusColor = sf::Color::Blue);
    
    /// \brief Construit un objet Button.
    /// \param text Un sf::Text représentant le texte affiché par le bouton.
    /// \param pos La position du bouton.
    /// \param task Un pointeur vers la tâche effectué par le bouton.
    /// \param app Un pointeur vers l'application à laquelle le bouton appartient.
    /// \param focusColor La couleur du surlignage lorsque le bouton a le focus.
    Button(sf::Text text, sf::Vector2f pos, void(App::*task)(void) = nullptr, App *app = nullptr, sf::Color focusColor = sf::Color::Blue);

    /// \brief Destructeur.
    ~Button();

    /// \brief Met à jour l'état du bouton à partir de la position du curseur.
    ///
    /// Si le bouton est actif et que le curseur est sur son rectangle, il est marqué comme possédant le focus.
    /// \param cursorPos La position du curseur.
    void update(sf::Vector2f cursorPos);

    /// \brief Dessine les éléments du bouton.
    ///
    /// S'il possède le focus, son rectangle prend une couleur décidée par focusColor (modifiée pour être un peu transparente). 
    /// Sinon, il est transparent et seul le texte est dessiné.
    void draw(sf::RenderTarget &target, sf::RenderStates states)const;

    /// \brief Modifie le texte affiché par le bouton.
    /// \param text Une std::string représentant le nouveau texte.
    void setText(std::string text);

    /// \brief Modifie la police utilisée par le bouton.
    /// \param font La police à utiliser.
    void setFont(sf::Font &font);

    /// \brief Récupère une référence vers l'objet sf::Text utilisé par le bouton.
    /// \return text Référence vers le sf::Text du bouton.
    sf::Text& getText(void);

    /// \brief Modifie la couleur du texte affiché par le bouton.
    /// \param color La nouvelle couleur.
    void setTextColor(sf::Color color);

    /// Modifie la taille des caractères.
    void setCharacterSize(unsigned int size);

    /// \brief Renvoie la couleur du texte du bouton.
    /// \return color Une copie de la couleur du texte du bouton.
    sf::Color getTextColor(void);

    /// \brief Modifie la propriété "focus" du bouton.
    /// \param focus Le nouvel état du bouton.
    void setFocus(bool focus);

    /// \brief Donne l'état du focus du bouton.
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

    /// \brief Récupère la taille du retangle englobant le bouton.
    sf::Vector2f getSize(void);

    /// \brief Définition de l'opérateur d'égalité
    ///
    /// Cet opérateur est utilisé dans le fonctionnement interne de la classe.
    bool operator==(Button &right);
    
    /// \brief Modifie la fonction liée au bouton
    /// \param task Un pointeur vers une fonction de prototype void fonction(void);.
    void bind(void(App::*task)(void));
    
    /// \brief Exécute la fonction à laquelle le bouton est lié.
    ///
    /// Si le bouton n'est lié à aucune fonction ou n'est pas actif, cette fonction ne fait rien.
    void doTask(void);

    /// \brief Modifie l'application à laquelle le bouton appartient.
    /// \param app Un pointeur vers une instance de la classe App.
    void setApp(App *app);

    /// \brief Retourne l'état d'activité du bouton.
    /// \return True si le bouton est actif et false sinon.
    bool isActive(void);

    /// \brief Modifie l'état d'activité du bouton.
    void setActive(bool active);

    /// \brief Indique si le bouton est actuellement dessiné sur l'écran ou non (c'est-à-dire s'il est autorisé à l'être).
    bool onScreen(void);

    /// \brief Indique si le bouton doit être dessiné ou non.
    void setOnScreen(bool onScreen);

    // membres de la classe
    static std::vector<Button*> buttonList;	///> Un tableau de pointeurs vers tous les boutons créés.
private:							
    sf::Text m_text;						///> Le sf::Text utilisé par le bouton.		
    sf::RectangleShape m_shape;			    ///> Le sf::RectangleShape utilisé par le bouton.
    sf::Color m_focusColor;					///> La couleur prise par le rectangle du bouton si celui-ci a le focus.
    App *m_app;								///> Un pointeur vers l'application à laquelle appartient le bouton.
    bool m_hasFocus;						///> booléen stockant l'état focus du bouton.
    bool m_active;							///> booléen stokant l'état d'activité du bouton.
    bool m_onScreen;						///> Booléen indiquant que le bouton doit être dessiné.
    void(App::*m_task)(void);				///> Un pointeur vers la fonction à appeler lorsque le bouton est déclenché.

};

#endif
#include "Game/Keyboard.hpp"

VKeyboard::VKeyboard()
{}

VKeyboard::VKeyboard(sf::Font &font, unsigned int charSize, float x, float y, App *app) : m_done(false), m_active(true),
m_onScreen(true)
{
    m_string = "";
    sf::Vector2f size;
    for (int i = 0; i < 26; i++)
    {
        m_letters[i].setFont(font);
        m_letters[i].setText(sf::String(char(65 + i)));
        m_letters[i].setCharacterSize(charSize);
        m_letters[i].setTextColor(sf::Color::White);
        m_letters[i].setFocusColor(sf::Color::Blue);
        size.x = m_letters[i].getText().getGlobalBounds().width;
        size.y = m_letters[i].getText().getGlobalBounds().height;
        
        //on affiche 10 boutons par ligne, on a donc un tableau de 10x3 (ou presque, il y a 29 boutons)
        int j = i / 10; //ligne sur laquelle placer ce bouton
        int k = i % 10; //colonne
        m_letters[i].setPosition(sf::Vector2f(x+2*(size.x+1)*k,y+2*(size.y+1)*j));
        m_letters[i].bind(&VKeyboard::addCharacter);
        m_letters[i].setKeyboard(this);
        m_letters[i].setApp(app);
    }
    // bouton back
    m_back.setFont(font);
    m_back.setText("Back");
    m_back.setCharacterSize(charSize);
    m_back.setTextColor(sf::Color::White);
    m_back.setFocusColor(sf::Color::Blue);
    m_back.setPosition(sf::Vector2f(x+2*(size.x+1)*6,y+2*(size.y+1)*2));
    m_back.bind(&VKeyboard::eraseCharacter);
    m_back.setKeyboard(this);
    m_back.setApp(app);
    // bouton entrée
    m_enter.setFont(font);
    m_enter.setText("Enter");
    m_enter.setCharacterSize(charSize);
    m_enter.setTextColor(sf::Color::White);
    m_enter.setFocusColor(sf::Color::Blue);
    sf::Vector2f backPos = m_back.getPosition();
    m_enter.setPosition(sf::Vector2f(backPos.x+m_back.getSize().x+1,backPos.y));
    m_enter.bind(&VKeyboard::setDone);
    m_enter.setKeyboard(this);
    m_enter.setApp(app);
    // rectangle englobant du clavier
    m_rectangle.setFillColor(sf::Color(32,32,32,225));
    m_rectangle.setPosition(x, y);
    sf::Vector2f rectSize;
    rectSize.x = m_enter.getPosition().x + m_enter.getSize().x - x;
    //rectSize.x = 2*(size.x+1)*11;
    rectSize.y = size.y * 5;
    m_rectangle.setSize(rectSize);
}

sf::Vector2f VKeyboard::getSize(void)
{
    return m_rectangle.getSize();
}

void VKeyboard::setPosition(int x, int y)
{
    m_rectangle.setPosition(x, y);
    sf::Vector2f size;
    for (int i = 0; i < 26; i++)
    {
        int j = i / 10;
        int k = i % 10;
        size.x = 20;
        size.y = 15;
        m_letters[i].setPosition(sf::Vector2f(x+2*(size.x+1)*k,y+2*(size.y+1)*j));
    }
    m_back.setPosition(sf::Vector2f(x + 2 * (size.x + 1) * 6, y + 2 * (size.y + 1) * 2));
    sf::Vector2f backPos = m_back.getPosition();
    m_enter.setPosition(sf::Vector2f(backPos.x + m_back.getSize().x + 5, backPos.y));
    return;
}

sf::String VKeyboard::getString(void)
{
    sf::String tmp(m_string);
    // on efface la chaine si elle est complète
    if (m_done)
    {
        m_string.clear();
        m_done = false;	
    }
    
    return tmp;
}

void VKeyboard::eraseCharacter(int &n)
{
    if (!m_string.isEmpty())
    {
        m_string.erase(m_string.getSize() - n, n);
    }
    return;
}

void VKeyboard::addCharacter(char &c)
{
    int tmp = m_string.getSize();
    m_string.insert(tmp, c);
    return;
}

void VKeyboard::setDone(bool &done)
{
    m_active = false;
    m_done = done;
    return;
}

void VKeyboard::draw(sf::RenderTarget &target, sf::RenderStates states)const
{
    // seulement si actif
    if (m_onScreen)
    {
        target.draw(m_rectangle);
        for (int i = 0; i < 26; i++)
        {
            target.draw(m_letters[i]);
        }
        target.draw(m_enter);
        target.draw(m_back);
    }
    return;
}

bool VKeyboard::stringReady(void)
{
    return m_done;
}

void VKeyboard::update(sf::Vector2f cursorPos)
{
    // seulement si le clavier est marqué comme actif
    if (m_active)
    {
        for (int i = 0; i < 26; i++)
        {
            m_letters[i].update(cursorPos);
        }
        m_enter.update(cursorPos);
        m_back.update(cursorPos);
    }
    return;
}

void VKeyboard::checkForTask(void)
{
    // seulement si le clavier est marqué comme actif
    if (m_active)
    {
        for (int i = 0; i < 26; i++)
        {
            if (m_letters[i].hasFocus())
            {
                char c = 65 + i; // part de A (65) et décale de 1 par indice (donc i=0:A, i=1:B, i=2:C,...)
                m_letters[i].doTask(c);
            }
        }
        if (m_enter.hasFocus())
        {
            bool tmp = (m_done) ? false : true;
            m_enter.doTask(tmp);
        }
        if (m_back.hasFocus())
        {
            int n(1);
            m_back.doTask(n);
        }
    }
    return;
}

void VKeyboard::setActive(bool active)
{
    m_active = active;
    // on met aussi à jour l'état des boutons du claviers
    for (int i = 0; i < 26; i++)
    {
        m_letters[i].setActive(active);
    }
    m_enter.setActive(active);
    m_back.setActive(active);
    return;
}

bool VKeyboard::isActive(void)
{
    return m_active;
}

bool VKeyboard::onScreen(void)
{
    return m_onScreen;
}

void VKeyboard::setOnScreen(bool onScreen)
{
    m_onScreen = onScreen;
    for (int i = 0; i < 26; i++)
    {
        m_letters[i].setOnScreen(onScreen);
    }
    m_back.setOnScreen(onScreen);
    m_enter.setOnScreen(onScreen);
}
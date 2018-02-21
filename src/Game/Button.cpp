#include "Game/Button.hpp"

// Un bouton créé par défaut n'est pas complet et ne peut pas être utilisé sans travail supplémentaire. Ce constructeur par défaut
// est prévu pour initialiser les boutons membres d'une classe (qui sont forcément initialisés par défaut).
Button::Button() : m_focusColor(sf::Color::Blue), m_hasFocus(false), m_task(nullptr), m_app(nullptr), m_active(true),
m_onScreen(true)
{
    m_text = sf::Text();
    m_text.setPosition(sf::Vector2f(0, 0));
    m_text.setFillColor(sf::Color::White);
    sf::Vector2f size;
    size.x = m_text.getGlobalBounds().width;
    size.y = m_text.getGlobalBounds().height;
    m_shape.setSize(size);
    // la position d'un sf::Text est décalée par rapport à l'endroit où ses caractères sont dessinés. D'après le forum SFML:
    /* "The text is not aligned on top, it is aligned on its baseline (like everywhere else, for obvious reasons). So unless your
    text contains the highest character of the font (most likely accentuated capitals), it will always be slightly below the top.
    The offset should be text.getLocalBounds().top." */
    m_shape.setPosition(0, 0 + m_text.getLocalBounds().top);
    m_shape.setFillColor(sf::Color::Transparent);
    m_shape.setOutlineColor(sf::Color::Transparent);
    buttonList.push_back(this);
}


Button::Button(sf::Text text, float x, float y, void(App::*task)(void), App *app, sf::Color focusColor) :
    m_focusColor(focusColor), m_hasFocus(false), m_task(task), m_app(app), m_active(true), m_onScreen(true)
{
    m_text = text;
    m_text.setPosition(sf::Vector2f(x, y));
    m_text.setFillColor(sf::Color::White);
    sf::Vector2f size;
    size.x = m_text.getGlobalBounds().width;
    size.y = m_text.getGlobalBounds().height;
    m_shape.setSize(size);
    // la position d'un sf::Text est décalée par rapport à l'endroit où ses caractères sont dessinés. D'après le forum SFML:
    /* "The text is not aligned on top, it is aligned on its baseline (like everywhere else, for obvious reasons). So unless your
    text contains the highest character of the font (most likely accentuated capitals), it will always be slightly below the top.
    The offset should be text.getLocalBounds().top." */
    m_shape.setPosition(x, y + m_text.getLocalBounds().top);
    m_shape.setFillColor(sf::Color::Transparent);
    m_shape.setOutlineColor(sf::Color::Transparent);
    buttonList.push_back(this);
}

Button::Button(sf::Text text, sf::Vector2f pos, void(App::*task)(void), App *app, sf::Color focusColor) :
    m_focusColor(focusColor), m_hasFocus(false), m_task(task), m_app(app), m_active(true), m_onScreen(true)
{
    m_text = text;
    m_text.setPosition(pos);
    m_text.setFillColor(sf::Color::White);
    sf::Vector2f size;
    size.x = m_text.getGlobalBounds().width;
    size.y = m_text.getGlobalBounds().height;
    m_shape.setSize(size);
    m_shape.setPosition(pos.x, pos.y + m_text.getLocalBounds().top);
    m_shape.setFillColor(sf::Color::Transparent);
    m_shape.setOutlineColor(sf::Color::Transparent);
    buttonList.push_back(this);
}

Button::~Button()
{
    auto it = buttonList.begin();
    for (auto i = buttonList.begin(); i < buttonList.end(); i++)
    {
        if (*i == this)
        {
            it = i;
        }
    }
    buttonList.erase(it);
}

void Button::update(sf::Vector2f cursorPos)
{
    // seulement s'il est actif
    if (m_active)
    {
        sf::FloatRect pos = m_text.getGlobalBounds();
        if (pos.contains(cursorPos))
        {
            m_hasFocus = true;
            sf::Color tmp = m_focusColor * sf::Color(255, 255, 255, 64);
            m_shape.setFillColor(tmp);
        }
        else
        {
            m_hasFocus = false;
            m_shape.setFillColor(sf::Color::Transparent);
        }
    }
    return;
}

void Button::draw(sf::RenderTarget &target, sf::RenderStates states)const
{
    if (m_onScreen)
    {
        target.draw(m_text, states);
        target.draw(m_shape, states);
    }
    return;
}

void Button::setText(std::string text)
{
    m_text.setString(text);
    // recalculer la taille du rectangle englobant
    sf::Vector2f size;
    std::string tmp = m_text.getString();
    size.x = m_text.getGlobalBounds().width;
    size.y = m_text.getGlobalBounds().height;
    m_shape.setSize(size);
    return;
}

void Button::setFont(sf::Font &font)
{
    m_text.setFont(font);
    // on recalcule la taille du rectangle englobant (vu que la police a changée)
    sf::Vector2f size;
    std::string tmp = m_text.getString();
    size.x = m_text.getGlobalBounds().width;
    size.y = m_text.getGlobalBounds().height;
    m_shape.setSize(size);
    return;
}

sf::Text& Button::getText(void)
{
    return m_text;
}

void Button::setTextColor(sf::Color color)
{
    m_text.setFillColor(color);
    return;
}

sf::Color Button::getTextColor(void)
{
    return m_text.getFillColor();
}

void Button::setCharacterSize(unsigned int size)
{
    m_text.setCharacterSize(size);
    return;
}


void Button::setFocus(bool focus)
{
    m_hasFocus = focus;
    return;
}

bool Button::hasFocus(void)
{
    return m_hasFocus;
}

void Button::setFocusColor(sf::Color color)
{
    m_focusColor = color;
    return;
}

void Button::setPosition(sf::Vector2f pos)
{
    m_text.setPosition(pos);
    m_shape.setPosition(pos.x, pos.y + m_text.getLocalBounds().top);
    return;
}

sf::Vector2f Button::getPosition(void)
{
    return m_text.getPosition();
}

sf::Vector2f Button::getSize(void)
{
    return m_shape.getSize();
}

bool Button::operator==(Button &right)
{
    bool equal(false);
    equal |= m_text.getString() == right.m_text.getString();
    equal &= m_focusColor == right.m_focusColor;
    equal &= m_hasFocus == right.m_hasFocus;
    equal &= m_shape.getPosition() == right.m_shape.getPosition();
    equal &= m_shape.getSize() == right.m_shape.getSize();
    equal &= m_shape.getFillColor() == right.m_shape.getFillColor();
    return equal;
}

void Button::bind(void(App::*task)(void))
{
    m_task = task;
    return;
}

void Button::doTask(void)
{
    if (m_active && m_task != nullptr)
    {
        (m_app->*m_task)();
    }
    return;
}
void Button::setApp(App *app)
{
    m_app = app;
}

bool Button::isActive(void)
{
    return m_active;
}

void Button::setActive(bool active)
{
    m_active = active;
    return;
}

bool Button::onScreen(void)
{
    return m_onScreen;
}

void Button::setOnScreen(bool onScreen)
{
    m_onScreen = onScreen;
    return;
}
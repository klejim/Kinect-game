
#include "Game/App.hpp"
#include "Game/Button.hpp"

/* � faire : 
    - nettoyer
*/
std::vector<Button*> Button::buttonList = {};
// entr�e du programme : on ne fait rien d'autre qu'appeler App::run (qui ouvre le menu principal)
int main(void)
{
    App app;
    app.run();
    return 0;
}
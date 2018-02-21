#include "SFML\Graphics.hpp"

#include "Game/App.hpp"
#include "Game/Button.hpp"
#include "Game/Keyboard.hpp"
#include "KinectInterface\Kinect.hpp"

#include <fstream> // ifstream et ofstream
#include <iostream> // std::cout
#include <map> // std::map
#include <string> // std::string

// fonctions utilisées pour lire et enregistrer les scores générés à la fin de la partie
void saveScores(std::map<unsigned int, std::string> &scores);
std::map<unsigned int, std::string> loadScores(void);

// Définition des différents écrans:
//		- menu principal (App::run)
//		- lancement du jeu et enregistrement du score (App::startGame)
//		- démonstration (App::optionMenu)
//		- sortie de l'application (App::quit)
//		- le jeu (App::game) est dans le fichier game.cpp 
void App::run(void)
{
    // création de la fenêtre
    window.create(sf::VideoMode::getDesktopMode(), "Game");
    window.setMouseCursorVisible(false);
    // chargement des sprites représentant le curseur
    sf::Texture handOpen;
    sf::Texture handClosed;
    handOpen.loadFromFile("resources/cursor-open.png");
    handClosed.loadFromFile("resources/cursor-closed.png");
    cursor.setTextures(handOpen, handClosed);
    // des couleurs utiles
    sf::Color background(128, 128, 128, 255);
    sf::Color focusColor(0, 0, 255, 255); 
    // création des boutons
    sf::Font font;
    font.loadFromFile("resources/F25_Executive.otf");
    sf::Text text1("Commencer la partie", font);
    sf::Text text2("Démonstration", font);
    sf::Text text3("Quitter", font);
    
    std::vector<Button*> buttons; // un tableau contenant des pointeurs vers les boutons créés
    sf::Vector2f winsize = window.getDefaultView().getSize();
    Button startPlaying(text1, winsize.x / 2 - text1.getGlobalBounds().width / 2, winsize.y / 3, &App::startGame, this, focusColor);
    buttons.push_back(&startPlaying); // ajout au vector
    Button options(text2, winsize.x / 2 - text2.getGlobalBounds().width / 2, winsize.y / 3 + 50, (&App::optionMenu), this, focusColor);
    buttons.push_back(&options);
    Button quit(text3, winsize.x / 2 - text3.getGlobalBounds().width / 2, winsize.y / 3 + 100, &App::quit, this, focusColor);
    buttons.push_back(&quit);

    // initialisation de l'objet KinectInterface
    KInterface.initialize(KtI::INITIALIZE_DEPTH | KtI::INITIALIZE_SKELETON);
    tracker::SoloTracker tracker;
    KInterface.setTrackingMethod(&tracker);
    KInterface.setTargetImageSize(winsize.x, winsize.y);
    KInterface.start();
    KInterface.setOrientation(5);
    
    while (window.isOpen())
    {
        // boucle d'évènement Kinect
        KtI::KinectEvent kEvent;
        while (KInterface.pollEvent(kEvent))
        {
            if (kEvent.type == KtI::KinectEvent::HandCursorMoved)
            {
                // mise à jour de l'état des boutons (focus ou pas) indépendamment des évènements pour que cela soit fait
                // à chaque intération
                sf::Vector2f move;
                move.x = kEvent.hand.position.x;
                move.y = kEvent.hand.position.y;
                cursor.setPosition(move);
                //sf::Mouse::setPosition(window.mapCoordsToPixel(move), window);
                for (int i = 0; i < buttons.size(); i++)
                {
                    if (buttons[i]->isActive())
                    {
                        buttons[i]->update(move);
                    }
                }
            }
            else if (kEvent.type == KtI::KinectEvent::HandGrip)
            {
                // on vérifie si ce click a eu lieu alors qu'un bouton avait le focus
                // si c'est le cas, la fonction de ce bouton est appelée.
                cursor.handClosed = true;
                for (int i = 0; i < buttons.size(); i++)
                {
                    if (buttons[i]->isActive() && buttons[i]->hasFocus())
                    {
                        buttons[i]->doTask();
                        //window.setView(window.getDefaultView()); // seulement utile en sortant de App::game, pour restaurer la vue d'origine
                    }
                }
            }
            else if (kEvent.type == KtI::KinectEvent::HandRelease)
            {
                cursor.handClosed = false;
            }
            else if (kEvent.type == KtI::KinectEvent::PositionChanged)
            {
                if (kEvent.position.type == KtI::PositionType::STOP)
                    window.close();
            }
        }
        // boucle d'évènement SFML
        // on n'y fait rien, mais elle est quand même nécessaire au  fonctionnement interne de la SFML
        sf::Event event;
        while (window.pollEvent(event));

        // il reste ensuite à dessiner tous ces objets.
        window.clear(background);
        // les boutons sont dessinés s'ils sont actifs
        for (int i = 0; i<buttons.size(); i++)
        {
            if (buttons[i]->onScreen())
            {
                window.draw(*buttons[i]);
            }
        }
        window.draw(cursor);
        window.display();
    }
    // avant de quitter, on doit signifier au thread traitant les trames qu'il doit cesser
    KInterface.stop();
    return;
}

void App::optionMenu(void)
{
    /// \brief Enumération représentant les différentes étapes de l'initialisation de la Kinect (dans l'ordre chronologique).
    enum InitStep { Detection, WidthTest, HeightTest, GripTest, ReleaseTest, PushTest, Done };
    sf::Vector2f winsize = window.getDefaultView().getSize();
    sf::Color background(55, 94, 80, 255);
    // bouton de retour au menu
    sf::Font font;
    font.loadFromFile("F25_Executive.otf");
    sf::Text text1("Retour", font,35);
    Button backButton(text1, winsize.x/5*4+(winsize.x/5-text1.getLocalBounds().width)/2, winsize.y/10*9);
    backButton.setActive(false); // désactivé jusqu'à la fin de l'initialisation
    // textes d'instructions
    sf::String hint("En attente de détection...");
    sf::Text instruction(hint, font);
    instruction.setPosition(winsize.x/30, winsize.y/10*9);
    sf::Text steps("Etapes:\n\n1-Détection\n\n2-Positions\n\n3-Attraper\n\n4-Relâcher\n\n5-Pousser", font, 25);
    steps.setPosition(winsize.x/5*4+(winsize.x/5-steps.getLocalBounds().width)/2, winsize.y/10);
    sf::Text intro("Suivez les instructions en bas de l'écran pour progresser.", font, 20);
    intro.setPosition(winsize.x / 30, winsize.y/20);
    // image 
    sf::RectangleShape skelBackground;
    skelBackground.setFillColor(sf::Color::Black);
    skelBackground.setSize(sf::Vector2f(winsize.x/5*4,winsize.y/10*8));
    skelBackground.setPosition(0, winsize.y / 10);
    sf::Vector2f skelSize = skelBackground.getSize();
    sf::CircleShape articulations[20];
    for (int i = 0 ; i < 20; i++)
    {
        articulations[i].setFillColor(sf::Color::Blue);
        articulations[i].setRadius(8);
    }

    bool quit(false);
    
    InitStep initState(Detection);

    KtI::KinectEvent kEvent;
    
    KInterface.setTargetImageSize(winsize.x, winsize.y);
    while (window.isOpen() && !quit)
    {
        while (KInterface.pollEvent(kEvent))
        {
            if ( initState == Detection && kEvent.type == KtI::KinectEvent::CannotSeeUser)
            {
                if (kEvent.quality.state == KtI::NOT_CLIPPED)
                {
                    initState = WidthTest;
                    instruction.setString("Ecartez les bras.");
                }
            }
            else if (kEvent.type == KtI::KinectEvent::SkeletonMoved)
            {
                for (int i = 0; i < 20; i++)
                {
                    // il faut adapter les coordonnées au morceau de fenêtre dans lequel on les affiche
                    sf::Vector2f pos;
                    pos.x = kEvent.skeleton.positions[i].x * skelSize.x/winsize.x;
                    pos.y = kEvent.skeleton.positions[i].y * skelSize.y / winsize.y + 2*winsize.y / 10 ;
                    
                    articulations[i].setPosition(pos);
                }
            }
            // test de place (largeur)
            else if (kEvent.type == KtI::KinectEvent::PositionChanged)
            {
                if (initState == WidthTest && kEvent.position.type == KtI::PositionType::T)
                {
                    initState = HeightTest;
                    instruction.setString("Levez les bras.");
                }
                // 
                else if (initState == HeightTest && kEvent.position.type == KtI::PositionType::Y)
                {
                    initState = GripTest;
                    instruction.setString("Fermez la main.");
                }
                //
                else if (kEvent.position.type == KtI::PositionType::STOP)
                {
                    window.close();
                }
            }
            // fermer la main
            else if (initState == GripTest && kEvent.type == KtI::KinectEvent::HandGrip)
            {
                initState = ReleaseTest;
                instruction.setString("Rouvrez la main.");
            }
            // l'ouvrir
            else if (initState == ReleaseTest && kEvent.type == KtI::KinectEvent::HandRelease)
            {
                initState = PushTest;
                instruction.setString("Avancez la main.");
            }
            //
            else if (initState == PushTest && kEvent.type == KtI::KinectEvent::HandPush)
            {
                initState = Done;
                backButton.setActive(true);
                instruction.setString("Terminé! Appuyez sur Retour.");
            }
            else if (kEvent.type == KtI::KinectEvent::HandCursorMoved)
            {
                // mise à jour du bouton à partir de la position du curseur
                sf::Vector2f move;
                move.x = kEvent.hand.position.x;
                move.y = kEvent.hand.position.y;
                cursor.setPosition(move);
                //sf::Mouse::setPosition(window.mapCoordsToPixel(move), window);
                backButton.update(move);
            }
            else if (kEvent.type == KtI::KinectEvent::HandGrip)
            {
                cursor.handClosed = true;
                if (backButton.isActive() && backButton.hasFocus())
                {
                    quit = true;
                }
            }
            else if (kEvent.type == KtI::KinectEvent::HandRelease)
            {
                cursor.handClosed = false;
            }

        }
        sf::Event event;
        while (window.pollEvent(event));
        
        // dessin des éléments
        window.clear(background);
        window.draw(skelBackground);
        for (int i = 0; i < 20; i++)
        {
            window.draw(articulations[i]);
        }
        if (backButton.isActive())
        {
            window.draw(backButton);
        }
        window.draw(instruction);
        window.draw(steps);
        window.draw(intro);
        window.draw(cursor);
        window.display();
    }
    return;
}

void App::startGame(void)
{
    std::map<unsigned int, std::string> scores = loadScores();
    unsigned int score = game();
    window.setView(window.getDefaultView());// nécessaire en sortant du jeu
    // si le score est parmi les cinq meilleurs, on enregistre le joueur
    bool saveScore(false);
    for (auto it = scores.begin(); it != scores.end(); it++)
    {
        saveScore |= (score > it->first); // saveScore est vrai si que le score est supérieur à au moins un des scores du fichier
    }
    if (saveScore)
    {
        sf::Vector2u winsize = window.getSize();
        //récupérer le nom du joueur puis le passer à game
        sf::Texture texture;
        texture.create(winsize.x, winsize.y);
        texture.update(window);
        sf::Sprite background(texture);
        // création du clavier
        sf::Font font;
        font.loadFromFile("F25_Executive.otf");
        sf::Text result; // un texte affichant le résultat sur la fenêtre
        result.setString("result");
        result.setFont(font);
        sf::FloatRect rect = result.getGlobalBounds();
        result.setPosition(winsize.x/2-rect.width/2-rect.width/2, winsize.y/4-rect.height/2);
        result.setFillColor(sf::Color::Black);
        VKeyboard keyboard(font, 30, winsize.x / 3, winsize.y / 4 * 3, this);
        sf::Vector2f ksize = keyboard.getSize();
        keyboard.setPosition(winsize.x / 2 - ksize.x / 2, winsize.y / 2 - ksize.y / 2);
        // this est le pointeur vers l'application (le clavier en a besoin pour appeller les fonctions liées à ses boutons).
        bool done(false);
        while (window.isOpen() && !done)
        {
            KtI::KinectEvent kEvent;
            while (KInterface.pollEvent(kEvent))
            {
                if (kEvent.type == KtI::KinectEvent::HandCursorMoved)
                {
                    // mise à jour de l'état des boutons (focus ou pas) du clavier
                    sf::Vector2f move;
                    move.x = kEvent.hand.position.x;
                    move.y = kEvent.hand.position.y;
                    cursor.setPosition(move);
                    //sf::Mouse::setPosition(window.mapCoordsToPixel(move), window);
                    if (keyboard.isActive())
                    {
                        keyboard.update(move);
                    }
                }
                else if (kEvent.type == KtI::KinectEvent::HandGrip)
                {
                    cursor.handClosed = true;
                    // on vérifie si le click a eu lieu au-dessus d'un bouton du clavier (il vérifie lui-même si ses boutons doivent agir).
                    if (keyboard.isActive())
                    {
                        keyboard.checkForTask();
                    }
                }
                else if (kEvent.type == KtI::KinectEvent::HandRelease)
                {
                    cursor.handClosed = false;
                }
                else if (kEvent.type == KtI::KinectEvent::PositionChanged)
                {
                    if (kEvent.position.type == KtI::PositionType::STOP)
                        window.close();
                }
            }
            // boucle d'évènement SFML
            sf::Event event;
            while (window.pollEvent(event));

            // Si l'utilisateur a fini d'entrer une chaîne (appui sur entrée), on désactive le clavier
            if (keyboard.stringReady())
            {
                // on récupère la chaîne
                result.setString(keyboard.getString());
                scores[score] = result.getString();
                // on supprime ensuite le score le plus faible, qui correspond au premier couple (score, nom) (ordonné par score croissant)
                if (scores.size() > 5)
                {
                    scores.erase(scores.begin());
                }
                saveScores(scores);
                done = true;
            }
            else // on récupère quand même la chaîne pour l'afficher à chaque changement
            {
                result.setString(keyboard.getString());
            }
            // dessin des objets
			result.setString("CLEMENT");
            window.clear();
            window.draw(background);
            if (keyboard.onScreen())
            {
                window.draw(keyboard);
            }
            window.draw(result);
            window.draw(cursor);
            window.display();
        }
    }
    return;
}

void App::quit(void)
{
    window.close();
    return;
}

std::map<unsigned int, std::string> loadScores(void)
{
    // format : #[rang] nom ([score])
    // ex : #1 clément (120)
    std::ifstream file;
    file.open("scores.txt");
    bool done(false);
    std::map<unsigned int, std::string> scores;
    while (!done)
    {
        char c = file.get();
        if (c == '#')
        {
            char tmp[255];
            file.getline(tmp, 255, '\n');
            std::string line(tmp);
            line = line.substr(2);
            // on récupère le nom
            int endName = line.find(' ');
            std::string name = line.substr(0, endName);
            line = line.substr(endName + 1);
            // puis le score
            int pos1 = line.find('('), pos2 = line.find(')');
            int score = strtol(line.substr(pos1 + 1, pos2 - 1).c_str(), nullptr, 10);
            // si le nom est valide (pas une ligne de ---), on l'ajoute
            if (name.compare("----"))
            {
                scores[score] = name;
            }
        }
        if (c == -1)
        {
            done = true;
        }
    }
    return scores;
}

void saveScores(std::map<unsigned int, std::string> &scores)
{
    std::ofstream file;
    int rank(1);
    file.open("scores.txt");
    // scores étant triée par score croissant, on le parcourt à l'envers pour écrire les scores du meilleur au moins bon
    for (auto it = scores.rbegin(); it != scores.rend(); it++)
    {
        unsigned int score = it->first;
        std::string name = it->second;
        //rang
        char tmp[3] = "  ";
        _ltoa_s(rank++,tmp,10);
        std::string line("#");
        line.append(tmp);
        //nom
        line.append(" " + name + " ");
        //score
        line.append("(" + std::to_string(score) + ")\n");
        file.write(line.c_str(), line.size());
    }
    // s'il y avait moins de cinq scores, on remplit ainsi : #[rang] ---- (--)
    for (int i=5-scores.size();i > 0;i--)// s'il y a déjà 5 scores, la condition d'entrée dans la boucle n'est pas validée
    {
        //rang
        char tmp[3] = "  ";
        _ltoa_s(rank++, tmp, 10);
        std::string line("#");
        line.append(tmp);
        //nom
        line.append(" " + std::string("----") + " ");
        //score
        line.append("(" + std::string("--") + ")\n");
        file.write(line.c_str(), line.size());
    }
    return;
}



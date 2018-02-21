#pragma once
#ifndef KINECTINTERFACE_HPP
#define KINECTINTERFACE_HPP
namespace KtI {
    namespace NUI {
#include <Windows.h>
#include "NuiApi.h"
#include "KinectInteraction.h"
    }
}
#include "Mutex.hpp"

#include <atomic> // atomic_long, atomic_bool, atomic<unsigned int>
#include <thread> // std::thread
#include <vector> // std::vector
namespace KtI
{
    using namespace NUI;
/// \brief Classe de base utilisée pour déterminer quels squelettes traquer lorsque en mode manuel.
/// 
/// Cette classe n'a qu'une unique méthode : choseTrackedSkeletons(), qui prend en argument une trame de données sur les squelettes
/// et doit renvoyer un tableau de deux identifiants qui seront ceux que la kinect traquera. La classe étant purement virtuelle, il
/// est impératif d'implémenter une classe en héritant pour pouvoir utiliser la sélection manuelle d'utilisateurs à traquer.
class SkeletonTracker
{
public:
    /// \brief Méthode servant à ajuster les paramètres de la trame d'interactions.
    ///
    /// \warning Cette fonction renvoyant un pointeur vers un tableau local, il est nécessaire de créer celui-ci avec une allocation
    /// dynamique. L'interface s'attend à ce que ce soit le cas et génèrera une erreur si le pointeur est nul en sortie de la fonction.
    /// \param skeletonFrame Référence à la trame d'interaction courante qui est analysée pour la sélection.
    /// \return ids Un pointeur vers le premier élément d'un tableau de deux identifiants qui sont ceux des utilisateurs à traquer.
    virtual DWORD* choseTrackedSkeletons(NUI_SKELETON_FRAME &skeletonFrame)=0;
};

/// \brief Classe utilisée par l'API INuiInteractionStream pour ajuster les données d'une trame d'interactions à travers sa méthode
/// GetInteractionInfoAtLocation().
class InteractionClient : public INuiInteractionClient
{
public:
    //to compiler : stop yelling please
    STDMETHODIMP_(ULONG) AddRef() { return 1; }
    STDMETHODIMP_(ULONG) Release() { return 0; }
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) {
        return S_OK;
    }
    // utile
    /// \brief Fonction servant à ajuster les données de la trame d'interactions.
    /// 
    /// La comportement voulu est le suivant : toutes les mains détectées seront susceptibles d'attraper ou de relacher. Les autres
    /// paramètres sont recommandés à plusieurs reprises sur les forums de microsoft.
    COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetInteractionInfoAtLocation(DWORD skeletonTrackingId, NUI_HAND_TYPE handType,
        FLOAT x, FLOAT y, NUI_INTERACTION_INFO *pInteractionInfo)
    {
        pInteractionInfo->IsGripTarget = true;
        pInteractionInfo->IsPressTarget = true;
        pInteractionInfo->PressAttractionPointX = 0.5;
        pInteractionInfo->PressAttractionPointY = 0.5;
        pInteractionInfo->PressTargetControlId = 0;
        return S_OK;
    }
};


class KinectEvent;

typedef enum class Trinary{ SET, RESET, MEM} trinary;

/// \brief Redéfinition des constantes décrivant la qualité d'un squelette
enum SkeletonQuality
{
    NOT_CLIPPED = 0,
    CLIPPED_RIGHT = NUI_SKELETON_QUALITY_CLIPPED_RIGHT,	
    CLIPPED_LEFT = NUI_SKELETON_QUALITY_CLIPPED_LEFT,
    CLIPPED_TOP = NUI_SKELETON_QUALITY_CLIPPED_TOP,
    CLIPPED_BOTTOM = NUI_SKELETON_QUALITY_CLIPPED_BOTTOM			 
};

/// \brief Redéfinition de l'énumération NUI_SKELETON_POSITION_TRACKING_STATE.
enum JointTrackingState
{
    NOT_TRACKED = NUI_SKELETON_POSITION_NOT_TRACKED,
    POSITION_INFERRED = NUI_SKELETON_POSITION_INFERRED,
    POSITION_TRACKED = NUI_SKELETON_POSITION_TRACKED
};

/// \brief Enumération redéfinissant l'énumération NUI_POSITION_INDEX par commodité d'accès.
enum SkeletonJoints
{
    SKELETON_HIP_CENTER = 0,
    SKELETON_SPINE,
    SKELETON_SHOULDER_CENTER,
    SKELETON_HEAD,
    SKELETON_SHOULDER_LEFT,
    SKELETON_ELBOW_LEFT,
    SKELETON_WRIST_LEFT,
    SKELETON_HAND_LEFT,
    SKELETON_SHOULDER_RIGHT,
    SKELETON_ELBOW_RIGHT,
    SKELETON_WRIST_RIGHT,
    SKELETON_HAND_RIGHT,
    SKELETON_HIP_LEFT,
    SKELETON_KNEE_LEFT,
    SKELETON_ANKLE_LEFT,
    SKELETON_FOOT_LEFT,
    SKELETON_HIP_RIGHT,
    SKELETON_KNEE_RIGHT,
    SKELETON_ANKLE_RIGHT,
    SKELETON_FOOT_RIGHT,
    SKELETON_JOINTS_COUNT
};


/// \brief Correspond aux positions reconnues par l'interface.
enum class PositionType
{
    UNKNOWN, ///< Non reconnue.
    T,		 ///< Les bras à l'horizontale et le corps droit.
    Y,		 ///< Les bras et les jambes à la verticale (bras vers le bas).
    STOP,	 ///< Le corps droit et les bras croisés à 45°.
};

/// \brief Des flags utilisés pour initialiser la Kinect (voir <a href="https://msdn.microsoft.com/en-us/library/hh855368.aspx"/> NUI Constants </a>).
/// 
///Par commodité, les flags INITIALIZE_AUDIO, INITIALIZE_SKELETON, INITIALIZE_DEPTH et INITIALIZE_COLOR sont passés tels quels à NuiInitialize, ils ont donc la valeur définie 
/// par NuiApi.h. La valeur des autres flags est analysée pour appeler les fonctions concernées avec les paramètres correspondant.
enum KInitializeFlags
{
    // NUI_INITIALIZE_FLAGS
    // les flags NUI_INITIALIZE_ sont envoyés tels quels à la fonction NuiInitialize
    INITIALIZE_COLOR = 0x00000002,				///< Utilise la détection d'images couleur.
    INITIALIZE_SKELETON = 0x00000008,			///< Utilise la détection de squelettes.
    INITIALIZE_DEPTH = 0x00000020,				///< Utilise la détection détection de frame de profondeurs.
    INITIALIZE_AUDIO = 0x10000000,				///< Détecte l'émission de son.
    INITIALIZE_ALL = 0x1000002A,				///< Utilise tous les flux possibles.
    // NUI_SKELETON_TRACKING_FLAGS
    // ceux-là sont "traduits" avant d'être passés à leurs fonctions (leurs valeurs poseraient des conflits sinon)
    SKELETON_SEATED_SUPPORT = 0x00000010,		///< Traque les squelettes en seated mode (sans le bas du corps). 
    SKELETON_NEAR_MODE = 0x00000004,			///< Traque en mode proximité (de 40 cm à 3 m).
};


/// \brief Définit une interface pour communiquer avec la Kinect.
/// \ingroup kinectgroup
class KinectInterface
{
public:
    

    ///\brief Constructeur par défaut. N'est pas supposé être appelé et ne fait rien.
    KinectInterface();

    /// \brief Constructeur.
    /// \param flags Une suite de flags (combinés par un OU bit-à-bit) précisant la manière dont la Kinect sera initialisée.
    KinectInterface(unsigned long flags);

    /// \brief Destructeur. Libère les ressources allouées et s'assure que les threads finissent normalement.
    ~KinectInterface();

    ///\brief Initialise l'objet.
    /// Ne fait rien si l'objet a déjà été initialisé.
    void initialize(unsigned long flags);

    /// \brief Débute le traitement des données envoyées par le capteur.
    /// 
    /// Cette fonction lance un thread qui s'occupera du traitement, pour éviter que ce dernier ralentisse l'application
    /// en cas d'appel à des fonctions bloquantes ou de calculs trop lourds.
    void start(void);

    /// \brief Interromp le traitement des données envoyées par la Kinect.
    ///
    /// Cette fonction se contente d'interrompre le traitement, elle ne réinitialise pas la Kinect ni ne libère de ressources. Un
    /// appel à la méthode start() relancera le traitement.
    /// \warning L'état interne de l'objet n'est pas réinitialisé par l'appel à stop(). Notamment, la pile des évènements ainsi que
    /// les données du traitement courant resteront figées jusqu'au prochain appel à start() ou la destruction de l'objet.
    void stop(void);

    /// \brief Modifie l'm_orientation du capteur. N'a aucun effet si l'argument n'est pas compris entre -27° et +27°.
    /// \param command La nouvelle m_orientation. Doit être compris entre -27° et +27°.
    void setOrientation(long command);

    /// \brief Retire de la pile le plus ancien des évènements et le stocke dans l'évènement passé en argument.
    ///
    /// Cette fonction accède à la pile d'évènement m_events pour en retirer le premier élément et le copier dans celui passé
    /// en argument. L'usage prévu de cette fonction est en tant que condition de sortie d'une boucle de lecture des évènements. Pour
    /// cette raison, un appel à pollEvent() alors que la pile n'est pas vide bloque le mutex associé jusqu'à ce que pollEvent() ait
    /// lu le dernier évènement de la pile.
    ///
    /// \note Pour parer au cas où la pile serait modifiée entre deux appels à pollEvent() pendant la boucle de lecture, le mutex
    /// n'est pas automatiquement déverrouillé avant le retour de la fonction. Il est déverrouillé uniquement si la fonction lit le
    /// dernier élément de la pile d'évènement. 
    /// \param event Référence vers l'évènement acceuillant l'évènement retiré de la pile.
    /// \return True si un évènement a été lu de la pile et false sinon.
    bool pollEvent(KinectEvent& event);

    /// \brief Modifie la méthode de sélection des utilisateurs à traquer.
    ///
    /// Lorsqu'elle est appelée pour la première fois, cette fonction réinitialise la reconnaissance de squelettes en lui retirant
    /// la responsabilité de la spécification des utilisateurs à traquer. A partir de ce moment, l'interface utilisera l'objet 
    /// fourni (pointé par m_tracker) pour traquer les utilisateurs. Une valeur de nullptr représente la volonté de
    /// revenir à la sélection par défaut.
    /// \param tracker L'adresse de l'objet contenant la méthode de sélection.
    void setTrackingMethod(SkeletonTracker *tracker = nullptr);

    /// \brief Fixe l'échelle du système de coordonnées que suivent les évènements générés par l'interface.
    /// \param width La largeur maximale.
    /// \param height La hauteur maximale.
    void setTargetImageSize(unsigned int width, unsigned int height);

private:

    /// \brief Un pointeur vers le capteur Kinect utilisé.
    INuiSensor *m_kinect;

    /// \brief La dernière trame (squeletes) reçue par l'instance.
    NUI_SKELETON_FRAME m_skeletonFrame;
    /// \brief Pointeur vers l'évènement recevant les nouvelles trames.
    HANDLE m_nextSkeletonFrame;

    /// \brief La dernière trame (image) traitée par l'instance.
    /// 
    /// Par rapport à la variable locale newFrame de KtI::KinectInterface::processFrame, il s'agit de la trame précédant celle récupérée.
    NUI_IMAGE_FRAME m_imageFrame;
    /// \brief Pointeur vers le flux d'image.
    HANDLE m_imageStream;
    /// \brief Pointeur vers l'évènement recevant les nouvelles trames.
    HANDLE m_nextImageFrame;

    /// \brief La dernière trame de profondeur reçue par l'instance.
    NUI_IMAGE_FRAME m_depthFrame;
    /// \brief Pointeur vers le flux de profondeur.
    HANDLE m_depthStream;
    /// \brief Pointeur vers l'évènement recevant la trame de profondeur.
    HANDLE m_nextDepthFrame;

    /// \brief Pointeur vers le flux d'interaction.
    INuiInteractionStream *m_interactionStream;

    /// \brief L'objet Interaction servant à ajuster les données des interactions.
    InteractionClient m_IClient;

    /// \brief Trame d'interaction courante.
    NUI_INTERACTION_FRAME m_interactionFrame;

    /// \brief Pointeur vers l'évènement indiquant une nouvelle trame d'interaction.
    HANDLE m_nextInteractionFrame;

    /// \brief Vaut true si le constructeur a reçu SKELETON_SEATED_SUPPORT.
    bool m_seatedSupport;
    /// \brief Vaut true si le constructeur a reçu SKELETON_NEAR_MODE.
    bool m_nearMode;
    /// \brief Commande l'arrêt de la réception et du traitement des trames.
    std::atomic_bool m_stopProcessing;

    /// \brief Angle voulu pour le capteur Kinect.
    std::atomic_long m_orientation;

    /// \brief Un thread exécutant la majorité du traitement effectué par la classe.
    std::thread m_processingThread;

    /// \brief Contient les évènements détectés.
    ///
    /// Ce tableau est lu par la méthode KtI::KinectInterface::pollEvent(). Les évènements détectés pendant la lecture sont stockés 
    /// dans le tableau m_eventsBuffer.
    std::vector<KinectEvent> m_events;

    /// \brief Mutex gérant l'accès à la ressource m_events.
    smart_mutex m_eventAccess;

    /// \brief Sert de tableau tampon pour contenir les évènements détecté lorsque m_events est lu par l'application.
    ///
    /// Ce tableau est utilisé lorsque la méthode KtI::KinectInterface::pollEvent() est appelée pour lire depuis m_events. Il stocke
    /// les évènements détectés pendant cet intervalle de temps jusqu'à ce que m_events soit de nouveau accessible à l'interface.
    std::vector<KinectEvent> m_eventsBuffer;

    /// \brief Pointeur vers l'objet responsable du choix des squelettes à traquer (une classe dérivée de SkeletonTracker).
    SkeletonTracker *m_tracker;

    /// \brief Variable ternaire indiquant l'état d'initialisation de la reconnaissance de squelette.
    trinary m_skeletonTrackingState;

    /// \brief Mutex régulant l'accès à m_tracker et m_skeletonTrackingState.
    std::mutex m_trackerAccess;
    
    /// \brief Les dimensions de l'image cible à laquelle les coordonnées contenues dans les évènements seront adaptés.
    ///
    /// Par exemple, si on affiche le squelette dans une fenêtre de 1000x800, appeller cette fonction avec ces dimensions en arguments
    /// adapte les coordonnées contenues dans l'évènement SkeletonMovedEvent (la position maximale sera 1000,800). Si cette méthode n'est
    /// pas appelée, les coordonnées seront des coordonnées relatives (entre 0 et 1).
    std::atomic<unsigned int> m_targetImgWidth, m_targetImgHeight;

    //méthodes
    
    /// \brief Cherche un capteur Kinect valide et s'y connecte. m_kinect vaudra NULL en cas d'échec.
    void connectToSensor();

    /// \brief Initialise la reconnaissance des squelettes.
    /// 
    /// Cette fonction analyse les paramètres du constructeurs et fait appel à NuiSkeletonTrackingEnable() en lui donnant des paramètres
    /// correspondant à ces flags. 
    /// \param flags Les flags reçus par le constructeur de l'instance.
    /// \return hr Résultat de l'opération (vaut S_OK si aucuns problèmes).
    HRESULT initializeSkeletonTracking(DWORD flags);

    /// \brief Initialise le flux d'image couleur.
    /// 
    /// Cette fonction analyse les paramètres reçus par le constructeur et choisit la manière d'initialiser le flux. Elle choisit
    /// toujours la meilleure résolution possible (en gardant les performances à l'esprit) qui est souvent 640x480 et ne considère pas
    /// pour l'instant d'autre format d'image que le RGB. Elle ignore de fait certains paramètres.
    /// \param flags Les flags reçus par le constructeur de l'instance.
    /// \return hr Résultat de l'initialisation du flux (S_OK s'il n'y a pas eu d'erreurs).
    HRESULT initializeImageStream(DWORD flags);

    /// \brief Initialise le flux de profondeur.
    ///
    /// Cette fonction analyse les paramètres reçus par le constructeur et décide des paramètres à envoyer pour initialiser le flux.
    /// Elle ignore certains paramètres.
    /// \param flags Les flags reçus par le constructeur de l'instance.
    /// \return hr Résultat de l'initialisation du flux (S_OK s'il n'y a pas eu d'erreurs).
    HRESULT initializeDepthStream(DWORD flags);

    /// \brief Initialise la reconnaissance d'interactions.
    ///
    /// Cette fonction est activée par défaut lorsque les flux de profondeur et de reconnaissance du squelette sont initialisés.
    /// \param IClient
    /// \return hr Résultat de l'initialisation (S_OK si pas d'erreurs).
    HRESULT initializeInteractionStream(InteractionClient &IClient);

    /// \brief La fonction se chargeant de recevoir et traiter les frames envoyées par le capteur.
    void processFrames();

    /// \brief Traitement de la trame de profondeur.
    /// \warning Incomplet!
    void processDepthFrame(NUI_IMAGE_FRAME &newDepthFrame);

    /// \brief Traitement des trames de squelettes.
    void procesSkeletonFrame(NUI_SKELETON_FRAME &newSkeletonFrame);

    /// \brief Traitement de la trame d'interaction.
    void processInteractionFrame(NUI_INTERACTION_FRAME &newInteractionFrame);

    /// \brief Tente de reconnaître la position du squelette parmi des positions prédéfinies.
    PositionType getPosition(NUI_SKELETON_DATA &data);
    

    /// \brief Ajoute l'évènement courant à la pile.
    ///
    /// Cette fonction tente d'accéder à la pile d'évènement m_events. Si elle n'y parvient pas (parce qu'un autre thread est en
    /// train de la lire), elle stocke l'évènement dans une pile tampon qu'elle copie dans la pile principale dès qu'elle en regagne
    /// l'accès.
    /// \param newEvent Evènement courant à ajouter dans la pile.
    void addEventToStack(KinectEvent &newEvent);

    /// \brief Fonction déterminant si un utilisateur ou un squelette a bougé entre deux frames.
    ///
    /// Cette fonction compare les données de la dernière trame avec celles de la trame précédente pour déterminer si un mouvement
    /// a eu lieu. Elle n'ignore aucun mouvement, aussi petit soit son amplitude.
    /// \param newData Les dernières données reçues.
    /// \return hasMoved Booléen indiquant s'il y a eu mouvement (true : mouvement, false : pas de mouvement).
    bool moved(NUI_SKELETON_DATA &newData)const;

    /// \brief Décide des utilisateurs à traquer complètement.
    ///
    /// Cette fonction fait appel à la méthode setTrackedSkeletons() de l'objet pointé par m_tracker pour le choix.
    /// \param newSkeletonFrame La trame courante (celle actuellement traitée par processSkeletonFrame).
    void setTrackedSkeletons(NUI_SKELETON_FRAME &newSkeletonFrame);

    
};
} // namespace KtI
#endif


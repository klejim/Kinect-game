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
/// \brief Classe de base utilis�e pour d�terminer quels squelettes traquer lorsque en mode manuel.
/// 
/// Cette classe n'a qu'une unique m�thode : choseTrackedSkeletons(), qui prend en argument une trame de donn�es sur les squelettes
/// et doit renvoyer un tableau de deux identifiants qui seront ceux que la kinect traquera. La classe �tant purement virtuelle, il
/// est imp�ratif d'impl�menter une classe en h�ritant pour pouvoir utiliser la s�lection manuelle d'utilisateurs � traquer.
class SkeletonTracker
{
public:
    /// \brief M�thode servant � ajuster les param�tres de la trame d'interactions.
    ///
    /// \warning Cette fonction renvoyant un pointeur vers un tableau local, il est n�cessaire de cr�er celui-ci avec une allocation
    /// dynamique. L'interface s'attend � ce que ce soit le cas et g�n�rera une erreur si le pointeur est nul en sortie de la fonction.
    /// \param skeletonFrame R�f�rence � la trame d'interaction courante qui est analys�e pour la s�lection.
    /// \return ids Un pointeur vers le premier �l�ment d'un tableau de deux identifiants qui sont ceux des utilisateurs � traquer.
    virtual DWORD* choseTrackedSkeletons(NUI_SKELETON_FRAME &skeletonFrame)=0;
};

/// \brief Classe utilis�e par l'API INuiInteractionStream pour ajuster les donn�es d'une trame d'interactions � travers sa m�thode
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
    /// \brief Fonction servant � ajuster les donn�es de la trame d'interactions.
    /// 
    /// La comportement voulu est le suivant : toutes les mains d�tect�es seront susceptibles d'attraper ou de relacher. Les autres
    /// param�tres sont recommand�s � plusieurs reprises sur les forums de microsoft.
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

/// \brief Red�finition des constantes d�crivant la qualit� d'un squelette
enum SkeletonQuality
{
    NOT_CLIPPED = 0,
    CLIPPED_RIGHT = NUI_SKELETON_QUALITY_CLIPPED_RIGHT,	
    CLIPPED_LEFT = NUI_SKELETON_QUALITY_CLIPPED_LEFT,
    CLIPPED_TOP = NUI_SKELETON_QUALITY_CLIPPED_TOP,
    CLIPPED_BOTTOM = NUI_SKELETON_QUALITY_CLIPPED_BOTTOM			 
};

/// \brief Red�finition de l'�num�ration NUI_SKELETON_POSITION_TRACKING_STATE.
enum JointTrackingState
{
    NOT_TRACKED = NUI_SKELETON_POSITION_NOT_TRACKED,
    POSITION_INFERRED = NUI_SKELETON_POSITION_INFERRED,
    POSITION_TRACKED = NUI_SKELETON_POSITION_TRACKED
};

/// \brief Enum�ration red�finissant l'�num�ration NUI_POSITION_INDEX par commodit� d'acc�s.
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
    T,		 ///< Les bras � l'horizontale et le corps droit.
    Y,		 ///< Les bras et les jambes � la verticale (bras vers le bas).
    STOP,	 ///< Le corps droit et les bras crois�s � 45�.
};

/// \brief Des flags utilis�s pour initialiser la Kinect (voir <a href="https://msdn.microsoft.com/en-us/library/hh855368.aspx"/> NUI Constants </a>).
/// 
///Par commodit�, les flags INITIALIZE_AUDIO, INITIALIZE_SKELETON, INITIALIZE_DEPTH et INITIALIZE_COLOR sont pass�s tels quels � NuiInitialize, ils ont donc la valeur d�finie 
/// par NuiApi.h. La valeur des autres flags est analys�e pour appeler les fonctions concern�es avec les param�tres correspondant.
enum KInitializeFlags
{
    // NUI_INITIALIZE_FLAGS
    // les flags NUI_INITIALIZE_ sont envoy�s tels quels � la fonction NuiInitialize
    INITIALIZE_COLOR = 0x00000002,				///< Utilise la d�tection d'images couleur.
    INITIALIZE_SKELETON = 0x00000008,			///< Utilise la d�tection de squelettes.
    INITIALIZE_DEPTH = 0x00000020,				///< Utilise la d�tection d�tection de frame de profondeurs.
    INITIALIZE_AUDIO = 0x10000000,				///< D�tecte l'�mission de son.
    INITIALIZE_ALL = 0x1000002A,				///< Utilise tous les flux possibles.
    // NUI_SKELETON_TRACKING_FLAGS
    // ceux-l� sont "traduits" avant d'�tre pass�s � leurs fonctions (leurs valeurs poseraient des conflits sinon)
    SKELETON_SEATED_SUPPORT = 0x00000010,		///< Traque les squelettes en seated mode (sans le bas du corps). 
    SKELETON_NEAR_MODE = 0x00000004,			///< Traque en mode proximit� (de 40 cm � 3 m).
};


/// \brief D�finit une interface pour communiquer avec la Kinect.
/// \ingroup kinectgroup
class KinectInterface
{
public:
    

    ///\brief Constructeur par d�faut. N'est pas suppos� �tre appel� et ne fait rien.
    KinectInterface();

    /// \brief Constructeur.
    /// \param flags Une suite de flags (combin�s par un OU bit-�-bit) pr�cisant la mani�re dont la Kinect sera initialis�e.
    KinectInterface(unsigned long flags);

    /// \brief Destructeur. Lib�re les ressources allou�es et s'assure que les threads finissent normalement.
    ~KinectInterface();

    ///\brief Initialise l'objet.
    /// Ne fait rien si l'objet a d�j� �t� initialis�.
    void initialize(unsigned long flags);

    /// \brief D�bute le traitement des donn�es envoy�es par le capteur.
    /// 
    /// Cette fonction lance un thread qui s'occupera du traitement, pour �viter que ce dernier ralentisse l'application
    /// en cas d'appel � des fonctions bloquantes ou de calculs trop lourds.
    void start(void);

    /// \brief Interromp le traitement des donn�es envoy�es par la Kinect.
    ///
    /// Cette fonction se contente d'interrompre le traitement, elle ne r�initialise pas la Kinect ni ne lib�re de ressources. Un
    /// appel � la m�thode start() relancera le traitement.
    /// \warning L'�tat interne de l'objet n'est pas r�initialis� par l'appel � stop(). Notamment, la pile des �v�nements ainsi que
    /// les donn�es du traitement courant resteront fig�es jusqu'au prochain appel � start() ou la destruction de l'objet.
    void stop(void);

    /// \brief Modifie l'm_orientation du capteur. N'a aucun effet si l'argument n'est pas compris entre -27� et +27�.
    /// \param command La nouvelle m_orientation. Doit �tre compris entre -27� et +27�.
    void setOrientation(long command);

    /// \brief Retire de la pile le plus ancien des �v�nements et le stocke dans l'�v�nement pass� en argument.
    ///
    /// Cette fonction acc�de � la pile d'�v�nement m_events pour en retirer le premier �l�ment et le copier dans celui pass�
    /// en argument. L'usage pr�vu de cette fonction est en tant que condition de sortie d'une boucle de lecture des �v�nements. Pour
    /// cette raison, un appel � pollEvent() alors que la pile n'est pas vide bloque le mutex associ� jusqu'� ce que pollEvent() ait
    /// lu le dernier �v�nement de la pile.
    ///
    /// \note Pour parer au cas o� la pile serait modifi�e entre deux appels � pollEvent() pendant la boucle de lecture, le mutex
    /// n'est pas automatiquement d�verrouill� avant le retour de la fonction. Il est d�verrouill� uniquement si la fonction lit le
    /// dernier �l�ment de la pile d'�v�nement. 
    /// \param event R�f�rence vers l'�v�nement acceuillant l'�v�nement retir� de la pile.
    /// \return True si un �v�nement a �t� lu de la pile et false sinon.
    bool pollEvent(KinectEvent& event);

    /// \brief Modifie la m�thode de s�lection des utilisateurs � traquer.
    ///
    /// Lorsqu'elle est appel�e pour la premi�re fois, cette fonction r�initialise la reconnaissance de squelettes en lui retirant
    /// la responsabilit� de la sp�cification des utilisateurs � traquer. A partir de ce moment, l'interface utilisera l'objet 
    /// fourni (point� par m_tracker) pour traquer les utilisateurs. Une valeur de nullptr repr�sente la volont� de
    /// revenir � la s�lection par d�faut.
    /// \param tracker L'adresse de l'objet contenant la m�thode de s�lection.
    void setTrackingMethod(SkeletonTracker *tracker = nullptr);

    /// \brief Fixe l'�chelle du syst�me de coordonn�es que suivent les �v�nements g�n�r�s par l'interface.
    /// \param width La largeur maximale.
    /// \param height La hauteur maximale.
    void setTargetImageSize(unsigned int width, unsigned int height);

private:

    /// \brief Un pointeur vers le capteur Kinect utilis�.
    INuiSensor *m_kinect;

    /// \brief La derni�re trame (squeletes) re�ue par l'instance.
    NUI_SKELETON_FRAME m_skeletonFrame;
    /// \brief Pointeur vers l'�v�nement recevant les nouvelles trames.
    HANDLE m_nextSkeletonFrame;

    /// \brief La derni�re trame (image) trait�e par l'instance.
    /// 
    /// Par rapport � la variable locale newFrame de KtI::KinectInterface::processFrame, il s'agit de la trame pr�c�dant celle r�cup�r�e.
    NUI_IMAGE_FRAME m_imageFrame;
    /// \brief Pointeur vers le flux d'image.
    HANDLE m_imageStream;
    /// \brief Pointeur vers l'�v�nement recevant les nouvelles trames.
    HANDLE m_nextImageFrame;

    /// \brief La derni�re trame de profondeur re�ue par l'instance.
    NUI_IMAGE_FRAME m_depthFrame;
    /// \brief Pointeur vers le flux de profondeur.
    HANDLE m_depthStream;
    /// \brief Pointeur vers l'�v�nement recevant la trame de profondeur.
    HANDLE m_nextDepthFrame;

    /// \brief Pointeur vers le flux d'interaction.
    INuiInteractionStream *m_interactionStream;

    /// \brief L'objet Interaction servant � ajuster les donn�es des interactions.
    InteractionClient m_IClient;

    /// \brief Trame d'interaction courante.
    NUI_INTERACTION_FRAME m_interactionFrame;

    /// \brief Pointeur vers l'�v�nement indiquant une nouvelle trame d'interaction.
    HANDLE m_nextInteractionFrame;

    /// \brief Vaut true si le constructeur a re�u SKELETON_SEATED_SUPPORT.
    bool m_seatedSupport;
    /// \brief Vaut true si le constructeur a re�u SKELETON_NEAR_MODE.
    bool m_nearMode;
    /// \brief Commande l'arr�t de la r�ception et du traitement des trames.
    std::atomic_bool m_stopProcessing;

    /// \brief Angle voulu pour le capteur Kinect.
    std::atomic_long m_orientation;

    /// \brief Un thread ex�cutant la majorit� du traitement effectu� par la classe.
    std::thread m_processingThread;

    /// \brief Contient les �v�nements d�tect�s.
    ///
    /// Ce tableau est lu par la m�thode KtI::KinectInterface::pollEvent(). Les �v�nements d�tect�s pendant la lecture sont stock�s 
    /// dans le tableau m_eventsBuffer.
    std::vector<KinectEvent> m_events;

    /// \brief Mutex g�rant l'acc�s � la ressource m_events.
    smart_mutex m_eventAccess;

    /// \brief Sert de tableau tampon pour contenir les �v�nements d�tect� lorsque m_events est lu par l'application.
    ///
    /// Ce tableau est utilis� lorsque la m�thode KtI::KinectInterface::pollEvent() est appel�e pour lire depuis m_events. Il stocke
    /// les �v�nements d�tect�s pendant cet intervalle de temps jusqu'� ce que m_events soit de nouveau accessible � l'interface.
    std::vector<KinectEvent> m_eventsBuffer;

    /// \brief Pointeur vers l'objet responsable du choix des squelettes � traquer (une classe d�riv�e de SkeletonTracker).
    SkeletonTracker *m_tracker;

    /// \brief Variable ternaire indiquant l'�tat d'initialisation de la reconnaissance de squelette.
    trinary m_skeletonTrackingState;

    /// \brief Mutex r�gulant l'acc�s � m_tracker et m_skeletonTrackingState.
    std::mutex m_trackerAccess;
    
    /// \brief Les dimensions de l'image cible � laquelle les coordonn�es contenues dans les �v�nements seront adapt�s.
    ///
    /// Par exemple, si on affiche le squelette dans une fen�tre de 1000x800, appeller cette fonction avec ces dimensions en arguments
    /// adapte les coordonn�es contenues dans l'�v�nement SkeletonMovedEvent (la position maximale sera 1000,800). Si cette m�thode n'est
    /// pas appel�e, les coordonn�es seront des coordonn�es relatives (entre 0 et 1).
    std::atomic<unsigned int> m_targetImgWidth, m_targetImgHeight;

    //m�thodes
    
    /// \brief Cherche un capteur Kinect valide et s'y connecte. m_kinect vaudra NULL en cas d'�chec.
    void connectToSensor();

    /// \brief Initialise la reconnaissance des squelettes.
    /// 
    /// Cette fonction analyse les param�tres du constructeurs et fait appel � NuiSkeletonTrackingEnable() en lui donnant des param�tres
    /// correspondant � ces flags. 
    /// \param flags Les flags re�us par le constructeur de l'instance.
    /// \return hr R�sultat de l'op�ration (vaut S_OK si aucuns probl�mes).
    HRESULT initializeSkeletonTracking(DWORD flags);

    /// \brief Initialise le flux d'image couleur.
    /// 
    /// Cette fonction analyse les param�tres re�us par le constructeur et choisit la mani�re d'initialiser le flux. Elle choisit
    /// toujours la meilleure r�solution possible (en gardant les performances � l'esprit) qui est souvent 640x480 et ne consid�re pas
    /// pour l'instant d'autre format d'image que le RGB. Elle ignore de fait certains param�tres.
    /// \param flags Les flags re�us par le constructeur de l'instance.
    /// \return hr R�sultat de l'initialisation du flux (S_OK s'il n'y a pas eu d'erreurs).
    HRESULT initializeImageStream(DWORD flags);

    /// \brief Initialise le flux de profondeur.
    ///
    /// Cette fonction analyse les param�tres re�us par le constructeur et d�cide des param�tres � envoyer pour initialiser le flux.
    /// Elle ignore certains param�tres.
    /// \param flags Les flags re�us par le constructeur de l'instance.
    /// \return hr R�sultat de l'initialisation du flux (S_OK s'il n'y a pas eu d'erreurs).
    HRESULT initializeDepthStream(DWORD flags);

    /// \brief Initialise la reconnaissance d'interactions.
    ///
    /// Cette fonction est activ�e par d�faut lorsque les flux de profondeur et de reconnaissance du squelette sont initialis�s.
    /// \param IClient
    /// \return hr R�sultat de l'initialisation (S_OK si pas d'erreurs).
    HRESULT initializeInteractionStream(InteractionClient &IClient);

    /// \brief La fonction se chargeant de recevoir et traiter les frames envoy�es par le capteur.
    void processFrames();

    /// \brief Traitement de la trame de profondeur.
    /// \warning Incomplet!
    void processDepthFrame(NUI_IMAGE_FRAME &newDepthFrame);

    /// \brief Traitement des trames de squelettes.
    void procesSkeletonFrame(NUI_SKELETON_FRAME &newSkeletonFrame);

    /// \brief Traitement de la trame d'interaction.
    void processInteractionFrame(NUI_INTERACTION_FRAME &newInteractionFrame);

    /// \brief Tente de reconna�tre la position du squelette parmi des positions pr�d�finies.
    PositionType getPosition(NUI_SKELETON_DATA &data);
    

    /// \brief Ajoute l'�v�nement courant � la pile.
    ///
    /// Cette fonction tente d'acc�der � la pile d'�v�nement m_events. Si elle n'y parvient pas (parce qu'un autre thread est en
    /// train de la lire), elle stocke l'�v�nement dans une pile tampon qu'elle copie dans la pile principale d�s qu'elle en regagne
    /// l'acc�s.
    /// \param newEvent Ev�nement courant � ajouter dans la pile.
    void addEventToStack(KinectEvent &newEvent);

    /// \brief Fonction d�terminant si un utilisateur ou un squelette a boug� entre deux frames.
    ///
    /// Cette fonction compare les donn�es de la derni�re trame avec celles de la trame pr�c�dente pour d�terminer si un mouvement
    /// a eu lieu. Elle n'ignore aucun mouvement, aussi petit soit son amplitude.
    /// \param newData Les derni�res donn�es re�ues.
    /// \return hasMoved Bool�en indiquant s'il y a eu mouvement (true : mouvement, false : pas de mouvement).
    bool moved(NUI_SKELETON_DATA &newData)const;

    /// \brief D�cide des utilisateurs � traquer compl�tement.
    ///
    /// Cette fonction fait appel � la m�thode setTrackedSkeletons() de l'objet point� par m_tracker pour le choix.
    /// \param newSkeletonFrame La trame courante (celle actuellement trait�e par processSkeletonFrame).
    void setTrackedSkeletons(NUI_SKELETON_FRAME &newSkeletonFrame);

    
};
} // namespace KtI
#endif


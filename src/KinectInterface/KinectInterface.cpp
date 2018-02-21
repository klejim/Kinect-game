#include <cmath> // std::acos(), std::abs(), std::sqrt()
#include <fstream>
#include <iostream> // std::cout
#include <stdexcept> // std::runtime_error

#include "KinectInterface/KinectInterface.hpp"
#include "KinectInterface/KinectEvent.hpp"

namespace KtI
{
KinectInterface::KinectInterface()
{ }

KinectInterface::KinectInterface(unsigned long flags)
{
    initialize(flags);
}

void KinectInterface::initialize(unsigned long flags)
{
    // on initialise uniquement au premier appel 
    if (m_kinect == nullptr)
    {
        m_tracker = nullptr;
        m_skeletonTrackingState = trinary::MEM;
        m_nearMode = false;
        m_seatedSupport = false;
        m_targetImgWidth = 1;
        m_targetImgHeight = 1;
        HRESULT hr(S_OK);
        // d'abord trouver le capteur
        // on cherche une kinect opérationelle
        connectToSensor();
        // si aucun capteur n'a été trouvé, m_kinect est encore nul
        if (m_kinect == nullptr)
        {
            throw std::runtime_error("Aucune Kinect valide trouvee.");
        }
        // si tout va bien, on peut initialiser le capteur
        // on recrée les flags passés à NuiInitialize à partir des arguments du constructeur
        long initFlags = 0;
        initFlags = (flags & INITIALIZE_AUDIO) | flags & INITIALIZE_COLOR | flags & INITIALIZE_SKELETON;
        if (flags & INITIALIZE_DEPTH)
        {
            if (flags & INITIALIZE_SKELETON)
            {
                initFlags |= NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;
            }
            else initFlags |= NUI_INITIALIZE_FLAG_USES_DEPTH;
        }

        if (m_kinect->NuiInitialize(initFlags) != S_OK)
        {
            m_kinect->NuiShutdown();
            throw std::runtime_error("Impossible d'initialiser la Kinect.");
        }
        // maintenant on initialise chacun des types de frames

        //le squelette
        if (flags & INITIALIZE_SKELETON)
        {
            hr = initializeSkeletonTracking(flags);
            if (hr != S_OK)
            {
                m_kinect->NuiShutdown();
                throw std::runtime_error("Impossible d'initialiser la reconnaissance de squelette.");
            }
        }
        // les images couleur
        if (flags & INITIALIZE_COLOR)
        {
            hr = initializeImageStream(flags);
            if (hr != S_OK)
            {
                m_kinect->NuiShutdown();
                throw std::runtime_error("Impossible d'ouvrir un flux d'image.");
            }
        }
        // la profondeur
        if (flags & INITIALIZE_DEPTH)
        {
            hr = initializeDepthStream(flags);
            if (hr != S_OK)
            {
                m_kinect->NuiShutdown();
                throw std::runtime_error("Impossible d'ouvrir un flux de profondeur.");
            }
        }
        // interaction
        if (flags & INITIALIZE_DEPTH && flags & INITIALIZE_SKELETON)
        {
            m_IClient = InteractionClient();
            hr = initializeInteractionStream(m_IClient);
            if (hr != S_OK)
            {
                m_kinect->NuiShutdown();
                throw std::runtime_error("Impossible d'initiliaser le flux d'interactions.");
            }
        }
    }
}

KinectInterface::~KinectInterface()
{
    if (m_processingThread.joinable())
    {
        m_processingThread.join();
    }
    m_kinect->NuiCameraElevationSetAngle(0);
    m_kinect->NuiShutdown();
}

void KinectInterface::connectToSensor()
{
    m_kinect = nullptr;
    int nb_sensors;
    NuiGetSensorCount(&nb_sensors);
    for (int i = 0; i < nb_sensors; i++)
    {
        if (NuiCreateSensorByIndex(i, &m_kinect) == S_OK)
        {
            break; // trouvée, inutile de parcourir les autres
        }
    }
    return;
}

HRESULT KinectInterface::initializeDepthStream(DWORD flags)
{
    m_nextDepthFrame = CreateEvent(NULL, TRUE, FALSE, NULL);
    NUI_IMAGE_TYPE imgType;
    NUI_IMAGE_RESOLUTION imgRes;

    // si on initialise seulement la profondeur (image ou pas n'a aucune importance)
    if (flags & INITIALIZE_DEPTH && !(flags & INITIALIZE_SKELETON))
    {
        imgType = NUI_IMAGE_TYPE_DEPTH;
    }
    // si on s'intéresse aussi au squelettes, le type est différent
    if (flags & INITIALIZE_DEPTH && flags & INITIALIZE_SKELETON)
    {
        imgType = NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX;
    }
    // dans tous les cas, on prend la meilleure résolution
    imgRes = NUI_IMAGE_RESOLUTION_640x480;

    HRESULT hr = m_kinect->NuiImageStreamOpen(imgType, imgRes, NUI_IMAGE_STREAM_FLAG_SUPPRESS_NO_FRAME_DATA |
        (NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE & (m_nearMode << 17)), 2, m_nextDepthFrame, &m_depthStream);
    return hr;
}

HRESULT KinectInterface::initializeImageStream(DWORD flags)
{
    m_nextImageFrame = CreateEvent(NULL, TRUE, FALSE, NULL);
    // on ignore pas mal de paramètres parmi lesquels il faudrait choisir. En règle générale, on prend la meilleure résolution et
    // le format le plus pratique.
    HRESULT hr = m_kinect->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480,
        NUI_IMAGE_STREAM_FLAG_SUPPRESS_NO_FRAME_DATA, 2, m_nextImageFrame, &m_imageStream);
    return hr;
}

HRESULT KinectInterface::initializeSkeletonTracking(DWORD flags)
{
    m_nextSkeletonFrame = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (flags & SKELETON_NEAR_MODE)
    {
        m_nearMode = true;
    }
    if (flags & SKELETON_SEATED_SUPPORT)
    {
        m_seatedSupport = true;
    }
    DWORD tmpflags = NUI_SKELETON_TRACKING_FLAG_SUPPRESS_NO_FRAME_DATA | NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT & (m_seatedSupport << 2) |
        NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE & (m_nearMode << 3);
    // m_skeletonTrackingState est mis à SET si l'application active la reconnaissance manuelle de squelettes.
    if (m_skeletonTrackingState == trinary::SET)
    {
        tmpflags |= NUI_SKELETON_TRACKING_FLAG_TITLE_SETS_TRACKED_SKELETONS;
    }
    std::cout << "Skeleton flags : " << tmpflags << std::endl;
    HRESULT hr = m_kinect->NuiSkeletonTrackingEnable(m_nextSkeletonFrame, tmpflags);
    return hr;
}

HRESULT KinectInterface::initializeInteractionStream(InteractionClient &IClient)
{
    m_nextInteractionFrame = CreateEvent(NULL, TRUE, FALSE, NULL);
    HRESULT hr = NuiCreateInteractionStream(m_kinect, &IClient, &m_interactionStream);
    if (hr == S_OK)
    {
        hr = m_interactionStream->Enable(m_nextInteractionFrame);
    }
    return hr;
}

void KinectInterface::processFrames()
{
    /// \todo Après processSkeletonFrame:
    ///		- ProcessImageFrame
    ///		- ProcessAudio
    m_stopProcessing = false;
    // les nouvelles trames qui seront traitées ce tour de boucle (les membres de la classe sont les trames déjà traitées).
    NUI_SKELETON_FRAME newSkeletonFrame;
    NUI_IMAGE_FRAME newDepthFrame;
    NUI_INTERACTION_FRAME newInteractionFrame;
    
    int dbg_cpt = 0;
    LONG angle; //angle que forme le capteur par rapport à l'horizontale
    ///\todo DEBUG STUFF A ENLEVER
   // m_kinect->NuiCameraElevationSetAngle(10);
    ///
    while (!m_stopProcessing)
    {
        // modification de l'angle du capteur
        LONG orientation = m_orientation; // lu plutôt qu'utilisé directement pour éviter des conflits entre les threads
        HRESULT hr = m_kinect->NuiCameraElevationGetAngle(&angle);
        if (std::abs(angle - orientation) > 3)
        {
            hr = m_kinect->NuiCameraElevationSetAngle(orientation);
        }
        // Avant de traiter les nouvelles trames, on vérifie s'il est nécessaire de réinitialiser la reconnaissance de squelette
        // Cette décision dépend des états du pointeur m_tracker et de la variable ternaire m_skeletonTrackingState. 
        // L'initialisation de base (celle du constructeur) ayant marchée, on ne vérifie pas que celle-ci fonctionne.
        if (m_kinect->NuiInitializationFlags() & NUI_INITIALIZE_FLAG_USES_SKELETON)
        {
            // on réinitialise uniquement si la variable ternaire ne vaut pas MEM, pour ne pas avoir à le faire à toutes
            // les itérations.
            if (m_skeletonTrackingState == trinary::SET && m_tracker != nullptr) // tracker == nullptr est impossible dans ce cas
            {
                initializeSkeletonTracking(NUI_INITIALIZE_FLAG_USES_SKELETON);
                m_skeletonTrackingState = trinary::MEM;
            }
            else if (m_skeletonTrackingState == trinary::RESET && m_tracker == nullptr) //tracker!=nullptr est impossible
            {
                // l'état de m_skeletonTrackingState est directement pris en compte par initializeSkeletonTracking, donc on 
                // se contente de l'appeler à nouveau.
                initializeSkeletonTracking(NUI_INITIALIZE_FLAG_USES_SKELETON);
                m_skeletonTrackingState = trinary::MEM;
            }
        }
        
        // profondeur
        if (WaitForSingleObject(m_nextDepthFrame, 0) == WAIT_OBJECT_0)
        {
            m_kinect->NuiImageStreamGetNextFrame(m_depthStream, INFINITE, &newDepthFrame);
            processDepthFrame(newDepthFrame);
            //
            ResetEvent(m_nextDepthFrame);
            m_depthFrame = newDepthFrame;
            m_kinect->NuiImageStreamReleaseFrame(m_depthStream, &newDepthFrame);
        }
        
        // skeleton
        if (WaitForSingleObject(m_nextSkeletonFrame, 0) == WAIT_OBJECT_0)
        {
            m_kinect->NuiSkeletonGetNextFrame(0, &newSkeletonFrame);
            m_kinect->NuiTransformSmooth(&newSkeletonFrame, 0);
        
            // analyse de la trame actuelle et comparaison avec l'ancienne.
            procesSkeletonFrame(newSkeletonFrame);
            // Si l'application a validé la sélection manuelle des squelettes à traquer, on exécute la méthode pointée par m_tracker.
            if (m_tracker != nullptr)
            {
                setTrackedSkeletons(newSkeletonFrame);
            }
            ResetEvent(m_nextSkeletonFrame);
            m_skeletonFrame = newSkeletonFrame;
        }
        // interaction
        if (WaitForSingleObject(m_nextInteractionFrame,0) == WAIT_OBJECT_0)
        {
            m_interactionStream->GetNextFrame(INFINITE, &newInteractionFrame);
            processInteractionFrame(newInteractionFrame);
            m_interactionFrame = newInteractionFrame;
            ResetEvent(m_nextInteractionFrame);
        }


    }
    return;
}

void KinectInterface::setTrackedSkeletons(NUI_SKELETON_FRAME &newSkeletonFrame)
{
    DWORD *skeletons = m_tracker->choseTrackedSkeletons(newSkeletonFrame);
    m_kinect->NuiSkeletonSetTrackedSkeletons(skeletons);
    delete skeletons;
    return;
}

void KinectInterface::processInteractionFrame(NUI_INTERACTION_FRAME &newInteractionFrame)
{
    
    for (int i = 0; i < NUI_SKELETON_COUNT; i++)
    {
        NUI_USER_INFO user = newInteractionFrame.UserInfos[i];
        for (int j = 0; j < NUI_USER_HANDPOINTER_COUNT; j++)
        {
            if (user.HandPointerInfos[j].State & NUI_HANDPOINTER_STATE_INTERACTIVE
                && user.HandPointerInfos[j].State & NUI_HANDPOINTER_STATE_TRACKED)
            {
                KinectEvent newEvent;
                newEvent.type = KinectEvent::HandCursorMoved;
                newEvent.hand.id = user.SkeletonTrackingId;
                newEvent.hand.position.x = user.HandPointerInfos[j].X*m_targetImgWidth;
                newEvent.hand.position.y = user.HandPointerInfos[j].Y*m_targetImgHeight;
                addEventToStack(newEvent);
            
                // gripEvent
                if (user.HandPointerInfos[j].HandEventType == NUI_HAND_EVENT_TYPE_GRIP)
                {
                    KinectEvent newEvent;
                    newEvent.type = KinectEvent::HandGrip;
                    newEvent.handGrip.id = user.SkeletonTrackingId;
                    newEvent.handGrip.position.x = user.HandPointerInfos[j].X*m_targetImgWidth;
                    newEvent.handGrip.position.y = user.HandPointerInfos[j].Y*m_targetImgHeight;
                    addEventToStack(newEvent);
                }
                // releaseEvent
                if (user.HandPointerInfos[j].HandEventType == NUI_HAND_EVENT_TYPE_GRIPRELEASE)
                {
                    KinectEvent newEvent;
                    newEvent.type = KinectEvent::HandRelease;
                    newEvent.handRelease.id = user.SkeletonTrackingId;
                    newEvent.handRelease.position.x = user.HandPointerInfos[j].X*m_targetImgWidth;
                    newEvent.handRelease.position.y = user.HandPointerInfos[j].Y*m_targetImgHeight;
                    addEventToStack(newEvent);
                }
                // pushEvent
                if (user.HandPointerInfos[j].State & NUI_HANDPOINTER_STATE_PRESSED)
                {
                    KinectEvent newEvent;
                    newEvent.type = KinectEvent::HandPush;
                    newEvent.handPush.id = user.SkeletonTrackingId;
                    addEventToStack(newEvent);
                }
            }
        }
    }
    return;
}

void KinectInterface::processDepthFrame(NUI_IMAGE_FRAME &newDepthFrame)
{
    /// \todo compléter le traitement pour faire autre chose que les interactions.
    BOOL tmp = 1;
    INuiFrameTexture *texture;
    m_kinect->NuiImageFrameGetDepthImagePixelFrameTexture(m_depthStream, &newDepthFrame, &tmp, &texture);
    NUI_LOCKED_RECT rect;
    texture->LockRect(0, &rect, NULL, 0);
    HRESULT hr = m_interactionStream->ProcessDepth(rect.size, rect.pBits, newDepthFrame.liTimeStamp);
    newDepthFrame.pFrameTexture->UnlockRect(0);
    return;
}

void KinectInterface::procesSkeletonFrame(NUI_SKELETON_FRAME &newSkeletonFrame)
{
    // interaction
    Vector4 acc;
    m_kinect->NuiAccelerometerGetCurrentReading(&acc);
    HRESULT hr = m_interactionStream->ProcessSkeleton(NUI_SKELETON_COUNT, newSkeletonFrame.SkeletonData, &acc, newSkeletonFrame.liTimeStamp);
    for (int i = 0; i < NUI_SKELETON_COUNT; i++)
    {
        NUI_SKELETON_DATA data = newSkeletonFrame.SkeletonData[i];
        if (data.eTrackingState == NUI_SKELETON_POSITION_ONLY && moved(data))
        {
            // création d'un nouvel évènement 
            KinectEvent newEvent;
            newEvent.type = KinectEvent::UserMoved;
            newEvent.user.id = data.dwTrackingID;
            // on convertit les coordonnées squelettes en coordonnées images
            NuiTransformSkeletonToDepthImage(data.Position, &newEvent.user.position.x, &newEvent.user.position.y,
                NUI_IMAGE_RESOLUTION_640x480);
            // on adapte les coordonnées à l'image cible (entre 0 et 1 si elle n'est pas précisée)
            newEvent.user.position.x = newEvent.user.position.x / 640 * m_targetImgWidth;
            newEvent.user.position.y = newEvent.user.position.y / 480 * m_targetImgHeight;
            // on ajoute le nouvel évènement à la pile ou au buffer (géré par la fonction)
            addEventToStack(newEvent);
        }
        else if (data.eTrackingState == NUI_SKELETON_TRACKED)
        {
            // qualité de la détection
            // on crée l'évènement et parcourt le détail pour le remplir
            // si tout est parfait, on n'ajoute pas l'évènement à la pile
            KinectEvent newEvent;
            newEvent.type = KinectEvent::CannotSeeUser;
            newEvent.quality.state = data.dwQualityFlags;
            bool perfectQuality(true);
            for (int n = 0; n < NUI_SKELETON_POSITION_COUNT; n++)
            {
                newEvent.quality.jointstates[n] = data.eSkeletonPositionTrackingState[n];
                if (data.eSkeletonPositionTrackingState[n] != NUI_SKELETON_POSITION_TRACKED)
                {
                    perfectQuality = false;
                }
            }
            // ajout à la pile d'évènement
            if (!perfectQuality)
            {
                addEventToStack(newEvent);
            }

            // nouvel évènement SkeletonMoved
            if (moved(data))
            {
                KinectEvent newEvent;
                newEvent.type = KinectEvent::SkeletonMoved;
                newEvent.skeleton.id = data.dwTrackingID;
                for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
                {
                    // on convertit les coordonnées squelettes en coordonnées images
                    NuiTransformSkeletonToDepthImage(data.SkeletonPositions[j], &newEvent.skeleton.positions[j].x,
                        &newEvent.skeleton.positions[j].y, NUI_IMAGE_RESOLUTION_640x480);
                    // on adapte les coordonnées à l'image cible
                    newEvent.skeleton.positions[i].x = newEvent.skeleton.positions[i].x / 640 * m_targetImgWidth;
                    newEvent.skeleton.positions[i].y = newEvent.skeleton.positions[i].y / 480 * m_targetImgHeight;
                }
                
                // ajout à la pile d'évènements
                addEventToStack(newEvent);
                // position
                PositionType newpos = getPosition(data);
                KinectEvent newPosEvent;
                newPosEvent.type = KinectEvent::PositionChanged;
                newPosEvent.position.id = data.dwTrackingID;
                newPosEvent.position.type = newpos;
                addEventToStack(newPosEvent);
            }
        }
    }
    return;
}

PositionType KinectInterface::getPosition(NUI_SKELETON_DATA &data)
{
    const float PI = std::acos(-1);
    auto pos = data.SkeletonPositions;
    const int O = 0, BG(1), BD(2), JG(3), JD(4);
    Vector4 vectors[7];
    /* correspondance indice/axe
    0 : axe central (hanches -> tête)
    1 : bras gauche (épaule -> poignet)
    2 : bras droit (idem)
    3 : jambe gauche (hanche -> cheville)
    4 : jambe droite (idem)
    5 : avant-bras gauche (coude -> main)
    6 : avant-bras droit (idem)
    */
    // calcul des vecteurs
    // centre
    vectors[0].x = pos[SKELETON_SHOULDER_CENTER].x - pos[SKELETON_HIP_CENTER].x;
    vectors[0].y = pos[SKELETON_SHOULDER_CENTER].y - pos[SKELETON_HIP_CENTER].y;
    vectors[0].w = std::sqrt(std::pow(vectors[0].x, 2) + pow(vectors[0].y, 2)); //norme
    // bras gauche
    vectors[1].x = pos[SKELETON_WRIST_LEFT].x - pos[SKELETON_SHOULDER_LEFT].x;
    vectors[1].y = pos[SKELETON_WRIST_LEFT].y - pos[SKELETON_SHOULDER_LEFT].y;
    vectors[1].w = std::sqrt(std::pow(vectors[1].x, 2) + pow(vectors[1].y, 2));
    // bras droit
    vectors[2].x = pos[SKELETON_WRIST_RIGHT].x - pos[SKELETON_SHOULDER_RIGHT].x;
    vectors[2].y = pos[SKELETON_WRIST_RIGHT].y - pos[SKELETON_SHOULDER_RIGHT].y;
    vectors[2].w = std::sqrt(std::pow(vectors[2].x, 2) + pow(vectors[2].y, 2));
    //jambe gauche
    vectors[3].x = pos[SKELETON_ANKLE_LEFT].x - pos[SKELETON_HIP_LEFT].x;
    vectors[3].y = pos[SKELETON_ANKLE_LEFT].y - pos[SKELETON_HIP_LEFT].y;
    vectors[3].w = std::sqrt(std::pow(vectors[3].x, 2) + pow(vectors[3].y, 2));
    // jambe droite
    vectors[4].x = pos[SKELETON_ANKLE_RIGHT].x - pos[SKELETON_HIP_RIGHT].x;
    vectors[4].y = pos[SKELETON_ANKLE_RIGHT].y - pos[SKELETON_HIP_RIGHT].y;
    vectors[4].w = std::sqrt(std::pow(vectors[4].x, 2) + pow(vectors[4].y, 2));
    // avant bras gauche
    vectors[5].x = pos[SKELETON_WRIST_LEFT].x - pos[SKELETON_ELBOW_LEFT].x;
    vectors[5].y = pos[SKELETON_WRIST_LEFT].y - pos[SKELETON_ELBOW_LEFT].y;
    vectors[5].w = std::sqrt(std::pow(vectors[5].x, 2) + std::pow(vectors[5].y, 2));
    // avant bras droit
    vectors[6].x = pos[SKELETON_WRIST_RIGHT].x - pos[SKELETON_ELBOW_RIGHT].x;
    vectors[6].y = pos[SKELETON_WRIST_RIGHT].y - pos[SKELETON_ELBOW_RIGHT].y;
    vectors[6].w = std::sqrt(std::pow(vectors[6].x, 2) + std::pow(vectors[6].y, 2));
    float angles[7];
    /* correspondance indice/angle (tous par rapport à la verticale:
    0 : axe central (hanches -> tête)
    1 : bras gauche (épaule -> poignet)
    2 : bras droit (idem)
    3 : jambe gauche (hanche -> cheville)
    4 : jambe droite (idem)
    5 : avant-bras gauche (coude -> main)
    6 : avant-bras droit (idem)
    */
    float pdtvecY(0),pdtvecX(0);
    for (int i = 0; i < 7; i++)
    {
        // l'angle est calculé via le produit vectoriel
        // u.v = |u|*|v|*cos(a) = u.x*v.x + u.y*v.y => a = arccos(pdtvec/(|u|*|v|))

        // pour obtenir un angle entre 0 et 3.14 rad, on calcule deux orientation par membre : celle par rapport à la verticale
        // et une seconde par rapport à l'horizontale. Ensuite, on identifie le quadrant dans lequel on se trouve pour déterminer
        // l'angle

        // L'origine du cercle trigo est à droite de l'utilisateur lorsqu'il fait face à la Kinect. Le sens trigo va de sa droite
        // vers sa gauche

        // produit vectoriel par rapport à la verticale
        pdtvecY = vectors[i].y;
        // par rapport à l'horizontale
        pdtvecX = vectors[i].x;
        // orientations par rapport aux axes
        float aX = std::acos(pdtvecX / vectors[i].w);
        float aY = std::acos(pdtvecY / vectors[i].w);
        //reconnaissance du quadrant
        if (aY < PI/2) //deux premiers cadrants
        {
            angles[i] = aX;
        }
        else if (aY > PI/2 && aX > PI / 2)//troisième quadrant
        {
            angles[i] = 2*PI-aX;
        }
        else if (aY > PI/2 && aX < PI/2)//4eme quadrant
        {
            angles[i] = 2*PI-aX;
        }
    }
    //debug
    /*std::cout << "angles :\n";
    for (int i = 0; i < 7; i++)
    {
        std::cout << "angles[" << i << "] = " << angles[i] << std::endl;
        if (i == 6)
            std::cout << std::endl;
    }*/

    // maintenant que les angles sont calculés, il faut encore reconnaître la position
    PositionType position(PositionType::UNKNOWN);
    // on considère qu'un angle a atteint une valeur s'il vaut cette valeur +- 20° (0.34 radians).
    // on teste l'égalité à 0 et à 2PI séparément (l'une ou l'autre)
    if (std::abs(angles[0]-PI/2) < 0.34 && std::abs(angles[1]-PI) < 0.34 && (std::abs(angles[2]-0) < 0.34 
        || std::abs(angles[2] - 2*PI) < 0.34) && std::abs(angles[3]-3*PI/2) < 0.34 && std::abs(angles[4]-3*PI/2) < 0.34)
    { 
        position = PositionType::T;
    }
    else if (std::abs(angles[0]-PI/2) < 0.34 && std::abs(angles[1]-PI/2) < 0.34 && std::abs(angles[2]-PI/2) < 0.34 
        && std::abs(angles[3]-3*PI/2) < 0.34 && std::abs(angles[4]-3*PI/2) < 0.34)
    {
        position = PositionType::Y;
    }
    else if (std::abs(angles[0]-PI/2) < 0.34 && std::abs(angles[5]-PI/4) < 0.34 && std::abs(angles[6]-3*PI/4) < 0.34 
        && std::abs(angles[3]-3*PI/2) < 0.34 && std::abs(angles[4]-3*PI/2) < 0.34)
    {
        position = PositionType::STOP;
    }
    return position;
}

bool KinectInterface::pollEvent(KinectEvent &event)
{
    bool stackEmpty(m_events.empty());
    if (!stackEmpty)
    {
        
        m_eventAccess.is_locked_by_caller() ? 0: m_eventAccess.lock();
        event = m_events[0];
        m_events.erase(m_events.begin());
    }
    else
    {
        m_eventAccess.is_locked_by_caller() ? m_eventAccess.unlock() : 0;
    }
    return !stackEmpty;
}

void KinectInterface::addEventToStack(KinectEvent &newEvent) 
{
    if (m_eventAccess.try_lock())
    {
        // d'abord vérifier que la pile tampon ne contient rien
        // si elle n'est pas vide, on ajoute son contenu avant l'évènement courant
        if (!m_eventsBuffer.empty())
        {
            m_events.insert(m_events.end(), m_eventsBuffer.begin(), m_eventsBuffer.end());
            m_eventsBuffer.clear();
        }
        // ensuite on peut ajouter l'évènement
        m_events.push_back(newEvent);
        m_eventAccess.unlock();
    }
    else
    {
        m_eventsBuffer.push_back(newEvent);
    }
    return;
}

bool KinectInterface::moved(NUI_SKELETON_DATA &newData)const
{
    bool moved(false);
    switch (newData.eTrackingState)
    {
    case NUI_SKELETON_POSITION_ONLY:
        // retrouver l'utilisateur correspondant dans la trame précédente
        for (int i = 0; i < NUI_SKELETON_COUNT; i++)
        {
            NUI_SKELETON_DATA data = m_skeletonFrame.SkeletonData[i];
            // vérifier s'il a bougé (on valide les mouvements les plus petits)
            if (data.dwTrackingID == newData.dwTrackingID)
            {
                if (newData.Position.x != data.Position.x || newData.Position.y != data.Position.y
                    || newData.Position.z != data.Position.z)
                {
                    moved = true;
                }
            }
        }
    case NUI_SKELETON_TRACKED:
        // retrouver le squelette correspondant
        for (int i = 0; i < NUI_SKELETON_COUNT; i++)
        {
            NUI_SKELETON_DATA data = m_skeletonFrame.SkeletonData[i];
            // on vérifie s'il y a eu mouvement pour chaque articulation
            for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
            {
                if (newData.SkeletonPositions[j].x != data.SkeletonPositions[j].x
                    || newData.SkeletonPositions[j].y != data.SkeletonPositions[j].y
                    || newData.SkeletonPositions[j].z != data.SkeletonPositions[j].z)
                {
                    moved |= true;
                }
            }
        }
    }
    return moved;
}

void KinectInterface::setTrackingMethod(SkeletonTracker *tracker)
{
    m_trackerAccess.lock();
    m_tracker = tracker;
    m_skeletonTrackingState = (tracker == nullptr ? trinary::RESET : trinary::SET);
    m_trackerAccess.unlock();
    return;
}

void KinectInterface::setTargetImageSize(unsigned int width, unsigned int height)
{
    m_targetImgWidth = width;
    m_targetImgHeight = height;
    return;
}

void KinectInterface::start(void)
{
    m_processingThread = std::thread(&KinectInterface::processFrames, this);
    return;
}

void KinectInterface::stop(void)
{
    m_stopProcessing = true; //signifie au thread effectuant processFrames qu'il doit s'arrêter
    // on attend ensuite que le thread ait terminé (pour avoir la certitude qu'il a fini à la sortie de stop)
    m_processingThread.joinable() ? m_processingThread.join() : 0;
    return;
}

void KinectInterface::setOrientation(long command)
{
    if (command > NUI_CAMERA_ELEVATION_MINIMUM && command < NUI_CAMERA_ELEVATION_MAXIMUM)
    {
        m_orientation = command;
    }
    return;
}

struct Bitmap_data_size {
    char *data;
    int size;
};


struct Bitmap_data_size encode_bitmap(unsigned char bits[], int size, int bitsperpixel, int width, int height, char name[])
{
    BITMAPFILEHEADER header;
    BITMAPINFOHEADER info;
    int bytesperpixel = bitsperpixel / 8;
    /*int extrabytes_nb = 4 - (bytesperpixel * width) % 4;
    int paddedsize = (width*bytesperpixel + extrabytes_nb) * height;
    */
    header.bfType = 0x4D42;
    header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width*height*bytesperpixel;
    header.bfReserved1 = 0;
    header.bfReserved2 = 0;
    header.bfOffBits = 54;

    info.biSize = sizeof(BITMAPINFOHEADER);
    info.biWidth = width;
    info.biHeight = -height;
    info.biPlanes = 1;
    info.biBitCount = bitsperpixel;
    info.biCompression = 0;
    info.biSizeImage = width*height*bytesperpixel;
    info.biXPelsPerMeter = 2835;
    info.biYPelsPerMeter = 2835;
    info.biClrUsed = 0;
    info.biClrImportant = 0;

    struct Bitmap_data_size result;
    result.data = new char[header.bfSize + info.biSizeImage];
    result.size = header.bfSize + info.biSizeImage;

    memcpy(result.data, (char*)&header, sizeof(BITMAPFILEHEADER));
    memcpy(result.data + sizeof(BITMAPFILEHEADER), &info, info.biSize);
    memcpy(result.data + header.bfSize, bits, size);

    std::ofstream output;
    output.open(name, std::ofstream::out | std::ofstream::binary);
    output.write((char*)&header, sizeof(BITMAPFILEHEADER));
    output.write((char*)&info, sizeof(BITMAPINFOHEADER));

    output.write((char*)bits, info.biSizeImage);

    output.close();
    return result;
}
} // namespace KtI
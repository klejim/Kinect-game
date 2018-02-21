#ifndef KINECTEVENT_HPP
#define KINECTEVENT_HPP
#pragma once
namespace KtI {
    namespace NUI {
#include <Windows.h>
#include "NuiApi.h"
    }
}
#include "KinectInterface.hpp"


/// \brief Regroupe ce qui appartient au module KinectInterface
/// \todo classe KinectInterface.
namespace KtI
{

    /// \brief Définit un système d'évènements générés par le traitement des données envoyées par la Kinect.
    class KinectEvent
    {
    private:
        /// \brief Informations sur l'évènement CannotSeeUser.
        struct CannotSeeUserEvent
        {
            unsigned long id;		///< L'identifiant du squelette concerné.
            unsigned long state;	///< Combinaison de SkeletonQuality flags décrivant la qualité globale de l'information reçue.
            unsigned long jointstates[SKELETON_JOINTS_COUNT];	///< Tableau de SkeletonQuality flags décrivant la qualité de chacun des points.
        };
        /// \brief Informations sur l'évènement SkeletonMoved.
        struct SkeletonMovedEvent
        {
            unsigned long id;	///< L'identifiant du squelette concerné (attribut dwTrackingId de la structure NUI_SKELETON_DATA). 
            struct
            {
                float x;
                float y;
            } positions[SKELETON_JOINTS_COUNT]; ///< La nouvelle position du squelette en coordonnées correspondant à une image de 640*480 pixels. 
        };

        /// \brief Informations sur l'évènement UserMoved.
        struct UserMovedEvent
        {
            unsigned long id;	///< L'identifiant de l'utilisateur concerné (attribué par le capteur).
            struct
            {
                float x;
                float y;
            } position; ///< La nouvelle position de l'utilisateur en coordonnées correspondant à une image de 640x480 pixels.
        };

        ///\brief Informations sur l'évènement HandCursorMoved
        struct HandCursorMovedEvent
        {
            unsigned long id;	///< L'identifiant du squelette auquel la main appartient.
            struct 
            {
                float x;
                float y;
            } position;		    ///< La position de la main, relative à la fenêtre.
        };

        /// \brief Informations sur l'évènement HandGrip.
        struct HandGripEvent
        {
            unsigned long id;	///< L'identifiant du squelette auquel la main appartient.
            struct 
            {
                float x;
                float y;
            } position;			///< La position relative de la main dans la zone d'interaction. Les valeurs varient entre 0 et 1.
        };
        /// \brief Informations sur l'évènement HandRelease.
        struct HandReleaseEvent
        {
            unsigned long id;	///< L'identifiant du squelette auquel la main appartient.
            struct
            {
                float x;
                float y;
            } position;			///< La position relative de la main dans la zone d'interaction. Les valeurs varient entre 0 et 1.
        };

        /// \brief Informations sur l'évènement HandPush.
        struct HandPushEvent
        {
            unsigned long id;   ///< L'identifiant du squelette auquel la main appartient.
        };

        /// \brief Informations sur l'évènement PositionChanged.
        struct PositionChangedEvent
        {
            unsigned long id;	///< L'identifiant du squelette concerné (attribut dwTrackingId de la structure NUI_SKELETON_DATA).
            PositionType type;	///< La nouvelle position du squelette.
                                ///< Les angles de chaque axe.

        };

    public:
        /// \enum KinectEventType
        /// \brief Enumération contenant les différentes catégories d'évènements reconnus.
        enum KinectEventType
        {
            CannotSeeUser,	   ///< L'utilisateur est au moins partiellement caché (par l'environnement ou parce qu'il est sorti de la zone de reconnaissance).
            SkeletonMoved,     ///< Un squelette a bougé entre deux frames.
            UserMoved,		   ///< Un utilisateur a bougé (les utilisateurs sont les personnes traquées partiellement par la Kinect).
            HandCursorMoved,   ///< La main d'un squelette a bougée.
            HandGrip,		   ///< La main d'un squelette s'est fermée.
            HandRelease,	   ///< La main d'un squelette s'est ouverte.
            HandPush,		   ///< Un squelette a poussé sa main vers l'avant.
            PositionChanged,   ///< La position d'un squelette a changée (positions précises reconnues).
        };
        // membres de la classe
        KinectEventType type; ///< Type d'évènement.
        union
        {
            CannotSeeUserEvent quality;		///< Informations sur la qualité des données reçues sur un squelette (KinectEvent::CannotSeeUser).
            SkeletonMovedEvent skeleton;	///< Paramètres du déplacement (KinectEvent::SkeletonMoved).
            UserMovedEvent user;			///< Paramètres du déplacement (KinectEvent::UserMoved).
            HandCursorMovedEvent hand;		///< Paramètres du déplacement de la main.
            HandGripEvent handGrip;		    ///< Paramètres de l'évènement HandGrip.
            HandReleaseEvent handRelease;	///< Paramètres de l'évènement HandRelease.
            HandPushEvent handPush;			///< Paramètres de l'évènement HandPush.
            PositionChangedEvent position;	///< Informations sur la nouvelle position (KinectEvent::PositionChanged).
        };
    };

} // namespace KtI
  

#endif // KINECTEVENT_HPP

  /// \class KtI::KinectEvent
  /// \ingroup kinectgroup
  ///
  /// Une classe contenant les types d'évènements reconnus par la classe KinectInterface par comparaison des frames de données.
  /// La méthode KtI::KtI::KinectInterface::pollEvent permet de récupérer tous les éléments qui ont eu lieu entre le dernier appel à
  /// pollEvent et celui-ci. 
  /// 
  /// Un évènement est caractérisé par son type et des paramètres le précisant. 
  /// \warning Les paramètres sont stockés dans une union et il est donc important de ne lire que le champ correspondant au type
  /// d'évènement, car seul ce dernier sera correctement rempli.
  ///
  /// Ce système d'évènement est volontairement conçu pour être semblable à ceux utilisés par des bibliothèques graphiques telles que
  /// la SDL ou la SFML (qui est utilisée dans le projet). Un exemple d'utilisation serait:
  /// \code
  /// KtI::KinectEvent Kevent;
  /// while (KInterface.pollEvent(Kevent)
  /// {
  ///     if (Kevent.type == KtI::KinectEvent::UserMoved){
  ///         //mettre à jour la position de l'utilisateur
  ///         auto id = event.user.id;
  ///         users[id].position = Kevent.user.position;//users étant une liste indexée par l'id des utilisateurs
  ///     }
  /// }
  /// \endcode

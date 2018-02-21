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

    /// \brief D�finit un syst�me d'�v�nements g�n�r�s par le traitement des donn�es envoy�es par la Kinect.
    class KinectEvent
    {
    private:
        /// \brief Informations sur l'�v�nement CannotSeeUser.
        struct CannotSeeUserEvent
        {
            unsigned long id;		///< L'identifiant du squelette concern�.
            unsigned long state;	///< Combinaison de SkeletonQuality flags d�crivant la qualit� globale de l'information re�ue.
            unsigned long jointstates[SKELETON_JOINTS_COUNT];	///< Tableau de SkeletonQuality flags d�crivant la qualit� de chacun des points.
        };
        /// \brief Informations sur l'�v�nement SkeletonMoved.
        struct SkeletonMovedEvent
        {
            unsigned long id;	///< L'identifiant du squelette concern� (attribut dwTrackingId de la structure NUI_SKELETON_DATA). 
            struct
            {
                float x;
                float y;
            } positions[SKELETON_JOINTS_COUNT]; ///< La nouvelle position du squelette en coordonn�es correspondant � une image de 640*480 pixels. 
        };

        /// \brief Informations sur l'�v�nement UserMoved.
        struct UserMovedEvent
        {
            unsigned long id;	///< L'identifiant de l'utilisateur concern� (attribu� par le capteur).
            struct
            {
                float x;
                float y;
            } position; ///< La nouvelle position de l'utilisateur en coordonn�es correspondant � une image de 640x480 pixels.
        };

        ///\brief Informations sur l'�v�nement HandCursorMoved
        struct HandCursorMovedEvent
        {
            unsigned long id;	///< L'identifiant du squelette auquel la main appartient.
            struct 
            {
                float x;
                float y;
            } position;		    ///< La position de la main, relative � la fen�tre.
        };

        /// \brief Informations sur l'�v�nement HandGrip.
        struct HandGripEvent
        {
            unsigned long id;	///< L'identifiant du squelette auquel la main appartient.
            struct 
            {
                float x;
                float y;
            } position;			///< La position relative de la main dans la zone d'interaction. Les valeurs varient entre 0 et 1.
        };
        /// \brief Informations sur l'�v�nement HandRelease.
        struct HandReleaseEvent
        {
            unsigned long id;	///< L'identifiant du squelette auquel la main appartient.
            struct
            {
                float x;
                float y;
            } position;			///< La position relative de la main dans la zone d'interaction. Les valeurs varient entre 0 et 1.
        };

        /// \brief Informations sur l'�v�nement HandPush.
        struct HandPushEvent
        {
            unsigned long id;   ///< L'identifiant du squelette auquel la main appartient.
        };

        /// \brief Informations sur l'�v�nement PositionChanged.
        struct PositionChangedEvent
        {
            unsigned long id;	///< L'identifiant du squelette concern� (attribut dwTrackingId de la structure NUI_SKELETON_DATA).
            PositionType type;	///< La nouvelle position du squelette.
                                ///< Les angles de chaque axe.

        };

    public:
        /// \enum KinectEventType
        /// \brief Enum�ration contenant les diff�rentes cat�gories d'�v�nements reconnus.
        enum KinectEventType
        {
            CannotSeeUser,	   ///< L'utilisateur est au moins partiellement cach� (par l'environnement ou parce qu'il est sorti de la zone de reconnaissance).
            SkeletonMoved,     ///< Un squelette a boug� entre deux frames.
            UserMoved,		   ///< Un utilisateur a boug� (les utilisateurs sont les personnes traqu�es partiellement par la Kinect).
            HandCursorMoved,   ///< La main d'un squelette a boug�e.
            HandGrip,		   ///< La main d'un squelette s'est ferm�e.
            HandRelease,	   ///< La main d'un squelette s'est ouverte.
            HandPush,		   ///< Un squelette a pouss� sa main vers l'avant.
            PositionChanged,   ///< La position d'un squelette a chang�e (positions pr�cises reconnues).
        };
        // membres de la classe
        KinectEventType type; ///< Type d'�v�nement.
        union
        {
            CannotSeeUserEvent quality;		///< Informations sur la qualit� des donn�es re�ues sur un squelette (KinectEvent::CannotSeeUser).
            SkeletonMovedEvent skeleton;	///< Param�tres du d�placement (KinectEvent::SkeletonMoved).
            UserMovedEvent user;			///< Param�tres du d�placement (KinectEvent::UserMoved).
            HandCursorMovedEvent hand;		///< Param�tres du d�placement de la main.
            HandGripEvent handGrip;		    ///< Param�tres de l'�v�nement HandGrip.
            HandReleaseEvent handRelease;	///< Param�tres de l'�v�nement HandRelease.
            HandPushEvent handPush;			///< Param�tres de l'�v�nement HandPush.
            PositionChangedEvent position;	///< Informations sur la nouvelle position (KinectEvent::PositionChanged).
        };
    };

} // namespace KtI
  

#endif // KINECTEVENT_HPP

  /// \class KtI::KinectEvent
  /// \ingroup kinectgroup
  ///
  /// Une classe contenant les types d'�v�nements reconnus par la classe KinectInterface par comparaison des frames de donn�es.
  /// La m�thode KtI::KtI::KinectInterface::pollEvent permet de r�cup�rer tous les �l�ments qui ont eu lieu entre le dernier appel �
  /// pollEvent et celui-ci. 
  /// 
  /// Un �v�nement est caract�ris� par son type et des param�tres le pr�cisant. 
  /// \warning Les param�tres sont stock�s dans une union et il est donc important de ne lire que le champ correspondant au type
  /// d'�v�nement, car seul ce dernier sera correctement rempli.
  ///
  /// Ce syst�me d'�v�nement est volontairement con�u pour �tre semblable � ceux utilis�s par des biblioth�ques graphiques telles que
  /// la SDL ou la SFML (qui est utilis�e dans le projet). Un exemple d'utilisation serait:
  /// \code
  /// KtI::KinectEvent Kevent;
  /// while (KInterface.pollEvent(Kevent)
  /// {
  ///     if (Kevent.type == KtI::KinectEvent::UserMoved){
  ///         //mettre � jour la position de l'utilisateur
  ///         auto id = event.user.id;
  ///         users[id].position = Kevent.user.position;//users �tant une liste index�e par l'id des utilisateurs
  ///     }
  /// }
  /// \endcode

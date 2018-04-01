// =---------------------------------------------------------------------------
// n o t i f i c a t i o n _ m a n a g e r
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    Whenever anyone needs to be notified of something, a request is made
//    to an instance of this class, notification_manager.
//
//    Generally, notifications are asynchronous and don't require an
//    immediate acknowledgement.
//
//    For instance, suppose a group needs to be deleted. All clients in
//    that group need to be notified that their group is being pulled
//    out from underneath them. A vector of notify_recipients is created
//    and combined with a notify_message. This message is then queued for
//    transmission.
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Jun 15   nic   Created.
//


#ifdef __NOTIFICATION_MANAGER_H__
#else __NOTIFICATION_MANAGER_H__

class notification_manager
{
public:
   notification_manager();
   ~ notification_manager();

protected:
   queue<package> notify_queue;
};

#endif

/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "types.h"
#include "connection.h"

Connection::Connection() 
{
}

Connection::~Connection(void)
{
}

void
Connection::Init(Poll *poll, Callbacks *callbacks, GGPOConnection* ggpo_connection)
{
   _callbacks = callbacks;
   _ggpo_connection = ggpo_connection;
   _poll = poll;
   _poll->RegisterLoop(this);
}

void
Connection::SendTo(char *buffer, int len, int flags, int player_num)
{
    _ggpo_connection->send_to(_ggpo_connection->instance, buffer, len, flags, player_num);
}

bool
Connection::OnLoopPoll(void *cookie)
{
   uint8          recv_buf[MAX_CONNECTION_PACKET_SIZE];

   for (;;) {
      int player_id = -1;
      int len = _ggpo_connection->receive_from(_ggpo_connection->instance, (char*)recv_buf, MAX_CONNECTION_PACKET_SIZE, 0, &player_id);
      // TODO: handle len == 0... indicates a disconnect.

      if (len == -1) {
         break;
      } else if (len > 0) {

         Log("recvfrom returned (len:%d  from player: %d).\n", len, player_id );
         if (recv_buf[0] == MsgType::GgpoMessage) {

             uint8* ggpoMsg = new uint8[len - 1];
             memcpy(ggpoMsg, recv_buf + 1, len - 1);
             ConnectionMsg* msg = (ConnectionMsg*)ggpoMsg;
             _callbacks->OnMsg(player_id, msg, len - 1);
             delete ggpoMsg;
         }
         else if (recv_buf[0] == MsgType::SyncState) {
             int frame = 0;
             memcpy(&frame, recv_buf + 1, sizeof(int));
             auto msg = recv_buf + 1 + sizeof(int);
             _callbacks->OnResnc(frame, msg ,len- 1 - sizeof(int));
         }
      } 
   }
   return true;
}


void
Connection::Log(const char *fmt, ...)
{
   char buf[1024];
   size_t offset;
   va_list args;

   strcpy_s(buf, "connection | ");
   offset = strlen(buf);
   va_start(args, fmt);
   vsnprintf(buf + offset, ARRAY_SIZE(buf) - offset - 1, fmt, args);
   buf[ARRAY_SIZE(buf)-1] = '\0';
   ::Log(buf);
   va_end(args);
}

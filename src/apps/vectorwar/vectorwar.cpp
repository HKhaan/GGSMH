#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <math.h>
#include <stdio.h>

#include "vectorwar.h"
#include "ggpo_perfmon.h"
#include "udp_connection.h"

//#define SYNC_TEST    // test: turn on synctest
#define MAX_PLAYERS     64


/* 
 * Simple checksum function stolen from wikipedia:
 *
 *   http://en.wikipedia.org/wiki/Fletcher%27s_checksum
 */

int
fletcher32_checksum(short *data, size_t len)
{
   int sum1 = 0xffff, sum2 = 0xffff;

   while (len) {
      size_t tlen = len > 360 ? 360 : len;
      len -= tlen;
      do {
         sum1 += *data++;
         sum2 += sum1;
      } while (--tlen);
      sum1 = (sum1 & 0xffff) + (sum1 >> 16);
      sum2 = (sum2 & 0xffff) + (sum2 >> 16);
   }

   /* Second reduction step to reduce sums to 16 bits */
   sum1 = (sum1 & 0xffff) + (sum1 >> 16);
   sum2 = (sum2 & 0xffff) + (sum2 >> 16);
   return sum2 << 16 | sum1;
}

/*
 * vw_begin_game_callback --
 *
 * The begin game callback.  We don't need to do anything special here,
 * so just return true.
 */
bool __cdecl
vw_begin_game_callback(VectorWar* self, const char *)
{
   return true;
}

/*
 * vw_on_event_callback --
 *
 * Notification from GGPO that something has happened.  Update the status
 * text at the bottom of the screen to notify the user.
 */
bool __cdecl
vw_on_event_callback(VectorWar* self, GGPOEvent *info)
{
   int progress;
   switch (info->code) {
   case GGPO_EVENTCODE_CONNECTED_TO_PEER:
      self->ngs.SetConnectState(info->u.connected.player, Synchronizing);
      break;
   case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
      progress = 100 * info->u.synchronizing.count / info->u.synchronizing.total;
      self->ngs.UpdateConnectProgress(info->u.synchronizing.player, progress);
      break;
   case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
      self->ngs.UpdateConnectProgress(info->u.synchronized.player, 100);
      break;
   case GGPO_EVENTCODE_RUNNING:
      self->ngs.SetConnectState(Running);
      self->renderer->SetStatusText("");
      break;
   case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
      self->ngs.SetDisconnectTimeout(info->u.connection_interrupted.player,
                               timeGetTime(),
                               info->u.connection_interrupted.disconnect_timeout);
      break;
   case GGPO_EVENTCODE_CONNECTION_RESUMED:
       self->ngs.SetConnectState(info->u.connection_resumed.player, Running);
      break;
   case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
      self->ngs.SetConnectState(info->u.disconnected.player, Disconnected);
      break;
   case GGPO_EVENTCODE_TIMESYNC:
      Sleep(1000 * info->u.timesync.frames_ahead / 60);
      break;
   }
   return true;
}


/*
 * vw_advance_frame_callback --
 *
 * Notification from GGPO we should step foward exactly 1 frame
 * during a rollback.
 */
bool __cdecl
vw_advance_frame_callback(VectorWar* self, int)
{
   int inputs[MAX_SHIPS] = { 0 };
   int disconnect_flags;

   // Make sure we fetch new inputs from GGPO and use those to update
   // the game state instead of reading from the keyboard.
   ggpo_synchronize_input(self->ggpo, (void *)inputs, sizeof(int) * MAX_SHIPS, &disconnect_flags);
   self->AdvanceFrame(inputs, disconnect_flags);
   return true;
}

/*
 * vw_load_game_state_callback --
 *
 * Makes our current state match the state passed in by GGPO.
 */
bool __cdecl
vw_load_game_state_callback(VectorWar* self, unsigned char *buffer, int len)
{
   memcpy(&self->gs, buffer, len);
   return true;
}

/*
 * vw_save_game_state_callback --
 *
 * Save the current state to a buffer and return it to GGPO via the
 * buffer and len parameters.
 */
bool __cdecl
vw_save_game_state_callback(VectorWar* self, unsigned char **buffer, int *len, int *checksum, int)
{
   *len = sizeof(self->gs);
   *buffer = (unsigned char *)malloc(*len);
   if (!*buffer) {
      return false;
   }
   memcpy(*buffer, &self->gs, *len);
   *checksum = fletcher32_checksum((short *)*buffer, *len / 2);
   return true;
}

/*
 * vw_log_game_state --
 *
 * Log the gamestate.  Used by the synctest debugging tool.
 */
bool __cdecl
vw_log_game_state(VectorWar* self, char *filename, unsigned char *buffer, int)
{
   FILE* fp = nullptr;
   fopen_s(&fp, filename, "w");
   if (fp) {
      GameState *gamestate = (GameState *)buffer;
      fprintf(fp, "GameState object.\n");
      fprintf(fp, "  bounds: %d,%d x %d,%d.\n", gamestate->_bounds.left, gamestate->_bounds.top,
              gamestate->_bounds.right, gamestate->_bounds.bottom);
      fprintf(fp, "  num_ships: %d.\n", gamestate->_num_ships);
      for (int i = 0; i < gamestate->_num_ships; i++) {
         Ship *ship = gamestate->_ships + i;
         fprintf(fp, "  ship %d position:  %.4f, %.4f\n", i, ship->position.x, ship->position.y);
         fprintf(fp, "  ship %d velocity:  %.4f, %.4f\n", i, ship->velocity.dx, ship->velocity.dy);
         fprintf(fp, "  ship %d radius:    %d.\n", i, ship->radius);
         fprintf(fp, "  ship %d heading:   %d.\n", i, ship->heading);
         fprintf(fp, "  ship %d health:    %d.\n", i, ship->health);
         fprintf(fp, "  ship %d speed:     %d.\n", i, ship->speed);
         fprintf(fp, "  ship %d cooldown:  %d.\n", i, ship->cooldown);
         fprintf(fp, "  ship %d score:     %d.\n", i, ship->score);
         for (int j = 0; j < MAX_BULLETS; j++) {
            Bullet *bullet = ship->bullets + j;
            fprintf(fp, "  ship %d bullet %d: %.2f %.2f -> %.2f %.2f.\n", i, j,
                    bullet->position.x, bullet->position.y,
                    bullet->velocity.dx, bullet->velocity.dy);
         }
      }
      fclose(fp);
   }
   return true;
}

/*
 * vw_free_buffer --
 *
 * Free a save state buffer previously returned in vw_save_game_state_callback.
 */
void __cdecl 
vw_free_buffer(VectorWar* self, void *buffer)
{
   free(buffer);
}


/*
 * VectorWar::Init --
 *
 * Initialize the vector war game.  This initializes the game state and
 * the video renderer and creates a new network session.
 */
void
VectorWar::Init(HWND hwnd, unsigned short localport, int num_players, GGPOPlayer *players, int num_spectators)
{
   GGPOErrorCode result;
   renderer = new GDIRenderer(hwnd);

   // Initialize the game state
   gs.Init(hwnd, num_players);
   ngs.num_players = num_players;

   // Fill in a ggpo callbacks structure to pass to start_session.
   GGPOSessionCallbacks cb = { 0 };
   cb.instance = this;
   cb.begin_game = [](void* self, const char* name) {return vw_begin_game_callback((VectorWar*)self,name); };
   cb.advance_frame = [](void* self, int flags) {return vw_advance_frame_callback((VectorWar*)self, flags); };
   cb.load_game_state = [](void* self, unsigned char* buffer, int len) {return vw_load_game_state_callback((VectorWar*)self, buffer, len); };
   cb.save_game_state = [](void* self, unsigned char** buffer, int* len, int* checksum, int frame) {return vw_save_game_state_callback((VectorWar*)self,buffer,len,checksum,frame); };
   cb.free_buffer = [](void* self, void* buffer) {return vw_free_buffer((VectorWar*)self,buffer); };
   cb.on_event = [](void* self, GGPOEvent* info) {return vw_on_event_callback((VectorWar*)self,info); };
   cb.log_game_state = [](void* self, char* filename, unsigned char* buffer, int len) {return vw_log_game_state((VectorWar*)self,filename,buffer,len); };
   UdpConnection* udp_connection =  new UdpConnection(localport);
   GGPOConnection* connection = udp_connection->get_ggpo_connection();
#if defined(SYNC_TEST)
   result = ggpo_start_synctest(&ggpo, &cb, "vectorwar", num_players, sizeof(int), 1);
#else
   result = ggpo_start_session(&ggpo, &cb, connection, "vectorwar", num_players, sizeof(int));
#endif
   // automatically disconnect clients after 3000 ms and start our count-down timer
   // for disconnects after 1000 ms.   To completely disable disconnects, simply use
   // a value of 0 for ggpo_set_disconnect_timeout.
   ggpo_set_disconnect_timeout(ggpo, 3000);
   ggpo_set_disconnect_notify_start(ggpo, 1000);

   int i;
   for (i = 0; i < num_players + num_spectators; i++) {
      GGPOPlayerHandle handle;
      if (players[i].type != GGPO_PLAYERTYPE_LOCAL) {
          players[i].player_id = udp_connection->add_connection(players[i].u.remote.ip_address, players[i].u.remote.port);
      }
      result = ggpo_add_player(ggpo, players + i, &handle);
      
      ngs.players[i].handle = handle;
      ngs.players[i].type = players[i].type;
      if (players[i].type == GGPO_PLAYERTYPE_LOCAL) {
         ngs.players[i].connect_progress = 100;
         ngs.local_player_handle = handle;
         ngs.SetConnectState(handle, Connecting);
         ggpo_set_frame_delay(ggpo, handle, FRAME_DELAY);
      } else {
         ngs.players[i].connect_progress = 0;
      }
   }

   ggpoutil_perfmon_init(hwnd);
   renderer->SetStatusText("Connecting to peers.");
}

/*
 * VectorWar::InitSpectator --
 *
 * Create a new spectator session
 */
void
VectorWar::InitSpectator(HWND hwnd, unsigned short localport, int num_players, char *host_ip, unsigned short host_port)
{
	GGPOErrorCode result;
	renderer = new GDIRenderer(hwnd);

	// Initialize the game state
	gs.Init(hwnd, num_players);
	ngs.num_players = num_players;

	// Fill in a ggpo callbacks structure to pass to start_session.
	GGPOSessionCallbacks cb = { 0 };
	cb.instance = this;
	cb.begin_game = [](void* self, const char* name) {return vw_begin_game_callback((VectorWar*)self, name); };
	cb.advance_frame = [](void* self, int flags) {return vw_advance_frame_callback((VectorWar*)self, flags); };
	cb.load_game_state = [](void* self, unsigned char* buffer, int len) {return vw_load_game_state_callback((VectorWar*)self, buffer, len); };
	cb.save_game_state = [](void* self, unsigned char** buffer, int* len, int* checksum, int frame) {return vw_save_game_state_callback((VectorWar*)self, buffer, len, checksum, frame); };
	cb.free_buffer = [](void* self, void* buffer) {return vw_free_buffer((VectorWar*)self, buffer); };
	cb.on_event = [](void* self, GGPOEvent* info) {return vw_on_event_callback((VectorWar*)self, info); };
	cb.log_game_state = [](void* self, char* filename, unsigned char* buffer, int len) {return vw_log_game_state((VectorWar*)self, filename, buffer, len); };
	UdpConnection* udp_connection = new UdpConnection(localport);
	GGPOConnection* connection = udp_connection->get_ggpo_connection();
	auto playernum = udp_connection->add_connection(host_ip, host_port);
	result = ggpo_start_spectating(&ggpo, &cb, connection, "vectorwar", num_players, sizeof(int), playernum);

	ggpoutil_perfmon_init(hwnd);

	renderer->SetStatusText("Starting new spectator session");
}


/*
 * VectorWar::DisconnectPlayer --
 *
 * Disconnects a player from this session.
 */

void
VectorWar::DisconnectPlayer(int player)
{
   if (player < ngs.num_players) {
      char logbuf[128];
      GGPOErrorCode result = ggpo_disconnect_player(ggpo, ngs.players[player].handle);
      if (GGPO_SUCCEEDED(result)) {
         sprintf_s(logbuf, ARRAYSIZE(logbuf), "Disconnected player %d.\n", player);
      } else {
         sprintf_s(logbuf, ARRAYSIZE(logbuf), "Error while disconnecting player (err:%d).\n", result);
      }
      renderer->SetStatusText(logbuf);
   }
}


/*
 * VectorWar::DrawCurrentFrame --
 *
 * Draws the current frame without modifying the game state.
 */
void
VectorWar::DrawCurrentFrame()
{
   if (renderer != nullptr) {
      renderer->Draw(gs, ngs);
   }
}

/*
 * VectorWar::AdvanceFrame --
 *
 * Advances the game state by exactly 1 frame using the inputs specified
 * for player 1 and player 2.
 */
void VectorWar::AdvanceFrame(int inputs[], int disconnect_flags)
{
   gs.Update(inputs, disconnect_flags);

   // update the checksums to display in the top of the window.  this
   // helps to detect desyncs.
   ngs.now.framenumber = gs._framenumber;
   ngs.now.checksum = fletcher32_checksum((short *)&gs, sizeof(gs) / 2);
   if ((gs._framenumber % 90) == 0) {
      ngs.periodic = ngs.now;
   }

   // Notify ggpo that we've moved forward exactly 1 frame.
   ggpo_advance_frame(ggpo);

   // Update the performance monitor display.
   GGPOPlayerHandle handles[MAX_PLAYERS];
   int count = 0;
   for (int i = 0; i < ngs.num_players; i++) {
      if (ngs.players[i].type == GGPO_PLAYERTYPE_REMOTE) {
         handles[count++] = ngs.players[i].handle;
      }
   }
   ggpoutil_perfmon_update(ggpo, handles, count);
}


/*
 * ReadInputs --
 *
 * Read the inputs for player 1 from the keyboard.  We never have to
 * worry about player 2.  GGPO will handle remapping his inputs 
 * transparently.
 */
int
ReadInputs(HWND hwnd)
{
   static const struct {
      int      key;
      int      input;
   } inputtable[] = {
      { VK_UP,       INPUT_THRUST },
      { VK_DOWN,     INPUT_BREAK },
      { VK_LEFT,     INPUT_ROTATE_LEFT },
      { VK_RIGHT,    INPUT_ROTATE_RIGHT },
      { 'D',         INPUT_FIRE },
      { 'S',         INPUT_BOMB },
   };
   int i, inputs = 0;

   if (GetForegroundWindow() == hwnd) {
      for (i = 0; i < sizeof(inputtable) / sizeof(inputtable[0]); i++) {
         if (GetAsyncKeyState(inputtable[i].key)) {
            inputs |= inputtable[i].input;
         }
      }
   }
   
   return inputs;
}

/*
 * VectorWar::RunFrame --
 *
 * Run a single frame of the game.
 */
void
VectorWar::RunFrame(HWND hwnd)
{
  GGPOErrorCode result = GGPO_OK;
  int disconnect_flags;
  int inputs[MAX_SHIPS] = { 0 };

  if (ngs.local_player_handle != GGPO_INVALID_HANDLE) {
     int input = ReadInputs(hwnd);
#if defined(SYNC_TEST)
     input = rand(); // test: use random inputs to demonstrate sync testing
#endif
     result = ggpo_add_local_input(ggpo, ngs.local_player_handle, &input, sizeof(input));
  }

   // synchronize these inputs with ggpo.  If we have enough input to proceed
   // ggpo will modify the input list with the correct inputs to use and
   // return 1.
  if (GGPO_SUCCEEDED(result)) {
     result = ggpo_synchronize_input(ggpo, (void *)inputs, sizeof(int) * MAX_SHIPS, &disconnect_flags);
     if (GGPO_SUCCEEDED(result)) {
         // inputs[0] and inputs[1] contain the inputs for p1 and p2.  Advance
         // the game by 1 frame using those inputs.
         VectorWar::AdvanceFrame(inputs, disconnect_flags);
     }
  }
  VectorWar::DrawCurrentFrame();
}

/*
 * VectorWar::Idle --
 *
 * Spend our idle time in ggpo so it can use whatever time we have left over
 * for its internal bookkeeping.
 */
void
VectorWar::Idle(int time)
{
   ggpo_idle(ggpo, time);
}

void
VectorWar::Exit()
{
   memset(&gs, 0, sizeof(gs));
   memset(&ngs, 0, sizeof(ngs));

   if (ggpo) {
      ggpo_close_session(ggpo);
      ggpo = NULL;
   }
   delete renderer;
   renderer = NULL;
}

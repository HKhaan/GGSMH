#ifndef _GGPO_EXTERNAL_H_
#define _GGPO_EXTERNAL_H_
#include "ggponet.h"
#include <stdint.h>



#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#  if defined(GGPO_SHARED_LIB)
#  ifdef GGPO_SDK_EXPORT
#       define GGPO_EXTERNAL __declspec(dllexport)
#     else
#       define GGPO_EXTERNAL __declspec(dllimport)
#     endif
#  else
#     define GGPO_EXTERNAL
#  endif
#else
#  define GGPO_EXTERNAL
#endif
	//TODO: find a way to keep track of ggpo_connection without making it a global var
	extern GGPOConnection* ggpo_connection;

	typedef void (*log_cb)(void* self, const char* text);
	typedef bool (*begin_game_cb)(void* self, const char* text);
	typedef bool (*advance_frame_cb)(void* self, int flags);
	typedef bool (*load_game_state_cb)(void* self, unsigned char* buffer, int length);
	typedef bool (*log_game_state_cb)(void* self, char* text, unsigned char* buffer, int length);
	typedef bool (*save_game_state_cb)(void* self, unsigned char** buffer, int* len, int* checksum, int frame);
	typedef void (*free_buffer_cb)(void* self, void* buffer);
	typedef bool (*on_event_cb)(void* self, GGPOEvent* info);
	typedef void (*send_to_cb)(void* self, const char* buffer, int len, int flags, int player_num);
	typedef int (*receive_from_cb)(void* self, char* buffer, int len, int flags, int* player_num);
	typedef intptr_t GGPOPtr;

	int ext_ggpo_start_session(GGPOPtr& sessionRef,
		begin_game_cb beginGame,
		advance_frame_cb advanceFrame,
		load_game_state_cb loadGameState,
		log_game_state_cb logGameState,
		save_game_state_cb saveGameState,
		free_buffer_cb freeBuffer,
		on_event_cb onEvent,
		send_to_cb sendTo,
		receive_from_cb receiveFrom,
		const char* game, int num_players);
	int ext_ggpo_start_spectating(GGPOPtr& sessionRef,
		begin_game_cb beginGame,
		advance_frame_cb advanceFrame,
		load_game_state_cb loadGameState,
		log_game_state_cb logGameState,
		save_game_state_cb saveGameState,
		free_buffer_cb freeBuffer,
		on_event_cb onEvent,
		send_to_cb sendTo,
		receive_from_cb receiveFrom,
		const char* game, int num_players, int player_id);
	int ext_ggpo_set_disconnectnotify_start(GGPOPtr ggpo, int timeout);
	int ext_ggpo_set_disconnect_timeout(GGPOPtr ggpo, int timeout);
	int ext_ggpo_synchronize_input(GGPOPtr ggpo, uint64_t* inputs, int length, int& disconnect_flags);
	int ext_ggpo_add_local_input(GGPOPtr ggpo, int local_player_handle, uint64_t input);
	int ext_ggpo_close_session(GGPOPtr ggpo);
	int UggIdle(GGPOPtr ggpo, int timeout);
	int UggAddPlayer(GGPOPtr ggpo,
		int player_type,
		int player_num,
		const char* player_ip_address,
		unsigned short player_port,
		int& phandle);
	int ext_ggpo_disconnect_player(GGPOPtr ggpo, int phandle);
	int ext_ggpo_set_frame_delay(GGPOPtr ggpo, int phandle, int frame_delay);
	int ext_ggpo_advance_frame(GGPOPtr ggpo);
	void ext_ggpo_log(GGPOPtr ggpo, const char* text);
	int UggGetNetworkStats(GGPOPtr ggpo, int phandle,
		int& send_queue_len,
		int& recv_queue_len,
		int& ping,
		int& kbps_sent,
		int& local_frames_behind,
		int& remote_frames_behind);



#ifdef __cplusplus
};
#endif

#endif
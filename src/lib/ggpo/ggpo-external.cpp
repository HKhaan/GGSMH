#include "ggpo-external.h"

#include <stdio.h>
#include <vector>
#include <string>
#include "ggponet.h"
#include <memory>
#include <iostream>
#include <string>
#include <cstdio>
#include "types.h"

GGPOConnection* ggpo_connection = new GGPOConnection();

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
	const char* game, int num_players)
{
	GGPOSessionCallbacks cb;
	cb.advance_frame = advanceFrame;
	cb.load_game_state = loadGameState;
	cb.begin_game = beginGame;
	cb.save_game_state = saveGameState;
	cb.load_game_state = loadGameState;
	cb.log_game_state = logGameState;
	cb.free_buffer = freeBuffer;
	cb.on_event = onEvent;
	ggpo_connection->send_to = sendTo;
	ggpo_connection->receive_from = receiveFrom;
	
	GGPOSession* ggpo;
	auto ret = ggpo_start_session(&ggpo, &cb, ggpo_connection, game, num_players, sizeof(uint64_t));
	sessionRef = (GGPOPtr)ggpo;
	return ret;
}

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
	const char* game, int num_players, int player_id)
{
	GGPOSessionCallbacks cb;
	cb.advance_frame = advanceFrame;
	cb.load_game_state = loadGameState;
	cb.begin_game = beginGame;
	cb.save_game_state = saveGameState;
	cb.load_game_state = loadGameState;
	cb.log_game_state = logGameState;
	cb.free_buffer = freeBuffer;
	cb.on_event = onEvent;
	ggpo_connection->send_to = sendTo;
	ggpo_connection->receive_from = receiveFrom;

	GGPOSession* ggpo;
	auto ret = ggpo_start_spectating(&ggpo, &cb, ggpo_connection, game, num_players, sizeof(uint64_t), player_id);
	return ret;
}

int ext_ggpo_set_disconnectnotify_start(GGPOPtr ggpo, int timeout)
{
	return ggpo_set_disconnect_notify_start((GGPOSession*)ggpo, timeout);
}

int ext_ggpo_set_disconnect_timeout(GGPOPtr ggpo, int timeout)
{
	return ggpo_set_disconnect_timeout((GGPOSession*)ggpo, timeout);
}

int ext_ggpo_synchronize_input(GGPOPtr ggpo, uint64_t* inputs, int length, int& disconnect_flags)
{
	return ggpo_synchronize_input((GGPOSession*)ggpo, inputs, sizeof(uint64_t) * length, &disconnect_flags);
}

int ext_ggpo_add_local_input(GGPOPtr ggpo, int local_player_handle, uint64_t input)
{
	return ggpo_add_local_input((GGPOSession*)ggpo, local_player_handle, &input, sizeof(uint64_t));
}

int ext_ggpo_close_session(GGPOPtr ggpo)
{
	return ggpo_close_session((GGPOSession*)ggpo);
}

int UggIdle(GGPOPtr ggpo, int timeout)
{
	return ggpo_idle((GGPOSession*)ggpo, timeout);
}

int UggAddPlayer(GGPOPtr ggpo,
	int player_type,
	int player_num,
	const char* player_ip_address,
	unsigned short player_port,
	int& phandle)
{
	GGPOPlayer player;
	player.size = sizeof(GGPOPlayer);
	player.type = (GGPOPlayerType)player_type;
	player.player_num = player_num;
	strcpy_s(player.u.remote.ip_address, player_ip_address);
	player.u.remote.port = player_port;
	return ggpo_add_player((GGPOSession*)ggpo, &player, &phandle);
}

int ext_ggpo_disconnect_player(GGPOPtr ggpo, int phandle)
{
	return ggpo_disconnect_player((GGPOSession*)ggpo, phandle);
}

int ext_ggpo_set_frame_delay(GGPOPtr ggpo, int phandle, int frame_delay)
{
	return ggpo_set_frame_delay((GGPOSession*)ggpo, phandle, frame_delay);
}

int ext_ggpo_advance_frame(GGPOPtr ggpo)
{
	return ggpo_advance_frame((GGPOSession*)ggpo);
}

void ext_ggpo_log(GGPOPtr ggpo, const char* text)
{
	ggpo_log((GGPOSession*)ggpo, text);
}

int ext_ggpo_get_network_stats(GGPOPtr ggpo, int phandle,
	int& send_queue_len,
	int& recv_queue_len,
	int& ping,
	int& kbps_sent,
	int& local_frames_behind,
	int& remote_frames_behind)
{
	GGPONetworkStats stats;
	auto result = ggpo_get_network_stats((GGPOSession*)ggpo, phandle, &stats);
	send_queue_len = stats.network.send_queue_len;
	recv_queue_len = stats.network.recv_queue_len;
	ping = stats.network.ping;
	kbps_sent = stats.network.kbps_sent;
	local_frames_behind = stats.timesync.local_frames_behind;
	remote_frames_behind = stats.timesync.remote_frames_behind;
	return result;
}
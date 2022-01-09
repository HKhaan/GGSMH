#ifndef _VECTORWAR_H
#define _VECTORWAR_H

#include "ggponet.h"
#include "gdi_renderer.h"
#include "gamestate.h"
#include "nongamestate.h"
/*
 * vectorwar.h --
 *
 * Interface to the vector war application.
 *
 */

enum VectorWarInputs {
	INPUT_THRUST = (1 << 0),
	INPUT_BREAK = (1 << 1),
	INPUT_ROTATE_LEFT = (1 << 2),
	INPUT_ROTATE_RIGHT = (1 << 3),
	INPUT_FIRE = (1 << 4),
	INPUT_BOMB = (1 << 5),
};

class VectorWar {
public:
	GameState gs = {0};
	NonGameState ngs = {0};
	Renderer* renderer = nullptr;
	GGPOSession* ggpo = nullptr;
	void Init(HWND hwnd, unsigned short localport, int num_players, GGPOPlayer* players, int num_spectators);
	void InitSpectator(HWND hwnd, unsigned short localport, int num_players, char* host_ip, unsigned short host_port);
	void DrawCurrentFrame();
	void AdvanceFrame(int inputs[], int disconnect_flags);
	void RunFrame(HWND hwnd);
	void Idle(int time);
	void DisconnectPlayer(int player);
	void Exit();
};

#define ARRAY_SIZE(n)      (sizeof(n) / sizeof(n[0]))
#define FRAME_DELAY        2

#endif

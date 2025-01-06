#pragma once
#include <vector>
#include "gmath.h"
#include "Windows.h"

struct player {
	uintptr_t pawn;
	FVector2d neck;
	FVector2d chest;
	FVector2d left_shoulder;
	FVector2d left_elbow;
	FVector2d left_hand;
	FVector2d right_shoulder;
	FVector2d right_elbow;
	FVector2d right_hand;
	FVector2d pelvis;
	FVector2d left_hip;
	FVector2d left_knee;
	FVector2d left_foot;
	FVector2d right_hip;
	FVector2d right_knee;
	FVector2d right_foot;
	double distance;
	bool isVisible;
	player() {

	}
};

namespace globals {
	static double width = 1920;
	static double height = 1080;
	static int smooth = 3.0f;

	static std::vector<player> Players;
}
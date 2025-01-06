#include "main.h"


void update_cache() {
	uint64_t process_base = pbase();
	pointer->uworld = read<__int64>(process_base + offset::UWorld);
	pointer->game_state = read<__int64>(pointer->uworld + offset::GameState);
	pointer->player_array = read<uintptr_t>(pointer->game_state + offset::PlayerArray);
	pointer->player_array_size = read<int>(pointer->game_state + (offset::PlayerArray + sizeof(uintptr_t)));
	pointer->game_instance = read<uintptr_t>(pointer->uworld + offset::OwningGameInstance);
	uintptr_t local_players = read<uintptr_t>(pointer->game_instance + offset::LocalPlayers);
	pointer->local_player = read<uintptr_t>(local_players);
	pointer->player_controller = read<uintptr_t>(pointer->local_player + offset::PlayerController);
	pointer->acknowledged_pawn = read<uintptr_t>(pointer->player_controller + offset::AcknowledgedPawn);
	pointer->player_state = read<uintptr_t>(pointer->acknowledged_pawn + offset::PlayerState);
	pointer->team_index = read<int>(pointer->player_state + offset::Team_Index);
}


int main(int argc, char** argv) {
	SPOOF_FUNC;

	if (init(L"FortniteClient-Win64-Shipping.exe") == true) {

		std::cout << "successfull attachment\n";
		uint64_t process_base = pbase();
		std::cout << "Process base(new): " << std::hex << process_base << std::endl;
		pointer->uworld = read<__int64>(process_base + offset::UWorld);
		std::cout << "uworld: " << pointer->uworld << "\n";
		pointer->game_state = read<__int64>(pointer->uworld + offset::GameState);
		std::cout << "gamestate: " << pointer->game_state << "\n";
		pointer->player_array = read<uintptr_t>(pointer->game_state + offset::PlayerArray);
		std::cout << "playerarray: " << std::hex << pointer->player_array << "\n";
		pointer->player_array_size = read<int>(pointer->game_state + (offset::PlayerArray + sizeof(uintptr_t)));
		std::cout << "player array size: " << std::dec << pointer->player_array_size << "\n";
		pointer->game_instance = read<uintptr_t>(pointer->uworld + offset::OwningGameInstance);
		std::cout << "gameinstance " << std::hex << pointer->game_instance << "\n";
		uintptr_t local_players = read<uintptr_t>(pointer->game_instance + offset::LocalPlayers);
		pointer->local_player = read<uintptr_t>(local_players);
		std::cout << "local players: " << std::hex << pointer->local_player << "\n";
		pointer->player_controller = read<uintptr_t>(pointer->local_player + offset::PlayerController);
		std::cout << "player control: " << std::hex << pointer->player_controller << "\n";
		pointer->acknowledged_pawn = read<uintptr_t>(pointer->player_controller + offset::AcknowledgedPawn);
		std::cout << "ac pawn: " << std::hex << pointer->acknowledged_pawn << std::endl;
		pointer->player_state = read<uintptr_t>(pointer->acknowledged_pawn + offset::PlayerState);
		pointer->team_index = read<int>(pointer->player_state + offset::Team_Index);

		
		std::thread newThread(render);


		while (true) {
			update_cache();
			int p_count = 0;
			std::vector<player> players_temp = {};
			uintptr_t pl_state;
			uintptr_t c_actor;
			uintptr_t skeletalmesh;
			for (int i = 0; i < pointer->player_array_size; ++i) {
			pl_state = read<uintptr_t>(pointer->player_array + (i * sizeof(uintptr_t)));
			c_actor = read<uintptr_t>(pl_state + offset::PawnPrivate);
			if (!c_actor) continue;
			skeletalmesh = read<uintptr_t>(c_actor + offset::Mesh);
			if (!skeletalmesh) continue;
			if (c_actor == pointer->acknowledged_pawn) continue;
			if (pointer->team_index == read<int>(pl_state + offset::Team_Index)) continue;
				
				



				try {
				p_count += 1;
				Utilities->UpdateCamera();
				player plc;
				plc.distance = camera::location.Distance(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_HEAD));
				plc.neck = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_HEAD));
				plc.chest = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_NECK));
				plc.left_shoulder = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_LSHOULDER));
				plc.left_elbow = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_LELBOW));
				plc.left_hand = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_LHAND));
				plc.right_shoulder = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_RSHOULDER));
				plc.right_elbow = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_RELBOW));
				plc.right_hand = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_RHAND));
				plc.pelvis = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_PELVIS));
				plc.left_hip = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_LTHIGH));
				plc.left_knee = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_LKNEE));
				plc.left_foot = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_LFOOT));
				plc.right_hip = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_RTHIGH));
				plc.right_knee = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_RKNEE));
				plc.right_foot = Utilities->WorldToScreen(Utilities->GetBoneLocation(skeletalmesh, perfect_skeleton::BONE_RFOOT));
				plc.isVisible = Utilities->IsVisible(skeletalmesh);
				players_temp.push_back(plc);
				}
				catch (const std::string& ex) {
					continue;
				}
			}
			globals::Players = players_temp;
			if (p_count == 0) {
				globals::Players.clear();
			}
			/*while (true) {
				if (GetAsyncKeyState('Z') & 0x8000) {
					std::cout << "'Z' key is pressed!" << std::endl;
					break;
				}
			}*/
		}
		
	}
	else {
		std::cout << "finished with error";
	}
	stop();
	return 0;
}
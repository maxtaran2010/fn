#include <iostream>
#include "main.h"
#include <vector>

int main() {
	SPOOF_FUNC;
	if (init(L"FortniteClient-Win64-Shipping.exe") == true) {
		std::cout << "successfull attachment\n";
		uint64_t process_base = pbase();
		std::cout << "Process base(new): " << std::hex << process_base << std::endl;
		__int64 uworld;
		uworld = read<__int64>(process_base + offset::UWorld);
		std::cout << "uworld: " << uworld << "\n";
		__int64 gs = read<__int64>(uworld + offset::GameState);
		std::cout << "gamestate: " << gs << "\n";
		uintptr_t pa = read<uintptr_t>(gs + offset::PlayerArray);
		std::cout << "playerarray: " << std::hex << pa << "\n";
		int pa_size = read<int>(gs + (offset::PlayerArray + sizeof(uintptr_t)));
		std::cout << "player array size: " << std::dec << pa_size << "\n";
		uintptr_t gameinstance = read<uintptr_t>(uworld + offset::OwningGameInstance);
		std::cout << "gameinstance " << std::hex << gameinstance << "\n";
		uintptr_t local_player = read<uintptr_t>(gameinstance + offset::LocalPlayers);
		std::cout << "local players: " << std::hex << local_player << "\n";
		uintptr_t player_control = read<uintptr_t>(local_player + offset::PlayerController);
		std::cout << "player control: " << std::hex << player_control << "\n";
		uintptr_t ac_pawn = read<uintptr_t>(player_control + offset::AcknowledgedPawn);
		/*for (int i = 0x0; i < 0x400; ++i) {
			ac_pawn = read<uintptr_t>(player_control + i);
			 std::cout << i << "acpawn: " << std::hex << ac_pawn << "\n";
		}*/
		std::vector < entity > players;
		std::cout << "acpawn: " << std::hex << ac_pawn << "\n";


		for (int i = 0; i < pa_size; ++i) {
			auto pl_state = read<uintptr_t>(pa + (i * sizeof(uintptr_t)));
			auto c_actor = read<uintptr_t>(pl_state+offset::PawnPrivate);
			auto skeletalmesh = read<uintptr_t>(c_actor+ offset::Mesh);
			if (!skeletalmesh) continue;
			if (c_actor == ac_pawn) std::cout << c_actor;
			auto base_bone = Utilities->GetBoneLocation(skeletalmesh, bone::HumanBase);
			

			std::cout << c_actor << " " << "" << " " << base_bone.x << " " << base_bone.y << " " << base_bone.z << "\n";
			
		}
	}
	stop();
	return 0;
}
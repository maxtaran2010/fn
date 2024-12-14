#include <iostream>
#include "main.h"

int main() {
	SPOOF_FUNC;
	if (init(L"FortniteClient-Win64-Shipping.exe") == true) {
		std::cout << "successfull attachment\n";
		uint64_t process_base = pbase();
		std::cout << "Process base(new): " << std::hex << process_base << std::endl;
		__int64 uworld;
		for (int i = 0; i < 25; i++) {
			uworld = read<__int64>(process_base + offset::UWorld + (i*0x1000)+0x250);
			std::cout << uworld;
		}
		__int64 gs = read<__int64>(uworld + offset::GameState);
		uintptr_t pa = read<uintptr_t>(gs + offset::PlayerArray);
		int pa_size = read<int>(gs + (offset::PlayerArray + sizeof(uintptr_t)));
		uintptr_t gameinstance = read<uintptr_t>(uworld + offset::OwningGameInstance);
		uintptr_t local_player = read<uintptr_t>(gameinstance + offset::LocalPlayers);
		uintptr_t player_control = read<uintptr_t>(local_player + offset::PlayerController);
		uintptr_t ac_pawn = read<uintptr_t>(player_control + offset::AcknowledgedPawn);
		std::cout << uworld;
		
		for (int i = 0; i < pa_size; ++i) {
			auto pl_state = read<uintptr_t>(pa + (i * sizeof(uintptr_t)));
			std::cout << "pl";
			auto c_actor = read<uintptr_t>(pl_state+offset::PawnPrivate);
			std::cout << "cact";
			if (c_actor == ac_pawn) continue;
			auto skeletalmesh = read<uintptr_t>(c_actor+ offset::Mesh);
			std::cout << "skm";
			if (!skeletalmesh) continue;
			auto base_bone = Utilities->GetBoneLocation(skeletalmesh, bone::HumanBase);
			std::cout << base_bone.x << " " << base_bone.y << " " << base_bone.z << "\n";
		}
	}
	stop();
	return 0;
}
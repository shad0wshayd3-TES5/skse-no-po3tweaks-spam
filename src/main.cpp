namespace
{
	struct ConsoleLogPatch :
		Xbyak::CodeGenerator
	{
		ConsoleLogPatch(std::uintptr_t a_func, std::uintptr_t a_retn)
		{
			Xbyak::Label func;
			Xbyak::Label retn;

			mov(rcx, rax);
			sub(rsp, 0x20);
			call(ptr[rip + func]);
			add(rsp, 0x20);

			jmp(ptr[rip + retn]);

			L(func);
			dq(a_func);

			L(retn);
			dq(a_retn);
		}
	};

	void ConsoleLogPrint(float a_value)
	{
		const auto ui = RE::UI::GetSingleton();
		const auto player = RE::PlayerCharacter::GetSingleton();
		if (ui->IsMenuOpen(RE::Console::MENU_NAME) && player->Is3DLoaded())
		{
			const auto log = RE::ConsoleLog::GetSingleton();
			log->Print("IsFurnitureAnimType >> %0.2f", a_value);
		}
	}

	void MessageCallback(SKSE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type)
		{
		case SKSE::MessagingInterface::kPostLoad:
		{
			auto hndl = REX::W32::GetModuleHandleA("po3_Tweaks");
			if (hndl)
			{
				auto target = reinterpret_cast<std::uintptr_t>(hndl);
				SKSE::log::info("Found po3_Tweaks: {:016X}"sv, target);

				static constexpr auto TARGET_ADDR{ 0x305AC };
				static constexpr auto TARGET_RETN{ 0x3060A };
				static constexpr auto TARGET_FILL{ TARGET_RETN - TARGET_ADDR };
				REL::safe_fill(target + TARGET_ADDR, REL::NOP, TARGET_FILL);

				auto patch = ConsoleLogPatch(
					reinterpret_cast<std::uintptr_t>(ConsoleLogPrint),
					target + TARGET_RETN);
				patch.ready();
				assert(patch.getSize() < TARGET_FILL);
				REL::safe_write(target + TARGET_ADDR, std::span{ patch.getCode(), patch.getSize() });
			}

			break;
		}
		default:
			break;
		}
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* a_SKSE)
{
	SKSE::Init(a_SKSE);

	SKSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}

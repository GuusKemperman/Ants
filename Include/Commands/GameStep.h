#pragma once
#include "Commands/CommandBuffer.h"
#include "Commands/InteractCommand.h"
#include "Commands/MoveCommand.h"

namespace Ant
{
	template<typename... T>
	class GameStepBase;

	template <>
	class GameStepBase<>
	{
	public:
		void AddCommand() = delete;
		void ForEachCommandBuffer(const auto&) {};
		void ForEachCommandBuffer(const auto&) const {};
	};

	template<typename T, typename... O>
	class GameStepBase<T, O...> : public GameStepBase<O...>
	{
		using Base = GameStepBase<O...>;

	public:
		using Base::AddCommand;

		void AddCommand(T&& command)
		{
			mBuffer.AddCommand(std::move(command));
		}

		void ForEachCommandBuffer(const auto& func)
		{
			func(mBuffer);
			Base::ForEachCommandBuffer(func);
		}

		void ForEachCommandBuffer(const auto& func) const
		{
			func(mBuffer);
			Base::ForEachCommandBuffer(func);
		}

		template<typename D>
		auto& GetCommandBuffer() = delete;

		template<typename D>
		const auto& GetCommandBuffer() const = delete;

		template<>
		auto& GetCommandBuffer<T>()
		{
			return mBuffer;
		}

		template<>
		const auto& GetCommandBuffer<T>() const
		{
			return mBuffer;
		}

	private:
		CommandBuffer<T> mBuffer{};
	};

	// Could also be a "using GameStep = ...", but now we can forward declare GameStep
	class GameStep final : public GameStepBase<MoveCommand, InteractCommand> {};
}
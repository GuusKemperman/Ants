#pragma once
#include "SenseCommand.h"
#include "Commands/CommandBuffer.h"
#include "Commands/InteractCommand.h"
#include "Commands/MoveCommand.h"
#include "Commands/SpawnAntCommand.h"
#include "Commands/SpawnFoodCommand.h"
#include "Commands/EmitPheromoneCommand.h"
#include "Commands/SenseCommand.h"

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

		template<typename C>
		CommandBuffer<C>& GetBuffer()
		{
			if constexpr (std::is_same_v<C, T>)
			{
				return mBuffer;
			}
			else
			{
				return Base::template GetBuffer<C>();
			}
		}

		template<typename C>
		const CommandBuffer<C>& GetBuffer() const
		{
			if constexpr (std::is_same_v<C, T>)
			{
				return mBuffer;
			}
			else
			{
				return Base::template GetBuffer<C>();
			}
		}

	private:
		CommandBuffer<T> mBuffer{};
	};

	// Could also be a "using GameStep = ...", but now we can forward declare GameStep
	class GameStep final : public GameStepBase<MoveCommand, EmitPheromoneCommand, InteractCommand, SpawnAntCommand, SpawnFoodCommand, SenseCommand> {};
}
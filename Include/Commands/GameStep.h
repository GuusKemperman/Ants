#pragma once
#include "Commands/CommandBuffer.h"
#include "Commands/InteractCommand.h"
#include "Commands/MoveCommand.h"

namespace Ant
{
	// TODO don't hardcode the two command types
	class GameStep
	{
	public:
		void AddCommand(MoveCommand&& command)
		{
			mMoveCommandBuffer.AddCommand(std::move(command));
		}

		void AddCommand(InteractCommand&& command)
		{
			mInteractCommandBuffer.AddCommand(std::move(command));
		}

		template<typename T>
		auto& GetCommandBuffer() = delete;

		template<typename T>
		const auto& GetCommandBuffer() const = delete;

		template<>
		auto& GetCommandBuffer<MoveCommand>()
		{
			return mMoveCommandBuffer;
		}

		template<>
		auto& GetCommandBuffer<InteractCommand>()
		{
			return mInteractCommandBuffer;
		}

		template<>
		const auto& GetCommandBuffer<MoveCommand>() const
		{
			return mMoveCommandBuffer;
		}

		template<>
		const auto& GetCommandBuffer<InteractCommand>() const
		{
			return mInteractCommandBuffer;
		}

		void ForEachCommandBuffer(const auto& func)
		{
			func(mMoveCommandBuffer);
			func(mInteractCommandBuffer);
		}

		void ForEachCommandBuffer(const auto& func) const
		{
			func(mMoveCommandBuffer);
			func(mInteractCommandBuffer);
		}

	private:
		CommandBuffer<MoveCommand> mMoveCommandBuffer{};
		CommandBuffer<InteractCommand> mInteractCommandBuffer{};
	};

}
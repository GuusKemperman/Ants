#pragma once
#include "Meta/MetaTypeId.h"

namespace Ant
{
	template<typename T>
	class CommandBuffer
	{
	public:
		CommandBuffer() = default;

		CommandBuffer(CommandBuffer&&) noexcept = default;

		CommandBuffer(const CommandBuffer& other) :
			mCommands([](const CommandBuffer& other) -> decltype(mCommands)
				{
					auto storedCommands = other.GetStoredCommands();
					return { storedCommands.begin(), storedCommands.end() };
				}(other)),
			mCommandsInUse(other.mCommandsInUse.load())
		{}

		CommandBuffer& operator=(CommandBuffer&&) noexcept = default;

		CommandBuffer& operator=(const CommandBuffer& other)
		{
			mCommands = other.mCommands;
			mCommandsInUse = other.mCommandsInUse.load();
			return *this;
		}

		~CommandBuffer() = default;

		template<typename... Args>
		void AddCommand(Args&&... args);

		void Clear()
		{
			mCommands.clear();
			mCommandsInUse = 0;
		}

		void Reserve(size_t num)
		{
			mCommands.resize(num);
			mCommands.resize(mCommands.capacity());
		}

		size_t GetNumSubmittedCommands() const { return mCommandsInUse; }

		std::span<T> GetStoredCommands() 
		{ 
			const CommandBuffer& self = *this;
			std::span<const T> constSpan = self.GetStoredCommands();

			return { const_cast<T*>(constSpan.data()), constSpan.size() };
		}
		
		std::span<const T> GetStoredCommands() const
		{
			if (mCommandsInUse > mCommands.size())
			{
				LOG(LogGame,
					Warning,
					"{} commands of type {} could not be submitted due to the buffer being full",
					mCommandsInUse - mCommands.size(),
					CE::MakeTypeName<T>());
				return { mCommands.data(), mCommands.size() };
			}

			return { mCommands.data(), mCommandsInUse }; 
		}

	private:
		std::vector<T> mCommands{};
		std::atomic<size_t> mCommandsInUse{};
	};

	template<typename T>
	template<typename ...Args>
	void CommandBuffer<T>::AddCommand(Args&& ...args)
	{
		size_t index = mCommandsInUse++;

		if (index >= mCommands.size())
		{
			return;
		}

		mCommands[index] = T{ std::forward<Args>(args)... };
	}
}
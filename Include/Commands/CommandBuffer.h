#pragma once

namespace Ant
{
	template<typename T>
	class CommandBuffer
	{
	public:
		template<typename... Args>
		void AddCommand(Args&&... args);

		void Clear()
		{
			mCommands.clear();
			mCommandsInUse = 0;
		}

		std::span<T> GetSubmittedCommands() { return { mCommands.data(), mCommandsInUse }; }
		std::span<const T> GetSubmittedCommands() const { return { mCommands.data(), mCommandsInUse }; }

		std::vector<T> mCommands{};
		std::atomic<size_t> mCommandsInUse{};
	};

	template<typename T>
	template<typename ...Args>
	void CommandBuffer<T>::AddCommand(Args&& ...args)
	{
		mCommands[mCommandsInUse++] = T{ std::forward<Args>(args)... };
	}
}
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
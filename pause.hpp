#pragma once

#include <chrono>
#include <thread>

// пауза, чтобы планировщик успел отдать работу другому потоку
// в тестах приходит 0, и функция сразу выходит

inline void pauseMilliseconds(const int milliseconds)
{
	if (milliseconds <= 0) {
		return;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

#pragma once

#include "matrix.hpp"

// считает A * B в threadCount потоках
// delayMs — пауза на одну клетку, в самой программе это 100, в тестах 0
// log — печатать, какой поток какую клетку взял, тестам это не нужно
Matrix multiplyParallel(
	const Matrix& a,
	const Matrix& b,
	int threadCount,
	int delayMs,
	bool log);

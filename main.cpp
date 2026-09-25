// умножение матриц несколькими потоками pthread
//
// сборка:
//   mingw32-make
// тесты:
//   mingw32-make test
// запуск примера:
//   cmd /c "matrix.exe < sample.in"

#include "matrix.hpp"
#include "multiply.hpp"
#include "protocol.hpp"

#include <pthread.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace {

constexpr int COMPUTE_DELAY_MS = 100;
constexpr const char* PROTOCOL_FILE = "protocol.txt";

}  // namespace

int main()
{
#if defined(_WIN32)
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	std::cout << "Введите m n k p:\n";
	std::cout << "m — строки A, n — столбцы A и строки B, k — столбцы B, p — число потоков\n";

	int m = 0;
	int n = 0;
	int k = 0;
	int p = 0;
	if (!(std::cin >> m >> n >> k >> p)) {
		std::cerr << "Нужны четыре целых числа: m n k p\n";
		return EXIT_FAILURE;
	}
	if (m <= 0 || n <= 0 || k <= 0 || p <= 0) {
		std::cerr << "m, n, k и p должны быть больше нуля\n";
		return EXIT_FAILURE;
	}

	try {
		std::cout << "Введите матрицу A (" << m << " x " << n << "):\n";
		const Matrix a = Matrix::read(std::cin, m, n);
		std::cout << "Введите матрицу B (" << n << " x " << k << "):\n";
		const Matrix b = Matrix::read(std::cin, n, k);

		const Matrix c = multiplyParallel(a, b, p, COMPUTE_DELAY_MS, true);

		std::cout << "\nМатрица C = A x B:\n";
		c.print(std::cout);

		// job лежит здесь до pthread_join. Поток-писатель держит только адрес
		ProtocolJob job;
		job.a = &a;
		job.b = &b;
		job.c = &c;
		job.threadCount = p;
		job.delayMs = COMPUTE_DELAY_MS;
		job.fileName = PROTOCOL_FILE;

		pthread_t writer;
		const int writerCode = pthread_create(&writer, nullptr, protocolWriterRoutine, &job);
		if (writerCode != 0) {
			std::cerr << "Не удалось создать поток записи протокола\n";
			return EXIT_FAILURE;
		}

		void* writerResult = nullptr;
		pthread_join(writer, &writerResult);
		if (writerResult != nullptr) {
			std::cerr << "Не удалось открыть " << PROTOCOL_FILE << '\n';
			return EXIT_FAILURE;
		}
	} catch (const std::exception& error) {
		std::cerr << error.what() << '\n';
		return EXIT_FAILURE;
	}

	std::cout << "Результат дописан в конец файла " << PROTOCOL_FILE << '\n';
	return EXIT_SUCCESS;
}

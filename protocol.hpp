#pragma once

#include "matrix.hpp"
#include <iostream>

// текст протокола в любой поток: на экран, в строку для теста, в файл
void writeProtocol(
	std::ostream& out,
	const Matrix& a,
	const Matrix& b,
	const Matrix& c,
	int threadCount);

// дописывает протокол в конец файла, false — файл не открылся
bool appendProtocol(
	const char* fileName,
	const Matrix& a,
	const Matrix& b,
	const Matrix& c,
	int threadCount);

// всё, что нужно потоку-писателю, указатели живы, пока main не сделает join
// объекты матриц лежат в main, поток хранит только адреса
struct ProtocolJob {
	const Matrix* a;
	const Matrix* b;
	const Matrix* c;
	int threadCount;
	int delayMs;
	const char* fileName;
};

void* protocolWriterRoutine(void* raw);

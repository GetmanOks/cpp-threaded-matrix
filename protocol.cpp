#include "protocol.hpp"

#include "pause.hpp"

#include <ctime>
#include <fstream>

namespace {

// localtime отдаёт указатель на свой внутренний буфер, не на нашу память
// его не удаляют, текст сразу копируется в свой массив buffer
void writeTimestamp(std::ostream& out)
{
	const std::time_t now = std::time(nullptr);
	const std::tm* local = std::localtime(&now);
	if (local == nullptr) {
		return;
	}

	char buffer[32] = {};
	if (std::strftime(buffer, sizeof buffer, "%Y-%m-%d %H:%M:%S", local) != 0) {
		out << buffer << '\n';
	}
}

}  // namespace

void writeProtocol(
	std::ostream& out,
	const Matrix& a,
	const Matrix& b,
	const Matrix& c,
	const int threadCount)
{
	out << "==============================\n";
	writeTimestamp(out);
	out << "Потоков: " << threadCount << '\n';
	out << "Матрица A (" << a.rows() << "x" << a.cols() << "):\n";
	a.print(out);
	out << "Матрица B (" << b.rows() << "x" << b.cols() << "):\n";
	b.print(out);
	out << "Матрица C (" << c.rows() << "x" << c.cols() << "):\n";
	c.print(out);
	out << '\n';
}

bool appendProtocol(
	const char* fileName,
	const Matrix& a,
	const Matrix& b,
	const Matrix& c,
	const int threadCount)
{
	std::ofstream out(fileName, std::ios::app);
	if (!out) {
		return false;
	}
	writeProtocol(out, a, b, c, threadCount);
	return static_cast<bool>(out);
}

void* protocolWriterRoutine(void* raw)
{
	const ProtocolJob* job = static_cast<const ProtocolJob*>(raw);
	pauseMilliseconds(job->delayMs);
	if (!appendProtocol(job->fileName, *job->a, *job->b, *job->c, job->threadCount)) {
		// любой ненулевой адрес для main значит "файл не открылся"
		return raw;
	}
	return nullptr;
}

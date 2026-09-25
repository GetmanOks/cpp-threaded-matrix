#include "matrix.hpp"
#include "multiply.hpp"
#include "protocol.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

int gFailed = 0;

void check(const bool condition, const char* name)
{
	if (condition) {
		std::cout << "ок      " << name << '\n';
		return;
	}
	std::cout << "ошибка  " << name << '\n';
	++gFailed;
}

Matrix matrixFrom(const std::vector<std::vector<long long>>& values)
{
	return Matrix(static_cast<int>(values.size()), static_cast<int>(values[0].size()), values);
}

void testSizeAndWrite()
{
	Matrix matrix(2, 3);
	check(matrix.rows() == 2 && matrix.cols() == 3, "размер 2 x 3");
	check(matrix.at(0, 0) == 0, "новая матрица заполнена нулями");

	matrix.at(1, 2) = 9;
	check(matrix.at(1, 2) == 9, "запись в клетку");
}

void testConstRead()
{
	const std::vector<std::vector<long long>> values = {{5, 6}};
	const Matrix matrix(1, 2, values);
	const long long first = matrix.at(0, 0);
	check(first == 5 && matrix.at(0, 1) == 6, "чтение константной матрицы");
}

void testBadSizeThrows()
{
	bool threw = false;
	try {
		const Matrix matrix(0, 2);
		(void)matrix;
	} catch (const std::invalid_argument&) {
		threw = true;
	}
	check(threw, "нулевой размер запрещён");
}

void testBadCellThrows()
{
	const Matrix matrix(2, 2);
	bool threw = false;
	try {
		const long long value = matrix.at(2, 0);
		(void)value;
	} catch (const std::out_of_range&) {
		threw = true;
	}
	check(threw, "клетки за краем матрицы нет");
}

void testBrokenRowThrows()
{
	const std::vector<std::vector<long long>> values = {
		{1, 2},
		{3},
	};
	bool threw = false;
	try {
		const Matrix matrix(2, 2, values);
		(void)matrix;
	} catch (const std::invalid_argument&) {
		threw = true;
	}
	check(threw, "рваная строка в данных запрещена");
}

void testKnownProduct()
{
	const Matrix a = matrixFrom({
		{1, 2},
		{3, 4},
	});
	const Matrix b = matrixFrom({
		{5, 6},
		{7, 8},
	});

	check(a.productCell(b, 0, 0) == 19, "клетка c11 равна 19");
	check(a.productCell(b, 0, 1) == 22, "клетка c12 равна 22");
	check(a.productCell(b, 1, 0) == 43, "клетка c21 равна 43");
	check(a.productCell(b, 1, 1) == 50, "клетка c22 равна 50");

	const Matrix expected = matrixFrom({
		{19, 22},
		{43, 50},
	});
	check(a.multiplySequential(b) == expected, "последовательное произведение 2 x 2");
}

void testThinProduct()
{
	const Matrix a = matrixFrom({{1, 2, 3}});
	const Matrix b = matrixFrom({
		{4},
		{5},
		{6},
	});
	const Matrix expected = matrixFrom({{32}});
	check(a.multiplySequential(b) == expected, "произведение 1 x 3 на 3 x 1 равно 32");
}

void testIncompatibleProductThrows()
{
	const Matrix a = matrixFrom({
		{1, 2, 3},
		{4, 5, 6},
	});
	const Matrix b = matrixFrom({
		{1, 2},
		{3, 4},
	});
	bool threw = false;
	try {
		const Matrix product = a.multiplySequential(b);
		(void)product;
	} catch (const std::invalid_argument&) {
		threw = true;
	}
	check(threw, "несовместимые размеры не умножаются");
}

void testParallelMatchesSequential()
{
	const Matrix a = matrixFrom({
		{1, 2},
		{3, 4},
		{5, 6},
	});
	const Matrix b = matrixFrom({
		{7, 8, 9},
		{10, 11, 12},
	});
	const Matrix expected = a.multiplySequential(b);

	// delayMs = 0 и log = false: тест не спит и не засоряет экран
	const Matrix oneThread = multiplyParallel(a, b, 1, 0, false);
	const Matrix twoThreads = multiplyParallel(a, b, 2, 0, false);
	const Matrix manyThreads = multiplyParallel(a, b, 8, 0, false);

	check(oneThread == expected, "один поток даёт то же произведение");
	check(twoThreads == expected, "два потока дают то же произведение");
	check(manyThreads == expected, "потоков больше, чем клеток, произведение то же");
}

void testRead()
{
	std::istringstream input("1 2 3 4");
	const Matrix matrix = Matrix::read(input, 2, 2);
	check(matrix == matrixFrom({{1, 2}, {3, 4}}), "чтение матрицы из потока");
}

void testShortReadThrows()
{
	std::istringstream input("1 2");
	bool threw = false;
	try {
		const Matrix matrix = Matrix::read(input, 2, 2);
		(void)matrix;
	} catch (const std::runtime_error&) {
		threw = true;
	}
	check(threw, "обрыв ввода замечен");
}

void testProtocolText()
{
	const Matrix a = matrixFrom({
		{1, 2},
		{3, 4},
	});
	const Matrix b = matrixFrom({
		{5, 6},
		{7, 8},
	});
	const Matrix c = a.multiplySequential(b);

	std::ostringstream text;
	writeProtocol(text, a, b, c, 2);
	const std::string protocol = text.str();

	check(protocol.find("Потоков: 2") != std::string::npos, "в протоколе есть число потоков");
	check(protocol.find("19 22") != std::string::npos, "в протоколе есть первая строка C");
	check(protocol.find("43 50") != std::string::npos, "в протоколе есть вторая строка C");
}

void testProtocolAppends()
{
	const char* fileName = "protocol-test.txt";
	std::remove(fileName);

	const Matrix a = matrixFrom({{1, 2}});
	const Matrix b = matrixFrom({
		{3},
		{4},
	});
	const Matrix c = a.multiplySequential(b);

	const bool first = appendProtocol(fileName, a, b, c, 1);
	const bool second = appendProtocol(fileName, a, b, c, 1);

	std::string protocol;
	{
		// пока поток открыт, Windows не даёт удалить файл
		// скобки закрывают поток до std::remove
		std::ifstream in(fileName);
		std::ostringstream text;
		text << in.rdbuf();
		protocol = text.str();
	}
	const std::size_t firstMark = protocol.find("==============================");
	const std::size_t secondMark = protocol.find("==============================", firstMark + 1);

	check(first && second, "файл протокола открылся дважды");
	check(firstMark != std::string::npos && secondMark != std::string::npos, "вторая запись дописана в конец");
	check(std::remove(fileName) == 0, "тестовый протокол удалён");
}

}  // namespace

int main()
{
	testSizeAndWrite();
	testConstRead();
	testBadSizeThrows();
	testBadCellThrows();
	testBrokenRowThrows();
	testKnownProduct();
	testThinProduct();
	testIncompatibleProductThrows();
	testParallelMatchesSequential();
	testRead();
	testShortReadThrows();
	testProtocolText();
	testProtocolAppends();

	if (gFailed == 0) {
		std::cout << "\nВсе тесты прошли\n";
		return EXIT_SUCCESS;
	}

	std::cout << "\nОшибок: " << gFailed << '\n';
	return EXIT_FAILURE;
}

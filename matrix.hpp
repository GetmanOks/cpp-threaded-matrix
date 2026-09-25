#pragma once


#include <iostream>
#include <vector>

class Matrix {
public:
	// пустая матрица заданного размера, все числа — нули
	Matrix(int rows, int cols);

	// те же размеры, но клетки уже заполнены
	Matrix(int rows, int cols, const std::vector<std::vector<long long>>& values);

	int rows() const;
	int cols() const;

	// у обычной матрицы клетка меняется, у константной ссылка тоже константная
	long long& at(int row, int col);
	const long long& at(int row, int col) const;

	// одна клетка произведения this * right.
	long long productCell(const Matrix& right, int row, int col) const;

	// всё произведение в одном потоке, чтобы с чем-то сравнивать многопоточный результат
	Matrix multiplySequential(const Matrix& right) const;

	bool operator==(const Matrix& other) const;

	void print(std::ostream& out) const;

	// читает rows * cols чисел из потока, если чисел не хватило, бросает исключение
	static Matrix read(std::istream& in, int rows, int cols);

private:
	const int rows_;
	const int cols_;

	std::vector<long long> cells_;

	std::size_t offset(int row, int col) const;
};

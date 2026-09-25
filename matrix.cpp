#include "matrix.hpp"

#include <stdexcept>

Matrix::Matrix(const int rows, const int cols)
	: rows_(rows)
	, cols_(cols)
{
	if (rows <= 0 || cols <= 0) {
		throw std::invalid_argument("Число строк и столбцов должно быть больше нуля");
	}

	const std::size_t count = static_cast<std::size_t>(rows_) * static_cast<std::size_t>(cols_);
	cells_.assign(count, 0);
}

Matrix::Matrix(const int rows, const int cols, const std::vector<std::vector<long long>>& values)
	: Matrix(rows, cols)
{
	if (static_cast<int>(values.size()) != rows_) {
		throw std::invalid_argument("Число строк в данных не совпало с размером матрицы");
	}

	for (int row = 0; row < rows_; ++row) {
		if (static_cast<int>(values[row].size()) != cols_) {
			throw std::invalid_argument("Число столбцов в данных не совпало с размером матрицы");
		}
		for (int col = 0; col < cols_; ++col) {
			at(row, col) = values[row][col];
		}
	}
}

int Matrix::rows() const
{
	return rows_;
}

int Matrix::cols() const
{
	return cols_;
}

std::size_t Matrix::offset(const int row, const int col) const
{
	if (row < 0 || col < 0 || row >= rows_ || col >= cols_) {
		throw std::out_of_range("Нет такой клетки матрицы");
	}
	return static_cast<std::size_t>(row) * static_cast<std::size_t>(cols_) + static_cast<std::size_t>(col);
}

long long& Matrix::at(const int row, const int col)
{
	return cells_[offset(row, col)];
}

const long long& Matrix::at(const int row, const int col) const
{
	return cells_[offset(row, col)];
}

long long Matrix::productCell(const Matrix& right, const int row, const int col) const
{
	if (cols_ != right.rows_) {
		throw std::invalid_argument("Для умножения столбцы левой матрицы должны совпасть со строками правой");
	}
	if (row < 0 || row >= rows_ || col < 0 || col >= right.cols_) {
		throw std::out_of_range("Нет такой клетки произведения");
	}

	long long sum = 0;
	for (int t = 0; t < cols_; ++t) {
		sum += at(row, t) * right.at(t, col);
	}
	return sum;
}

Matrix Matrix::multiplySequential(const Matrix& right) const
{
	Matrix result(rows_, right.cols());
	for (int row = 0; row < rows_; ++row) {
		for (int col = 0; col < right.cols(); ++col) {
			result.at(row, col) = productCell(right, row, col);
		}
	}
	return result;
}

bool Matrix::operator==(const Matrix& other) const
{
	return rows_ == other.rows_ && cols_ == other.cols_ && cells_ == other.cells_;
}

void Matrix::print(std::ostream& out) const
{
	for (int row = 0; row < rows_; ++row) {
		for (int col = 0; col < cols_; ++col) {
			if (col != 0) {
				out << ' ';
			}
			out << at(row, col);
		}
		out << '\n';
	}
}

Matrix Matrix::read(std::istream& in, const int rows, const int cols)
{
	Matrix matrix(rows, cols);
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			long long value = 0;
			if (!(in >> value)) {
				throw std::runtime_error("Не удалось прочитать элемент матрицы");
			}
			matrix.at(row, col) = value;
		}
	}
	return matrix;
}

#ifndef _CMATRIX_HPP_
#define _CMATRIX_HPP_

#include <iostream>
#include <vector>

template <typename TypeD>
class CMatrix {
public:
	// Constructor
	CMatrix(size_t r, size_t c, TypeD initValue = TypeD())
		: rows(r)
		, cols(c)
		, data(r, std::vector<TypeD>(c, initValue)) { }

	// Constructor from vector of vectors
	CMatrix(const std::vector<std::vector<TypeD>> & values)
		: rows(values.size())
		, cols(values[0].size())
		, data(values) { }

	// Element access
	TypeD & operator()(size_t i, size_t j) { return data[i][j]; }
	const TypeD & operator()(size_t i, size_t j) const { return data[i][j]; }

	size_t numRows() const { return rows; }
	size_t numCols() const { return cols; }

	// Matrix addition
	CMatrix<TypeD> operator+(const CMatrix<TypeD> & other) const {
		if (rows != other.rows || cols != other.cols) {
			std::cout << "Error: matrices must have the same size for addition.\n";
			return *this;
		}

		CMatrix<TypeD> result(rows, cols);
		for (size_t i = 0; i < rows; ++i)
			for (size_t j = 0; j < cols; ++j)
				result(i, j) = data[i][j] + other(i, j);

		return result;
	}

	// Matrix multiplication
	CMatrix<TypeD> operator*(const CMatrix<TypeD> & other) const {
		if (cols != other.rows) {
			std::cout << "Error: number of columns of A must equal number of rows of B for multiplication.\n";
			return CMatrix<TypeD>(rows, other.cols);
		}

		CMatrix<TypeD> result(rows, other.cols, TypeD());
		for (size_t i = 0; i < rows; ++i) {
			for (size_t j = 0; j < other.cols; ++j) {
				for (size_t k = 0; k < cols; ++k) {
					result(i, j) += data[i][k] * other(k, j);
				}
			}
		}
		return result;
	}

	// Print matrix
	void print() const {
		for (size_t i = 0; i < rows; ++i) {
			for (size_t j = 0; j < cols; ++j)
				std::cout << data[i][j] << " ";
			std::cout << "\n";
		}
	}

private:
	std::vector<std::vector<TypeD>> data;
	size_t rows;
	size_t cols;
};

#endif // _CMATRIX_HPP_

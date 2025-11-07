/**
 * \class CMatrix
 *
 * \brief Generic template class for representing and manipulating 2D matrices.
 *
 * \details This class provides a lightweight and flexible implementation of a mathematical matrix,
 * supporting initialization, element access, and common matrix operations such as addition and
 * multiplication. It is implemented as a C++ template to support multiple data types (e.g., float, double, int).
 *
 * \note The vector multiplication operator is designed for 3×3 matrices only and returns a rotated or
 * transformed vector depending on the context (e.g. in 3D rotation operations).
 *
 * \tparam TypeD The data type of the matrix elements (e.g., float, double).
 *
 * \date November 2025
 *
 * \author R. Liceras-Ramírez  
 * \b Tutor: A. Reyes-Lecuona (University of Málaga)  
 * \b Institution: Universidad de Málaga
 *
 * \b Contact: areyes@uma.es
 *
 * \b Copyright:
 * © 2025 University of Málaga. All rights reserved.
 *
 * \b Context:
 * Developed as part of the Bachelor's Thesis (TFG) of R. Liceras-Ramírez
 * under the supervision of Prof. A. Reyes-Lecuona at the University of Málaga.
 * This class is part of the CAMBISONIC framework and is used for matrix operations
 * in spatial transformations and Ambisonic rotation.
 *
 * \b Remarks:
 * - Internally stores matrix data as a 2D std::vector.
 * - Includes operator overloads for matrix arithmetic and printing.
 * - The class does not perform bounds checking beyond basic size validation.
 *
 * \b Licence:
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 */


#ifndef _CMATRIX_HPP_
#define _CMATRIX_HPP_

#include <iostream>
#include <vector>

template <typename TypeD>
class CMatrix {
public:
	CMatrix(size_t r, size_t c, TypeD initValue = TypeD())
		: rows(r)
		, cols(c)
		, data(r, std::vector<TypeD>(c, initValue)) { }

	CMatrix(const std::vector<std::vector<TypeD>> & values)
		: rows(values.size())
		, cols(values[0].size())
		, data(values) { }

	//Para inicializacion con llaves {{1,2,3},{4,5,6}}
	CMatrix(std::initializer_list<std::initializer_list<TypeD>> initList) {
		rows = initList.size();
		cols = initList.begin()->size();
		data.assign(rows, std::vector<TypeD>(cols));
		size_t i = 0;
		for (const auto & row : initList) {
			size_t j = 0;
			for (const auto & val : row)
				data[i][j++] = val;
			++i;
		}
	}

	CMatrix<TypeD> Transpose() const {
		CMatrix<TypeD> transposed(cols, rows, TypeD());

		for (size_t i = 0; i < rows; ++i) {
			for (size_t j = 0; j < cols; ++j) {
				transposed(j, i) = data[i][j];
			}
		}

		return transposed;
	}

	CMatrix & operator=(std::initializer_list<std::initializer_list<TypeD>> initList) {
		size_t i = 0;
		for (const auto & row : initList) {
			size_t j = 0;
			for (const auto & val : row)
				data[i][j++] = val;
			++i;
		}
		return *this;
	}

	TypeD & operator()(size_t i, size_t j) { return data[i][j]; }
	const TypeD & operator()(size_t i, size_t j) const { return data[i][j]; }

	size_t numRows() const { return rows; }
	size_t numCols() const { return cols; }

	CMatrix<TypeD> operator+(const CMatrix<TypeD> & other) const {
		CMatrix<TypeD> result(rows, cols);
		for (size_t i = 0; i < rows; ++i)
			for (size_t j = 0; j < cols; ++j)
				result(i, j) = data[i][j] + other(i, j);
		return result;
	}

	CMatrix<TypeD> operator*(const CMatrix<TypeD> & other) const {
		CMatrix<TypeD> result(rows, other.cols, TypeD());
		for (size_t i = 0; i < rows; ++i)
			for (size_t j = 0; j < other.cols; ++j)
				for (size_t k = 0; k < cols; ++k)
					result(i, j) += data[i][k] * other(k, j);
		return result;
	}

	Common::CVector3 operator*(const Common::CVector3 & v) const {
		if (rows != 3 || cols != 3) {
			return Common::CVector3(0.0, 0.0, 0.0);
		}

		return Common::CVector3(
			data[0][0] * v.x + data[0][1] * v.y + data[0][2] * v.z,
			data[1][0] * v.x + data[1][1] * v.y + data[1][2] * v.z,
			data[2][0] * v.x + data[2][1] * v.y + data[2][2] * v.z);
	}


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

#endif

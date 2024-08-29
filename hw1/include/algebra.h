#ifndef AUT_AP_2024_Spring_HW1
#define AUT_AP_2024_Spring_HW1

#include <format>
#include <iostream>
#include <optional>
#include <random>
#include <vector>

namespace algebra {
// Matrix data structure
template <typename T>
using MATRIX = std::vector<std::vector<T>>;

// Matrix initialization types
enum class MatrixType { Zeros, Ones, Identity, Random };

// generate random value in matrix
template <typename T, typename dist_type>
static void gen_random_matrix(MATRIX<T>& matrix, std::mt19937& engine,
                              dist_type& dist);

// Function template for matrix initialization
template <typename T>
MATRIX<T> create_matrix(std::size_t rows, std::size_t columns,
                        std::optional<MatrixType> type = MatrixType::Zeros,
                        std::optional<T> lowerBound = std::nullopt,
                        std::optional<T> upperBound = std::nullopt);

// Function template for matrix display
template <typename T>
void display(const MATRIX<T>& matrix);

// Helper function for getting matrix size.
template <typename T>
std::pair<size_t, size_t> matrix_size(const MATRIX<T>& matrix);

// Matrix sum or sub
template <typename T>
MATRIX<T> sum_sub(const MATRIX<T>& matrixA, const MATRIX<T>& matrixB,
                  std::optional<std::string> operation = "sum");

// Matrix scalar multiplication
template <typename T>
MATRIX<T> multiply(const MATRIX<T>& matrix, const T scalar);

// Matrix multiplication
template <typename T>
MATRIX<T> multiply(const MATRIX<T>& matrixA, const MATRIX<T>& matrixB);

////////////////////////////
////// Implementation //////
////////////////////////////

template <typename T, typename dist_type>
static void gen_random_matrix(MATRIX<T>& matrix, std::mt19937& engine,
                              dist_type& dist) {
  for (size_t i = 0; i < matrix.size(); i++) {
    for (size_t j = 0; j < matrix[0].size(); j++) {
      matrix[i][j] = dist(engine);
    }
  }
}

template <typename T>
MATRIX<T> create_matrix(std::size_t rows, std::size_t columns,
                        std::optional<MatrixType> type,
                        std::optional<T> lowerBound,
                        std::optional<T> upperBound) {
  // check matrix dimension
  if (rows == 0 or columns == 0) {
    if (rows == columns) {
      MATRIX<T> m;
      return m;
    }
    throw std::logic_error("The matrix dimension must be larger than 0.");
  }

  MATRIX<T> m = MATRIX<T>(rows, std::vector<T>(columns));
  switch (type.value()) {
    case MatrixType::Zeros:
      for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < columns; j++) {
          m[i][j] = 0;
        }
      }
      break;
    case MatrixType::Ones:
      for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < columns; j++) {
          m[i][j] = 1;
        }
      }
      break;
    case MatrixType::Identity:
      if (rows != columns) {
        throw std::logic_error("An identity matrix must be square.");
      }
      for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < columns; j++) {
          if (i == j)
            m[i][j] = 1;
          else
            m[i][j] = 0;
        }
      }
      break;
    case MatrixType::Random:
      if (!(lowerBound.has_value() && upperBound.has_value())) {
        throw std::logic_error("The value bound must be set.");
      }
      if (!(*lowerBound < *upperBound)) {
        throw std::logic_error(
            "The lower bound must be smaller than the upper one.");
      }
      std::random_device rd;
      std::mt19937 engine(rd());
      if constexpr (std::is_integral<T>::value) {
        std::uniform_int_distribution<T> dist(*lowerBound, *upperBound);
        gen_random_matrix(m, engine, dist);
      } else if (std::is_floating_point<T>::value) {
        std::uniform_real_distribution<T> dist(*lowerBound, *upperBound);
        gen_random_matrix(m, engine, dist);
      } else {
        throw std::logic_error(
            "The template type must be integer or floating point number.");
      }
      break;
  }
  return m;
}

template <typename T>
void display(const MATRIX<T>& matrix) {
  for (const auto& row : matrix) {
    for (const double elem : row) {
      const int max_width = 7;
      int num_width = max_width;
      if (elem < 0) num_width -= 1;  // negative sign
      std::string str = std::format("{:^{}.{}}", elem, max_width, num_width);
      if (str.length() > max_width) {
        num_width -= 5;  // scientific format
        str = std::format("{:^{}.{}}", elem, max_width, num_width);
      }
      std::cout << "|" << str;
    }
    std::cout << "|\n";
  }
}

template <typename T>
std::pair<size_t, size_t> matrix_size(const MATRIX<T>& matrix) {
  size_t rows = matrix.size();
  size_t columns = (rows == 0) ? 0 : (matrix[0]).size();
  return std::make_pair(rows, columns);
}

template <typename T>
MATRIX<T> sum_sub(const MATRIX<T>& matrixA, const MATRIX<T>& matrixB,
                  std::optional<std::string> operation) {
  auto sizeA = matrix_size(matrixA);
  auto sizeB = matrix_size(matrixB);
  if (sizeA != sizeB) {
    throw std::logic_error("Matrix dimensions are not same.");
  }
  if (!operation.has_value() or
      !(operation.value() == "sum" or operation.value() == "sub")) {
    operation = "sum";
  }

  MATRIX<T> res = matrixA;
  size_t rowsA = sizeA.first;
  size_t columnsA = sizeA.second;
  if (operation.value() == "sum") {
    for (size_t i = 0; i < rowsA; i++)
      for (size_t j = 0; j < columnsA; j++) res[i][j] += matrixB[i][j];
  } else {
    for (size_t i = 0; i < rowsA; i++)
      for (size_t j = 0; j < columnsA; j++) res[i][j] -= matrixB[i][j];
  }
  return res;
}

template <typename T>
MATRIX<T> multiply(const MATRIX<T>& matrix, const T scalar) {
  MATRIX<T> res = matrix;
  for (auto& row : res)
    for (auto& elem : row) elem *= scalar;
  return res;
}

template <typename T>
MATRIX<T> multiply(const MATRIX<T>& matrixA, const MATRIX<T>& matrixB) {
  const auto sizeA = matrix_size(matrixA);
  const auto sizeB = matrix_size(matrixB);
  if (sizeA.first == 0 or sizeB.first == 0) {
    throw std::logic_error("Matrix is empty.");
  }
  if (sizeA.second != sizeB.first) {
    throw std::logic_error("Matrix dimensions do not match.");
  }

  MATRIX<T> res = create_matrix<T>(sizeA.first, sizeB.second, MatrixType::Zeros);
  for (size_t i = 0; i < sizeA.first; i++) {
    for (size_t k = 0; k < sizeA.second; k++) {
      T tmp = matrixA[i][k];
      for (size_t j = 0; j < sizeB.second; j++) {
        res[i][j] += (tmp * matrixB[k][j]);
      }
    }
  }
  return res;
}

}  // namespace algebra

#endif  // AUT_AP_2024_Spring_HW1

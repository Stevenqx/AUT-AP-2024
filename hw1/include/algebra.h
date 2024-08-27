#ifndef AUT_AP_2024_Spring_HW1
#define AUT_AP_2024_Spring_HW1

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

// generate random value in matrix
template <typename T, typename dist_type>
static void gen_random_matrix(MATRIX<T>& matrix, std::mt19937& engine,
                              dist_type& dist) {
  for (size_t i = 0; i < matrix.size(); i++) {
    for (size_t j = 0; j < matrix[0].size(); j++) {
      matrix[i][j] = dist(engine);
    }
  }
}

// Function template for matrix initialization
template <typename T>
MATRIX<T> create_matrix(std::size_t rows, std::size_t columns,
                        std::optional<MatrixType> type,
                        std::optional<T> lowerBound,
                        std::optional<T> upperBound) {
  // check matrix dimension
  if (rows == 0 or columns == 0) {
    throw "The matrix dimension must be larger than 0.";
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
        throw "An identity matrix must be square.";
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
        throw "The value bound must be set.";
      }
      if (!(*lowerBound < *upperBound)) {
        throw "The lower bound must be smaller than the upper one.";
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
        throw "The template type must be integer or floating point number.";
      }
      break;
  }
  return m;
}

}  // namespace algebra

#endif  // AUT_AP_2024_Spring_HW1

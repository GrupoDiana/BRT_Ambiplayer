/**
 * \class CommonDefinitions
 *
 * \brief Provides global mathematical constants used in Ambisonic processing.
 *
 * \date April 2026
 *
 * \author R. Liceras-Ramírez  
 * \b Tutor: A. Reyes-Lecuona and D. Gonzalez-Toledo  
 * \b Institution: Universidad de Málaga
 *
 * \b Context:
 * This header is part of a real-time higher-order Ambisonic spatial audio
 * system developed for a Bachelor's Thesis (TFG). It supports efficient
 * spherical harmonic computations used in encoding and decoding stages.
 *
 * \b Licence:
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 */

#ifndef COMMON_DEFINITIONS_HPP_
#define COMMON_DEFINITIONS_HPP_

namespace CommonDefinitions {
constexpr float SQRT_3 = 1.7320508f;
constexpr float SQRT_3_DIV_4 = 0.8660254f;
constexpr float SQRT_5_DIV_8 = 0.7905694f;
constexpr float SQRT_15 = 3.8729833f;
constexpr float SQRT_3_DIV_8 = 0.6123724f;
constexpr float SQRT_15_DIV_4 = 1.9364917f;
}

#endif // COMMON_DEFINITIONS_HPP_

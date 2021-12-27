/* Fill in your Name and GNumber in the following two comment fields
 * Name: Connor Baker
 * GNumber: G01094252
 */
#include "./fp_functs.h"
#include "./fp.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Overview of format
// padding (17b)           sign (1b)  exp (5b)  frac (9b)     (32bits total)
// 0 0000 0000 0000 0000   0          0 0000    0 0000 0000
#define CUSTOM_BIAS 15

#define CUSTOM_PAD_MASK 0x1FFu
#define CUSTOM_PAD_OFFSET 23u
#define CUSTOM_SIGN_MASK 1u
#define CUSTOM_SIGN_OFFSET 14u
#define CUSTOM_EXP_MASK 0x1Fu
#define CUSTOM_EXP_OFFSET 9u
#define CUSTOM_MANTISSA_MASK 0x1FFu
#define CUSTOM_MANTISSA_OFFSET 0u

#define CUSTOM_POS_ZERO 0u
#define CUSTOM_NEG_ZERO (1u << CUSTOM_SIGN_OFFSET)
#define CUSTOM_POS_INFINITY (CUSTOM_EXP_MASK << CUSTOM_EXP_OFFSET)
#define CUSTOM_NEG_INFINITY (CUSTOM_NEG_ZERO | CUSTOM_POS_INFINITY)
#define CUSTOM_POS_NAN (CUSTOM_POS_INFINITY | 1u)
#define CUSTOM_NEG_NAN (CUSTOM_NEG_ZERO | CUSTOM_POS_NAN)

/* Checks if -0.0 on input value.
 * Returns 1 if -0.0
 * Returns 0 otherwise.
 */
static int is_negative_zero(float val) {
  return (val == 0.0 && ((1.f / val) < 0));
}

/* input: float value to be represented
 * output: integer representation of the input float
 *
 * Perform this the same way we did in class -
 *   either dividing or multiplying the value by 2
 *   until it is in the correct range (between 1 and 2).
 *
 * Follow the Project Documentation for Instructions
 */
int compute_fp(float val) {
  float temp = val;
  int32_t s = (0.0f == temp) ? is_negative_zero(temp) : temp <= 0.0f;
  int32_t exp = CUSTOM_BIAS;
  int32_t m = 0;

  // Check for zero
  if (0.0f == temp) {
    return s ? CUSTOM_NEG_ZERO : CUSTOM_POS_ZERO;
  }

  // Check for infinity
  if (isinf(temp)) {
    return s ? CUSTOM_NEG_INFINITY : CUSTOM_POS_INFINITY;
  }

  // Check for NaN
  if (isnan(temp)) {
    return s ? CUSTOM_NEG_NAN : CUSTOM_POS_NAN;
  }

  // Take the absolute value of the float now that we know the sign
  temp *= (temp > 0.0f) ? 1.0f : -1.0f;

  // Check if the number is less than one
  while (temp < 1.0f) {
    temp *= 2.0f;
    exp--;
  }

  // Check if the number is greater than or equal to two
  while (2.0f <= temp) {
    temp /= 2.0f;
    exp++;
  }

  // Check for underflow
  if (exp <= 0) {
    return s ? CUSTOM_NEG_ZERO : CUSTOM_POS_ZERO;
  }

  // Check for overflow
  if (CUSTOM_EXP_MASK <= exp) {
    return s ? CUSTOM_NEG_INFINITY : CUSTOM_POS_INFINITY;
  }

  // Calculate the mantissa, sign, and exp
  m = (uint32_t)((temp - 1.0f) * 512.0f);
  s = (uint32_t)s << CUSTOM_SIGN_OFFSET;
  exp = (uint32_t)exp << CUSTOM_EXP_OFFSET;

  uint32_t ret = s | exp | m;

  return (int)ret;
}

/* input: integer representation of our floats
 * output: float value represented by our integer encoding
 *
 * Follow the Project Documentation for Instructions
 */
float get_fp(int val) {
  /* Implement this function */
  uint32_t s = (uint32_t)val & (CUSTOM_SIGN_MASK << CUSTOM_SIGN_OFFSET);
  int32_t E = (((uint32_t)val & (CUSTOM_EXP_MASK << CUSTOM_EXP_OFFSET)) >>
               CUSTOM_EXP_OFFSET) -
              CUSTOM_BIAS;

  // Check for zero
  if (CUSTOM_NEG_ZERO == val || CUSTOM_POS_ZERO == val) {
    return s ? -0.0f : 0.0f;
  }

  // Check for infinity
  if (CUSTOM_NEG_INFINITY == val || CUSTOM_POS_INFINITY == val) {
    return s ? -1.0f * INFINITY : INFINITY;
  }

  // Check for NaN
  if (CUSTOM_NEG_NAN == val || CUSTOM_POS_NAN == val) {
    return s ? -0.0f / 1.0f / 0.0f : 0.0f / -1.0f / 0.0f;
  }

  float temp = ((uint32_t)val & CUSTOM_MANTISSA_MASK) / 512.0f + 1.0f;

  while (E < 0) {
    temp /= 2.0f;
    E++;
  }

  while (0 < E) {
    temp *= 2.0f;
    E--;
  }

  temp *= s ? -1.0f : 1.0f;

  return temp;
}

/* input: Two integer representations of our floats
 * output: The multiplication result in our integer representation
 *
 * You must implement this using hte algorithm described in class:
 *   Add the exponents: E = E1 + E2
 *   Multiply the Frac Values: M = M1 * M2
 *   If M is not in a valid range, adjust M and E.
 *
 * Follow the Project Documentation for Instructions
 */
int mult_vals(int source1, int source2) {
  // Get the different components of x and y
  // Get the sign bits
  uint32_t x_s =
      ((uint32_t)source1 & (CUSTOM_SIGN_MASK << CUSTOM_SIGN_OFFSET)) >>
      CUSTOM_SIGN_OFFSET;
  uint32_t y_s =
      ((uint32_t)source2 & (CUSTOM_SIGN_MASK << CUSTOM_SIGN_OFFSET)) >>
      CUSTOM_SIGN_OFFSET;
  uint32_t s = x_s ^ y_s;

  // Get the exp from x and y
  uint32_t x_exp =
      ((uint32_t)source1 & (CUSTOM_EXP_MASK << CUSTOM_EXP_OFFSET)) >>
      CUSTOM_EXP_OFFSET;
  uint32_t y_exp =
      ((uint32_t)source2 & (CUSTOM_EXP_MASK << CUSTOM_EXP_OFFSET)) >>
      CUSTOM_EXP_OFFSET;
  // Cast to signed to avoid doing unsigned arithmetic
  int32_t exp = ((int32_t)x_exp) + ((int32_t)y_exp) - CUSTOM_BIAS;

  // Calculate the new mantissa next
  uint32_t x_m = (uint32_t)source1 & CUSTOM_MANTISSA_MASK;
  uint32_t y_m = (uint32_t)source2 & CUSTOM_MANTISSA_MASK;

  // Check for special cases up front
  // If either of them is NaN, return NaN
  // If one is infinity and the other is not zero return a sign appropriate
  // infinity If one is infinity and the other is zero return NaN
  if (CUSTOM_NEG_NAN == source1 || CUSTOM_POS_NAN == source1 ||
      CUSTOM_NEG_NAN == source2 || CUSTOM_POS_NAN == source2) {
    return s ? CUSTOM_NEG_NAN : CUSTOM_POS_NAN;
  } else if (CUSTOM_EXP_MASK == x_exp || CUSTOM_EXP_MASK == y_exp) {
    // If one is infinity and the other is not zero, return a sign appropriate
    // infinity
    if (((CUSTOM_NEG_INFINITY == source1 || CUSTOM_POS_INFINITY == source1) &&
         (CUSTOM_NEG_ZERO == source2 || CUSTOM_POS_ZERO == source2)) ||
        ((CUSTOM_NEG_INFINITY == source2 || CUSTOM_POS_INFINITY == source2) &&
         (CUSTOM_NEG_ZERO == source1 || CUSTOM_POS_ZERO == source1))) {
      return CUSTOM_POS_NAN;
    } else {
      return s ? CUSTOM_NEG_INFINITY : CUSTOM_POS_INFINITY;
    }
  }

  // Check for zero, which is a denormalized value
  if (CUSTOM_POS_ZERO == source1 || CUSTOM_NEG_ZERO == source1 ||
      CUSTOM_POS_ZERO == source2 || CUSTOM_NEG_ZERO == source2) {
    return s ? CUSTOM_NEG_ZERO : CUSTOM_POS_ZERO;
  }

  // Turn on the implicit one bits now that we know we'll multiply
  // Each one is 9 bits. The maximum number of digits required in integer bases
  // for the square of an n-digit number is 2n. So when we're checking for
  // overflow, we should shift the product 9 times to the right, and then
  // continue to shift until we're left with zero. As a corollary, that means
  // that the product will fit inside the 32bit integer type we're using.
  x_m |= 1u << CUSTOM_EXP_OFFSET;
  y_m |= 1u << CUSTOM_EXP_OFFSET;
  uint32_t m = (x_m * y_m) >> CUSTOM_EXP_OFFSET;

  // After shifting, the bits that are directly to the left of the decimal place
  // have four possible values
  //  00 <=> 0           <= a + b < 2
  //  01 <=> 2           <= a + b < sqrt(2) + 1
  //  10 <=> sqrt(2) + 1 <= a + b < sqrt(3) + 1
  //  11 <=> sqrt(3) + 1 <= a + b < 4
  // where a and b are the decimal number representations of the floats encoded
  // by the custom type.
  if (m & (1u << CUSTOM_EXP_OFFSET)) {
    // Case 01_2
    // 2 <= a + b < sqrt(2) + 1
    // Turn off the one
    m &= ~(1u << CUSTOM_EXP_OFFSET);
  } else if (m & (2u << CUSTOM_EXP_OFFSET)) {
    // Case 10_2
    // sqrt(2) + 1 <= a + b < sqrt(3) + 1
    // Turn off the two
    m &= ~(2u << CUSTOM_EXP_OFFSET);
    // Divide by two and update exp
    m >>= 1u;
    exp++;
  } else if (m & (3u << CUSTOM_EXP_OFFSET)) {
    // Case 11_2
    // sqrt(3) + 1 <= a + b < 4
    m &= ~(3u << CUSTOM_EXP_OFFSET);
    // Divide by two and update exp
    m >>= 1u;
    exp++;
  } else {
    // Case 00_2
    // 0 <= a + b < 2
    // The product is less than one. No need to turn any bits off.
  }

  // Check for overflow or underflow
  if (exp <= 0) {
    // Underflow!
    return s ? CUSTOM_NEG_ZERO : CUSTOM_POS_ZERO;
  } else if (CUSTOM_EXP_MASK <= exp) {
    return s ? CUSTOM_NEG_INFINITY : CUSTOM_POS_INFINITY;
  } else {
    // All is as expected
  }

  // Position the sign and exp
  s <<= CUSTOM_SIGN_OFFSET;
  exp <<= CUSTOM_EXP_OFFSET;

  uint32_t ret = s | exp | m;

  return ret;
}

/* input: Two integer representations of our floats
 * output: The addition result in our integer representation
 *
 * You must implement this using hte algorithm described in class:
 *   If needed, adjust the numbers to get the same exponent E
 *   Add the two fractional parts: F = F1' + F2
 *   (where F1' is the adjusted F1 if needed)
 *   Adjust the sum F and E so that F is in the correct range.
 *
 * Follow the Project Documentation for Instructions
 */
int add_vals(int source1, int source2) {
  // Get the different components of x and y
  // Get the sign bits
  uint32_t x_s =
      ((uint32_t)source1 & (CUSTOM_SIGN_MASK << CUSTOM_SIGN_OFFSET)) >>
      CUSTOM_SIGN_OFFSET;
  uint32_t y_s =
      ((uint32_t)source2 & (CUSTOM_SIGN_MASK << CUSTOM_SIGN_OFFSET)) >>
      CUSTOM_SIGN_OFFSET;

  // Get the exp from x and y
  uint32_t x_exp =
      ((uint32_t)source1 & (CUSTOM_EXP_MASK << CUSTOM_EXP_OFFSET)) >>
      CUSTOM_EXP_OFFSET;
  uint32_t y_exp =
      ((uint32_t)source2 & (CUSTOM_EXP_MASK << CUSTOM_EXP_OFFSET)) >>
      CUSTOM_EXP_OFFSET;

  // Get the new mantissa next
  int64_t x_m = ((uint32_t)source1 & CUSTOM_MANTISSA_MASK);
  int64_t y_m = ((uint32_t)source2 & CUSTOM_MANTISSA_MASK);

  // Check for special cases up front
  // If either of them is NaN, return NaN
  // If both are infinity of the same sign, return that infinity
  // If both are infinities of different types, return NaN
  // If one is infinity and one is not, return the infinity
  if (CUSTOM_NEG_NAN == source1 || CUSTOM_POS_NAN == source1) {
    return source1;
  } else if (CUSTOM_NEG_NAN == source2 || CUSTOM_POS_NAN == source2) {
    return source2;
  } else if (CUSTOM_EXP_MASK == x_exp && CUSTOM_EXP_MASK == y_exp) {
    if ((x_m == y_m) && (x_s == y_s)) {
      // If they're both special and the same, return that
      return source1;
    } else {
      // If they're both special and different, return NaN
      return CUSTOM_POS_NAN;
    }
  } else if ((CUSTOM_EXP_MASK == x_exp) ^ (CUSTOM_EXP_MASK == y_exp)) {
    // If only one of them is special, return the special one
    if (CUSTOM_EXP_MASK == x_exp) {
      return source1;
    } else {
      return source2;
    }
  }

  x_m |= 1u << CUSTOM_EXP_OFFSET;
  y_m |= 1u << CUSTOM_EXP_OFFSET;

  // Check for zero, which is a denormalized value
  if (CUSTOM_POS_ZERO == source1) {
    if (CUSTOM_NEG_ZERO == source2) {
      return source1;
    }
    return source2;
  } else if (CUSTOM_POS_ZERO == source2) {
    if (CUSTOM_NEG_ZERO == source1) {
      return source2;
    }
    return source1;
  } else if (CUSTOM_NEG_ZERO == source1) {
    return source2;
  } else if (CUSTOM_NEG_ZERO == source2) {
    return source1;
  }

  // Use the smallest exponent as the baseline
  if (x_exp > y_exp) {
    uint32_t move = x_exp - y_exp;
    while (move > 0) {
      x_m <<= 1u;
      move--;
    }
  } else {
    uint32_t move = y_exp - x_exp;
    while (move > 0) {
      y_m <<= 1u;
      move--;
    }
  }

  // Modify the signs so that they are correct
  x_m *= x_s ? -1 : 1;
  y_m *= y_s ? -1 : 1;

  // Do the deed
  int64_t m_signed = x_m + y_m;

  // Get the sign of the result
  int32_t s = (m_signed < 0) ? 1 : 0;

  if (0 == m_signed) {
    return s ? CUSTOM_NEG_ZERO : CUSTOM_POS_ZERO;
  }

  // Now that we have the sign, make the mantissa positive
  uint64_t m = (m_signed < 0) ? ((uint64_t)-1 * m_signed) : (uint64_t)m_signed;

  // Set exp to the smaller of the two
  int32_t exp = (x_exp > y_exp) ? y_exp : x_exp;

  // We need to shift so that we have the mantissa back into the correct format
  // (at least a one bit in var[9])
  while ((~((uint64_t)CUSTOM_MANTISSA_MASK) << 1u) & m) {
    m >>= 1u;
    exp++;
  }

  while (!(m & (1u << (CUSTOM_EXP_OFFSET)))) {
    m <<= 1u;
    exp--;
  }

  // Turn off the implicit one bit
  m &= CUSTOM_MANTISSA_MASK;

  // Check for overflow or underflow
  if (exp <= 0) {
    // Underflow!
    return s ? CUSTOM_NEG_ZERO : CUSTOM_POS_ZERO;
  } else if (CUSTOM_EXP_MASK <= exp) {
    return s ? CUSTOM_NEG_INFINITY : CUSTOM_POS_INFINITY;
  } else {
    // All is as expected
  }

  // Position the sign and exp
  s <<= CUSTOM_SIGN_OFFSET;
  exp <<= CUSTOM_EXP_OFFSET;

  uint32_t ret = s | exp | m;

  return ret;
}

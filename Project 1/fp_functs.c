/* Fill in your Name and GNumber in the following two comment fields
 * Name: Siddharthkumar Patel
 * GNumber: G01075115
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fp.h"

//CONSTANTS
#define PZERO (0u)
#define NZERO (1 << 14)


//Preprocessors
void findMandEFromFloat(float *M, int *E);
void getFracfromME(float M, int E, int *frac);
int getBias();
void findSignFromFloat(int *S, float val);
void extractFracFromVal(int *frac, fp_gmu val);
void setCustom(fp_gmu *mini_float, int *S, int *exp, int *frac);

/**
 * This function calculates the bias based on the exp bits and returns it.
 */
int getBias() {
	int bias = 31;
	return bias;
}

/**
 * Given a floating-point representation, this function assigns a sign to the S variable
 * based on whether the floating-point representation is positive or negative.
 */
void findSignFromFloat(int *S, float val) {
	if(val < 0) {
		*S = 1;
	}
	else {
		*S = 0;
	}
}

/**
 * This function takes an M and E value and does normalized calculation by shifting M until
 * it is in the range [1,2].
 */
void findMandEFromFloat(float *M, int *E) {
	while(*M < 1.0f) {
		*M = (*M) * 2.0f;
		*E -= 1;
	}

	while(*M >= 2.0f) {
		*M = (*M) / 2.0f;
		*E += 1;
	}
}

/**
 * This function is used to shift a denormalized number that is greater than -30 to E = 30.
 */
void shiftDenormalized(float *M, int *E) {
	while(*E != -30) {
		*M = *M/2.0f;
		*E = *E + 1;
	}
}

/**
 * This function finds a fraction from Mantissa and E for a normalized floating point.
 */
void getFracfromME(float M, int E, int *frac) {
	*frac = (M - 1) * 256.0f;
}


/**
 * This extracts fracation from a floating point number.
 */
void extractFracFromVal(int *frac, fp_gmu val) {
	val &= 0xFF;
	*frac = val;
}

void setCustom(fp_gmu *mini_float, int *S, int *exp, int *frac) {
	*S = *S << 14;
	*mini_float = *S;
	*exp = *exp << FRACTION_BITS;
	*mini_float |= *exp;
	*mini_float |= *frac;
}

void setCustomDenormalized(fp_gmu *mini_float, int *S, int *frac) {
	*S = *S << 14;
	*mini_float = *S;
	*mini_float |= *frac;
}

/**
 * Checks whether the sign of zero is positive or negative.
 */
int signZero(float num) {
	int s = (1.0f/num) < 0;
	return s;
}

/* input: float value to be represented
 * output: fp_gmu representation of the input float
 *
 * Follow the Project Documentation for Instructions
 */
fp_gmu compute_fp(float val) {
  /* Implement this function */
	int S;
	float M = val;
	int E;
	int frac;
	int exp;
	float temp = val;
	int signBit = NULL;

	findSignFromFloat(&S, val);
	if(M < 0) {
    M = M * -1.0f;
	}

  if(val == 0.0f) {
    signBit = signZero(val);
		if(signBit) {
      return NZERO;
		} 
		else {
      return PZERO;
		}
	}

	findMandEFromFloat(&M, &E);
	exp = E + getBias();
	if(exp > 0) {
		//Now that we have M, we will get the frac value.
		getFracfromME(M, E, &frac);
	}
	else {
		shiftDenormalized(&M, &E);
		frac = M * 256.0f;
	}

	//Creating gmu_fp type
	fp_gmu mini_float;
	if(exp <= 0) {
		setCustomDenormalized(&mini_float, &S, &frac);
	}
	else {
		setCustom(&mini_float, &S, &exp, &frac);
	}
  return mini_float;
}

int findSFromCustom(fp_gmu val) {
	int sign_bit = 0x4000;
	return (val & sign_bit) >> 14;
}

int findExpFromCustom(fp_gmu val) {
	int exp_bit = 0x3F00;
	return (val & exp_bit) >> 8;
}

float findMFromCustom(fp_gmu val) {
	int frac_bit = 0xFF;
	return (val & frac_bit)/256.0f;
}

/* input: fp_gmu representation of our floats
 * output: float value represented by our fp_gmu encoding
 *
 * If your input fp_gmu is Infinity, return the constant INFINITY
 * If your input fp_gmu is -Infinity, return the constant -INFINITY
 * If your input fp_gmu is NaN, return the constant NAN
 *
 * Follow the Project Documentation for Instructions
 */
float get_fp(fp_gmu val) {
  /* Implement this function */

	int S = findSFromCustom(val);

	int exp = findExpFromCustom(val);
	int E = exp - getBias();
  //zero check
	if(val == PZERO || val == NZERO) {
    if(S) {
       return -0.0f;
		} else {
       return 0.0f;
		}
	}

  int frac = val & 0xFF;

	if(exp == 0 && frac >= 1) {
		int exp = 0;
		int E = -30;
		int d_sign = S;
		float num = frac/256.0f;

		while (E < exp) {
      num /= 2.0f;
			exp = exp - 1;
		}

    if(d_sign) {
      num = num * -1.0f;
		} 
		else {
			num = num * 1.0f;
		}
		return num;
	}

	float M = findMFromCustom(val);
	M = M + 1;

	while(E > 0) {
		M = M * 2.0f;
		E = E - 1;
	}
	while(E < 0) {
		M = M/2.0f;
		E = E + 1;
	}
	if(S) {
		M = M * -1.0f;
	}
	else {
		M = M * 1.0f;
	}
  return M;
}

/* input: Two fp_gmu representations of our floats
 * output: The multiplication result in our fp_gmu representation
 *
 * You must implement this using the algorithm described in class:
 *   Xor the signs: S = S1 ^ S2
 *   Add the exponents: E = E1 + E2
 *   Multiply the Frac Values: M = M1 * M2
 *   If M is not in a valid range for normalized, adjust M and E.
 *
 * Follow the Project Documentation for Instructions
 */
fp_gmu mult_vals(fp_gmu source1, fp_gmu source2) {
  /* Implement this function */
	int sign1 = 0;
	int exp1 = 0;
	int E1 = 0;
	float mant1 = 0;

	int sign2 = 0;
	int exp2 = 0;
	int E2 = 0;
	float mant2 = 0;

	int S = 0;
	float M = 0;
	int E = 0;
	int exp = 0;
	int frac = 0;

	sign1 = findSFromCustom(source1);
	sign2 = findSFromCustom(source2);

	exp1 = findExpFromCustom(source1);
	E1 = exp1 - getBias();
	exp2 = findExpFromCustom(source2);
	E2 = exp2 - getBias();

	mant1 = get_fp(source1);
	mant2 = get_fp(source2);

	S = sign1 ^ sign2;
	E = E1 + E2;
	exp = E + getBias();

	int frac1 = source1 & 0xFF;
  int frac2 = source2 & 0xFF;

	mant1 = (frac1 / 256) + 1.0f;
	mant2 = (frac2 / 256) + 1.0f;

	M = mant1 * mant2;

	if(M < 0) {
   M = M * -1.0f; 
	}
  
	float final_frac = (M - 1.0f);
	final_frac = final_frac * 256.0f;

	int fr = final_frac;
   printf(" exp %d\n",exp);
	 printf("frac %d\n",fr);
	int mini_float = ( (S << 14u) | (exp << 8u) | fr);

  return mini_float;
}

void alignE(float *M1, int *E1, float *M2, int *E2) {
	while(*E2 < *E1) {
		*M2 = *M2 / 2.0f;
		*E2 = *E2 + 1;
	}
	while(*E1 < *E2) {
		*M1 = *M1 / 2.0f;
		*E1 = *E1 + 1;
	}
}

/* input: Two fp_gmu representations of our floats
 * output: The addition result in our fp_gmu representation
 *
 * You must implement this using the algorithm described in class:
 *   If needed, adjust the numbers to get the same exponent E
 *   Add the two adjusted Mantissas: M = M1 + M2
 *   Adjust M and E so that it is in the correct range for Normalized
 *
 * Follow the Project Documentation for Instructions
 */
fp_gmu add_vals(fp_gmu source1, fp_gmu source2) {
  /* Implement this function */
	int sign1 = 0;
	int exp1 = 0;
	int E1 = 0;
	float mant1 = 0;

	int sign2 = 0;
	int exp2 = 0;
	int E2 = 0;
	float mant2 = 0;

	int S = 0;
	float M = 0;
	int E = 0;
	int exp = 0;
	int frac = 0;

	sign1 = findSFromCustom(source1);
	sign2 = findSFromCustom(source2);

	exp1 = findExpFromCustom(source1);
	E1 = exp1 - getBias();
	exp2 = findExpFromCustom(source2);
	E2 = exp2 - getBias();

	mant1 = get_fp(source1);
	mant2 = get_fp(source2);

	alignE(&mant1, &E1, &mant2, &E2);
	M = mant1 + mant2;
	E = E1;
	findSignFromFloat(&S, M);

	fp_gmu mini_float;
	findMandEFromFloat(&M, &E);
	exp = E + getBias();
	getFracfromME(M, E, &frac);

	setCustom(&mini_float, &S, &exp, &frac);

  return mini_float;
}

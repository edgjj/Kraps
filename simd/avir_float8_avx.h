//$ nobt
//$ nocpp

/**
 * @file avir_float8_avx.h
 *
 * @brief Inclusion file for the "float8" type.
 *
 * This file includes the "float8" AVX-based type used for SIMD variable
 * storage and processing.
 *
 * AVIR Copyright (c) 2015-2020 Aleksey Vaneev
 */

#ifndef AVIR_FLOAT8_AVX_INCLUDED
#define AVIR_FLOAT8_AVX_INCLUDED

#include <immintrin.h>
#include "avx_mathfun.h"
#include <cstdint>
namespace kraps {

/**
 * @brief SIMD packed 8-float type.
 *
 * This class implements a packed 8-float type that can be used to perform
 * parallel computation using SIMD instructions on AVX-enabled processors.
 */

class float8
{
public:
	float8() : value ( _mm256_set1_ps (0.0f))
	{

	}

	float8( const float8& s )
		: value( s.value )
	{
	}

	float8( const __m256 s )
		: value( s )
	{
	}

	float8(const __m256i s)
		: value(_mm256_cvtepi32_ps(s))
	{
	}

	float8( const float s )
		: value( _mm256_set1_ps( s ))
	{
	}

	float8& operator = ( const float8& s )
	{
		value = s.value;
		return( *this );
	}

	float8& operator = ( const __m256 s )
	{
		value = s;
		return( *this );
	}

	float8& operator = ( const float s )
	{
		value = _mm256_set1_ps( s );
		return( *this );
	}

	operator float () const
	{
		return( _mm_cvtss_f32( _mm256_extractf128_ps( value, 0 )));
	}

	operator __m256 () const
	{
		return value;
	}

	/**
	 * @param p Pointer to memory from where the value should be loaded,
	 * should be 32-byte aligned.
	 * @return float8 value loaded from the specified memory location.
	 */

	static float8 load( const float* const p )
	{
		return( _mm256_load_ps( p ));
	}

	/**
	 * @param p Pointer to memory from where the value should be loaded,
	 * may have any alignment.
	 * @return float8 value loaded from the specified memory location.
	 */

	static float8 loadu( const float* const p )
	{
		return( _mm256_loadu_ps( p ));
	}

	/**
	 * @param p Pointer to memory from where the value should be loaded,
	 * may have any alignment.
	 * @param lim The maximum number of elements to load, >0.
	 * @return float8 value loaded from the specified memory location, with
	 * elements beyond "lim" set to 0.
	 */

	static float8 loadu( const float* const p, const int lim )
	{
		__m128 lo;
		__m128 hi;

		if( lim > 4 )
		{
			lo = _mm_loadu_ps( p );
			hi = loadu4( p + 4, lim - 4 );
		}
		else
		{
			lo = loadu4( p, lim );
			hi = _mm_setzero_ps();
		}

		return( _mm256_insertf128_ps( _mm256_castps128_ps256( lo ), hi, 1 ));
	}

	/**
	 * Function stores *this value to the specified memory location.
	 *
	 * @param[out] p Output memory location, should be 32-byte aligned.
	 */

	void store( float* const p ) const
	{
		_mm256_store_ps( p, value );
	}

	/**
	 * Function stores *this value to the specified memory location.
	 *
	 * @param[out] p Output memory location, may have any alignment.
	 */

	void storeu( float* const p ) const
	{
		_mm256_storeu_ps( p, value );
	}


	/**
	 * Function stores "lim" lower elements of *this value to the specified
	 * memory location.
	 *
	 * @param[out] p Output memory location, may have any alignment.
	 * @param lim The number of lower elements to store, >0.
	 */

	void storeu( float* p, int lim ) const
	{
		__m128 v;

		if( lim > 4 )
		{
			_mm_storeu_ps( p, _mm256_extractf128_ps( value, 0 ));
			v = _mm256_extractf128_ps( value, 1 );
			p += 4;
			lim -= 4;
		}
		else
		{
			v = _mm256_extractf128_ps( value, 0 );
		}

		if( lim > 2 )
		{
			if( lim > 3 )
			{
				_mm_storeu_ps( p, v );
			}
			else
			{
				_mm_storel_pi( (__m64*) p, v );
				_mm_store_ss( p + 2, _mm_movehl_ps( v, v ));
			}
		}
		else
		{
			if( lim == 2 )
			{
				_mm_storel_pi( (__m64*) p, v );
			}
			else
			{
				_mm_store_ss( p, v );
			}
		}
	}

	float8 operator + (const float& s) const
	{
		return(_mm256_add_ps(value, _mm256_set1_ps (s)));
	}

	float8 operator - (const float& s) const
	{
		return(_mm256_sub_ps(value, _mm256_set1_ps(s)));
	}

	float8 operator * (const float& s) const
	{
		return(_mm256_mul_ps(value, _mm256_set1_ps(s)));
	}

	float8 operator / (const float& s) const
	{
		return(_mm256_div_ps(value, _mm256_set1_ps(s)));
	}

	/* packed ops */
	float8& operator += ( const float8& s )
	{
		value = _mm256_add_ps( value, s.value );
		return( *this );
	}

	float8& operator -= ( const float8& s )
	{
		value = _mm256_sub_ps( value, s.value );
		return( *this );
	}

	float8& operator *= ( const float8& s )
	{
		value = _mm256_mul_ps( value, s.value );
		return( *this );
	}

	float8& operator /= ( const float8& s )
	{
		value = _mm256_div_ps( value, s.value );
		return( *this );
	}
	
	float8 operator + ( const float8& s ) const
	{
		return( _mm256_add_ps( value, s.value ));
	}

	float8 operator - ( const float8& s ) const
	{
		return( _mm256_sub_ps( value, s.value ));
	}

	float8 operator * ( const float8& s ) const
	{
		return( _mm256_mul_ps( value, s.value ));
	}

	float8 operator / ( const float8& s ) const
	{
		return( _mm256_div_ps( value, s.value ));
	}

	float8& operator % (const float8& s)
	{
		__m256 mask = _mm256_cmp_ps(value, s.value, _CMP_GE_OQ);
		value = _mm256_blendv_ps(value, _mm256_sub_ps (value, s.value), mask);
		return(*this);
	}

	float8 operator!= (const float8& s) const
	{
		return (_mm256_cmp_ps(value, s.value, _CMP_NEQ_OQ));
	}

	float8 operator== (const float8& s) const
	{
		return (_mm256_cmp_ps(value, s.value, _CMP_EQ_OQ));
	}

	float8 operator> (const float8& s) const
	{
		return (_mm256_cmp_ps(value, s.value, _CMP_GT_OQ));
	}

	float8 operator< (const float8& s) const
	{
		return (_mm256_cmp_ps(value, s.value, _CMP_LT_OQ));
	}

	float8 operator>= (const float8& s) const
	{
		return (_mm256_cmp_ps(value, s.value, _CMP_GE_OQ));
	}

	float8 operator<= (const float8& s) const
	{
		return (_mm256_cmp_ps(value, s.value, _CMP_LE_OQ));
	}

	float8 operator& (const float8& s) const
	{
		return (_mm256_and_ps(value, s.value));
	}
	

	/**
	 * @return Horizontal sum of elements.
	 */

	float hadd() const
	{
		__m128 v = _mm_add_ps( _mm256_extractf128_ps( value, 0 ),
			_mm256_extractf128_ps( value, 1 ));

		v = _mm_hadd_ps( v, v );
		v = _mm_hadd_ps( v, v );
		return( _mm_cvtss_f32( v ));
		
	}


	/**
	 * Function performs in-place addition of a value located in memory and
	 * the specified value.
	 *
	 * @param p Pointer to value where addition happens. May be unaligned.
	 * @param v Value to add.
	 */

	static void addu( float* const p, const float8& v )
	{
		( loadu( p ) + v ).storeu( p );
	}

	/**
	 * Function performs in-place addition of a value located in memory and
	 * the specified value. Limited to the specfied number of elements.
	 *
	 * @param p Pointer to value where addition happens. May be unaligned.
	 * @param v Value to add.
	 * @param lim The element number limit, >0.
	 */

	static void addu( float* const p, const float8& v, const int lim )
	{
		( loadu( p, lim ) + v ).storeu( p, lim );
		
	}

	__m256 value; ///< Packed value of 8 floats.
		///<

private:
	/**
	 * @param p Pointer to memory from where the value should be loaded,
	 * may have any alignment.
	 * @param lim The maximum number of elements to load, >0.
	 * @return __m128 value loaded from the specified memory location, with
	 * elements beyond "lim" set to 0.
	 */

	static __m128 loadu4( const float* const p, const int lim )
	{
		if( lim > 2 )
		{
			if( lim > 3 )
			{
				return( _mm_loadu_ps( p ));
			}
			else
			{
				return( _mm_set_ps( 0.0f, p[ 2 ], p[ 1 ], p[ 0 ]));
			}
		}
		else
		{
			if( lim == 2 )
			{
				return( _mm_set_ps( 0.0f, 0.0f, p[ 1 ], p[ 0 ]));
			}
			else
			{
				return( _mm_load_ss( p ));
			}
		}
	}
};

namespace float8ops
{
inline float8 slog2(const float8& x)
{
	// its pretty dumb
	__m256i v = _mm256_cvtps_epi32(x.value);

	v = _mm256_andnot_si256(_mm256_srli_epi32(v, 8), v); // keep 8 MSB

	v = _mm256_castps_si256(_mm256_cvtepi32_ps(v)); // convert an integer to float
	v = _mm256_srli_epi32(v, 23); // shift down the exponent
	v = _mm256_subs_epu16(_mm256_set1_epi32(158), v); // undo bias
	v = _mm256_min_epi16(v, _mm256_set1_epi32(32)); // clamp at 32

	return _mm256_cvtepi32_ps(_mm256_sub_epi32(_mm256_set1_epi32(31), v));
}

inline float8 andnot(const float8& v1, const float8& v2)
{
	return (_mm256_andnot_ps(v1.value, v2.value));
}

inline float8 sfabs(const float8& v)
{
	float8 signbit(-0.0f);
	return andnot(signbit, v);
}

inline float8 ssqrt(const float8& v)
{
	return (_mm256_sqrt_ps(v.value));
}

inline float8 ftanh(const float8& x)
{
	// https://www.kvraudio.com/forum/viewtopic.php?f=33&t=521377
	float8 x_trans = x + float8(0.18) * (x * x * x);
	return x_trans / ssqrt(x_trans * x_trans + float8(1));
}

/**
 * SIMD rounding function, exact result.
 *
 * @param v Value to round.
 * @return Rounded SIMD value.
 */

inline float8 round(const float8& v)
{
	return(_mm256_round_ps(v.value,
		(_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)));
}

inline float8 roundneg(const float8& v)
{
	return(_mm256_round_ps(v.value,
		(_MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC)));
}

inline float8 blend(const float8& v1, const float8& v2, const float8& mask)
{
	return (_mm256_blendv_ps(v1.value, v2.value, mask.value));
}

/**
 * SIMD function "clamps" (clips) the specified packed values so that they are
 * not lesser than "minv", and not greater than "maxv".
 *
 * @param Value Value to clamp.
 * @param minv Minimal allowed value.
 * @param maxv Maximal allowed value.
 * @return The clamped value.
 */

inline float8 clamp(const float8& Value, const float8& minv,
	const float8& maxv)
{
	return(_mm256_min_ps(_mm256_max_ps(Value.value, minv.value),
		maxv.value));
}

inline float8 smax(const float8& Value, const float8& Value2)
{
	return (_mm256_max_ps(Value.value, Value2.value));
}

inline float8 smin(const float8& Value, const float8& Value2)
{
	return (_mm256_min_ps(Value.value, Value2.value));
}

inline float8 s_isnan(const float8& Value)
{
	return (_mm256_cmp_ps(Value.value, Value.value, _CMP_UNORD_Q));
}

inline int movemask(const float8& val)
{
	return (_mm256_movemask_ps(val.value));
}

}


class int8
{
public:
	int8() : value(_mm256_set1_epi32(0))
	{

	}

	int8(const int8& s)
		: value(s.value)
	{
	}

	int8(const __m256i s)
		: value(s)
	{
	}

	int8(const __m256 s)
		: value(_mm256_cvtps_epi32 (s))
	{
	}

	int8(const int32_t s)
		: value(_mm256_set1_epi32(s))
	{
	}

	int8& operator = (const int8& s)
	{
		value = s.value;
		return(*this);
	}

	int8& operator = (const __m256i s)
	{
		value = s;
		return(*this);
	}

	int8& operator = (const int32_t s)
	{
		value = _mm256_set1_epi32(s);
		return(*this);
	}

	operator int32_t() const
	{
		return _mm256_extract_epi32(value, 0);
	}

	operator __m256i () const
	{
		return value;
	}

	/**
	 * @param p Pointer to memory from where the value should be loaded,
	 * should be 32-byte aligned.
	 * @return float8 value loaded from the specified memory location.
	 */

	static int8 load(const int32_t* const p)
	{
		return(_mm256_load_si256( (__m256i*) p ));
	}

	/**
	 * @param p Pointer to memory from where the value should be loaded,
	 * may have any alignment.
	 * @return float8 value loaded from the specified memory location.
	 */

	static int8 loadu(const int32_t* const p)
	{
		return(_mm256_loadu_si256((__m256i*) p));
	}

	/**
	 * @param p Pointer to memory from where the value should be loaded,
	 * may have any alignment.
	 * @param lim The maximum number of elements to load, >0.
	 * @return float8 value loaded from the specified memory location, with
	 * elements beyond "lim" set to 0.
	 */

	static int8 loadu(const int32_t* const p, const int lim)
	{
		__m128i lo;
		__m128i hi;

		if (lim > 4)
		{
			lo = _mm_loadu_si128( (__m128i*) p);
			hi = loadu4(p + 4, lim - 4);
		}
		else
		{
			lo = loadu4(p, lim);
			hi = _mm_setzero_si128();
		}

		return(_mm256_insertf128_si256(_mm256_castsi128_si256(lo), hi, 1));
	}

	/**
	 * Function stores *this value to the specified memory location.
	 *
	 * @param[out] p Output memory location, should be 32-byte aligned.
	 */

	void store(int32_t* const p) const
	{
		_mm256_store_si256( (__m256i*)p, value);
	}

	/**
	 * Function stores *this value to the specified memory location.
	 *
	 * @param[out] p Output memory location, may have any alignment.
	 */

	void storeu(int32_t* const p) const
	{
		_mm256_storeu_si256((__m256i*)p, value);
	}


	/**
	 * Function stores "lim" lower elements of *this value to the specified
	 * memory location.
	 *
	 * @param[out] p Output memory location, may have any alignment.
	 * @param lim The number of lower elements to store, >0.
	 */

	void storeu(int32_t* p, int lim) const
	{
		__m128i v;

		if (lim > 4)
		{
			_mm_storeu_si128( (__m128i* )p, _mm256_extractf128_si256(value, 0));
			v = _mm256_extractf128_si256(value, 1);
			p += 4;
			lim -= 4;
		}
		else
		{
			v = _mm256_extractf128_si256(value, 0);
		}

		if (lim > 2)
		{
			if (lim > 3)
			{
				_mm_storeu_si128( (__m128i*)p, v);
			}
			else
			{
				_mm_storeu_si64( p, v);
				_mm_storeu_si32(p + 2, _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 3, 2, 3))); // _mm_shuffle_epi32 
			}
		}
		else
		{
			if (lim == 2)
			{
				_mm_storeu_si64((__m64*) p, v);
			}
			else
			{
				_mm_storeu_si32(p, v);
			}
		}
	}

	int8 operator + (const int32_t& s) const
	{
		return(_mm256_add_epi32(value, _mm256_set1_epi32(s)));
	}

	int8 operator - (const int32_t& s) const
	{
		return(_mm256_sub_epi32(value, _mm256_set1_epi32(s)));
	}

	int8 operator * (const int32_t& s) const
	{
		return(_mm256_mul_epi32(value, _mm256_set1_epi32(s)));
	}

	int8 operator / (const int32_t& s) const
	{
		return(_mm256_div_epi32(value, _mm256_set1_epi32(s)));
	}

	/* packed ops */
	int8& operator += (const int8& s)
	{
		value = _mm256_add_epi32(value, s.value);
		return(*this);
	}

	int8& operator -= (const int8& s)
	{
		value = _mm256_sub_epi32(value, s.value);
		return(*this);
	}

	int8& operator *= (const int8& s)
	{
		value = _mm256_mul_epi32(value, s.value);
		return(*this);
	}

	int8& operator /= (const int8& s)
	{
		value = _mm256_div_epi32(value, s.value);
		return(*this);
	}

	int8 operator + (const int8& s) const
	{
		return(_mm256_add_epi32(value, s.value));
	}

	int8 operator - (const int8& s) const
	{
		return(_mm256_sub_epi32(value, s.value));
	}

	int8 operator * (const int8& s) const
	{
		return(_mm256_mul_epi32(value, s.value));
	}

	int8 operator / (const int8& s) const
	{
		return(_mm256_div_epi32(value, s.value));
	}


	int8 operator== (const int8& s) const
	{
		return (_mm256_cmpeq_epi32(value, s.value));
	}

	int8 operator!= (const int8& s) const
	{
		__m256i temp = _mm256_cmpeq_epi32(value, s.value);
		return (_mm256_andnot_si256(temp, temp));
	}

	int8 operator> (const int8& s) const
	{
		return (_mm256_cmpgt_epi32(value, s.value));
	}

	int8 operator< (const int8& s) const
	{
		return (_mm256_cmpgt_epi32(s.value, value));
	}

	int8 operator<= (const int8& s) const
	{
		return (_mm256_cmpgt_epi32(s.value, value));
	}

	int8 operator& (const int8& s) const
	{
		return (_mm256_and_si256(s.value, value));
	}

	/**
	 * @return Horizontal sum of elements.
	 */

	int32_t hadd() const
	{
		__m128i v = _mm_add_epi32(_mm256_extractf128_si256(value, 0),
			_mm256_extractf128_si256(value, 1));

		v = _mm_hadd_epi32(v, v);
		v = _mm_hadd_epi32(v, v);
		return(_mm_cvtsi128_si32(v));

	}


	/**
	 * Function performs in-place addition of a value located in memory and
	 * the specified value.
	 *
	 * @param p Pointer to value where addition happens. May be unaligned.
	 * @param v Value to add.
	 */

	static void addu(int32_t* const p, const int8& v)
	{
		(loadu(p) + v).storeu(p);
	}

	/**
	 * Function performs in-place addition of a value located in memory and
	 * the specified value. Limited to the specfied number of elements.
	 *
	 * @param p Pointer to value where addition happens. May be unaligned.
	 * @param v Value to add.
	 * @param lim The element number limit, >0.
	 */

	static void addu(int32_t* const p, const int8& v, const int lim)
	{
		(loadu(p, lim) + v).storeu(p, lim);

	}

	__m256i value; ///< Packed value of 8 floats.
		///<

private:
	/**
	 * @param p Pointer to memory from where the value should be loaded,
	 * may have any alignment.
	 * @param lim The maximum number of elements to load, >0.
	 * @return __m128 value loaded from the specified memory location, with
	 * elements beyond "lim" set to 0.
	 */

	static __m128i loadu4(const int32_t* const p, const int lim)
	{
		if (lim > 2)
		{
			if (lim > 3)
			{
				return(_mm_loadu_si128( (__m128i *) p ));
			}
			else
			{
				return(_mm_set_epi32(0, p[2], p[1], p[0]));
			}
		}
		else
		{
			if (lim == 2)
			{
				return(_mm_set_epi32(0, 0, p[1], p[0]));
			}
			else
			{
				return(_mm_set_epi32(0, 0, 0, p[0]));
			}
		}
	}
};

namespace int8ops
{
inline int8 clamp(const int8& Value, const int8& minv,
	const int8& maxv)
{
	return(_mm256_min_epi32(_mm256_max_epi32(Value.value, minv.value),
		maxv.value));
}

inline int8 smax(const int8& Value, const int8& Value2)
{
	return (_mm256_max_epi32(Value.value, Value2.value));
}

inline int8 smin(const int8& Value, const int8& Value2)
{
	return (_mm256_min_epi32(Value.value, Value2.value));
}

inline int8 andnot(const int8& Value, const int8& Value2)
{
	return (_mm256_andnot_si256(Value.value, Value2.value));
}

}

} // namespace avir



#endif // AVIR_FLOAT8_AVX_INCLUDED

#ifndef RNG_XORSHIFT128PLUS_SSE2_H
#define RNG_XORSHIFT128PLUS_SSE2_H

#include <cstdint>
#include <emmintrin.h> // SSE2

namespace rng {

/**
 * SSE2-vectorized xorshift128+ PRNG that generates two doubles simultaneously.
 *
 * Runs two independent xorshift128+ instances in parallel using SSE2,
 * producing two 64-bit random values per step — ideal for generating
 * 2D points (x, y) in a single operation.
 *
 * Key properties:
 *   - One call to nextPoint() produces __m128d{x, y} — two independent doubles
 *   - All state and output live in SSE2 registers (no memory allocation)
 *   - ~6 SIMD instructions per pair of doubles generated
 *   - Passes BigCrush statistical quality tests
 *   - NOT cryptographically secure (fine for simulation)
 *
 * SSE2 context:
 *   - 128-bit registers (__m128d) hold exactly 2 doubles — natural fit for 2D points
 *   - SSE2 is universally available on all x86-64 CPUs
 *   - State fits in 2 × __m128i registers (32 bytes total)
 */
class Xorshift128PlusSSE2 {
public:
	/**
	 * Initialize from a single seed. Uses splitmix64 to derive
	 * independent state for both internal xorshift128+ instances.
	 */
	explicit Xorshift128PlusSSE2(uint64_t seed) {
		// Use splitmix64 to expand one seed into 4 independent state words
		uint64_t smState = seed;
		uint64_t s0a = splitmix64(smState);
		uint64_t s1a = splitmix64(smState);
		uint64_t s0b = splitmix64(smState);
		uint64_t s1b = splitmix64(smState);

		// Pack: each __m128i holds one state word per instance (lane 0 = inst A, lane 1 = inst B)
		m_state0 = _mm_set_epi64x(static_cast<int64_t>(s0b), static_cast<int64_t>(s0a));
		m_state1 = _mm_set_epi64x(static_cast<int64_t>(s1b), static_cast<int64_t>(s1a));
	}


	/**
	 * Generate two random uint64 values in a single __m128i.
	 * This is the raw PRNG step — use nextUniform01() or nextPoint() for doubles.
	 */
	__m128i nextBits() {
		__m128i s1 = m_state0;
		const __m128i s0 = m_state1;
		m_state0 = s0;

		s1 = _mm_xor_si128(s1, _mm_slli_epi64(s1, 23));
		m_state1 = _mm_xor_si128(
			_mm_xor_si128(s1, s0),
			_mm_xor_si128(_mm_srli_epi64(s1, 17), _mm_srli_epi64(s0, 26))
		);

		return _mm_add_epi64(m_state1, s0);
	}


	/**
	 * Generate two uniform doubles in [0.0, 1.0).
	 * Both values are independent and remain in an SSE2 register.
	 */
	__m128d nextUniform01() {
		const __m128i bits = nextBits();

		// Extract 52 mantissa bits from each 64-bit value, set exponent for [1.0, 2.0)
		const __m128i mantissa = _mm_srli_epi64(bits, 12);
		const __m128i exponent = _mm_set1_epi64x(0x3FF0000000000000LL);
		const __m128d oneToTwo = _mm_castsi128_pd(_mm_or_si128(mantissa, exponent));

		// Shift from [1.0, 2.0) to [0.0, 1.0)
		return _mm_sub_pd(oneToTwo, _mm_set1_pd(1.0));
	}


	/**
	 * Generate two uniform doubles in [lo, hi).
	 * Returns __m128d where both lanes are in the given range.
	 */
	__m128d nextUniform(double lo, double hi) {
		const __m128d u01 = nextUniform01();
		const __m128d range = _mm_set1_pd(hi - lo);
		const __m128d offset = _mm_set1_pd(lo);
		return _mm_add_pd(_mm_mul_pd(u01, range), offset);
	}


	/**
	 * Generate a random 2D point as __m128d{x, y}, both in [lo, hi).
	 * Semantically identical to nextUniform() — provided for clarity at call sites.
	 *
	 * Extract coordinates with:
	 *   double x = getX(point);  // or _mm_cvtsd_f64(point)
	 *   double y = getY(point);  // upper lane
	 */
	__m128d nextPoint(double lo, double hi) {
		return nextUniform(lo, hi);
	}


	// ── Helpers to extract individual doubles from __m128d ──

	/** Extract the lower double (x / first value). */
	static double getX(__m128d v) { return _mm_cvtsd_f64(v); }

	/** Extract the upper double (y / second value). */
	static double getY(__m128d v) { return _mm_cvtsd_f64(_mm_unpackhi_pd(v, v)); }


private:
	__m128i m_state0;
	__m128i m_state1;

	/**
	 * splitmix64 — high-quality hash used only for seeding.
	 * Advances `state` in place and returns the mixed output.
	 */
	static uint64_t splitmix64(uint64_t& state) {
		uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
		return z ^ (z >> 31);
	}
};

} // namespace rng

#endif // RNG_XORSHIFT128PLUS_SSE2_H

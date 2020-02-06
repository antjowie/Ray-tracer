#pragma once

// Period 2^96-1
// https://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c
class Xorshf96
{
public:
	Xorshf96(unsigned seed)
	{
		// Take some bits from the seed and divide it three
		constexpr unsigned xMask = 0xFF00FF00;
		constexpr unsigned yMask = 0xF0F0F0F0;
		constexpr unsigned zMask = 0x00FF00FF;

		m_x = seed ^ xMask;
		m_y = seed ^ yMask;
		m_z = seed ^ zMask;
	}

	unsigned random()
	{
		unsigned t;
		m_x ^= m_x << 16;
		m_x ^= m_x >> 5;
		m_x ^= m_x << 1;

		t = m_x;
		m_x = m_y;
		m_y = m_z;
		m_z = t ^ m_x ^ m_y;

		return m_z;
	}

	float random(float range)
	{
		auto rand = random();
		return ((float)rand / (float)std::numeric_limits<unsigned>::max()) * range;
	}

private:
	unsigned m_x;
	unsigned m_y;
	unsigned m_z;
};

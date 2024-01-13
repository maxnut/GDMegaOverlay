#pragma once
// https://github.com/geode-sdk/geode/blob/main/loader/include/Geode/utils/SeedValue.hpp

struct BaseSeedValue
{
	BaseSeedValue() = default;
	BaseSeedValue(BaseSeedValue const& other) = default;
	BaseSeedValue(BaseSeedValue&& other) = default;
	BaseSeedValue& operator=(BaseSeedValue const& other) = default;
	BaseSeedValue& operator=(BaseSeedValue&& other) = default;
};


struct SeedValueRSV : public BaseSeedValue
{
	int m_value_rand;
	int m_value_seed;
	int m_value;

	int value() const { return m_value; }
};

struct SeedValueVSR : public BaseSeedValue
{
	int m_value;
	int m_value_seed;
	int m_value_rand;

	int value() const { return m_value; }
};

struct SeedValueRS : public BaseSeedValue
{
	int m_value_rand;
	int m_value_seed;

	int value() const { return m_value_rand - m_value_seed; }
};

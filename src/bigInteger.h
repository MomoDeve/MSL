#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <exception>

namespace momo
{
	class big_integer
	{
		static const int32_t _base;
		static const int32_t _base_digits;

		std::vector<int32_t> _digits;
		bool _negative;
		bool _inf;

		static size_t to_buffer(std::stringstream& buffer, const std::string str);
		void normalize();
		void free();
		void from_buffer(std::stringstream& buffer, size_t size);
		void from_integer(unsigned long long value);
		int compare_abs(const big_integer& other) const;
		void sum_abs(const big_integer& other);
		void sub_abs(const big_integer& other);
		void mult_abs(unsigned long long value);
		void mult_base(size_t count);
		int check_inf(const big_integer& other) const;
	public:
		static const big_integer inf;
		bool is_inf() const;

		big_integer();
		big_integer(long long value);
		big_integer(unsigned long long value);
		big_integer(int value);
		big_integer(const std::string& value);
		big_integer(const char* value);
		big_integer(const big_integer& other);
		big_integer(big_integer&& other);

		big_integer& operator=(long long value);
		big_integer& operator=(unsigned long long value);
		big_integer& operator=(int value);
		big_integer& operator=(const std::string& value);
		big_integer& operator=(const char* value);
		big_integer& operator=(const big_integer& other);
		big_integer& operator=(big_integer&& other);

		big_integer& operator+=(const big_integer& other);
		big_integer& operator-=(const big_integer& other);
		big_integer& operator*=(const big_integer& other);
		big_integer& operator/=(const big_integer& other);
		big_integer& operator%=(const big_integer& other);

		big_integer operator+(const big_integer& other) const;
		big_integer operator-(const big_integer& other) const;
		big_integer operator*(const big_integer& other) const;
		big_integer operator/(const big_integer& other) const;
		big_integer operator%(const big_integer& other) const;

		bool operator==(const big_integer& other) const;
		bool operator!=(const big_integer& other) const;
		bool operator<(const big_integer& other) const;
		bool operator<=(const big_integer& other) const;
		bool operator>(const big_integer& other) const;
		bool operator>=(const big_integer& other) const;

		big_integer operator-() const;
		big_integer operator+() const;

		std::string to_string(const std::string& sep = "") const;

		friend std::ostream& operator<<(std::ostream& out, const big_integer& num);
		friend const big_integer& max(const big_integer& num1, const big_integer& num2);
		friend const big_integer& min(const big_integer& num1, const big_integer& num2);
		friend big_integer abs(big_integer num);
		friend big_integer pow(const big_integer& num, size_t power);
		friend big_integer pow(const big_integer& num, size_t power, const big_integer& mod);
		friend big_integer fact(big_integer num);
		friend big_integer sqrt(const big_integer& num);
	};

	typedef big_integer BigInteger;

	#define template_ops(operand) template<typename T> big_integer operator operand(T other, const big_integer& num) \
								  { return big_integer(other) operand num; }
	template_ops(+)
	template_ops(-)
	template_ops(*)
	template_ops(/ )
	template_ops(%)

	#undef template_ops
}
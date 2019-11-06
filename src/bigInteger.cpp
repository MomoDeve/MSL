#include "bigInteger.h"
#include <algorithm>

#define NOEXCEPT

namespace momo
{
	const int32_t big_integer::_base_digits = 9;
	const int32_t big_integer::_base = (int32_t)std::pow(10, big_integer::_base_digits);
	const big_integer big_integer::inf("inf");

	size_t big_integer::to_buffer(std::stringstream& buffer, const std::string& str)
	{
		bool is_signed = false;
		size_t digit_count = 0;
		if (str[0] == '-' || str[0] == '+')
		{
			is_signed = true;
		}

		std::string no_err_str;
		const std::string* str_ptr = &str;

		auto it_err = std::find_if(str.begin() + is_signed, str.end(), [](char c) {return c < '0' || c > '9'; });
		if (it_err != str.end())
		{
			no_err_str = std::string(str.begin(), it_err);
			str_ptr = &no_err_str;
		}

		size_t first_digits = (str_ptr->size() - is_signed) % _base_digits;
		if (first_digits != 0)
		{
			auto it_start = str_ptr->begin() + is_signed;
			auto it_end = it_start + (str_ptr->size() - is_signed) % _base_digits;
			buffer << std::string(it_start, it_end) << ' ';
			digit_count++;
		}

		size_t length = (str_ptr->size() - is_signed) / _base_digits;
		auto it = str_ptr->cbegin() + first_digits + is_signed;
		for (size_t i = 0; i < length; i++, digit_count++)
		{
			auto it_start = it + i * _base_digits;
			auto it_end = it + (i + 1) * _base_digits;
			buffer << std::string(it_start, it_end) << ' ';
		}
		return digit_count;
	}

	void big_integer::normalize()
	{
		while ((_digits.size() > 1) && (_digits.back() == 0)) _digits.pop_back();
		if (_digits[0] == 0 && _digits.size() == 1) _negative = false;
	}

	void big_integer::free()
	{
		_digits.clear();
		_digits.push_back(0);
	}

	void big_integer::from_buffer(std::stringstream& buffer, size_t size)
	{
		size = std::max(size, 1U);
		_digits.resize(size);
		for (size_t i = 0; i < size; i++)
		{
			buffer >> _digits[size - i - 1];
		}
		normalize();
	}

	void big_integer::from_integer(unsigned long long value)
	{
		_digits.clear();
		do
		{
			_digits.push_back(value % _base);
			value /= _base;
		} while (value > 0);
	}

	int big_integer::compare_abs(const big_integer& other) const
	{
		switch (check_inf(other))
		{
		#ifndef NOEXCEPT
		case 3:
			throw new std::exception("inf cannot be compared to inf");
			break;
		#endif
		case 2:
			return -1;
		case 1:
			return 1;
		default:
			if (_digits.size() != other._digits.size())
			{
				return (_digits.size() < other._digits.size() ? -1 : 1);
			}
			for (int i = (int)_digits.size() - 1; i >= 0; i--)
			{
				if (_digits[i] != other._digits[i])
				{
					return (_digits[i] < other._digits[i] ? -1 : 1);
				}
			}
			return 0;
		}
	}

	void big_integer::sum_abs(const big_integer& other)
	{
		if (check_inf(other) > 0)
		{
			_inf = true;
			free();
			return;
		}
		int max_length = std::max(_digits.size(), other._digits.size());
		_digits.resize(max_length + 1, 0);
		for (int i = 0; i < (int)_digits.size() - 1; i++)
		{
			if (i < (int)other._digits.size()) _digits[i] += other._digits[i];
			_digits[i + 1] += _digits[i] / _base;
			_digits[i] %= _base;
		}
		normalize();
	}

	void big_integer::sub_abs(const big_integer& other)
	{
		switch (check_inf(other))
		{
		#ifndef NOEXCEPT
		case 3:
			throw new std::exception("inf - inf undefined");
			break;
		#endif
		case 2:
			_inf = true;
			_negative = true;
			free();
			break;
		case 1:
			break;
		default:
			for (int i = 0; i < (int)_digits.size(); i++)
			{
				if (i < (int)other._digits.size()) _digits[i] -= other._digits[i];
				if (_digits[i] < 0)
				{
					_digits[i + 1]--;
					_digits[i] += _base;
				}
			}
			normalize();
			break;
		}
	}

	void big_integer::mult_abs(unsigned long long value)
	{
		_digits.push_back(0);
		unsigned long long carry = 0;
		for (int i = 0; i < (int)_digits.size(); i++)
		{
			unsigned long long tmp = _digits[i] * value + carry;
			carry = tmp / _base;
			_digits[i] = int32_t(tmp - carry * _base);
		}
		normalize();
	}

	void big_integer::mult_base(size_t count)
	{
		_digits.insert(_digits.begin(), count, 0);
	}

	int big_integer::check_inf(const big_integer& other) const
	{
		if (other._inf && _inf) return 3;
		else if (other._inf) return 2;
		else if (_inf) return 1;
		else return 0;
	}

	bool big_integer::is_inf() const
	{
		return _inf;
	}

	big_integer::big_integer()
		: _negative(false), _digits(1, 0), _inf(false) { }

	big_integer::big_integer(long long value)
		: _negative(value < 0), _inf(false)
	{
		from_integer(std::abs(value));
	}

	big_integer::big_integer(unsigned long long value)
		: _negative(false), _inf(false)
	{
		from_integer(value);
	}

	big_integer::big_integer(int value)
		: _negative(value < 0), _inf(false)
	{
		from_integer(std::abs(value));
	}

	big_integer::big_integer(const std::string& value)
		: _negative(value[0] == '-'), _inf(false)
	{
		if (value == "inf" || value == "-inf")
		{
			_inf = true;
		}
		else
		{
			std::stringstream buffer;
			size_t digits = to_buffer(buffer, value);
			from_buffer(buffer, digits);
		}
	}

	big_integer::big_integer(const char* value)
		: big_integer(std::string(value)) { }

	big_integer& big_integer::operator=(const std::string& value)
	{
		_negative = value[0] == '-';
		if (value == "inf" || value == "-inf")
		{
			_inf = true;
		}
		else
		{
			std::stringstream buffer;
			size_t digits = to_buffer(buffer, value);
			from_buffer(buffer, digits);
		}
		return *this;
	}

	big_integer& big_integer::operator=(const char* value)
	{
		*this = std::string(value);
		return *this;
	}

	big_integer& big_integer::operator+=(const big_integer& other)
	{
		if (_negative == other._negative)
		{
			sum_abs(other);
		}
		else if (compare_abs(other) == 1)
		{
			sub_abs(other);
		}
		else
		{
			big_integer res = other;
			res.sub_abs(*this);
			*this = res;
		}
		return *this;
	}

	big_integer& big_integer::operator-=(const big_integer& other)
	{
		*this += (-other);
		return *this;
	}

	big_integer& big_integer::operator*=(const big_integer& other)
	{
		*this = *this * other;
		return *this;
	}

	big_integer& big_integer::operator/=(const big_integer& other)
	{
		*this = *this / other;
		return *this;
	}

	big_integer & big_integer::operator%=(const big_integer & other)
	{
		*this = *this % other;
		return *this;
	}

	big_integer& big_integer::operator=(long long value)
	{
		_negative = value < 0;
		from_integer(std::abs(value));
		return *this;
	}

	big_integer& big_integer::operator=(unsigned long long value)
	{
		_negative = false;
		from_integer(value);
		return *this;
	}

	big_integer& big_integer::operator=(int value)
	{
		_negative = value < 0;
		from_integer(std::abs(value));
		return *this;
	}

	bool big_integer::operator==(const big_integer& other) const
	{
		return _negative == other._negative && _digits == other._digits;
	}

	bool big_integer::operator!=(const big_integer& other) const
	{
		return !(*this == other);
	}

	bool big_integer::operator<(const big_integer& other) const
	{
		if (_negative != other._negative) return _negative;
		if (_negative)
		{
			return compare_abs(other) == 1;
		}
		else
		{
			return compare_abs(other) == -1;
		}
	}

	bool big_integer::operator<=(const big_integer& other) const
	{
		return !(*this > other);
	}

	bool big_integer::operator>(const big_integer& other) const
	{
		return other < *this;
	}

	bool big_integer::operator>=(const big_integer& other) const
	{
		return !(*this < other);
	}

	big_integer big_integer::operator-() const
	{
		big_integer res = *this;
		res._negative = !_negative;
		return res;
	}

	big_integer big_integer::operator+() const
	{
		return *this;
	}

	std::string big_integer::to_string(std::string sep) const
	{
		std::stringstream res;
		if (_negative) res << '-';
		if (_inf) res << "inf";
		else
		{
			for (auto it = _digits.rbegin(); it != _digits.rend(); it++)
			{
				std::string digit = std::to_string(*it);
				if (it != _digits.rbegin())
				{
					res << std::string(_base_digits - digit.size(), '0');
				}
				res << digit << sep;
			}
		}
		return res.str();
	}

	double big_integer::to_double() const
	{
		double res = 0.0;
		for (size_t i = 0; i < _digits.size(); i++)
		{
			res *= _base;
			res += _digits[i];
		}
		if (_negative) res *= -1.0;
		return res;
	}

	size_t big_integer::size_bytes() const
	{
		return sizeof(BigInteger) + _digits.capacity() * sizeof(int32_t);
	}

	std::ostream& operator<<(std::ostream& out, const big_integer& num)
	{
		out << num.to_string();
		return out;
	}

	inline const big_integer& max(const big_integer& num1, const big_integer& num2)
	{
		return (num1 > num2 ? num1 : num2);
	}

	inline const big_integer& min(const big_integer& num1, const big_integer& num2)
	{
		return (num1 < num2 ? num1 : num2);
	}

	inline big_integer abs(big_integer num)
	{
		if (num._negative) num._negative = false;
		return num;
	}

	big_integer pow(const big_integer& num, const big_integer& power)
	{
		if (power == 0) return big_integer(1);
		if (power % 2 == 0)
		{
			return pow(num * num, power / 2);
		}
		else
		{
			return num * pow(num, power - 1);
		}
	}

	big_integer pow(const big_integer& num, size_t power, const big_integer& mod)
	{
		if (power == 0) return big_integer(1);
		if (power % 2 == 0)
		{
			return pow(num * num % mod, power / 2, mod) % mod;
		}
		else
		{
			return num * pow(num, power - 1, mod) % mod;
		}
	}

	big_integer fact(big_integer num)
	{
		big_integer res = 1;
		while (num > 0)
		{
			res *= num;
			num -= 1;
		}
		return res;
	}

	big_integer sqrt(const big_integer& num)
	{
		big_integer l, r = num;
		big_integer res;
		while (l <= r)
		{
			big_integer m = (l + r) / 2;
			if (m * m <= num)
			{
				res = m;
				l = m + 1;
			}
			else
			{
				r = m - 1;
			}
		}
		return res;
	}

	big_integer big_integer::operator+(const big_integer& other) const
	{
		big_integer res = *this;
		res += other;
		return res;
	}

	big_integer big_integer::operator-(const big_integer& other) const
	{
		big_integer res = *this;
		res -= other;
		return res;
	}
	big_integer big_integer::operator*(const big_integer& other) const
	{
		big_integer res;
		if (check_inf(other) > 0)
		{
			res._inf = true;
			res.free();
			#ifndef NOEXCEPT
			if (!_inf && _digits.size() == 1 && _digits.back() == 0 ||
				!other._inf && other._digits.size() == 1 && other._digits.back() == 0)
			{
				throw new std::exception("0 * inf undefined");
			}
			#endif
		}
		else if (compare_abs(other) == -1)
		{
			for (int i = 0; i < (int)_digits.size(); i++)
			{
				big_integer tmp(other);
				tmp.mult_abs(_digits[i]);
				tmp.mult_base(i);
				res += tmp;
			}
		}
		else
		{
			for (int i = 0; i < (int)other._digits.size(); i++)
			{
				big_integer tmp(*this);
				tmp.mult_abs(other._digits[i]);
				tmp.mult_base(i);
				res += tmp;
			}
		}
		res._negative = _negative != other._negative;
		return res;
	}
	big_integer big_integer::operator/(const big_integer& other) const
	{
		bool res_sign = other._negative != _negative;
		switch (check_inf(other))
		{
		#ifndef NOEXCEPT
		case 3:
			throw new std::exception("inf / inf undefined");
			break;
		#endif
		case 2:
			return big_integer(0);
			break;
		case 1:
			#ifndef NOEXCEPT
			if (other == 0)
			{
				throw new std::exception("inf / 0 undefined");
			}
			else
			#endif
			{
				big_integer res = big_integer::inf;
				res._negative = res_sign;
				return res;
			}
			break;
		}
		if (other == 0)
		{
			#ifndef NOEXCEPT
			if (*this == 0)
			{
				throw new std::exception("0 / 0 undefined");
			}
			else
			#endif
			{
				big_integer res = big_integer::inf;
				res._negative = res_sign;
				return res;
			}
		}
		big_integer a = abs(*this), b = abs(other);
		big_integer res, current;

		res._digits.resize(_digits.size());
		for (int i = (int)a._digits.size() - 1; i >= 0; i--)
		{
			current.mult_base(1);
			current._digits[0] = a._digits[i];
			current.normalize();
			int x = 0;
			int l = 0, r = _base;
			while (l <= r)
			{
				int m = (l + r) >> 1;
				big_integer cur = b * m;
				if (cur <= current)
				{
					x = m;
					l = m + 1;
				}
				else
				{
					r = m - 1;
				}
			}
			res._digits[i] = x;
			current = current - b * x;
		}
		res.normalize();
		res._negative = res_sign;
		return res;
	}

	big_integer big_integer::operator%(const big_integer& other) const
	{
		bool res_sign = other._negative != _negative;
		switch (check_inf(other))
		{
		#ifndef NOEXCEPT
		case 3:
			throw new std::exception("inf / inf undefined");
			break;
		#endif
		case 2:
			return *this;
			break;
		case 1:
			#ifndef NOEXCEPT
			if (other == 0)
			{
				throw new std::exception("inf / 0 undefined");
			}
			else
			#endif
			{
				return big_integer(0);
			}
			break;
		}
		if (other == 0)
		{
			#ifndef NOEXCEPT
			if (*this == 0)
			{
				throw new std::exception("0 / 0 undefined");
			}
			else
			#endif
			{
				return big_integer(0);
			}
		}
		big_integer a = abs(*this), b = abs(other);
		big_integer res, current;

		res._digits.resize(_digits.size());
		for (int i = (int)a._digits.size() - 1; i >= 0; i--)
		{
			current.mult_base(1);
			current._digits[0] = a._digits[i];
			current.normalize();
			int x = 0;
			int l = 0, r = _base;
			while (l <= r)
			{
				int m = (l + r) >> 1;
				big_integer cur = b * m;
				if (cur <= current)
				{
					x = m;
					l = m + 1;
				}
				else
				{
					r = m - 1;
				}
			}
			res._digits[i] = x;
			current = current - b * x;
		}
		current.normalize();
		current._negative = res_sign;
		return current;
	}
	#undef NOEXCEPT
}
#include "div.h"

#include <stdexcept>

uint64_t safe_divider::divide(uint64_t a, uint32_t b)
{
	if (b == 0)
		throw std::invalid_argument("Dividing by 0");
	uint64_t c = a / b;
	remainder += a - (c*b);
	if (remainder >= b)
	{
		++c;
		remainder = remainder - b;
	}
	return c;
}

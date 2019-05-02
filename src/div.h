#include <cstdint>

class safe_divider // class for dividing in a safe way
{
protected:
	uint64_t remainder{};
public:
	uint64_t divide(uint64_t a, uint32_t b);
};
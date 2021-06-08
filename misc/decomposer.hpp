#include "../processor/processor.hpp"

namespace kraps
{
namespace misc
{
class Decomposer : public Processor
{
public:
	Decomposer() : Processor (p_decomposer, 2, 16) { ; }
	~Decomposer() { ; }
	void process_callback() override
	{
		float data[8];
		for (int i = 0; i < inputs.size(); i++)
		{
			inputs[i]->src->val.storeu(data);
			for (int o = 0; o < 8; o++)
				*outputs[o + i*8] = float8(data[o]);
		}
	}

private:
	
};
}
}
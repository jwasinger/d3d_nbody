#ifndef ERROR_H
#define ERROR_H

#include <string>

namespace NBody
{
	class RuntimeError
	{
	private:
		std::string msg;

	public:
		RuntimeError(void) {};
		RuntimeError(std::string msg) { this->msg = msg; }

		std::string GetInfo(void) const 
		{
			return this->msg;
		}
	};
}

#endif
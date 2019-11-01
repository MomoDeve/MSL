#include "callPath.h"

namespace MSL
{
	namespace VM
	{
		const std::string* CallPath::GetNamespace() const
		{
			return path[0];
		}

		const std::string* CallPath::GetClass() const
		{
			return path[1];
		}

		const std::string* CallPath::GetMethod() const
		{
			return path[2];
		}

		void CallPath::SetNamespace(const std::string* ns)
		{
			path[0] = ns;
		}

		void CallPath::SetClass(const std::string* c)
		{
			path[1] = c;
		}

		void CallPath::SetMethod(const std::string* method)
		{
			path[2] = method;
		}

		Frame* CallPath::GetFrame()
		{
			return frame;
		}
		void CallPath::SetFrame(Frame* frame)
		{
			this->frame = frame;
		}

		size_t Frame::GetSize() const
		{
			return locals.size() * sizeof(std::pair<std::string, Local>);
		}
	}
}
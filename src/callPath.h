#pragma once

#include "objects.h"
#include <vector>
#include <string>

namespace MSL
{
	namespace VM
	{
		struct Frame
		{
			using LocalsTable = std::unordered_map<std::string, Local>;
			using LocalStorage = std::vector<std::unique_ptr<std::string>>;

			LocalsTable locals;
			LocalStorage localStorage;
			const NamespaceType* _namespace = nullptr;
			const ClassType* _class = nullptr;
			const MethodType* _method = nullptr;
			BaseObject* classObject = nullptr;
			size_t offset = 0;
			GCstate state = GCstate::UNMARKED;

			Frame() = default;
			Frame(Frame&&) = default;
			Frame& operator=(Frame&&) = default;
			size_t GetSize() const;
		};

		class CallPath
		{
			using Path = std::vector<const std::string*>;
			Path path = Path(3);
			Frame* frame = nullptr;
		public:
			const std::string* GetNamespace() const;
			const std::string* GetClass() const;
			const std::string* GetMethod() const;

			void SetNamespace(const std::string* ns);
			void SetClass(const std::string* c);
			void SetMethod(const std::string* method);

			Frame* GetFrame();
			void SetFrame(Frame* frame);

			CallPath() = default;
			CallPath(CallPath&&) = default;
			CallPath& operator=(CallPath&&) = default;
		};
	}
}
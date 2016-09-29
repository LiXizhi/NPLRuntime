#pragma once

#include <vector>
namespace ParaEngine
{
	using std::vector;

	template <typename T>
	class unordered_array : public vector<T>
	{
	public:
		typedef vector<T>  base_class_type;
		typedef typename base_class_type::iterator iterator;
		typedef typename base_class_type::const_iterator const_iterator;

		void erase(int nIndex)
		{
			if (nIndex < (int)base_class_type::size()){
				base_class_type::operator[](nIndex) = base_class_type::back();
			}
			base_class_type::pop_back();
		};

		iterator erase(iterator _Where)
		{
			*_Where = base_class_type::back();
			size_t nIndex = (_Where - base_class_type::begin());
			base_class_type::pop_back();
			return !base_class_type::empty() ? (base_class_type::begin()+nIndex) : base_class_type::end();
		}
	};

	
}



#include <map>
#include <list>
#include <cstdint>
#include <string>
#include <iostream>

namespace attrib {
	struct A;
	struct B;
	struct AB;
}
namespace key {
	struct A { 
		using attrib_type = attrib::A;
		uint32_t id; 
		A(const A &a) :id(a.id) {}
		A(uint32_t id) :id(id) {}
		A() :id(0) {}
		friend bool operator<(const A &a, const A &b) {
			return a.id < b.id;
		}
	};
	struct B { 
		using attrib_type = attrib::B;
		uint32_t id; 
		B(const B &a) : id(a.id) {}
		B(uint32_t id) :id(id) {}
		B() :id(0) {}
		friend bool operator<(const B &a, const B &b) {
			return a.id < b.id;
		}
	};
	struct AB : A,B { 
		using attrib_type = attrib::AB;
		AB(const AB &a) : A(a),B(a) {}
		AB(const A &a,const B &b) : A(a),B(b) {}
		AB() : A(0),B(0) {}
		friend bool operator<(const AB &a, const AB &b) {
			return (A&)a < (A&)b ? true :
						(A&)a < (A&)b ? false : 
							(B&)a < (B&)b ? true : false;
		}
	};
}
class Store;
namespace attrib {
	struct A { 
		std::string a_desc;
	};
	struct B {
		std::string b_desc;
	};
	struct AB {
		std::string ab_desc;
		std::pair<key::A, const A*> a_ref;
		std::pair<key::B, const B*> b_ref;
		void resolve(Store &s);
	};
}
class Store {
	template<class K>
	using M = std::map<K, typename K::attrib_type>;
	template<class K>
	M<K> &get_() {
		static M<K> rtn;
		return rtn;
	}
	Store() {}
public:
	template<class K>
	typename K::attrib_type &operator()(const K &k)  {
		return get_<K>()[k];
	}
	template<class K>
	const K *next(const K &k) {
		const M<K> &m = get_<K>();
		M<K>::const_iterator it = m.lower_bound(k);
		if (it == m.end()) return 0;
		it++;
		if (it == m.end()) return 0;
		return &it->first;
	}
	template<class K>
	std::list<K> domain() {
		std::list<K> rtn;
		for (const M<K>::value_type &m : get_<K>())
			rtn.push_back(m.first);
		return rtn;
	}
	static Store &getInstance() {
		static Store rtn;
		return rtn;
	}
};
void attrib::AB::resolve(Store &s) {
	a_ref.second = &s(a_ref.first);
	b_ref.second = &s(b_ref.first);
}

int main() {
	Store &store = Store::getInstance();
	key::A a(1);
	key::B b(2);
	key::AB ab(a, b);
	
	uint32_t *i = (uint32_t*)&a;
	size_t size=sizeof(key::A);

	store(key::A(1)).a_desc = "a1";
	store(key::A(2)).a_desc = "a2";
	store(key::A(3)).a_desc = "a3";

// find domain if A
	bool first = true;
	std::cout << "A domain" << std::endl;
	for (auto &v : store.domain<key::A>()) {
		std::cout << (first?"":",") << v.id;
		first = false;
	}
	std::cout << std::endl;
// iterator to next key
	const key::A *ptr = store.next(key::A(2));
	if (ptr)
		std::cout << "next after A(2) " << store(*ptr).a_desc << std::endl;
	
	store(key::B(2)).b_desc ="b2";

	store(ab).ab_desc = "a1b2";
	store(ab).a_ref.first = key::A(1);
	store(ab).b_ref.first = key::B(2);

	auto &v = store(ab);
//resolve pointer relationships
	v.resolve(store);
	std::cout << "refs" << v.a_ref.second->a_desc << ","
		<< v.b_ref.second->b_desc << std::endl;

	std::cout << "find A(1) " << store(key::A(1)).a_desc << std::endl;
	std::cout << "find ab " << store(ab).ab_desc << std::endl;
	
	return 0;
}
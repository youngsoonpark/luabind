#include <boost/shared_ptr.hpp>


namespace luabind
{
	namespace detail
	{
		template<class T>
		T* get_pointer(boost::shared_ptr<T>& p) { return p.get(); }
	}
}

#include "test.h"

namespace
{

	LUABIND_ANONYMOUS_FIX int feedback = 0;

	struct base
	{
		base(): n(4) { feedback = 3; }
		virtual ~base() { feedback = 1; }

		void f(int)
		{
			feedback = n;
		}

		int n;
	};

	// this is here to make sure the pointer offsetting works
	struct first_base
	{
		virtual void a() {}
		int padding;
	};

	struct derived: first_base, base
	{
		derived(): n2(7) { feedback = 7; }
		void f() { feedback = 5; }
		int n2;
	};

	void tester(base* t)
	{
		if (t->n == 4) feedback = 2;
	}

	void tester_(derived* t)
	{
		if (t->n2 == 7) feedback = 8;
	}

	void tester2(boost::shared_ptr<base> t)
	{
		if (t->n == 4) feedback = 9;
	}

	void tester3(boost::shared_ptr<const base> t)
	{
		if (t->n == 4) feedback = 10;
	}

} // anonymous namespace


namespace luabind
{
	template<class A>
	LUABIND_TYPE_INFO get_const_holder(luabind::detail::type<boost::shared_ptr<A> >)
	{
		return LUABIND_TYPEID(boost::shared_ptr<const A>);
	}
}

bool test_held_type()
{
	// This feature is not finished yet

	using namespace luabind;

	boost::shared_ptr<base> ptr(new base());

	{
		lua_State* L = lua_open();
		lua_closer c(L);
		int top = lua_gettop(L);

		open(L);

		function(L, "tester", &tester);
		function(L, "tester", &tester_);
		function(L, "tester2", &tester2);
		function(L, "tester3", &tester3);

		class_<base, boost::shared_ptr<base> >("base")
			.def(constructor<>())
			.def("f", &base::f)
			.commit(L)
			;

		class_<derived, base, boost::shared_ptr<base> >("derived")
			.def(constructor<>())
			.def("f", &derived::f)
			.commit(L)
			;

		object g = get_globals(L);
		g["test"] = ptr;
		if (dostring(L, "tester(test)")) return false;
		if (feedback != 2) return false;
		if (dostring(L, "a = base()")) return false;
		if (feedback != 3) return false;
		if (dostring(L, "b = derived()")) return false;
		if (feedback != 7) return false;
		if (dostring(L, "tester(a)")) return false;
		if (feedback != 2) return false;
		if (dostring(L, "tester(b)")) return false;
		if (feedback != 8) return false;
		if (dostring(L, "tester2(b)")) return false;
		if (feedback != 9) return false;
		if (dostring(L, "tester3(b)")) return false;
		if (feedback != 10) return false;

		if (top != lua_gettop(L)) return false;

	}

	ptr.reset();
	
	if (feedback != 1) return false;

	return true;
}


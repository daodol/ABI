#include <iostream>
#include <pybind11/pybind11.h>
namespace py = pybind11;
class NativeClass
{
public:
	NativeClass(const std::string abc) 
	{
		std::cout << "structure" << std::endl;
	}
	int add(int i = 0, int j = 0)
	{
		return i + j;
	};

	void setAge(int _age) 
	{
		age = _age;
	}
	int getAge() 
	{
		return age;
	}
	std::string getName() const
	{
		return name;
	}
	void setName(std::string _name)
	{
		name = _name;
	}
	int age;
private:
	std::string name="";
};

struct DynamicAttr
{
	std::string name;
	int age;
};

class Pet
{
public:
	Pet(std::string _name) :name(_name) {};
	std::string name;
};
class Dog:public Pet
{
public:
	Dog(std::string _name) :Pet(_name) {};
	std::string bark() { return "wffff"; };
private:

};

struct PetOverLoad {
	PetOverLoad(const std::string &name, int age) : name(name), age(age) { }

	void set(int age_) { age = age_; }
	void set(const std::string &name_) { name = name_; }

	std::string name;
	int age;
};

enum Kind
{
	dog,
	cat
};
PYBIND11_MODULE(NativeClass,m)
{
	py::class_<NativeClass>(m, "NativeClass")
		.def(py::init<const std::string>())
		.def("getAge", &NativeClass::getAge)
		.def("setAge", &NativeClass::setAge)
		.def_readwrite("age", &NativeClass::age)
		.def_property("name", &NativeClass::getName, &NativeClass::setName)
		.def("getName",&NativeClass::getName);

	py::class_<DynamicAttr>(m, "DynamicAttr", py::dynamic_attr())
		.def(py::init<>());

	py::class_<Pet>(m, "Pet")
		.def(py::init<const std::string &>())
		.def_readwrite("name", &Pet::name);

	py::class_<Dog, Pet /* <- specify C++ parent type */>(m, "Dog")
		.def(py::init<const std::string &>())
		.def("bark", &Dog::bark);

	//上面Pet&Dog也可写成:
	//py::class_<Pet> pet(m, "Pet");
	//pet.def(py::init<const std::string &>())
	//	.def_readwrite("name", &Pet::name);

	//// Method 2: pass parent class_ object:
	//py::class_<Dog>(m, "Dog", pet /* <- specify Python parent type */)
	//	.def(py::init<const std::string &>())
	//	.def("bark", &Dog::bark);

	py::class_<PetOverLoad>(m, "PetOverLoad")
		.def(py::init<const std::string &, int>())
		.def("set", (void (PetOverLoad::*)(int)) &PetOverLoad::set, "Set the PetOverLoad's age")
		.def("set", (void (PetOverLoad::*)(const std::string &)) &PetOverLoad::set, "Set the PetOverLoad's name");

	//c++14+
	/*py::class_<PetOverLoad>(m, "Pet")
		.def("set", py::overload_cast<int>(&PetOverLoad::set), "Set the pet's age")
		.def("set", py::overload_cast<const std::string &>(&PetOverLoad::set), "Set the pet's name");*/

	py::enum_<Kind>(m, "Kind")
		.value("dog", Kind::dog)
		.value("cat", Kind::cat)
		.export_values();
}


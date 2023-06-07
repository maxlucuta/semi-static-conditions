#include <iostream>
#include "branch.h"


int add(int a, int b) {
    return a + b; 
}

int sub(int a, int b) {
    return a - b; 
}

class Demo {
    private:
        const char* name;
    
    public:
        Demo(const char* name) : name(name) {}
        void foo() { std::cout << "Foo: " << name << std::endl; }
        void bar() { std::cout << "Bar: " << name << std::endl; }
};


int main() {

    BranchChanger branch = BranchChanger(add, sub);
    branch.setDirection(true);
    std::cout << branch.branch(2,3) << std::endl;
    branch.setDirection(false);
    std::cout << branch.branch(2,3) << std::endl;

    Demo demoObject1("Imperial");
    Demo demoObject2("UCL");
    BranchChangerClass branchClass(&Demo::foo, &Demo::bar);
    branchClass.setDirection(true);
    branchClass.branch(demoObject1);
    branchClass.branch(demoObject2);
    branch.setDirection(false);
    branchClass.branch(demoObject1);
    branchClass.branch(demoObject2);

    return 0;

}





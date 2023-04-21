#include <iostream>
#include "branch.h"


int add(int a, int b) {
    /* Test for function signature: int(*)(int,int). */
    return a + b; 
}

int sub(int a, int b) {
    /* Test for function signature: int(*)(int,int). */
    return a - b; 
}

void foo() {
    /* Test for function signature: void(*)(). */
    std::cout << "foo" << std::endl; 
}

void bar() {
    /* Test for function signature: void(*)(). */
    std::cout << "bar" << std::endl; 
}


int main() {

    /* Test for function signature: int(*)(int,int). */
    BranchChanger branch_1 = BranchChanger(add, sub);
    branch_1.set_direction(true);
    std::cout << branch_1.branch(1,2) << std::endl;
    branch_1.set_direction(false);
    std::cout << branch_1.branch(1,2) << std::endl;

    /* Test for function signature: void(*)(). */
    BranchChanger branch_2 = BranchChanger(foo, bar);
    branch_2.set_direction(true);
    branch_2.branch();
    branch_2.set_direction(false);
    branch_2.branch();
    
    return 0;

}






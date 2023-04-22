#include <iostream>
#include "branch.h"


int add(int a, int b) 
{
    return a + b; 
}

int sub(int a, int b) 
{
    return a - b; 
}

void foo() 
{
    std::cout << "foo" << std::endl; 
}

void bar() 
{
    std::cout << "bar" << std::endl; 
}


int main() {

    BranchChanger branch_1 = BranchChanger(add, sub);
    BranchChanger branch_2 = BranchChanger(foo, bar);
    for (int i = 0; i < 10; i ++)
    {
        bool runtime_condition = rand() % 2;
        branch_1.set_direction(runtime_condition);
        branch_2.set_direction(runtime_condition);
        std::cout << branch_1.branch(1,2) << std::endl;
        branch_2.branch();
    }
    
    return 0;

}






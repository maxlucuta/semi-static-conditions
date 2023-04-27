#include <iostream>
#include "branch.h"


int add(int a, int b) {
    return a + b; 
}

int sub(int a, int b) {
    return a - b; 
}


int main() {

    BranchChanger branch = BranchChanger(add, sub);
    branch.set_direction(true);
    std::cout << branch.branch(2,3) << std::endl;
    branch.set_direction(false);
    std::cout << branch.branch(2,3) << std::endl;
    return 0;

}





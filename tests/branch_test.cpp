#include <gtest/gtest.h>
#include <branch.hpp>


void func_1() { return; }
void func_2() { return; }
void func_3() { return; }


int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int mul(int a, int b) { return a * b; }


TEST(Jump, OffsetTest) {
    unsigned char* some_func = reinterpret_cast<unsigned char*>(0xff);
    unsigned char* some_other_func = (some_func) + 12;
    EXPECT_EQ(compute_jump_offset(some_func, some_other_func), -17);
}


TEST(ByteConversion1, OffsetTest) {
    intptr_t offset = 1220;
    #ifdef LITTLE_ENDIAN_BRANCH
    unsigned char result[4] = { 
        0xC4, 0x04, 0x00, 0x00 
    };
    #else
        #if ARM_BUILD_BRANCH
        unsigned char result[3] = { 
            0x00, 0x04, 0xC4 
        };
        #else
        unsigned char result[3] = { 
            0x00, 0x00, 0x04, 0xC4 
        };
        #endif
    #endif
    unsigned char dest[4];
    store_offset_as_bytes(offset, dest);
    for (int i = 0; i < OFFSET_; i++)
        EXPECT_EQ(dest[i], result[i]);
}


TEST(ByteConversion2, OffsetTest) {
    intptr_t offset = -1220;
    #ifdef LITTLE_ENDIAN_BRANCH
    unsigned char result[4] = { 
        0x3C, 0xFB, 0xFF, 0xFF  
    };
    #else
        #if ARM_BUILD_BRANCH
        unsigned char result[4] = { 
            0xFF, 0xFB, 0x3C 
        };
        #else
        unsigned char result[4] = { 
            0xFF, 0xFF, 0xFB, 0x3C 
        };
        #endif
    #endif
    unsigned char dest[4];
    store_offset_as_bytes(offset, dest);
    for (int i = 0; i < OFFSET_; i++)
        EXPECT_EQ(dest[i], result[i]);
}


TEST(BranchChanger1, OutOfBounds) {
    using ptr = int(*)(int, int);
    ptr func_1 = reinterpret_cast<ptr>(0xff);
    ptr func_2 = reinterpret_cast<ptr>(
        reinterpret_cast<intptr_t>(func_1) + 
        (static_cast<intptr_t>(1) << 34)
    );
    try {
        BranchChanger branch(func_1, func_2);
        FAIL() << "Expecting out of bounds exception.";
    } catch (branch_changer_error e) {
        EXPECT_EQ(e.what(), err_to_str(error_codes::BRANCH_TARGET_OUT_OF_BOUNDS));
    }
}


#ifndef GITHUB_WORKFLOW_WINDOWS

TEST(BranchChanger2, MultipleInstances) {
    BranchChanger branch_1(func_1, func_2);
    try {
        BranchChanger branch_2(func_2, func_3, func_1);
        FAIL() << "Expecting multiple instance exception.";
    } catch (branch_changer_error e) {
        EXPECT_EQ(e.what(), err_to_str(error_codes::MULTIPLE_INSTANCE_ERROR));
    }
}


TEST(BranchChanger3, Functionality) {
    BranchChanger branch(add, sub);
    bool condition = std::rand() % 2;
    for (int i = 0; i < 100; i++) {
        branch.set_direction(condition);
        EXPECT_EQ(branch.branch(1,2), condition ? 3 : -1);
        condition = std::rand() % 2;
    } 
}


TEST(BranchChanger4, Functionality) {
    BranchChanger branch(add, sub, mul);
    uint64_t condition = std::rand() % 3;
    for (int i = 0; i < 100; i++) {
        branch.set_direction(condition);
        int result = branch.branch(1,2);
        int answer;
        if (condition == 0) answer = 3;
        else if (condition == 1) answer = -1;
        else answer = 2;
        EXPECT_EQ(result, answer);
        condition = std::rand() % 3;
    } 
}

#endif

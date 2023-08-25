#ifndef BRANCH_MISC_HPP
#define BRANCH_MISC_HPP


#include <stdexcept>


enum class error_codes {
    BRANCH_TARGET_OUT_OF_BOUNDS,
    MULTIPLE_INSTANCE_ERROR,
    PAGE_PERMISSIONS_ERROR
};


std::string err_to_str(const error_codes& code);


class branch_changer_error : public std::runtime_error {       
public:
    explicit branch_changer_error(const error_codes& code);
};


#endif
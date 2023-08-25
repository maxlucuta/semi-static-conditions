
#include "builds/branch_misc.hpp"


std::string err_to_str(const error_codes& code) {
    switch (code) {

        case error_codes::BRANCH_TARGET_OUT_OF_BOUNDS:

            return R"(Supplied branch targets (as function pointers) exceed a 2GiB displacement
					  from the entry point in the text segment, and cannot be reached with a 32-bit
					  relative jump. Consider moving the entry point to different areas in the text
					  segment by altering hot/cold attributes.)";

        case error_codes::MULTIPLE_INSTANCE_ERROR:

            return R"(More than once instance for template specialised semi-static conditions detected.
					  Program terminated as multiple instances sharing the same entry point is dangerous
					  and results in undefined behaviour (multiple instances write to same function.))";

        case error_codes::PAGE_PERMISSIONS_ERROR:

            return R"("Unable to change page permissions for the given function pointers.)";

        default:

            return "Runtime error.";
    };
}


branch_changer_error::branch_changer_error(const error_codes& code) : std::runtime_error(err_to_str(code)) {}
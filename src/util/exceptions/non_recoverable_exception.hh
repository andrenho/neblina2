#ifndef NON_RECOVERABLE_EXCEPTION_HH
#define NON_RECOVERABLE_EXCEPTION_HH

#include <stdexcept>
#include <string>

#define NON_RECOVERABLE_RETURN_CODE 168

class NonRecoverableException : public std::runtime_error {
public:
    explicit NonRecoverableException(std::string const& what) : std::runtime_error(what) {}
};

#endif //NON_RECOVERABLE_EXCEPTION_HH

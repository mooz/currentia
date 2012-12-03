#include "currentia/core/object.h"

int main(int argc, char **argv)
{
    using namespace currentia;

    Object number_obj(2378);
    Object string_obj("fofofo");

    std::cout << "sizeof number_obj: " << sizeof(number_obj) << std::endl;
    std::cout << "sizeof string_obj: " << sizeof(string_obj) << std::endl;

    return 0;
}

#include "currentia/core/tuple.h"

#include <iostream>

int main(int argc, char **argv)
{
    using namespace currentia;

    Schema::ptr_t schema(new Schema(1));
    schema->add_attribute("Test", TYPE_INT);
    schema->add_attribute(std::string("Test2"), TYPE_STRING);

    // inefficient! (copy constructor of data_t runs many times)
    Tuple::data_t data;
    data.push_back(Object(1));
    data.push_back(Object("Test!"));

    Tuple::ptr_t tuple = Tuple::create(schema, data);

    if (!tuple) {
        std::cerr << "Failed to create Tuple!" << std::endl;
    } else {
        std::cerr << "OK, created tuple => " << tuple->toString() << std::endl;
    }

    return 0;
}

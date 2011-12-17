// -*- c++ -*-

#ifndef CURRENTIA_OBJECT_H__
#define CURRENTIA_OBJECT_H__

namespace currentia {
    enum Type {
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_STRING,
        /* TODO */
        /* CURRENTIA_TYPE_BLOB, */
    };

    struct Object {
        /* TAG */
        enum Type type;
    };
}

#endif  /* ! CURRENTIA_OBJECT_H__ */

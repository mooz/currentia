// -*- c++ -*-

#ifndef CURRENTIA_STREAM_HANDLER_H_
#define CURRENTIA_STREAM_HANDLER_H_

#include "currentia/core/schema.h"
#include "currentia/core/stream.h"
#include "currentia/core/tuple.h"

#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"

namespace currentia {
    class StreamHandler: private NonCopyable<StreamHandler>,
                         public Pointable<StreamHandler> {
    protected:
        Stream::ptr_t stream_ptr_;
        Schema::ptr_t schema_ptr_;

    public:
        StreamHandler(const Stream::ptr_t& stream_ptr,
                      const Schema::ptr_t& schema_ptr):
            stream_ptr_(stream_ptr),
            schema_ptr_(schema_ptr) {
        }

        long get_id() {
            return schema_ptr_->get_id();
        }

        Tuple::ptr_t create_tuple(const Tuple::data_t& data) {
            return Tuple::create(schema_ptr_, data);
        }
    };
}

#endif  /* ! CURRENTIA_STREAM_HANDLER_H_ */

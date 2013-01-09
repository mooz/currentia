// -*- c++ -*-

#ifndef CURRENTIA_STREAM_CONSUMER_H_
#define CURRENTIA_STREAM_CONSUMER_H_

#include "currentia/core/stream.h"
#include "currentia/core/tuple.h"
#include "currentia/core/schema.h"

namespace currentia {
    class StreamConsumer : public thread::Runnable,
                           private NonCopyable<StreamConsumer> {
        Stream::ptr_t stream_;

    public:
        explicit StreamConsumer(const Stream::ptr_t& stream):
            stream_(stream) {
        }

        void run() {
            while (!stopped()) {
                Tuple::ptr_t next_tuple = stream_->dequeue();
                if (next_tuple->is_eos())
                    break;
                consume_tuple(next_tuple);
            }

            std::cout << "StreamConsumer Finished" << std::endl;
        }

        virtual void consume_tuple(const Tuple::ptr_t& tuple) {
            std::cout << tuple->toString() << std::endl;
        }
    };
}

#endif  /* ! CURRENTIA_STREAM_CONSUMER_H_ */

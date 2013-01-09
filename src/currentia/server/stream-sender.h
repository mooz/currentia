// -*- c++ -*-

#ifndef CURRENTIA_STREAM_SENDER_H_
#define CURRENTIA_STREAM_SENDER_H_

#include "currentia/core/stream.h"
#include "currentia/core/tuple.h"
#include "currentia/core/schema.h"

namespace currentia {
    class StreamSender : public thread::Runnable {
    protected:
        Stream::ptr_t stream_;
        Schema::ptr_t schema_;

        long send_count_;
        long send_interval_;
        long total_tuples_;

    public:
        StreamSender(const Stream::ptr_t& stream,
                     long total_tuples,
                     long send_interval):
            stream_(stream),
            schema_(stream->get_schema()),
            send_count_(0),
            send_interval_(send_interval),
            total_tuples_(total_tuples) {
        }

        virtual ~StreamSender() = 0;

        void run() {
            for (int i = 0; i < total_tuples_ && !stopped(); ++i) {
                stream_->enqueue(get_next(i));
                if (send_interval_ > 0)
                    usleep(send_interval_);
            }
            stream_->enqueue(Tuple::create_eos());

#ifdef CURRENTIA_DEBUG
            std::cout << "StreamSender Finished" << std::endl;
#endif
        }

        virtual Tuple::ptr_t get_next(long i) = 0;
    };
    StreamSender::~StreamSender() {}
}

#endif  /* ! CURRENTIA_STREAM_SENDER_H_ */

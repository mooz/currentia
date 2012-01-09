// -*- c++ -*-

#include "currentia/core/thread.h"
#include "currentia/server/server.h"

int main(int argc, char **argv)
{
    using namespace currentia;

    typedef void* (*pthread_body_t)(void*);

    Schema::ptr_t schema = create_schema();
    Server server(schema);

    // launch 2 threads

    pthread_t listen_thread;
    pthread_t process_thread;

    pthread_create(&listen_thread, NULL, reinterpret_cast<pthread_body_t>(listen_thread_body), &server);
    pthread_create(&process_thread, NULL, reinterpret_cast<pthread_body_t>(process_thread_body), &server);

    pthread_join(listen_thread, NULL);
    pthread_join(process_thread, NULL);

    return 0;
}

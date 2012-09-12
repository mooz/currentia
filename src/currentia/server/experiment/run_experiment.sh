#!/bin/sh

# options:
#       --method                      consistency preserving method (string [=none])
#       --update-interval             update interval (unsigned int [=1000])
#       --update-time                 time needed to update a relation (unsigned int [=10])
#       --purchase-interval           Purchase interval (unsigned int [=1000])
#       --aggregation-window-width    window width for aggregation (int [=10])
#       --purchase-count              Purchase count (int [=1000])
#       --goods-count                 Goods count (int [=1000])
#       --max-price                   Max price for purchases (int [=100000])
#       --min-price                   Min price for purchases (int [=1000])
#       --selection-condition         Condition for selection (string [=PRICE < 5000])
#       --purchase-schema             Purchase schema (string [=CREATE STREAM PURCHASES(GOODS_ID INT, USER_ID INT)])
#       --goods-schema                Goods schema (string [=CREATE TABLE GOODS(ID INT, PRICE INT)])
#       --reuse-snapshot              Reuse snapshot when possible (bool [=1])

assert_dir_exist() {
    if [ ! -d $1 ]; then
        mkdir $1
    fi
}

# -------------------------------------------------- #
# Make result dirs
# -------------------------------------------------- #

RESULT_DIR=result
assert_dir_exist $RESULT_DIR

QUERY_VS_UPDATE_DIR=${RESULT_DIR}/query_vs_update
assert_dir_exist $QUERY_VS_UPDATE_DIR

UPDATE_VS_STREAM_DIR=${RESULT_DIR}/update_vs_stream
assert_dir_exist $UPDATE_VS_STREAM_DIR

UPDATE_VS_WINDOW_DIR=${RESULT_DIR}/update_vs_window
assert_dir_exist $UPDATE_VS_WINDOW_DIR

# -------------------------------------------------- #

make_query() {
    window_width=$1
    window_slide=$2

    echo "stream purchases(goods_id: int, user_id: int)
     relation goods(id: int, price: int)
     stream combined_stream from purchases { combine goods where purchases.id = goods.goods_id }
     stream mean_stream from combined_stream {
         select goods.price < 50000
         mean user.user_id [recent ${window_width} slide ${window_slide} ]
     }
     stream result from mean_stream"
}

BIN=/home/masa/src/currentia/build/src/currentia/server/experiment_lock
METHODS="none optimistic 2pl snapshot"

PURCHASE_COUNT=5000
WINDOW_WIDTH=5

interval_to_rate() {
    echo "1000 * 1000 / $1" | bc
}

do_bench_query_vs_update() {
    # 更新レートを上げていったときのスループット
    # (method, rate)

    method=$1
    rate=$2

    file_name=${method}_${rate}.txt
    interval=$(interval_to_rate $rate)

    echo "---------------------------------------------------------"
    echo "Query v.s. Update (Update Rate: ${rate}, Interval: ${interval}, Method: ${method})"
    echo "---------------------------------------------------------"

    make_query 5 2 | ${BIN} \
        --purchase-interval 0 \
        --purchase-count ${PURCHASE_COUNT} \
        --method ${method} \
        --update-interval ${interval} \
        2>&1 | tee ${QUERY_VS_UPDATE_DIR}/${file_name}
}

do_bench_update_vs_stream() {
    # ストリームレートを上げていったときの更新スループット
    # (method, rate)

    method=$1
    rate=$2

    file_name=${method}_${rate}.txt
    interval=$(interval_to_rate $rate)

    echo "---------------------------------------------------------"
    echo "Update v.s. Stream (Query Rate ${rate})"
    echo "---------------------------------------------------------"

    make_query 5 2 | ${BIN} \
        --purchase-interval ${interval} \
        --purchase-count ${PURCHASE_COUNT} \
        --method ${method} \
        --update-interval 0 \
        2>&1 | tee ${UPDATE_VS_STREAM_DIR}/${file_name}
}

do_bench_update_vs_window() {
    # ウィンドウサイズを上げていったときの更新スループット
    # (method, rate)

    method=$1
    window_size=$2
    window_slide=$(expr ${window_size} / 2)

    file_name=${method}_${window_size}.txt

    purchase_interval=0
    update_interval=$(interval_to_rate 10000)

    echo "---------------------------------------------------------"
    echo "Update v.s. Window (Window Size ${window_size}, Window Slide: ${window_slide})"
    echo "---------------------------------------------------------"

    make_query ${window_size} ${window_slide} | ${BIN} \
        --purchase-interval ${purchase_interval} \
        --purchase-count ${PURCHASE_COUNT} \
        --method ${method} \
        --update-interval ${update_interval} \
        2>&1 | tee ${UPDATE_VS_WINDOW_DIR}/${file_name}
}

# for method in ${METHODS}; do
#     for rate in 1 3.1622776601683795 10 31.622776601683793 100 316.22776601683796 1000 3162.2776601683786 10000 31622.77660168381 100000; do
#         do_bench_query_vs_update ${method} ${rate}
#     done
# done

# for method in ${METHODS}; do
#     for rate in 100 316.22776601683796 1000 3162.2776601683786 10000 31622.77660168381 100000; do
#         do_bench_update_vs_stream ${method} ${rate}
#     done
# done

for method in ${METHODS}; do
    for window_size in 5 50 500; do
        do_bench_update_vs_window ${method} ${window_size}
    done
done

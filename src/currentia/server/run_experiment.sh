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

# -------------------------------------------------- #

BIN=/home/masa/src/currentia/build/src/currentia/server/experiment_lock
METHODS="none lock versioning"

PURCHASE_COUNT=100000
WINDOW_WIDTH=5
SELECTION_CONDITION="PRICE < 50000" # 選択率がおよそ 0.5 程度に

interval_to_rate() {
    echo "1000 * 1000 / $rate" | bc
}

do_bench_query_vs_update() {
    # 更新レートを上げていったときのスループット
    # (method, rate)

    method=$1
    rate=$2

    file_name=${method}_${rate}.txt
    interval=$(interval_to_rate $rate)

    echo "---------------------------------------------------------"
    echo "Query v.s. Update (Update Rate ${rate})"
    echo "---------------------------------------------------------"

    ${BIN} \
        --purchase-interval 0 \
        --purchase-count ${PURCHASE_COUNT} \
        --aggregation-window-width ${WINDOW_WIDTH} \
        --method ${method} \
        --selection-condition "${SELECTION_CONDITION}" \
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

    ${BIN} \
        --purchase-interval ${interval} \
        --purchase-count 1000 \
        --aggregation-window-width ${WINDOW_WIDTH} \
        --method ${method} \
        --selection-condition "${SELECTION_CONDITION}" \
        --update-interval 0 \
        2>&1 | tee ${UPDATE_VS_STREAM_DIR}/${file_name}
}

for rate in 1 10 100 1000 10000; do
    for method in ${METHODS}; do
        do_bench_query_vs_update ${method} ${rate}
    done
done

for rate in 100 1000 10000 100000; do
    for method in ${METHODS}; do
        do_bench_update_vs_stream ${method} ${rate}
    done
done

#!/bin/sh

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

PURCHASE_COUNT=50000
WINDOW_WIDTH=5

interval_to_rate() {
    echo "1000 * 1000 / $1" | bc
}

make_query 5 2 | ${BIN} \
    --purchase-interval 0 \
    --purchase-count ${PURCHASE_COUNT} \
    --method none \
    --goods-count 1 \
    --update-interval 100000

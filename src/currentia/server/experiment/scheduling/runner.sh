#!/bin/sh

EXPERIMENT_BIN=$(git rev-parse --show-cdup)build/src/currentia/server/experiment_scheduling

cat scheduling_query_template.cpl | \
    sed s"/!range!/${WIDTH}/" | \
    sed s"/!slide!/${SLIDE}/" | \
    ${EXPERIMENT_BIN} --no-color --method=2pl --max-events-n-consume=${BATCH_SIZE} --update-interval=100000 $*

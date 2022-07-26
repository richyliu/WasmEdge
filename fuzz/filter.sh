#!/usr/bin/env bash

# set -x

# total=$(ls -1 crash-* | wc -l)
# i=0
# for f in crash-*; do
#     if ./run "$f" |& rg -q "you may set allocator_may_return_null=1"; then
#         mv "$f" other_crash/
#     else
#         mv "$f" crashes_1/
#     fi
#     (( i++ ))
#     if ! (( i % 20 )); then
#         printf "%s     %4d / %d\n" "$(date)" $i $total
#     fi
# done


total=$(ls -1 crashes_5/* | wc -l)
i=0
for f in crashes_5/*; do
    if ./run "$f" |& rg -q "std::length_error"; then
        mv "$f" crashes_5_length_error/
    else
        # mv "$f" crashes_1/
        echo
    fi
    (( i++ ))
    if ! (( i % 20 )); then
        printf "%s     %4d / %d\n" "$(date)" $i $total
    fi
done

#!/bin/sh

set -e;

input="$1"
output="$2"

r=$(sed -n 's,.*\$Release: \(.*\)\$$,\1,p' "$input")
d=$(sed -n 's,/,-,g;s,.*\$Date: \(..........\).*,\1,p' "$input")
D=""
if [ -n "$d" ]; then
  D="--date=$d"
fi

pod2man --utf8 --release="darktable $r" --center="darktable" "$D" "$input" \
  | sed -e '/.*DREGGNAUTHORS.*/d' > "$output" || rm "$output"

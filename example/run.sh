#!/bin/bash

set -e

rm -rf bin
rm -rf stage

if hash b2 2>/dev/null; then
  b2 "$@"
elif hash bjam 2>/dev/null; then
  bjam "$@"
else
  echo >&2 "Require b2 or bjam but it was not found. Aborting."
  exit 1
fi

set +e

for file in stage/*
do
  echo ${file} "..."
  ${file}
done

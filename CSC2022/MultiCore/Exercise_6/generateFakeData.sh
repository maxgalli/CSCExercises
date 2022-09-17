#!/bin/bash

for run in {1..10000}; do
  cat fakeData.txt
done | dd of=bigFakeData.txt bs=4M

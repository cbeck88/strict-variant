#!/bin/bash
find include/safe_variant test/include test/ -type f -name "*.?pp" -print0 | xargs -0 sed -i "s|namespace safe_variant{|namespace safe_variant {|g"

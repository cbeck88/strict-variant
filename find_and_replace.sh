#!/bin/bash
find include/strict_variant test/include test/ -type f -name "*.?pp" -print0 | xargs -0 sed -i "s|SAFE_VARIANT_|STRICT_VARIANT_|g"

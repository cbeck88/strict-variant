#!/bin/bash
find include bench example test -type f -name "*.?pp" -print0 | xargs -0 sed -i "s|//  (C) Copyright 2016 - 2017 Christopher Beck|//  (C) Copyright 2016 - 2018 Christopher Beck|g"

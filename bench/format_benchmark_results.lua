#!/usr/bin/lua
local string = require 'string'
local io = require 'io'
local math = require 'math'

local result = {}
local all_nums = {}

local variant_type = nil
local num_variants = nil

local line = io.read('*l')

local pastb2_part = false

while line and not pastb2_part do
  if (line:find('...updated %d+ targets...')) then
    pastb2_part = true
  end

  line = io.read('*l')
end

while line do
  -- print ('line:\n', line)
  if (line:find('.-:$')) then
    variant_type = line:match('(.-):$')
    assert(variant_type)
    -- print('matched a variant type: ', variant_type)

  elseif (line:find('  num_variants = ')) then
    local match = line:match('%d+')
    assert(match)
    num_variants = tonumber(match)
    assert(num_variants)
    assert(num_variants==math.floor(num_variants))
    all_nums[num_variants] = true

    -- print('matched a num_variants: ', num_variants)

  elseif (line:find('average nanoseconds per visit:')) then
    time = line:match('average nanoseconds per visit: (.+)$')
    assert(time)

    if not result[variant_type] then result[variant_type] = {} end
    result[variant_type][num_variants] = time

    -- print('matched a time: ', time)
  end

  line = io.read('*l')
end

-- print "all nums = "
-- for k, _ in pairs(all_nums) do
--   print(k)
-- end
-- print('(' .. #all_nums .. ' entries)')

local function start_column(name)
  return string.format('| %28s ', name)
end

local function add_entry(col, data)
  return string.format('%s| %9s ', col, data or 'N/A')
end

local line = start_column('Number of types')

for k, _ in pairs(all_nums) do
  line = add_entry(line, k)
end

io.write(line .. '|\n')

local line = start_column(string.rep('-', 28))

for k, _ in pairs(all_nums) do
  line = add_entry(line, '---------')
end

io.write(line .. '|\n')

for vname, vtab in pairs(result) do
  line = start_column('`' .. vname .. '`')
  for k, _ in pairs(all_nums) do
    line = add_entry(line, vtab[k])
  end
  io.write(line .. '|\n')
end

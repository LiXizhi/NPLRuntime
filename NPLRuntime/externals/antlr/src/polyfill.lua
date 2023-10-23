local codeblock_env = {}

-- imported code block functions:
-- TODO: this implementation is wrong. we should install locally per code block. 
local codeblock_funcs = {"print", "ask"}
local function _set_codeblock_env(env) for _, func_name in ipairs(codeblock_funcs) do codeblock_env['cb_' .. func_name] = env[func_name] end end

local max_bit_length = 32

local function check_int(n)
    -- checking not float
    if (n - math.floor(n) > 0) then error("trying to use bitwise operation on non-integer!") end
end

local function number_to_tbl(n)
    check_int(n)

    local tbl = {}

    if n >= 0 then
        for i = 1, max_bit_length do
            local last = math.mod(n, 2)
            tbl[i] = last
            n = math.floor(n / 2)
        end
    else -- n < 0
        local p = -n
        for i = 1, max_bit_length do
            local last = math.mod(p, 2)
            tbl[i] = last
            p = math.floor(p / 2)
        end

        -- invert
        local invert_tbl = {}
        for i = 1, max_bit_length do
            local b = tbl[i]
            if b == 1 then
                invert_tbl[i] = 0
            else
                invert_tbl[i] = 1
            end
        end
        -- plus 1
        local plus_one_tbl = {}
        local carry = 1
        for i = 1, max_bit_length do
            local b = invert_tbl[i]
            local s = b + carry
            if s == 2 then
                carry = 1
                plus_one_tbl[i] = 0
            else
                carry = 0
                plus_one_tbl[i] = s
            end
        end

        tbl = plus_one_tbl
    end

    return tbl
end

local function tbl_to_number(tbl)
    local power = 1
    local n = 0
    for i = 1, max_bit_length do
        local b = tbl[i]
        if i == max_bit_length and b == 1 then
            n = n - power
        else
            n = n + power * b
        end

        power = power * 2
    end

    return n
end

local function bit_and(left, right)
    local left_tbl = number_to_tbl(left)
    local right_tbl = number_to_tbl(right)

    local tbl = {}
    for i = 1, max_bit_length do
        if left_tbl[i] == 0 or right_tbl[i] == 0 then
            tbl[i] = 0
        else
            tbl[i] = 1
        end
    end

    return tbl_to_number(tbl)
end

local function bit_or(left, right)
    local left_tbl = number_to_tbl(left)
    local right_tbl = number_to_tbl(right)

    local tbl = {}
    for i = 1, max_bit_length do
        if left_tbl[i] == 0 and right_tbl[i] == 0 then
            tbl[i] = 0
        else
            tbl[i] = 1
        end
    end

    return tbl_to_number(tbl)
end

local function bit_not(n)
    local n_tbl = number_to_tbl(n)

    local tbl = {}
    for i = 1, max_bit_length do
        if n_tbl[i] == 0 then
            tbl[i] = 1
        else
            tbl[i] = 0
        end
    end

    return tbl_to_number(tbl)
end

local function bit_xor(left, right)
    local left_tbl = number_to_tbl(left)
    local right_tbl = number_to_tbl(right)

    local tbl = {}
    for i = 1, max_bit_length do
        if left_tbl[i] == right_tbl[i] then
            tbl[i] = 0
        else
            tbl[i] = 1
        end
    end

    return tbl_to_number(tbl)
end

local function bit_rshift(n, shift)
    check_int(n)
    assert(shift >= 0, "negative shift count")

    local tbl = number_to_tbl(n)
    local rshift_tbl = {}

    for i = 1, max_bit_length do
        local j = i + shift
        if j > max_bit_length then j = max_bit_length end
        rshift_tbl[i] = tbl[j]
    end

    return tbl_to_number(rshift_tbl)
end

local function bit_lshift(n, shift)
    check_int(n)
    assert(shift >= 0, "negative shift count")

    local tbl = number_to_tbl(n)
    local lshift_tbl = {}

    for i = 1, max_bit_length do
        local j = i - shift
        if j < 1 then
            lshift_tbl[i] = 0
        else
            lshift_tbl[i] = tbl[j]
        end
    end

    return tbl_to_number(lshift_tbl)
end

local bit = {band = bit_and, bor = bit_or, bnot = bit_not, bxor = bit_xor, brshift = bit_rshift, blshift = bit_lshift}

--[[
无论 + 左右哪个是 string 类型的值，都会引发元方法的调用
所以一旦调用这个方法，v1 v2 其中必定有一个 string 类型
]]
local string_meta = getmetatable("")
string_meta.__add = function(v1, v2)
    assert(type(v1) == "string" and type(v2) == "string", "can not explicitly do " .. type(v1) .. " + " .. type(v2))
    return v1 .. v2
end

local g_real_unpack = unpack or table.unpack

local unpack = function(t)
    if type(t) == "table" and t._is_list then return g_real_unpack(t._data) end
    return g_real_unpack(t)
end

local g_real_next = next

-- next(iterator[, default])
-- return the next item from the iterator. If default is given and the iterator
-- is exhausted, it is returned instead of raising StopIteration.
-- fixme: support generator only for now
local next = function(iter, default)
    local v = nil
    if iter._is_generator then v = iter.next() end
    v = v or default
    return v
end

-- range(stop) -> range object
-- range(start, stop[, step]) -> range object
-- return a sequence of numbers from start to stop by step.
local function range(...)
    local p, l = {...}, select('#', ...)
    assert(l ~= 0, 'range() expected 1 arguments, got 0')

    local start = nil
    local stop = nil
    local step = nil

    if l == 1 then
        stop = p[1]
    elseif l == 2 then
        start = p[1]
        stop = p[2]
    else
        start = p[1]
        stop = p[2]
        step = p[3]
    end

    start = start or 0
    step = step or 1

    assert(step ~= 0, 'range() arg 3 must not be zero')

    local i = start

    return function()
        ret = i
        if (step > 0 and i >= stop) or (step < 0 and i <= stop) then return nil, nil end

        i = i + step
        return i, ret
    end
end

-- represent nil in table
-- https://stackoverflow.com/questions/40441508/how-to-represent-nil-in-a-table
local function _null() end

local function _to_null(...)
    local t, n = {...}, select('#', ...)
    for k = 1, n do
        local v = t[k]
        if v == nil then t[k] = _null end
    end
    return unpack(t, 1, n)
end

local function _to_nil(...)
    local t, n = {...}, select('#', ...)
    for k = 1, n do
        local v = t[k]
        if v == _null then t[k] = nil end
    end
    return unpack(t, 1, n)
end

-- list() -> new empty list
-- list(iterable) -> new list initialized from iterable's items
local list = {}
setmetatable(list, {
    __call = function(_, t)
        local result = {}

        result._is_list = true
        result._data = {}
        result._len = 0

        if t ~= nil then
            if t.__iter__ then
                -- iterable
                local l = 0
                for _, v in t do
                    l = l + 1
                    result._data[l] = v
                end
                result._len = l
            else
                -- list literal
                local l = 0
                for k, v in pairs(t) do
                    l = l + 1
                    result._data[k] = _to_nil(v)
                end
                result._len = l
            end
        else
            -- list()
        end

        local py_to_lua_idx = function(i, size)
            if i >= 0 then
                i = i + 1
                if i > size then i = size end
            else
                i = i + size + 1
                if i < 1 then i = 1 end
            end
            return i
        end

        local py_idx = function(i, size)
            if i >= 0 then
                if i > size then i = size end
            else
                i = i + size
                if i < 0 then i = 0 end
            end
            return i
        end

        local methods = {}

        -- L.append(object) -> None -- append object to end
        methods.append = function(self, value)
            self._len = self._len + 1
            self._data[self._len] = value
        end

        -- L.clear() -> None -- remove all items from L
        methods.clear = function(self)
            self._len = 0
            self._data = {}
        end

        -- L.copy() -> list -- a shallow copy of L
        methods.copy = function(self)
            local c = list(self._data)
            c._len = self._len
            return c
        end

        -- L.count(value) -> integer -- return number of occurrences of value
        methods.count = function(self, value)
            local cnt = 0
            for i = 1, self._len do
                local v = self._data[i]
                if v == value then cnt = cnt + 1 end
            end
            return cnt
        end

        -- L.extend(iterable) -> None -- extend list by appending elements from the iterable
        methods.extend = function(self, iterable) for _, value in iterable do self.append(value) end end

        -- L.index(value, [start, [stop]]) -> integer -- return first index of value.
        -- 相当于在 L[start:stop] 中寻找 value
        -- 当 start stop 超过开始/结束的界限，算作界限本身
        methods.index = function(self, value, start, stop)
            local size = self._len

            if not start then
                start = 1
            else
                start = py_to_lua_idx(start, size)
            end

            if not stop then
                stop = size
            else
                stop = py_to_lua_idx(stop, size)
            end

            if start >= stop then return nil end

            for i = start, stop - 1, 1 do if result._data[i] == value then return i - 1 end end
        end

        -- L.insert(index, object) -- insert object before index
        methods.insert = function(self, index, value)
            local size = self._len

            if index + 1 > size then
                index = size + 1
            else
                index = py_to_lua_idx(index, size)
            end

            self._len = self._len + 1
            for i = self._len, 1, -1 do if i > index then self._data[i] = self._data[i - 1] end end
            self._data[index] = value
        end

        -- L.pop([index]) -> item -- remove and return item at index (default last).
        methods.pop = function(self, index)
            local size = self._len

            if not index then
                index = size
            else
                if index + 1 > size or index + 1 + size < 1 then
                    error("list pop index out of range")
                else
                    index = py_to_lua_idx(index, size)
                end
            end

            local value = result._data[index]

            for i = index + 1, self._len do self._data[i - 1] = self._data[i] end
            self._len = self._len - 1

            return value
        end

        -- L.remove(value) -> None -- remove first occurrence of value.
        methods.remove = function(self, value)
            for i = 1, self._len do
                v = self._data[i]
                if value == v then
                    self.pop(i - 1) -- use py index
                    break
                end
            end
        end

        -- L.reverse() -- reverse *IN PLACE*
        methods.reverse = function(self)
            local r = {}
            for i = 1, self._len do r[i] = self._data[self._len + 1 - i] end
            self._data = r
        end

        -- L.sort(key=None, reverse=False) -> None -- stable sort *IN PLACE*
        -- TODO: sort 的参数是键值参数，目前暂时不支持，所以还无法传递 key 和 reverse
        -- TODO: key is a callable
        methods.sort = function(self, key, reverse)
            key = key or function(itself) return itself end
            reverse = reverse or false

            assert(self.count(nil) == 0, "unorderable types: NoneType")

            table.sort(self._data, function(a, b)
                if reverse then return key(a) > key(b) end

                return key(a) < key(b)
            end)
        end

        -- __iter__
        -- delegate to metatable __call
        methods.__iter__ = function(self) return self end

        local iterator_index = 0

        setmetatable(result, {
            __index = function(self, index)
                if type(index) == "number" then
                    index = py_to_lua_idx(index, self._len)
                    return rawget(self._data, index)
                end

                if type(index) == "table" and index._is_slice == true then
                    local s = list()

                    local start = index.start or 0
                    start = py_idx(start, self._len)
                    local stop = index.stop or self._len
                    stop = py_idx(stop, self._len)
                    local step = index.step or 1

                    for _, i in range(start, stop, step) do s.append(self[i]) end
                    return s
                end

                return function(...) return methods[index](self, ...) end
            end,
            -- only number index is permitted in python
            __newindex = function(self, index, value)
                index = py_to_lua_idx(index, self._len)
                rawset(self._data, index, value)
            end,
            __call = function(self, _, idx)
                if idx == nil then iterator_index = 0 end

                iterator_index = iterator_index + 1
                local v = self._data[iterator_index]

                if iterator_index > self._len then
                    return nil, nil
                else
                    return iterator_index, v
                end
            end
        })

        return result
    end
})

-- bool(x) -> bool
-- returns True when the argument x is true, False otherwise.
-- the builtins True and False are the only two instances of the class bool.
-- the class bool is a subclass of the class int, and cannot be subclassed.
local function bool(x)
    if x == false or x == nil or x == 0 or x == '' then return false end

    if type(x) == "table" then if x._is_list or x._is_dict then return g_real_next(x._data) ~= nil end end

    return true
end

--[[
   builtin functions
   ref: https://docs.python.org/3.4/library/functions.html
--]]

-- abs(number) -> number
-- return the absolute value of the argument.
local abs = math.abs

-- all(iterable) -> bool
-- return true if bool(x) is true for all values x in the iterable.
-- if the iterable is empty, return True.
local function all(iterable)
    for _, element in iterable do if not bool(element) then return false end end
    return true
end

-- any(iterable) -> bool
-- return true if bool(x) is true for any x in the iterable.
-- if the iterable is empty, return False.
local function any(iterable)
    for _, element in iterable do if bool(element) then return true end end
    return false
end

-- divmod(x, y) -> (div, mod)
-- return the tuple ((x-x%y)/y, x%y).  Invariant: div*y + mod == x.
local function divmod(a, b)
    local d = math.floor(a / b)
    local m = a - d * b
    return d, m
end

-- ascii(object) -> string
-- as repr(), return a string containing a printable representation of an
-- object, but escape the non-ASCII characters in the string returned by
-- repr() using \x, \u or \U escapes.  This generates a string similar
-- to that returned by repr() in Python 2.
local function ascii(obj)
    -- todo
end

-- bin(number) -> string
-- return the binary representation of an integer.
-- >>> bin(2796202)
-- '0b1010101010101010101010'
local function bin(num)
    assert(type(num) == 'number', 'num is not a number in bin()')
    assert(math.floor(num) == num, 'num is a float in bin()')

    local prefix = '0b'

    local b = ''
    local m = 0
    repeat
        num, m = divmod(num, 2)
        b = tostring(m) .. b
    until num == 0

    return prefix .. b
end

-- callable(object) -> bool
-- return whether the object is callable (i.e., some kind of function).
-- note that classes are callable, as are instances of classes with a
-- __call__() method.
local function callable(obj)
    local obj_type = type(obj)
    if obj_type == "function" then return true end
    if obj_type == "table" then
        if obj._is_list or obj._is_dict then return false end

        local meta = getmetatable(obj)
        return type(meta.__call) == "function"
    end

    return false
end

-- enumerate(iterable[, start]) -> iterator for index, value of iterable
-- return an enumerate object.  iterable must be another object that supports
-- iteration.  The enumerate object yields pairs containing a count (from
-- start, which defaults to zero) and a value yielded by the iterable argument.
-- enumerate is useful for obtaining an indexed list:
--     (0, seq[0]), (1, seq[1]), (2, seq[2]), ...
local function enumerate(t, start)
    -- assume t is a list
    start = start or 0

    local i, v = t(nil, nil)
    return function()
        local index, value = i, v
        if index == nil then return nil end

        i, v = t(nil, i)

        return index, index + start - 1, value
    end
end

-- filter(function or None, iterable) --> filter object
-- return an iterator yielding those items of iterable for which function(item)
-- is true. If function is None, return the items that are true.
local function filter(func, iterable)
    func = func or bool
    -- fixme: use list for now, no lazy
    local l = list {}
    for _, item in iterable do if func(item) then l.append(item) end end
    return l
end

-- float(x) -> floating point number
-- convert a string or number to a floating point number, if possible.
local function float(x)
    local n = tonumber(x)
    assert(n ~= nil, "could not convert string to float " .. tostring(x))
    return n
end

-- hex(number) -> string
-- return the hexadecimal representation of an integer.
--   >>> hex(3735928559)
--   '0xdeadbeef'
local function hex(num)
    assert(type(num) == 'number', 'num is not a number in hex(num)')
    assert(math.floor(num) == num, 'num is a float in hex(num)')

    local int_hex_map = {
        [0] = '0',
        [1] = '1',
        [2] = '2',
        [3] = '3',
        [4] = '4',
        [5] = '5',
        [6] = '6',
        [7] = '7',
        [8] = '8',
        [9] = '9',
        [10] = 'a',
        [11] = 'b',
        [12] = 'c',
        [13] = 'd',
        [14] = 'e',
        [15] = 'f'
    }

    local prefix = '0x'
    local m = 0
    local h = ''
    repeat
        num, m = divmod(num, 16)
        h = int_hex_map[m] .. h
    until num == 0

    return prefix .. h
end

-- int(x=0) -> integer
-- int(x, base=10) -> integer
-- convert a number or string to an integer, or return 0 if no arguments
-- are given.  If x is a number, return x.__int__().  For floating point
-- numbers, this truncates towards zero.
-- if x is not a number or if base is given, then x must be a string,
-- bytes, or bytearray instance representing an integer literal in the
-- given base.
-- the literal can be preceded by '+' or '-' and be surrounded
-- by whitespace. The base defaults to 10.  Valid bases are 0 and 2-36.
-- base 0 means to interpret the base from the string as an integer literal.
--   >>> int('0b100', base=0)
--   4
local function int(x, base)
    if x == nil then return 0 end

    if type(x) == 'number' then
        if x >= 0 then
            return math.floor(x)
        else
            return math.ceil(x)
        end
    end

    assert(type(x) == 'string', "int() can't convert non-string with explicit base")

    base = base or 10

    if base == 0 then
        local is_neg = string.find(x, '-') ~= nil
        local pos = string.gsub(x, '-', '')

        if string.find(x, '0x') then
            base = 16
            pos = string.gsub(pos, '0x', '')
        elseif string.find(x, '0b') then
            base = 2
            pos = string.gsub(pos, '0b', '')
        else
            base = 10
        end

        local n = tonumber(pos, base)

        assert(n ~= nil, "invalid literal for int(): " .. x)

        if is_neg then n = -n end

        return n
    else
        assert(base >= 2 and base <= 40, "int() base must be >= 2 and <= 36")

        local is_neg = string.find(x, '-') ~= nil
        local pos = string.gsub(x, '-', '')
        local n = tonumber(pos, base)

        assert(n ~= nil, "invalid literal for int(): " .. x)

        if is_neg then n = -n end

        return n
    end

end

-- len(object)
-- return the number of items of a sequence or collection.
local function len(t)
    if type(t._data) == "table" then
        if t._is_list == true or t._is_tuple == true then
            return t._len
        else
            local l = 0
            for k, v in pairs(t._data) do l = l + 1 end
            return l
        end
    end

    return #t
end

-- different from coroutine.wrap
-- the wrapper return code and res
local function coroutine_wrap(func)
    local co = coroutine.create(func)
    local code, res = coroutine.resume(co)
    local ret_code
    local ret_res
    return function()
        local next_code, next_res = coroutine.resume(co)

        if next_code == false then
            ret_code = nil
        else
            ret_code = code
        end
        ret_res = res

        code = next_code
        res = next_res

        return ret_code, ret_res
    end
end

-- generator (and meta) class
local meta_generator = {}
setmetatable(meta_generator, {
    __call = function(_, t)
        local result = {}

        result._is_generator = true
        result._func = t

        local methods = {}

        methods.next = function(self)
            local stat, value = coroutine.resume(self._generator)
            return value
        end

        methods.send = function(self, ...)
            local stat, value = coroutine.resume(self._generator, ...)
            return value
        end

        methods.close = function(self) end

        setmetatable(result, {
            __call = function(self)
                local g = {}
                g._generator = coroutine.create(self._func)

                setmetatable(g, {__index = function(self, index) return function(...) return methods[index](self, ...) end end})

                return g
            end
        })

        return result
    end
})

-- map(func, *iterables) --> map object
-- make an iterator that computes the function using arguments from
-- each of the iterables.  Stops when the shortest iterable is exhausted.
local function map(func, ...)
    local iterables = list {...}
    local lists = list {}

    local iter_num = len(iterables)
    local min_len = math.huge

    for _, it in iterables do
        lists.append(list(it))

        local l = len(list(it))
        if l < min_len then min_len = l end
    end

    local res = list {}
    for _, nth in range(min_len) do
        local param = {}
        for _, ith in range(iter_num) do param[#param + 1] = lists[ith][nth] end
        res.append(func(unpack(param)))
    end

    return res
end

-- oct(number) -> string
-- return the octal representation of an integer.
--    >>> oct(342391)
--    '0o1234567'
local function oct(num)
    assert(type(num) == 'number', 'num is not a number in hex()')
    assert(math.floor(num) == num, 'num is a float in hex()')

    local prefix = '0o'

    local b = ''
    local m = 0
    repeat
        num, m = divmod(num, 8)
        b = tostring(m) .. b
    until num == 0

    return prefix .. b
end

-- max(iterable, *[, default=obj, key=func]) -> value
-- max(arg1, arg2, *args, *[, key=func]) -> value
-- with a single iterable argument, return its biggest item. The
-- default keyword-only argument specifies an object to return if
-- the provided iterable is empty.
-- with two or more arguments, return the largest argument.
local function max(arg1, ...)
    local rest = list {...}
    local iterable = arg1
    if len(rest) ~= 0 then
        rest.append(arg1)
        iterable = rest
    end

    local default = nil
    if len(iterable) == 0 then return default end

    local m = nil
    for item in iterable do
        if m == nil then
            m = item
        else
            if item > m then m = item end
        end
    end

    return m
end

local function memoryview(object)
    -- not support
end

-- min(iterable, *[, default=obj, key=func]) -> value
-- min(arg1, arg2, *args, *[, key=func]) -> value
-- with a single iterable argument, return its smallest item. The
-- default keyword-only argument specifies an object to return if
-- the provided iterable is empty.
-- with two or more arguments, return the smallest argument.
local function min(arg1, ...)
    local rest = list {...}
    local iterable = arg1
    if len(rest) ~= 0 then
        rest.append(arg1)
        iterable = rest
    end

    local default = nil
    if len(iterable) == 0 then return default end

    local m = nil
    for item in iterable do
        if m == nil then
            m = item
        else
            if item < m then m = item end
        end
    end

    return m
end

-- reduce(function, sequence[, initial]) -> value
-- apply a function of two arguments cumulatively to the items of a sequence,
-- from left to right, so as to reduce the sequence to a single value.
-- for example, reduce(lambda x, y: x+y, [1, 2, 3, 4, 5]) calculates
-- ((((1+2)+3)+4)+5).  If initial is present, it is placed before the items
-- of the sequence in the calculation, and serves as a default when the
-- sequence is empty.
local function reduce(func, seq, init)
    if len(seq) == 0 then return init end

    local r = init
    for e in seq do
        if init == nil then
            r = e
            init = true -- not nil
        else
            r = func(r, e)
        end
    end

    return r
end

-- reversed(sequence) -> reverse iterator over values of the sequence
-- return a reverse iterator
local function reversed(seq)
    local l = list(seq)
    l.reverse()
    return l
end

-- dict() -> new empty dictionary
-- dict(mapping) -> new dictionary initialized from a mapping object's
--     (key, value) pairs
-- dict(iterable) -> new dictionary initialized as if via:
--     d = {}
--     for k, v in iterable:
--         d[k] = v
-- dict(**kwargs) -> new dictionary initialized with the name=value pairs
--     in the keyword argument list.  For example:  dict(one=1, two=2)
local dict = {}
setmetatable(dict, {
    __call = function(_, t)
        local result = {}

        result._is_dict = true
        result._data = {}

        if t ~= nil then
            if t._is_dict then
                -- dict(d)
                for _, k, v in t.items() do result._data[_to_null(k)] = _to_null(v) end
            elseif t.__iter__ then
                -- dict(iterable)
                -- TODO: ?
                for k, v in t do result._data[k] = v end
            else
                -- dict literal
                -- None has been _null, both in keys and values
                for k, v in pairs(t) do result._data[k] = v end
            end
        else
            -- dict()
        end

        local methods = {}

        -- D.clear() -> None.  Remove all items from D.
        methods.clear = function(self) self._data = {} end

        -- D.copy() -> a shallow copy of D
        methods.copy = function(self) return dict(self._data) end

        -- d.fromkeys(iterable, value=None, /)
        -- returns a new dict with keys from iterable and values equal to value.
        methods.fromkeys = function(self, keys, value)
            value = _to_null(value or nil)

            local d = dict()
            for _, k in keys do d._data[_to_null(k)] = value end
            return d
        end

        -- D.get(k[,d]) -> D[k] if k in D, else d.  d defaults to None.
        methods.get = function(self, key, default)
            value = self._data[_to_null(key)] or _to_null(default or nil)
            return _to_nil(value)
        end

        -- D.items() -> a set-like object providing a view on D's items
        methods.items = function(self)
            return function(_, idx)
                idx, v = g_real_next(self._data, idx)
                return idx, _to_nil(idx), _to_nil(v)
            end
        end

        -- D.keys() -> a set-like object providing a view on D's keys
        methods.keys = function(self)
            return function(_, idx)
                idx, v = g_real_next(self._data, idx)
                return idx, _to_nil(idx)
            end
        end

        -- D.pop(k[,d]) -> v, remove specified key and return the corresponding value.
        -- if key is not found, d is returned if given, otherwise KeyError is raised
        methods.pop = function(self, key, default)
            value = self._data[_to_null(key)] or _to_null(default or nil)
            self._data[_to_null(key)] = nil
            return _to_nil(value)
        end

        -- D.popitem() -> (k, v), remove and return some (key, value) pair as a
        -- 2-tuple; but raise KeyError if D is empty.
        methods.popitem = function(self)
            local key, value = g_real_next(self._data)
            if key ~= nil then self._data[key] = nil end

            return _to_nil(key), _to_nil(value)
        end

        -- D.setdefault(k[,d]) -> D.get(k,d), also set D[k]=d if k not in D
        methods.setdefault = function(self, key, default)
            if self._data[_to_null(key)] == nil then self._data[_to_null(key)] = _to_null(default) end

            return self._data[_to_null(key)]
        end

        -- D.update([E, ]**F) -> None.  Update D from dict/iterable E and F.
        -- if E is present and has a .keys() method, then does:  for k in E: D[k] = E[k]
        -- if E is present and lacks a .keys() method, then does:  for k, v in E: D[k] = v
        -- in either case, this is followed by: for k in F:  D[k] = F[k]
        methods.update = function(self, t)
            if t._is_dict ~= true then return end

            for _, k, v in t.items() do self._data[_to_null(k)] = _to_null(v) end
        end

        -- D.values() -> an object providing a view on D's values
        methods.values = function(self)
            return function(_, idx)
                idx, v = g_real_next(self._data, idx)
                return idx, _to_nil(v)
            end
        end

        -- __iter__()
        -- delegate to dict __call
        methods.__iter__ = function(self) return self end

        setmetatable(result, {
            __index = function(self, index)
                if self._data[_to_null(index)] ~= nil then return _to_nil(self._data[_to_null(index)]) end

                if methods[index] ~= nil then return function(...) return methods[index](self, ...) end end
                return nil
            end,
            __newindex = function(self, index, value) self._data[_to_null(index)] = _to_null(value) end,
            __call = function(self, _, idx)
                idx, _ = g_real_next(self._data, idx)
                return idx, _to_nil(idx)
            end
        })

        return result
    end
})

local function staticmethod(old_fun)
    local wrapper = function(first, ...) return old_fun(...) end

    return wrapper
end

local function operator_in(item, items)
    if type(items) == "table" then
        for _, v in items do if v == item then return true end end
    elseif type(items) == "string" and type(item) == "string" then
        return string.find(items, item, 1, true) ~= nil
    end

    return false
end

local function operator_is(a, b)
    local type_a = type(a)
    local type_b = type(b)

    if type_a ~= type_b then return false end

    if type_a == "table" then
        -- memory address
        return tostring(a) == tostring(b)
    end

    return a == b
end

-- object class
local object = {}
object.__name__ = 'object'
object.__base__ = nil
object.__mro__ = list {object}

-- pow(x, y[, z]) -> number
-- with two arguments, equivalent to x**y.  With three arguments,
-- equivalent to (x**y) % z, but may be more efficient (e.g. for ints).
local function pow(x, y, z)
    local p = math.pow(x, y)
    if z == nil then
        return p
    else
        return math.fmod(p, z)
    end
end

-- round(number[, ndigits]) -> number
-- round a number to a given precision in decimal digits (default 0 digits).
-- this returns an int when called with one argument, otherwise the
-- same type as the number. ndigits may be negative.
local function round(number, ndigits)
    ndigits = ndigits or 0

    local shift = math.pow(10, ndigits)
    local n = number * shift
    local i, f = math.modf(n)
    if f >= 0.5 then i = i + 1 end

    n = i / shift
    return n
end

-- set() -> new empty set object
-- set(iterable) -> new set object
-- build an unordered collection of unique elements.
-- ref: https://docs.python.org/3/library/stdtypes.html#set-types-set-frozenset
local set = {}
setmetatable(set, {
    __call = function(_, t)
        local result = {}

        result._is_set = true
        result._data = {}

        if t ~= nil then
            if t.__iter__ then
                -- iterable
                for _, v in t do result._data[_to_null(v)] = true end
            else
                -- set literal
                -- None has passed to _null
                for _, v in pairs(t) do result._data[v] = true end
            end
        else
            -- set()
        end

        local methods = {}

        -- add an element to a set.
        -- this has no effect if the element is already present.
        methods.add = function(self, elem) self._data[_to_null(elem)] = true end

        -- remove all elements from this set.
        methods.clear = function(self) self._data = {} end

        -- return a shallow copy of a set.
        methods.copy = function(self) return set(self) end

        -- return the difference of two or more sets as a new set.
        -- (i.e. all elements that are in this set but not the others.)
        methods.difference = function(self, ...)
            local diff_set = self.copy()

            local others = list {...}
            for _, other_set in others do for _, elem in other_set do if operator_in(elem, diff_set) then diff_set.remove(elem) end end end

            return diff_set
        end

        -- remove all elements of another set from this set.
        methods.difference_update = function(self, ...)
            local others = list {...}
            for _, other_set in others do for _, elem in other_set do if operator_in(elem, self) then self.remove(elem) end end end
        end

        -- remove an element from a set if it is a member.
        -- if the element is not a member, do nothing.
        methods.discard = function(self, elem) if operator_in(elem, self) then self.remove(elem) end end

        -- return the intersection of two sets as a new set.
        -- (i.e. all elements that are in both sets.)
        methods.intersection = function(self, ...)
            local inter_set = self.copy()

            local others = list {...}
            for _, other_set in others do inter_set.difference_update(inter_set.difference(other_set)) end

            return inter_set
        end

        -- update a set with the intersection of itself and another.
        methods.intersection_update = function(self, ...)
            local others = list {...}
            for _, other_set in others do self.difference_update(self.difference(other_set)) end
        end

        -- return True if two sets have a null intersection.
        methods.isdisjoint = function(self, other)
            local inter_set = self.intersection(other)
            return len(inter_set) == 0
        end

        -- report whether another set contains this set.
        methods.issubset = function(self, other)
            local inter_set = self.intersection(other)
            return len(inter_set) == len(self)
        end

        -- report whether this set contains another set.
        methods.issuperset = function(self, other)
            local inter_set = self.intersection(other)
            return len(inter_set) == len(other)
        end

        -- remove and return an arbitrary set element.
        -- raises KeyError if the set is empty.
        methods.pop = function(self)
            assert(len(self) ~= 0, "set is empty")
            local elem = nil
            for _, e in self do elem = e end
            self.remove(elem)
            return elem
        end

        -- remove an element from a set; it must be a member.
        -- if the element is not a member, raise a KeyError.
        methods.remove = function(self, elem)
            assert(operator_in(elem, self), "elem not in set")
            elem = _to_null(elem)
            self._data[elem] = nil
        end

        -- return the symmetric difference of two sets as a new set.
        -- (i.e. all elements that are in exactly one of the sets.)
        methods.symmetric_difference = function(self, other)
            local union_set = self.union(other)
            local inter_set = self.intersection(other)
            return union_set.difference(inter_set)
        end

        -- update a set with the symmetric difference of itself and another.
        methods.symmetric_difference_update = function(self, other)
            local inter_set = self.intersection(other)
            self.update(other)
            self.difference_update(inter_set)
        end

        -- return the union of sets as a new set.
        -- (i.e. all elements that are in either set.)
        methods.union = function(self, ...)
            local union_set = self.copy()

            local others = list {...}
            for _, other_set in others do for _, elem in other_set do union_set.add(elem) end end

            return union_set
        end

        -- Update a set with the union of itself and others.
        methods.update = function(self, ...)
            local others = list {...}
            for _, other_set in others do for _, elem in other_set do self.add(elem) end end
        end

        -- __iter__
        -- delegate to metatable __call
        methods.__iter__ = function(self) return self end

        local iterator_index = nil

        setmetatable(result, {
            __index = function(self, index)
                if type(index) == "number" then error("'set' object does not support indexing") end

                return function(...) return methods[index](self, ...) end
            end,
            __call = function(self, _, idx)
                if idx == nil then iterator_index = nil end

                iterator_index, _ = g_real_next(self._data, iterator_index)

                return iterator_index, _to_nil(iterator_index)
            end
        })

        return result
    end
})

-- frozenset() -> empty frozenset object
-- frozenset(iterable) -> frozenset object
-- build an immutable unordered collection of unique elements.
local frozenset = {}
setmetatable(frozenset, {
    __call = function(_, t)
        local result = {}

        result._is_frozenset = true
        result._data = {}

        if t ~= nil then
            if t.__iter__ then
                -- iterable
                for _, v in t do result._data[_to_null(v)] = true end
            else
                -- no frozenset literal
            end
        else
            -- frozenset()
        end

        local methods = {}

        -- return a shallow copy of a frozenset.
        methods.copy = function(self) return self end

        -- return the difference of two or more sets as a new set.
        -- (i.e. all elements that are in this set but not the others.)
        methods.difference = function(self, ...)
            local diff_set = set(self)

            local others = list {...}
            for _, other_set in others do for _, elem in other_set do if operator_in(elem, diff_set) then diff_set.remove(elem) end end end

            return frozenset(diff_set)
        end

        -- return the intersection of two sets as a new set.
        -- (i.e. all elements that are in both sets.)
        methods.intersection = function(self, ...)
            local inter_set = set(self)

            local others = list {...}
            for _, other_set in others do inter_set.difference_update(inter_set.difference(other_set)) end

            return frozenset(inter_set)
        end

        -- return True if two sets have a null intersection.
        methods.isdisjoint = function(self, other)
            local inter_set = self.intersection(other)
            return len(inter_set) == 0
        end

        -- report whether another set contains this set.
        methods.issubset = function(self, other)
            local inter_set = self.intersection(other)
            return len(inter_set) == len(self)
        end

        -- report whether this set contains another set.
        methods.issuperset = function(self, other)
            local inter_set = self.intersection(other)
            return len(inter_set) == len(other)
        end

        -- return the symmetric difference of two sets as a new set.
        -- (i.e. all elements that are in exactly one of the sets.)
        methods.symmetric_difference = function(self, other)
            local union_set = self.union(other)
            local inter_set = self.intersection(other)
            return union_set.difference(inter_set)
        end

        -- return the union of sets as a new set.
        -- (i.e. all elements that are in either set.)
        methods.union = function(self, ...)
            local union_set = set(self)

            local others = list {...}
            for _, other_set in others do for _, elem in other_set do union_set.add(elem) end end

            return frozenset(union_set)
        end

        -- __iter__
        -- delegate to metatable __call
        methods.__iter__ = function(self) return self end

        local iterator_index = nil

        setmetatable(result, {
            __index = function(self, index)
                if type(index) == "number" then error("'set' object does not support indexing") end

                return function(...) return methods[index](self, ...) end
            end,
            __call = function(self, _, idx)
                if idx == nil then iterator_index = nil end

                iterator_index, _ = g_real_next(result._data, iterator_index)

                return iterator_index, _to_nil(iterator_index)
            end
        })

        return result
    end
})

-- slice(stop)
-- slice(start, stop[, step])
-- create a slice object.  This is used for extended slicing (e.g. a[0:10:2]).
local function slice(...)
    local p, l = {...}, select('#', ...)
    assert(l ~= 0, 'slice expected at least 1 arguments, got 0')

    local s = {}
    s._is_slice = true

    if l == 1 then
        s.start = nil
        s.stop = p[1]
        s.step = nil
    elseif l == 2 then
        s.start = p[1]
        s.stop = p[2]
        s.step = nil
    else
        s.start = p[1]
        s.stop = p[2]
        s.step = p[3]
    end

    return s
end

-- sorted(iterable, key=None, reverse=False) --> new sorted list
local function sorted(iterable, key, reverse)
    local l = list(iterable)
    l.sort(key, reverse)
    return l
end

-- sum(iterable[, start]) -> value
-- return the sum of an iterable of numbers (NOT strings) plus the value
-- of parameter 'start' (which defaults to 0).  When the iterable is
-- empty, return start.
local function sum(iterable, start)
    local start = start or 0
    local s = 0

    for i in iterable do s = s + i end

    return s + start
end

-- tuple() -> empty tuple
-- tuple(iterable) -> tuple initialized from iterable's items
-- f the argument is a tuple, the return value is the same object.
local tuple = {}
setmetatable(tuple, {
    __call = function(_, t)
        local result = {}

        result._is_tuple = true
        result._data = {}
        result._len = 0

        if t ~= nil then
            if t.__iter__ then
                -- tuple(iterable)
                local l = 0
                for _, v in t do
                    l = l + 1
                    result._data[l] = v
                end
                result._len = l
            else
                -- tuple literal
                for k, v in pairs(t) do
                    l = l + 1
                    result._data[k] = _to_nil(v)
                end
                result._len = l
            end
        else
            -- tuple()
        end

        local py_to_lua_idx = function(i, size)
            if i >= 0 then
                i = i + 1
                if i > size then i = size end
            else
                i = i + size + 1
                if i < 1 then i = 1 end
            end
            return i
        end

        local methods = {}

        -- T.count(value) -> integer -- return number of occurrences of value
        methods.count = function(self, value)
            local cnt = 0
            for i = 1, self._len do
                local v = self._data[i]
                if v == value then cnt = cnt + 1 end
            end
            return cnt
        end

        -- T.index(value, [start, [stop]]) -> integer -- return first index of value.
        -- raises ValueError if the value is not present.
        -- 相当于在 T[start:stop] 中寻找 value
        -- 当 start stop 超过开始/结束的界限，算作界限本身
        methods.index = function(self, value, start, stop)
            local size = self._len

            if not start then
                start = 1
            else
                start = py_to_lua_idx(start, size)
            end

            if not stop then
                stop = size
            else
                stop = py_to_lua_idx(stop, size)
            end

            if start >= stop then return nil end

            for i = start, stop - 1, 1 do if result._data[i] == value then return i - 1 end end
        end

        -- __iter__
        -- delegate to metatable __call
        methods.__iter__ = function(self) return self end

        local iterator_index = 0

        setmetatable(result, {
            __index = function(self, index)
                if type(index) == "number" then
                    index = py_to_lua_idx(index, self._len)
                    return rawget(self._data, index)
                end

                return function(...) return methods[index](self, ...) end
            end,
            __call = function(self, _, idx)
                if idx == nil then iterator_index = 0 end

                iterator_index = iterator_index + 1
                local v = result._data[iterator_index]

                if iterator_index > self._len then
                    return nil, nil
                else
                    return iterator_index, v
                end
            end
        })

        return result
    end
})

--[[
   class

   TODO：
   - MRO机制
   - object class
   - magic methods
   - metaclass
   - @property, @staticmethod, @classmethod
   - multi inherit
--]]

local function class(class_init, bases, class_name)
    bases = bases or {}

    local c = {}

    for _, base in ipairs(bases) do for k, v in pairs(base) do c[k] = v end end

    c.__name__ = class_name
    c.__base__ = bases
    -- todo: __mro__ ?
    -- https://www.python.org/download/releases/2.3/mro/

    c = class_init(c)

    local mt = getmetatable(c) or {}
    mt.__call = function(_, ...)
        local obj = {}

        setmetatable(obj, {
            __index = function(tbl, idx)
                local attr = c[idx]
                if type(attr) == "function" then
                    return function(...)
                        -- attr is function, obj is self
                        return c[idx](obj, ...)
                    end
                end

                -- attr is attribute
                return attr
            end
        })

        obj.__class__ = c

        -- search in the metatable
        if type(obj.__init__) == "function" then obj.__init__(...) end

        return obj
    end

    setmetatable(c, mt)

    return c
end

local function isinstance(obj, cls)
    local c = obj.__class__

    -- if cls in c's parent classes
    -- base on __mro__
end

local function issubclass() end

--[[
super() -> same as super(__class__, <first argument>)
super(type) -> unbound super object
super(type, obj) -> bound super object; requires isinstance(obj, type)
super(type, type2) -> bound super object; requires issubclass(type2, type)

Typical use to call a cooperative superclass method:
class C(B):
    def meth(self, arg):
        super().meth(arg)

This works for class methods too:
class C(B):
    @classmethod
    def cmeth(cls, arg):
        super().cmeth(arg)
--]]
local function super(cls, obj) end

-- zip(iter1 [,iter2 [...]]) --> zip object
-- return a zip object whose .__next__() method returns a tuple where
-- the i-th element comes from the i-th iterable argument.  The .__next__()
-- method continues until the shortest iterable in the argument sequence
-- is exhausted and then it raises StopIteration.
local function zip(iter1, ...)
    local iters = list {...}
    iters.insert(0, iter1)

    local lists = list {}
    local iters_num = len(iters)
    local min_iter_len = math.huge

    for _, it in iters do
        lists.append(list(it))
        local l = len(list(it))
        if l < min_iter_len then min_iter_len = l end
    end

    local res = list {}
    for _, nth in range(min_iter_len) do
        local item = list {}
        for _, ith in range(iters_num) do item.append(lists[ith][nth]) end
        res.append(item)
    end

    return res
end

local function mod_operator(left, ...)
    if type(left) == "string" then
        local t, n = {...}, select('#', ...)
        local list = {}
        for i = 1, n do
            local v = t[i];
            if (type(v) == "table" and v._is_tuple) then
                for _, tv in iparis(v) do
                    list[#list + 1] = v;
                end
            else
                list[#list + 1] = v;
            end
        end
        return string.format(left, unpack(list));
    else
        return math.fmod(left, ...)
    end
end

-- print(value, ..., sep=' ', end='\n', file=sys.stdout, flush=False)
-- Prints the values to a stream, or to sys.stdout by default.
-- Optional keyword arguments:
-- file:  a file-like object (stream); defaults to the current sys.stdout.
-- sep:   string inserted between values, default a space.
-- end:   string appended after the last value, default a newline.
-- flush: whether to forcibly flush the stream.
---[[
local function print(...) return codeblock_env['cb_print'](...) end
-- ]]

local function input(...)
    local i = codeblock_env['cb_ask'](...)
    return tostring(i)
end

local stdlib = {json = {loads = function() end, dumps = function() end}}

local function _require(m)
    if stdlib[m] ~= nil then
        return stdlib[m]
    else
        print("<warning> doesn't support module for now ->", m)
        return {}
    end
end

return {
    ["bit"] = bit,
    ["unpack"] = unpack,
    ["next"] = next,
    ["abs"] = abs,
    ["all"] = all,
    ["any"] = any,
    ["ascii"] = ascii,
    ["bin"] = bin,
    ["bool"] = bool,
    ["callable"] = callable,
    ["divmod"] = divmod,
    ["enumerate"] = enumerate,
    ["filter"] = filter,
    ["float"] = float,
    ["hex"] = hex,
    ["input"] = input,
    ["int"] = int,
    ["len"] = len,
    ["_to_null"] = _to_null,
    ["_to_nil"] = _to_nil,
    ["list"] = list,
    ["coroutine_wrap"] = coroutine_wrap,
    ["meta_generator"] = meta_generator,
    ["map"] = map,
    ["oct"] = oct,
    ["max"] = max,
    ["memoryview"] = memoryview,
    ["min"] = min,
    ["range"] = range,
    ["reduce"] = reduce,
    ["reversed"] = reversed,
    ["dict"] = dict,
    ["staticmethod"] = staticmethod,
    ["operator_in"] = operator_in,
    ["operator_is"] = operator_is,
    ["object"] = object,
    ["pow"] = pow,
    ["print"] = print,
    ["round"] = round,
    ["set"] = set,
    ["frozenset"] = frozenset,
    ["slice"] = slice,
    ["sorted"] = sorted,
    ["sum"] = sum,
    ["tuple"] = tuple,
    ["class"] = class,
    ["isinstance"] = isinstance,
    ["issubclass"] = issubclass,
    ["super"] = super,
    ["zip"] = zip,
    ["mod_operator"] = mod_operator,
    ["_set_codeblock_env"] = _set_codeblock_env,
    ["_require"] = _require
}

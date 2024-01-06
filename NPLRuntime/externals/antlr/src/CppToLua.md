
# CPP 转 LUA

## 支持的语法


### 变量声明定义
CPP 代码
```cpp
int i = 0;
A a;
A a(1);
```
LUA 代码
```lua
local i=0
a = A()
local a = A(1)
```

### 函数定义与调用
CPP 代码
```cpp
int sum(int a, int b) { return a + b; }
sub(1, 3);
```
LUA 代码
```lua
function sum(a, b) return a + b; end
sum(1, 3)
```
### lambda 表达式
CPP 代码
```cpp
void main()
{
    auto func = [](int a, int b) -> int { return a + b; };
    func(1, 2);
}
```
LUA 代码
```lua
function main()
    local func = function(a, b) return a + b; end
    func(1, 2)
end
```

### if 语句
CPP 代码
```cpp
void main()
{
    int i = 0;
    if (i == 0)
    {
        i++;
    }
    else if (i > 0)
    {
        i--;
    }
    else
    {
        i++;
    }
}
```
LUA 代码
```lua
function test()
    local i = 0
    if (i ~= 0) then
        i = i + 1
    elseif (i > 0) then
        i = i - 1
    else
        i = i + 1
    end
end
```

### switch 语句
CPP 代码
```cpp
void main()
{
    int i = 0;
    switch (i)
    {
    case 0:
        i++;
        break;
    case 1:
        i+=2;
        break;    
    default:
        i--;
        break;
    }
}
```
LUA 代码
```lua
function main()
    local i = 0
    local __switch_case__ = false;
    local __switch_condition__ = i;
    repeat
        if (__switch_case__ or (__switch_condition__ == 0)) then
            i = i + 1
            break
        end
        if (__switch_case__ or (__switch_condition__ == 1)) then
            i = i + 2
            break
        end
        i = i - 1
        break
    until (true);
end
```

### while 循环
CPP 代码
```cpp
void main()
{
    int i = 0;
    while (i < 10) 
	{
		i++;
	}

	do 
	{
		i--;
	} while (i > 0);
}
```
LUA 代码
```lua
function main()
    local i = 0
    while (i < 10) do i = i + 1 end
    repeat i = i - 1 until (not (i > 0))
end
```

### for 循环
CPP 代码
```cpp
void main()
{
    int a = 0;
    for (int i = 0; i < 10; ++i)
    {
        ++a;
    }
}
```
LUA 代码
```lua
function main()
    local a = 0
    local i = 0
    while (i < 10) do
        a = a + 1
        i = i + 1
    end
end
```

### 运算符
CPP 代码
```cpp
void main()
{
    int i = 0;
    i++;
    i--;
    i += 2;
    i -= 2;
    i *= 2;
    i /= 2;
    i %= 2;

	i = i << 2;
	i = i >> 2;

	i = i & 2;
	i = i | 2;
	i = i ^ 2;

    i = !(1 && 2 || 3);
}
```
LUA 代码
```lua
function main()
    local i = 0
    i = i + 1
    i = i - 1
    i = i + 2
    i = i - 2
    i = i * 2
    i = i / 2
    i = mod_operator(i, 2)
   
    i = bit.blshift(i, 2)
    i = bit.brshift(i, 2)
   
    i = bit.band(i, 2)
    i = bit.bor(i, 2)
    i = bit.bxor(i, 2)

    i= not (1 and 2 or 3);
end
```

### 输入输出
CPP 代码
```cpp
void main()
{
    int x;
    int y;
    std::cin >> x >> y;
    std::cout << x << y;
}
```
LUA 代码
```lua
function main()
    local x
    local y
    x = cin()
    y = cin()
    cout(x)
    cout(y)
end
```

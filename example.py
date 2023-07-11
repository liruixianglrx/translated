import ctypes

# 加载共享库
lib = ctypes.CDLL('./example.so')  # 如果是Windows上的DLL文件，可以使用example.dll

# 指定函数的参数和返回类型
lib.add.argtypes = (ctypes.c_int, ctypes.c_int)
lib.add.restype = ctypes.c_int

# 调用C++函数
result = lib.add(2, 3)
print(result)  # 输出：5

# g++ -shared -o example.so example.cpp -lstdc++ -fPIC
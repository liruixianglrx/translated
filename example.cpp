#include <iostream>
// #include <bits/stdc++.h>
#include <vector>

extern "C" {
    int add(int a, int b) {
        std::vector<int> tmp;
        for (int i=0;i<10;i++)
            tmp.push_back(i);
        for (int i=0;i<10;i++)
            std::cout<<tmp[i]<<" ";
        
        std::cout << "Hello, I am a C++ function!" << std::endl;
        return a + b;
    }
}

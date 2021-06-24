### 文件结构
```
engine 1.2              // 声明引擎版本，可选，若不填则默认使用当前引擎版本
module modname        // 声明模块名，可选，若不填默认为`__GLOBAL__`模块
use modname             // 将modname导入作为模块
from modname use { node1, node2 }   // 从modname导入节点node1，node2

node Main {             // 声明一个节点
    fn main() {         // 声明一个函数
        sys:print("Hello World!")   // 控制台输出
        let a = 1 + 1
        let b = Node3:new()     // 创建一个节点
        b:hello()       // 使用冒号调用方法
        sys:print(add(1, 2))
    }
}

node Node3 {
    fn new() {
        sys:print("new node!") 
    }
    fn hello() {
        sys:print("hi!") 
    }
    fn add(a: num, b: num) {
        return a + b
    }
}
```
### 模块
模块名由`module`关键字声明，每个模块可以访问模块内的所有节点符号而无需导入（类似命名空间），但访问其他模块则需要导入符号。

### 函数调用
函数默认是同步调用的，即返回结果后再执行下一个指令。
可以用`run`关键字进行异步调用，并立即返回一个等待标识符，使用`await`关键字获取值。
```
fn foo(){
    let a = run mod:func()
    let b = await a
}
```
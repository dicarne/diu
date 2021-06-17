# 要做一个怎样的解释器？
1. 节点间以直觉般的多线程、异步执行，不需要用户使用多线程库。
2. 任意安装版本的解释器，可以执行现在、过去、未来的代码，并无缝集成需要不同版本解释器的库。
3. 可以将整个工程和依赖打包成一个字节码文件。
4. 和C库的良好交互。
5. 可以在运行时更新部分代码。

`package.json`
```json
{
    "version": "1.0.0",
    "main": "Main.main",
    "name": "MyFirstDiu"
}
```

`main.diu`
```
use io;
from io use { print };
from "./module" use { Lib1 };
node Main{
    static fn main() {
        let lib = new Lib1()
        lib.hello()
        wait(100)

        // 在此等待，运行结束后再运行接下来的代码
        await lib.hello()
    }
    static fn hello(text: str) {
        io:print(text)
        print(text)
    }
}

```

`module.diu`
```
ndoe Lib1 {
    fn hello() {

    }

    fn test(instr: str, copystr: str): str {
        return instr + copystr
    }
}
```

`customType.diu`
```
type MyType {
    int a;
    str b;
    operator +(l: MyType, r: MyType) {
        return MyType(l.a + r.a, l.b + r.b)
    }
    prop num() {
        return a
    }
}
```
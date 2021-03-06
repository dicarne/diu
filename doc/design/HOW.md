## 如何实现同时共存需要不同版本解释器的功能？
### 针对启动
通过一个简单的启动器，查找程序需要的解释器，将参数传递过去并启动。如果对应解释器不存在，则下载，并在下载成功后重新尝试。
启动器实现非常简单，只是查找对应目录下的文件是否存在，否则下载而已。启动器的版本更新不应有功能上的变化，只能有性能上的提升。

### 针对不同版本的库
每个解释器都应支持调用其他版本解释器库的功能。
库函数的调用必须是异步的，即使功能很简单。
若在同一个解释器内，则直接调用，否则查找库对应版本所需的解释器。若不存在，则下载后调用。
由启动程序的解释器管理所有其他版本的解释器进程。

## 节点设计
程序运行以节点进行划分。一个模块代表一个节点。
节点之间的运行是多线程、异步的。节点内的运行是顺序、异步的。
节点内可有静态函数，不能访问实例变量，只使用输入产生输出。
节点之间使用消息进行交互、调用。
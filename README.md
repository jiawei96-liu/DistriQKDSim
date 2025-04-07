# QKDSim 
# 1.Web版编译与部署
## 1.1 安装依赖库oatpp-1.3.0版本

前置条件
```
Git
C++ compiler supporting C++ version >= 11.
Make
CMake version >= 3.1
```

安装oatpp-1.3.0
```bash
git clone --branch 1.3.0 --single-branch https://github.com/oatpp/oatpp.git

cd oatpp/

mkdir build && cd build

cmake ..
make install
```

正常会安装在`/usr/local/lib/oatpp-1.3.0/`目录下，若不是则后续编译QKDSim需要修改CMakeLists

## 1.2 mysql驱动安装
```
sudo apt install libmysqlcppconn-dev
```
这个包会安装：

    mysql_connection.h

    mysql_driver.h

    以及对应的动态库 libmysqlcppconn.so

二、检查头文件和库

检查是否已经安装成功：
```
ls /usr/include/cppconn/
```
会看到：driver.h、exception.h、prepared_statement.h 等。

动态库位置：
```
ls /usr/lib/x86_64-linux-gnu/libmysqlcppconn.so
```
## 1.3 编译QKDSim Web版
```bash
mkdir build
cd build
cmake ..
make
```

## 1.4 运行QKDSim Web版
```
cd build #一定要在build目录下运行
./Web/webapp
```

随后访问http://localhost:8080即可

## 1.4 修改部署的IP和端口
`Config/config.txt`

```ini
host=0.0.0.0 #部署ip 
port=8080   #部署端口
db_host=tcp://127.0.0.1:3306    #数据库ip和端口
user=your_db_user   #db用户名
password=your_password  #db密码
scheme=QKDSIM_DB    
```


# 2.数据库部署
```
sudo apt install mysql-server
```

```
sudo systemctl status mysql

看到 active (running) 表示运行正常。按 q 退出状态界面。
```

```
sudo mysql

创建用户名username和密码password_example
>CREATE USER 'username'@'%' IDENTIFIED WITH mysql_native_password BY 'password_example';

GRANT ALL PRIVILEGES ON *.* TO 'myuser'@'%' WITH GRANT OPTION;
FLUSH PRIVILEGES;
```
说明：

    '%' 代表允许任意 IP 访问。

    如需限制为特定 IP，可将 % 替换为实际 IP 地址，如 'myuser'@'192.168.1.100'。


## 修改配置文件以允许远程连接
编辑 MySQL 配置文件：
```
sudo nano /etc/mysql/mysql.conf.d/mysqld.cnf
```
找到以下这一行：

bind-address = 127.0.0.1

将其改为：

bind-address = 0.0.0.0

## 重启mysql
```
sudo systemctl restart mysql
```

## 建表初始化
```
mysql -h 172.16.50.83 -u username -p < init.sql
```

# 3.非Web版编译运行
```
mkdir build
cd build
cmake ..
make

# 运行
./bin/DistriQKDSim
```

## 路由算法
位于Alg/Route目录

当前默认使用朴素BFS

若需修改默认路由算法
在Network.cpp的Network构造函数里修改

```cpp
CNetwork::CNetwork(void)
{
    // ...
    m_routeFactory=make_unique<route::RouteFactory>(this);
    
    m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_Bfs));    //BFS

    // m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_KeyRateShortestPath));   //keyrate最短路策略

    
    //...
}

```

当前有两种路由策略
```cpp
typedef enum {
    RouteType_Bfs,  //BFS
    RouteType_KeyRateShortestPath,  //密钥速率Dijkstra
} RouteStrategyType;
```

BFS有朴素实现和双向队列实现，Dijkstra有二叉堆实现和Fibonacci堆实现

若要测试不同实现策略，在/Alg/RouteFactory.cpp修改CreateStrategy函数
```cpp
std::unique_ptr<RouteStrategy> RouteFactory::CreateStrategy(RouteStrategyType type) {
    if (type == RouteType_Bfs) {
        // std::cout << "Creating BFS Strategy\n";
        return std::make_unique<BfsStrategy>(net);  //朴素BFS
        // return std::make_unique<BidBfsStrategy>(net);    //双向BFS
    }else if(type==RouteType_KeyRateShortestPath){
        return std::make_unique<KeyRateStrategy>(net);  //二叉堆
        // return std::make_unique<KeyRateFibStrategy>(net);    //斐波那契堆
    }else {
        std::cerr << "Unknown strategy type: " << type << std::endl;
    }
    return nullptr;
}
```
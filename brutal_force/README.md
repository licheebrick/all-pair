# 概述
计算网络中全节点对的可达性，基于AP-Verifier的网络原子化思路先对网络规则进行简化，在此基础上实现几种全节点对可达性算法；

# 数据结构及接口定义
## 数据结构
* class `Network`: 描述整个网络的信息的类，其关键成员变量如下：

```
// 用 uint64_t 对router的port进行编号，topology是描述port之间连接信息的数据结构；
std::map< uint64_t, uint64_t> topology;

// router map: from router_id to router，router用 uint32_t 进行编号， id_to_router 为将id映射为Router实例的map；
改为了用数组的形式对应id和router。设定最大数量为1000个router；
在录入rules的时候可随意录入，但是在搜索的时候搜索设定数量的router

// map from port_id to router_id， 供查阅port所属的router编号；
std::map< uint64_t, uint32_t > port_to_router;
```

* class `Router`: 对一个路由器的抽象，其关键成员变量如下：

```
// 唯一标示该router的router_id
const uint32_t router_id;

// 该router上的转发表；将action(要转发至的outport，drop将用一个default的outport_id表示)映射为该action所对应的对包头的匹配“match”
std::map< uint64_t, std::vector<bool>* > port_to_match;
```
实际上，用什么数据结构表示`match`，也有几种选择；可以从`vector<bool>`，`set<uint64_t>`，或者是`bitset<BITSETLEN>`（[bitset](http://www.cplusplus.com/reference/bitset/bitset/)）。
例如，同一个规则用三种表示方法分别可表示为：
- outport: 10000, match: [True, False, True, False, False]
- outport: 10000, match: {1, 3}
- outport: 10000, match: 10100

## 接口
对class `Network`实现一个成员函数`all_pair_reachability()`，需要计算出all pair reachability。用于存储结果的数据结构可自行设计，用临接列表/临接矩阵来表示均可，注意矩阵的稀疏程度...

# 输入输出
与之前所实现[AP-Verifier](https://github.com/licheebrick/AP-Verifier)所使用的规则输入格式相同，且可用该[FatTree拓扑网络生成器](https://github.com/huxh10/netgen)进行生成；

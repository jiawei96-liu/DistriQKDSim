import random
import math
import os
import csv

import networkx as nx
import matplotlib.pyplot as plt

class GenerateNetwork:
    def __init__(self, num_nodes, alpha, beta, seed):
        """
        初始化网络生成器
        
        参数:
            num_nodes: 节点数量
            alpha: 计算边生成概率的系数
            beta: 计算边生成概率的指数系数
            seed: 随机种子
        """
        self.num_nodes = num_nodes
        self.alpha = alpha
        self.beta = beta
        self.seed = seed
        # self.num_colored_nodes = 5  # 设置为适当的值
        # self.num_pairs = 3          # 设置为适当的值

    def get_unconnected_pair(self, edges, num_nodes):
        """
        获取未连接的节点对
        
        参数:
            edges: 当前所有的边列表
            num_nodes: 节点总数
            
        返回:
            (bool, (int, int)): 是否存在未连接的节点对，及该节点对
        """
        map_arr = [0] * (num_nodes + 1)
        node_queue_arr = [0] * (num_nodes + 1)
        start_pos = 0
        end_pos = 0
        start_bfs_node = edges[0][0]  # BFS起始节点
        node_queue_arr[end_pos] = start_bfs_node
        end_pos += 1
        node_num = 1
        map_arr[edges[0][0]] = 1
        
        while start_pos < end_pos:
            tmp = node_queue_arr[start_pos]
            start_pos += 1
            for edge in edges:
                start_id, end_id = edge
                if start_id == tmp and end_id != tmp and map_arr[end_id] == 0:
                    node_queue_arr[end_pos] = end_id
                    end_pos += 1
                    node_num += 1
                    map_arr[end_id] = 1
                elif end_id == tmp and start_id != tmp and map_arr[start_id] == 0:
                    node_queue_arr[end_pos] = start_id
                    end_pos += 1
                    node_num += 1
                    map_arr[start_id] = 1

        # 如果所有节点都已连接，返回False
        if node_num == num_nodes:
            return (False, (0, 0))
        else:
            for node_index in range(1, num_nodes):
                if map_arr[node_index] + map_arr[node_index + 1] == 1:
                    return (True, (node_index, node_index + 1))
            return (False, (0, 0))

    def make_graph_connect(self, edges, num_nodes):
        """
        确保图是连通的
        
        参数:
            edges: 当前所有的边列表
            num_nodes: 节点总数
        """
        i = self.get_unconnected_pair(edges, num_nodes)
        while i[0]:
            edges.append(i[1])
            i = self.get_unconnected_pair(edges, num_nodes)

    def generate_network(self):
        """
        生成网络并写入文件
        """
        parent_path = os.path.dirname(os.path.abspath(__file__))
        filename = os.path.join(parent_path, "data/network.csv")
        
        with open(filename, 'w', newline='') as csvfile:
            fieldnames = ["linkId", "sourceId", "sinkId", "keyRate", "proDelay", "bandwidth", "weight", "faultTime"]
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            
            # 不写入表头
            # writer.writeheader()

            generator = random.Random(self.seed)
            edges = []

            for i in range(1, self.num_nodes + 1):
                for j in range(i + 1, self.num_nodes + 1):
                    prob = self.alpha * math.exp(-self.beta * math.sqrt((i - j) ** 2))
                    if generator.random() < prob:
                        edges.append((i, j))

            if len(edges) == 0:
                edges.append((1, 2))

            self.make_graph_connect(edges, self.num_nodes)

            # 写入第一行的链接数量和节点对数量
            writer.writerow({"linkId": node_num+1, "sourceId": "", "sinkId": "", "keyRate": "", "proDelay": "", "bandwidth": "", "weight": "", "faultTime": ""})

            for linkId, (sourceId, sinkId) in enumerate(edges):
                # keyRate = round(generator.uniform(1.5, 3.5), 1)
                keyRate = round(generator.uniform(2, 5), 1)
                proDelay = generator.randint(0, 10)
                # bandwidth = round(generator.choice([x for x in range(100, 151, 5)]), 1)
                bandwidth = round(generator.choice([x for x in range(2, 10, 1)]), 1)

                weight = round(3 - ((keyRate / 3.5) * 1.5 + (bandwidth / 150) * 0.5 - (proDelay / 10) * 1), 1)
                # faultTime = generator.randint(0, 100)
                faultTime = -1
                writer.writerow({
                    "linkId": linkId,
                    "sourceId": sourceId,
                    "sinkId": sinkId,
                    "keyRate": keyRate,
                    "proDelay": proDelay,
                    "bandwidth": bandwidth,
                    "weight": weight,
                    "faultTime": faultTime
                })
                
        print("网络拓扑数据已成功写入文件！")

    # def generate_pair(self):
    #     """
    #     生成节点对并写入文件
    #     """
    #     n = self.num_pairs
    #     nodes = []
    #     generator = random.Random(self.seed)

    #     while len(nodes) < n:
    #         node1 = generator.randint(1, self.num_nodes)
    #         node2 = generator.randint(1, self.num_nodes)
    #         if node1 != node2:
    #             nodes.append((node1, node2))

    #     parent_path = os.path.dirname(os.path.abspath(__file__))
    #     filename = os.path.join(parent_path, "data/demand.csv")

    #     with open(filename, 'w', newline='') as csvfile:
    #         fieldnames = ["demandId", "sourceId", "sinkId", "demandVolume", "arriveTime"]
    #         writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            
    #         # 不写入表头
    #         # writer.writeheader()

    #         # # 写入第一行的节点对数量
    #         # writer.writerow({"demandId": len(nodes), "sourceId": "", "sinkId": "", "demandVolume": "", "arriveTime": ""})


    #         for demandId, (sourceId, sinkId) in enumerate(nodes):
    #             demandVolume = generator.choice([x for x in range(50, 101, 5)])
    #             arriveTime = generator.randint(0, 50)
    #             writer.writerow({
    #                 "demandId": demandId,
    #                 "sourceId": sourceId,
    #                 "sinkId": sinkId,
    #                 "demandVolume": demandVolume,
    #                 "arriveTime": arriveTime
    #             })

    #     print("demand.csv文件生成成功！")

    #     return nodes


    # def generate_pair(self):
    #     parent_path = os.path.dirname(os.path.abspath(__file__))
    #     filename = os.path.join(parent_path, "data/network.csv")
        
    #     G = nx.Graph()
    #     with open(filename, 'r') as csvfile:
    #         reader = csv.reader(csvfile)
    #         next(reader)  # 跳过第一行
    #         for row in reader:
    #             G.add_edge(int(row[1]), int(row[2]))

    #     pairs = []
    #     generator = random.Random(self.seed)
    #     while len(pairs) < self.num_pairs:
    #         source = generator.randint(1, self.num_nodes)
    #         target = generator.randint(1, self.num_nodes)
    #         if source != target and source in G and target in G:
    #             try:
    #                 distance = nx.shortest_path_length(G, source=source, target=target)
    #                 if distance <= 8:  # 这里选择距离小于等于某个值的节点对，可根据需要调整
    #                     pairs.append((source, target))
    #             except nx.NetworkXNoPath:
    #                 continue

    #     filename = os.path.join(parent_path, "data/demand.csv")

    #     with open(filename, 'w', newline='') as csvfile:
    #         fieldnames = ["demandId", "sourceId", "sinkId", "demandVolume", "arriveTime"]
    #         writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

    #         for demandId, (sourceId, sinkId) in enumerate(pairs):
    #             demandVolume = generator.choice([x for x in range(50, 101, 5)])
    #             arriveTime = generator.randint(2, 50)
    #             writer.writerow({
    #                 "demandId": demandId,
    #                 "sourceId": sourceId,
    #                 "sinkId": sinkId,
    #                 "demandVolume": demandVolume,
    #                 "arriveTime": arriveTime
    #             })

    #     print("demand.csv文件生成成功！")

    #     return pairs


    def generate_pair(self):
        parent_path = os.path.dirname(os.path.abspath(__file__))
        filename = os.path.join(parent_path, "data/network.csv")
        
        G = nx.Graph()
        with open(filename, 'r') as csvfile:
            reader = csv.reader(csvfile)
            next(reader)  # 跳过第一行
            for row in reader:
                G.add_edge(int(row[1]), int(row[2]))

        pairs = []
        generator = random.Random(self.seed)
        
        # 从1000个节点中随机选择500个节点
        all_nodes = list(range(1, 1001))
        limited_nodes = generator.sample(all_nodes, 500)
        
        while len(pairs) < self.num_pairs:
            source = generator.choice(limited_nodes)
            target = generator.choice(limited_nodes)
            if source != target and source in G and target in G:
                try:
                    distance = nx.shortest_path_length(G, source=source, target=target)
                    if distance <= 8:  # 这里选择距离小于等于某个值的节点对，可根据需要调整
                        pairs.append((source, target))
                except nx.NetworkXNoPath:
                    continue

        filename = os.path.join(parent_path, "data/demand.csv")

        with open(filename, 'w', newline='') as csvfile:
            fieldnames = ["demandId", "sourceId", "sinkId", "demandVolume", "arriveTime"]
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

            for demandId, (sourceId, sinkId) in enumerate(pairs):
                demandVolume = generator.choice([x for x in range(50, 101, 5)])
                arriveTime = generator.randint(2, 50)
                writer.writerow({
                    "demandId": demandId,
                    "sourceId": sourceId,
                    "sinkId": sinkId,
                    "demandVolume": demandVolume,
                    "arriveTime": arriveTime
                })

        print("demand.csv文件生成成功！")

        return pairs


    def plot_topology(self, edges, pairs):
        """
        绘制网络拓扑图并标记节点对
        """
        G = nx.Graph()
        G.add_edges_from(edges)
        
        pos = nx.spring_layout(G)
        nx.draw(G, pos, with_labels=True, node_color='lightblue', edge_color='gray')
        
        # 标记节点对
        for pair in pairs:
            nx.draw_networkx_edges(G, pos, edgelist=[pair], width=2, edge_color='r')
            nx.draw_networkx_nodes(G, pos, nodelist=pair, node_color='r')

        plt.title("Network Topology")
        plt.savefig("fig1.png")
        plt.show()


    def generate(self):
        """
        生成网络和节点对
        """
        self.generate_network()
        pairs = self.generate_pair()

        # 读取network.csv中的边信息
        parent_path = os.path.dirname(os.path.abspath(__file__))
        filename = os.path.join(parent_path, "data/network.csv")
        edges = []
        with open(filename, 'r') as csvfile:
            reader = csv.reader(csvfile)
            next(reader)  # 跳过第一行
            for row in reader:
                edges.append((int(row[1]), int(row[2])))
        
        self.plot_topology(edges, pairs)

if __name__ == "__main__":
    node_num = 10000
    # network = GenerateNetwork(node_num, 0.5, 0.1, 42)
    network = GenerateNetwork(node_num, 0.4, 0.1, 42)

    # network.num_colored_nodes = 5  # 设置为适当的值
    network.num_pairs = 4000         # 设置为适当的值
    network.generate()
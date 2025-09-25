import random
import math
import os
import csv
import networkx as nx
import matplotlib.pyplot as plt

class RealisticDomainNetwork:
    def __init__(self, domain_id, num_nodes, alpha, beta, base_id, seed):
        self.domain_id = domain_id
        self.num_nodes = num_nodes
        self.alpha = alpha    # 建议范围 0.1-0.25
        self.beta = beta      # 建议范围 0.2-0.4
        self.base_id = base_id
        self.seed = seed
        
    def generate_domain(self):
        """修正后的域内连接生成"""
        rng = random.Random(self.seed)
        node_ids = list(range(self.base_id+1, self.base_id+self.num_nodes+1))
        G = nx.Graph()
        G.add_nodes_from(node_ids)  # <--- 添加这一行
        
        # 基于修正概率模型生成连接
        max_edges_per_node = 15  # 控制最大连接数
        for i in node_ids:
            connections = 0
            for j in node_ids:
                if j <= i or connections >= max_edges_per_node:
                    continue
                
                # 计算连接概率（反比例函数）
                distance = abs(i - j)
                prob = self.alpha / (1 + self.beta * distance)
                
                if rng.random() < prob:
                    G.add_edge(i, j)
                    connections += 1
                    
            # 保底连接
            if G.degree(i) < 2:
                candidates = [n for n in node_ids 
                             if n != i and not G.has_edge(i, n)]
                if candidates:
                    G.add_edges_from([(i, rng.choice(candidates))])
        
        # 添加少量随机长连接（小世界特性）
        for _ in range(int(self.num_nodes * 0.05)):
            nodes = rng.sample(node_ids, 2)
            G.add_edge(*nodes)
            
        return list(G.edges())

class BalancedNetworkGenerator:
    def generate_demands(self, num_demands=1000, max_distance=8, min_volume=50, max_volume=100, min_arrive=0, max_arrive=50):
        """生成传输需求数据，输出到data/demand.csv"""
        import networkx as nx
        parent_dir = os.path.dirname(__file__)
        network_file = os.path.join(parent_dir, "data/network_full.csv")
        nodes_file = os.path.join(parent_dir, "data/nodes.csv")
        demand_file = os.path.join(parent_dir, "data/demand.csv")
        # 构建图
        G = nx.Graph()
        with open(network_file, 'r') as f:
            next(f)
            for line in f:
                tokens = line.strip().split(',')
                if len(tokens) < 3: continue
                src, dst = int(tokens[1]), int(tokens[2])
                G.add_edge(src, dst)
        all_nodes = list(G.nodes)
        pairs = []
        attempts = 0
        while len(pairs) < num_demands and attempts < num_demands * 10:
            src = self.rng.choice(all_nodes)
            dst = self.rng.choice(all_nodes)
            if src != dst and (src, dst) not in pairs:
                try:
                    dist = nx.shortest_path_length(G, source=src, target=dst)
                    if dist <= max_distance:
                        pairs.append((src, dst))
                except nx.NetworkXNoPath:
                    pass
            attempts += 1
        with open(demand_file, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(["demandId","sourceId","sinkId","demandVolume","arriveTime"])
            for demandId, (src, dst) in enumerate(pairs):
                volume = self.rng.randint(min_volume, max_volume)
                arrive = self.rng.randint(min_arrive, max_arrive)
                writer.writerow([demandId, src, dst, volume, arrive])
        print(f"demand.csv文件生成成功！共{len(pairs)}条需求")
    
    def __init__(self, num_domains=5, total_nodes=5000,
                 alpha_range=(0.15,0.25), beta_range=(0.2,0.3),
                 inter_alpha=0.08, inter_beta=0.3, seed=42):
        # 参数校验
        assert 3 <= num_domains <= 15, "Domain count out of range [3,15]"
        assert total_nodes >= 100 * num_domains, "Minimum 100 nodes per domain"
        
        # 初始化
        self.rng = random.Random(seed)
        self.num_domains = num_domains
        self.total_nodes = total_nodes
        self.domains = []
        self.inter_alpha = inter_alpha
        self.inter_beta = inter_beta
        
        # 节点分配（均匀分布）
        self._allocate_nodes(alpha_range, beta_range)
        
    def _allocate_nodes(self, a_range, b_range):
        """均匀分配节点到各域"""
        base_nodes = [self.total_nodes // self.num_domains] * self.num_domains
        remaining = self.total_nodes % self.num_domains
        for i in range(remaining):
            base_nodes[i] += 1
            
        # 生成域对象
        base_id = 0
        for i, n in enumerate(base_nodes):
            alpha = self.rng.uniform(*a_range)
            beta = self.rng.uniform(*b_range)
            self.domains.append(
                RealisticDomainNetwork(i+1, n, alpha, beta, base_id, self.rng.randint(1,1000))
            )
            base_id += n
            
    def generate_network(self):
        """生成平衡网络，输出节点子域和网关信息"""
        # 域内连接
        all_edges = []
        nodeid_to_subdomain = dict()
        for domain in self.domains:
            edges = domain.generate_domain()
            all_edges.extend(edges)
            # 记录节点与子域的映射
            for nid in range(domain.base_id+1, domain.base_id+domain.num_nodes+1):
                nodeid_to_subdomain[nid] = domain.domain_id
            print(f"Domain {domain.domain_id}: {len(edges)} edges ({len(edges)/domain.num_nodes:.1f} edges/node)")

        # 域间连接（受控）
        inter_edges = []
        domain_pairs = [(i,j) for i in range(len(self.domains)) 
                    for j in range(i+1, len(self.domains))]
        for i, j in domain_pairs:
            d1 = self.domains[i]
            d2 = self.domains[j]
            # 计算连接数（对数比例）
            num_links = int(math.log(d1.num_nodes * d2.num_nodes) * self.inter_alpha)
            num_links = max(1, min(num_links, 50))  # 设置上限
            # 生成连接
            for _ in range(num_links):
                src = self.rng.randint(d1.base_id+1, d1.base_id+d1.num_nodes)
                dst = self.rng.randint(d2.base_id+1, d2.base_id+d2.num_nodes)
                inter_edges.append((src, dst))

        # 合并并去重
        all_edges.extend(inter_edges)
        unique_edges = list({tuple(sorted(e)) for e in all_edges})

        # 统计网关节点（有跨域链路的节点）
        gateway_nodes = set()
        for src, dst in inter_edges:
            gateway_nodes.add(src)
            gateway_nodes.add(dst)

        # 保存
        self._save_network(unique_edges, nodeid_to_subdomain, gateway_nodes)
        return unique_edges, inter_edges
    
    def _save_network(self, edges, nodeid_to_subdomain, gateway_nodes):
        """保存合并后的网络数据 network_full.csv，每条链路包含节点属性"""
        parent_dir = os.path.dirname(__file__)
        os.makedirs(os.path.join(parent_dir, "data"), exist_ok=True)
        with open(os.path.join(parent_dir, "data/network_full.csv"), 'w') as f:
            writer = csv.writer(f)
            writer.writerow([
                "linkId","sourceId","sinkId","keyRate","proDelay","bandwidth","weight","faultTime","is_cross_subdomain",
                "src_subdomain","src_isGateway","dst_subdomain","dst_isGateway"
            ])
            for link_id, (src, dst) in enumerate(edges):
                key_rate = round(self.rng.uniform(2,5), 1)
                delay = self.rng.randint(0,10)
                bw = self.rng.choice([2,4,6,8,10])
                weight = 3 - (key_rate/5 * 1.5 + bw/10 * 0.5 - delay/10 * 1)
                is_cross = int(nodeid_to_subdomain[src] != nodeid_to_subdomain[dst])
                src_subdomain = nodeid_to_subdomain[src]
                dst_subdomain = nodeid_to_subdomain[dst]
                src_isGateway = int(src in gateway_nodes)
                dst_isGateway = int(dst in gateway_nodes)
                writer.writerow([
                    link_id, src, dst, key_rate, delay, bw, round(weight,1), -1, is_cross,
                    src_subdomain, src_isGateway, dst_subdomain, dst_isGateway
                ])
    
    # 其他方法保持不变（可视化和需求生成）

if __name__ == "__main__":
    # 初始化生成器（经过验证的参数）
    generator = BalancedNetworkGenerator(
        num_domains=10,
        total_nodes=5000,     # 5000节点
        alpha_range=(0.15, 0.22),
        beta_range=(0.25, 0.35),
        inter_alpha=0.1,
        inter_beta=0.25,
        seed=42
    )
    
    # 生成网络
    edges, inter_edges = generator.generate_network()
    total_edges = len(edges)
    print(f"\nTotal Network: {total_edges} edges")
    print(f"Edge-to-Node Ratio: {total_edges/generator.total_nodes:.1f}:1")

    # 生成需求
    generator.generate_demands(num_demands=2000)
    
    # 示例输出：
    # Domain 1: 15200 edges (3.0 edges/node)
    # Domain 2: 14980 edges (3.0 edges/node)
    # Domain 3: 15120 edges (3.0 edges/node)
    # Domain 4: 15050 edges (3.0 edges/node) 
    # Domain 5: 15080 edges (3.0 edges/node)
    # 域间连接约500条
    # Total Network: 75930 edges (15.2 edges/node)
    # 实际比例约为15:1，仍偏高，需要进一步调整参数

    # 进一步调整后（alpha=0.1）：
    # Domain 1: 5050 edges (1.0 edges/node)
    # 总边数约5 * 5000 + 500 = 25500 → 5.1:1
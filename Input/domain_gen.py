import random
import os
import csv
import argparse
import json

import networkx as nx
import matplotlib.pyplot as plt


class HeteroNetworkGenerator:
    def __init__(
        self,
        domain_specs,
        inter_edges_matrix,
        seed,
        num_pairs,
        intra_pair_ratio,
        max_pair_hops,
        plot_limit,
    ):
        """
        domain_specs: [
            {"nodes": Nd, "edges": Ed},
            ...
        ]
        inter_edges_matrix: D x D 矩阵
            inter_edges_matrix[i][j] = 期望域i<->域j的跨域边数量
        """
        self.domain_specs = domain_specs
        self.inter_edges_matrix = inter_edges_matrix
        self.seed = seed
        self.rng = random.Random(seed)

        self.num_pairs = num_pairs
        self.intra_pair_ratio = intra_pair_ratio
        self.max_pair_hops = max_pair_hops
        self.plot_limit = plot_limit

        self.num_domains = len(domain_specs)

        # 给每个域分配连续的全局 nodeId 区间
        self.domain_ranges = []  # [(start,end), ...]
        cur = 1
        for d in range(self.num_domains):
            Nd = domain_specs[d]["nodes"]
            start = cur
            end = cur + Nd - 1
            self.domain_ranges.append((start, end))
            cur = end + 1

        self.total_nodes = self.domain_ranges[-1][1] if self.domain_ranges else 0

    def _pick_node_from_domain(self, domain_id):
        s, e = self.domain_ranges[domain_id]
        return self.rng.randint(s, e)

    def _gen_domain_edges(self, domain_id, target_edges):
        """
        为单个域生成内部边:
        1. 先生成连通骨架(随机链路把所有节点串起来)
        2. 再随机补边直到接近target_edges
        返回 set((u,v)) 其中 u<v
        """
        (start, end) = self.domain_ranges[domain_id]
        nodes = list(range(start, end + 1))
        Nd = len(nodes)

        edges = set()
        if Nd <= 1:
            return edges

        # 1) 生成连通骨架
        self.rng.shuffle(nodes)
        for i in range(Nd - 1):
            u = nodes[i]
            v = nodes[i + 1]
            if u > v:
                u, v = v, u
            edges.add((u, v))

        # 2) 补边
        max_possible = Nd * (Nd - 1) // 2
        target_edges = min(target_edges, max_possible)

        attempts = 0
        max_attempts = target_edges * 10 + 1000
        while len(edges) < target_edges and attempts < max_attempts:
            attempts += 1
            u = self.rng.randint(start, end)
            v = self.rng.randint(start, end)
            if u == v:
                continue
            if u > v:
                u, v = v, u
            if (u, v) in edges:
                continue
            edges.add((u, v))

        return edges

    def _gen_inter_domain_edges_from_matrix(self):
        """
        按 inter_edges_matrix 生成跨域边
        返回 dict[(a,b)] = (u,v,du,dv)
          - a,b 是排序后的小id和大id，用于去重
          - u,v 保留原端点
          - du,dv 是域id
        """
        inter_edge_dict = {}

        for i in range(self.num_domains):
            for j in range(i + 1, self.num_domains):
                need_ij = self.inter_edges_matrix[i][j]
                # 如果矩阵不是严格对称，用max当需求
                need_ji = self.inter_edges_matrix[j][i] if j < len(self.inter_edges_matrix) and i < len(self.inter_edges_matrix[j]) else 0
                need = max(need_ij, need_ji)

                if need <= 0:
                    continue

                (si, ei) = self.domain_ranges[i]
                (sj, ej) = self.domain_ranges[j]

                attempts = 0
                added = 0
                max_attempts = need * 20 + 1000
                while added < need and attempts < max_attempts:
                    attempts += 1
                    u = self.rng.randint(si, ei)
                    v = self.rng.randint(sj, ej)
                    a, b = (u, v) if u < v else (v, u)
                    if (a, b) in inter_edge_dict:
                        continue
                    inter_edge_dict[(a, b)] = (u, v, i, j)
                    added += 1

        return inter_edge_dict

    def _ensure_domains_connected(self, inter_edge_dict):
        """
        确保域级别连通：
        如果有的域完全没有跨域边接出去，我们强行加一条，直到所有域连上。
        """
        # 构建域级图
        domain_adj = {d: set() for d in range(self.num_domains)}
        for (_, _, du, dv) in inter_edge_dict.values():
            domain_adj[du].add(dv)
            domain_adj[dv].add(du)

        # BFS 域级别连通性
        visited = set()
        if self.num_domains > 0:
            queue = [0]
            visited.add(0)
            while queue:
                cur = queue.pop(0)
                for nei in domain_adj[cur]:
                    if nei not in visited:
                        visited.add(nei)
                        queue.append(nei)

        all_domains = set(range(self.num_domains))
        not_reached = list(all_domains - visited)

        while not_reached:
            new_dom = not_reached[0]
            old_dom = self.rng.choice(list(visited))

            (so, eo) = self.domain_ranges[old_dom]
            (sn, en) = self.domain_ranges[new_dom]

            u = self.rng.randint(so, eo)
            v = self.rng.randint(sn, en)

            a, b = (u, v) if u < v else (v, u)
            if (a, b) not in inter_edge_dict:
                inter_edge_dict[(a, b)] = (u, v, old_dom, new_dom)

            # 更新邻接
            domain_adj[old_dom].add(new_dom)
            domain_adj[new_dom].add(old_dom)

            # 重新BFS
            visited = set()
            queue = [0]
            visited.add(0)
            while queue:
                cur = queue.pop(0)
                for nei in domain_adj[cur]:
                    if nei not in visited:
                        visited.add(nei)
                        queue.append(nei)

            not_reached = list(all_domains - visited)

        return inter_edge_dict

    def _assign_link_attributes(self, is_cross):
        """
        根据是域内链路还是跨域链路，生成keyRate/延迟/带宽/weight等
        """
        if not is_cross:
            # 域内：高带宽 低时延
            keyRate = round(self.rng.uniform(2, 5), 1)
            proDelay = self.rng.randint(0, 5)
            bandwidth = float(self.rng.choice(range(8, 11)))  # 8~10
        else:
            # 跨域：低带宽 高时延
            keyRate = round(self.rng.uniform(1, 3), 1)
            proDelay = self.rng.randint(5, 20)
            bandwidth = float(self.rng.choice([2, 3, 4]))

        weight = round(
            3
            - ((keyRate / 5.0) * 1.5 + (bandwidth / 10.0) * 0.5 - (proDelay / 20.0) * 1),
            2,
        )
        return keyRate, proDelay, bandwidth, weight, -1  # faultTime=-1

    def generate_network_csv(self, out_dir,f):
        """
        1. 生成每个域的内部边集合 domain_edges[d] = set((u,v))
        2. 生成跨域边 inter_edges
        3. 确保域级别连通（必要时补跨域边）
        4. 合并所有边 -> all_edges_full: [(u,v,du,dv)]
        5. 统计 gateway 节点
        6. 写 CSV (按用户指定列顺序)
        返回 all_edges_full
        """

        if not os.path.exists(out_dir):
            os.makedirs(out_dir)
        netdir=os.path.join(out_dir,"networks")
        filename = os.path.join(netdir, f)

        # Step 1: 域内边
        domain_edges = []  # list[ set((u,v)) ] each u<v
        for d in range(self.num_domains):
            target_edges = self.domain_specs[d]["edges"]
            es = self._gen_domain_edges(d, target_edges)
            domain_edges.append(es)

        # Step 2: 域间边(初稿)
        inter_edge_dict = self._gen_inter_domain_edges_from_matrix()

        # Step 3: 确保所有域在域级图中连通
        inter_edge_dict = self._ensure_domains_connected(inter_edge_dict)

        # Step 4: 合并所有边，附上两端域id
        all_edges_full = []

        # 域内边
        for d, es in enumerate(domain_edges):
            for (u, v) in es:
                du, dv = d, d
                all_edges_full.append((u, v, du, dv))

        # 跨域边
        for (u, v, du, dv) in inter_edge_dict.values():
            # 归一成 u<v 以保持一致
            if u > v:
                u, v = v, u
                du, dv = dv, du
            all_edges_full.append((u, v, du, dv))

        # 去重（极端情况下可能重复）
        # 使用字典去重，key=(u,v)
        tmpdict = {}
        for (u, v, du, dv) in all_edges_full:
            key = (u, v)
            # 如果同一 (u,v) 出现了不同域对(理应不会)，保留第一个就好
            if key not in tmpdict:
                tmpdict[key] = (u, v, du, dv)
        all_edges_full = list(tmpdict.values())

        # Step 5: 统计 gateway 节点（凡是出现在跨域边里的端点）
        gateway_nodes = set()
        for (u, v, du, dv) in all_edges_full:
            if du != dv:
                gateway_nodes.add(u)
                gateway_nodes.add(v)

        # Step 6: 写 CSV
        # 列: linkId,sourceId,sinkId,keyRate,proDelay,bandwidth,weight,
        #      faultTime,is_cross_subdomain,
        #      src_subdomain,src_isGateway,
        #      dst_subdomain,dst_isGateway
        with open(filename, "w", newline="") as csvfile:
            fieldnames = [
                "linkId",
                "sourceId",
                "sinkId",
                "keyRate",
                "proDelay",
                "bandwidth",
                "weight",
                "faultTime",
                "is_cross_subdomain",
                "src_subdomain",
                "src_isGateway",
                "dst_subdomain",
                "dst_isGateway",
            ]
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            # 不写header，按照你上一版风格：直接数据行。如果你想要header可以writer.writeheader()
            writer.writeheader()
            for linkId, (u, v, du, dv) in enumerate(all_edges_full):
                is_cross = 1 if du != dv else 0

                keyRate, proDelay, bandwidth, weight, faultTime = self._assign_link_attributes(
                    is_cross == 1
                )

                src_isGateway = 1 if u in gateway_nodes else 0
                dst_isGateway = 1 if v in gateway_nodes else 0

                writer.writerow(
                    {
                        "linkId": linkId,
                        "sourceId": u,
                        "sinkId": v,
                        "keyRate": keyRate,
                        "proDelay": proDelay,
                        "bandwidth": bandwidth,
                        "weight": weight,
                        "faultTime": faultTime,
                        "is_cross_subdomain": is_cross,
                        "src_subdomain": du,
                        "src_isGateway": src_isGateway,
                        "dst_subdomain": dv,
                        "dst_isGateway": dst_isGateway,
                    }
                )

        print(f"[OK] 网络拓扑已写入 {filename}")
        return all_edges_full

    # ---------------- demand.csv 生成 ----------------

    def _gen_pairs(self, G):
        """
        根据比例生成同域/跨域的通信对，要求最短路跳数 <= max_pair_hops
        """
        pairs = []
        trials = 0
        max_trials = self.num_pairs * 50 + 1000

        while len(pairs) < self.num_pairs and trials < max_trials:
            trials += 1
            same_domain = (self.rng.random() < self.intra_pair_ratio)

            if same_domain:
                dom = self.rng.randint(0, self.num_domains - 1)
                src = self._pick_node_from_domain(dom)
                dst = self._pick_node_from_domain(dom)
            else:
                d1 = self.rng.randint(0, self.num_domains - 1)
                d2 = self.rng.randint(0, self.num_domains - 1)
                while d2 == d1 and self.num_domains > 1:
                    d2 = self.rng.randint(0, self.num_domains - 1)
                src = self._pick_node_from_domain(d1)
                dst = self._pick_node_from_domain(d2)

            if src == dst:
                continue
            if not (G.has_node(src) and G.has_node(dst)):
                continue

            try:
                dist = nx.shortest_path_length(G, source=src, target=dst)
            except nx.NetworkXNoPath:
                continue

            if dist <= self.max_pair_hops:
                pairs.append((src, dst))

        return pairs

    def generate_demand_csv(self, out_dir,f, G):
        demdir=os.path.join(out_dir,'demands')
        filename = os.path.join(demdir, f)
        with open(filename, "w", newline="") as csvfile:
            fieldnames = ["demandId", "sourceId", "sinkId", "demandVolume", "arriveTime"]
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()

            pairs = self._gen_pairs(G)
            for demandId, (src, dst) in enumerate(pairs):
                demandVolume = self.rng.choice([x for x in range(50, 101, 5)])
                arriveTime = self.rng.randint(2, 50)
                writer.writerow(
                    {
                        "demandId": demandId,
                        "sourceId": src,
                        "sinkId": dst,
                        "demandVolume": demandVolume,
                        "arriveTime": arriveTime,
                    }
                )

        print(f"[OK] 需求对已写入 {filename}")

    # ---------------- 绘图 ----------------

    def plot_topology(self, out_dir, all_edges_full):
        if self.total_nodes > self.plot_limit:
            print(
                f"[WARN] 节点数 {self.total_nodes} > plot_limit({self.plot_limit})，跳过绘图。"
            )
            return

        G = nx.Graph()
        for (u, v, du, dv) in all_edges_full:
            G.add_edge(u, v)

        pos = nx.spring_layout(G, seed=self.seed)
        nx.draw(
            G,
            pos,
            with_labels=False,
            node_size=50,
            node_color="lightblue",
            edge_color="gray",
            width=0.5,
        )
        plt.title("Heterogeneous Multi-Domain Network Topology")
        fig_path = os.path.join(out_dir, "topology.png")
        plt.savefig(fig_path, dpi=300)
        plt.close()
        print(f"[OK] 拓扑图已保存到 {fig_path}")

    # ---------------- 主流程 ----------------

    def generate_all(self, out_dir,filename):
        all_edges_full = self.generate_network_csv(out_dir,filename)

        # 用这些边构建图
        G = nx.Graph()
        for (u, v, du, dv) in all_edges_full:
            G.add_edge(u, v)

        self.generate_demand_csv(out_dir,filename, G)
        # self.plot_topology(out_dir, all_edges_full)


def parse_args():
    parser = argparse.ArgumentParser(description="异构多域网络拓扑生成器")

    parser.add_argument(
        "--filename",
        type=str,
        required=True,
        help='filename'
    )
    parser.add_argument(
        "--domains",
        type=str,
        required=True,
        help='JSON数组，如: [{"nodes":1200,"edges":5000},{"nodes":800,"edges":2000}]',
    )
    parser.add_argument(
        "--inter-edges",
        type=str,
        required=True,
        help='JSON矩阵，如: [[0,30,5],[30,0,10],[5,10,0]]',
    )

    parser.add_argument("--seed", type=int, default=42, help="随机种子")
    parser.add_argument("--num-pairs", type=int, default=4000, help="需求对数量")
    parser.add_argument(
        "--intra-pair-ratio",
        type=float,
        default=0.8,
        help="生成需求对时，同域pair的比例(0~1)",
    )
    parser.add_argument(
        "--max-pair-hops",
        type=int,
        default=8,
        help="仅接受最短路跳数<=该值的(source,dest)对",
    )
    parser.add_argument(
        "--plot-limit",
        type=int,
        default=1000,
        help="只有总节点数<=该值才绘图, 避免大图卡死",
    )
    parser.add_argument(
        "--out-dir",
        type=str,
        default="data",
        help="输出目录(network.csv / demand.csv / topology.png)",
    )

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    domain_specs = json.loads(args.domains)
    inter_edges_matrix = json.loads(args.inter_edges)

    gen = HeteroNetworkGenerator(
        domain_specs=domain_specs,
        inter_edges_matrix=inter_edges_matrix,
        seed=args.seed,
        num_pairs=args.num_pairs,
        intra_pair_ratio=args.intra_pair_ratio,
        max_pair_hops=args.max_pair_hops,
        plot_limit=args.plot_limit,
    )

    gen.generate_all(args.out_dir,args.filename)

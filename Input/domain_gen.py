import random
import os
import csv
import argparse
import json
import math
from enum import Enum
from datetime import datetime

import networkx as nx
import matplotlib.pyplot as plt


class DistributionType(Enum):
    UNIFORM = "uniform"
    NORMAL = "normal"
    EXPONENTIAL = "exponential"
    CONSTANT = "constant"


class DemandPattern(Enum):
    UNIFORM = "uniform"  # 均匀到达
    BURST = "burst"  # 短时突发
    PERIODIC = "periodic"  # 周期性到达
    PULSE = "pulse"  # 脉冲式到达


class NetworkConfigPreset(Enum):
    DEFAULT = "default"  # 默认配置
    HIGH_PERFORMANCE = "high_performance"  # 高性能网络
    LOW_LATENCY = "low_latency"  # 低延迟网络
    HIGH_CAPACITY = "high_capacity"  # 高容量网络


class DemandConfigPreset(Enum):
    UNIFORM = "uniform"  # 均匀需求
    BURSTY = "bursty"  # 突发需求
    PERIODIC = "periodic"  # 周期性需求
    MIXED = "mixed"  # 混合需求


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
        network_config_preset=None,  # 网络配置预设
        demand_config_preset=None,   # 需求配置预设
        custom_link_config=None,     # 自定义链路配置
        custom_demand_config=None,   # 自定义需求配置
    ):
        """
        domain_specs: [
            {"nodes": Nd, "edges": Ed},
            ...
        ]
        inter_edges_matrix: D x D 矩阵
            inter_edges_matrix[i][j] = 期望域i<->域j的跨域边数量
        
        新增参数:
        network_config_preset: 网络配置预设
        demand_config_preset: 需求配置预设
        custom_link_config: 自定义链路配置（覆盖预设）
        custom_demand_config: 自定义需求配置（覆盖预设）
        """
        self.domain_specs = domain_specs
        self.inter_edges_matrix = inter_edges_matrix
        self.seed = seed
        self.rng = random.Random(seed)

        self.num_pairs = num_pairs
        self.intra_pair_ratio = intra_pair_ratio
        self.max_pair_hops = max_pair_hops
        self.plot_limit = plot_limit
        
        # 设置网络配置
        self.link_attr_config = self._get_link_config(
            network_config_preset, custom_link_config
        )
        
        # 设置需求配置
        self.demand_config = self._get_demand_config(
            demand_config_preset, custom_demand_config
        )

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

    def _get_link_config(self, preset, custom_config):
        """根据预设和自定义配置获取链路配置"""
        if custom_config:
            return custom_config
        
        preset_configs = {
            NetworkConfigPreset.DEFAULT: {
                "intra_domain": {
                    "keyRate": {"type": DistributionType.UNIFORM, "params": {"low": 2.0, "high": 5.0}},
                    "proDelay": {"type": DistributionType.UNIFORM, "params": {"low": 0, "high": 5}},
                    "bandwidth": {"type": DistributionType.UNIFORM, "params": {"low": 8, "high": 10}}
                },
                "inter_domain": {
                    "keyRate": {"type": DistributionType.UNIFORM, "params": {"low": 1.0, "high": 3.0}},
                    "proDelay": {"type": DistributionType.UNIFORM, "params": {"low": 5, "high": 20}},
                    "bandwidth": {"type": DistributionType.UNIFORM, "params": {"low": 2, "high": 4}}
                }
            },
            NetworkConfigPreset.HIGH_PERFORMANCE: {
                "intra_domain": {
                    "keyRate": {"type": DistributionType.NORMAL, "params": {"mu": 4.0, "sigma": 0.5, "min": 3.0, "max": 5.0}},
                    "proDelay": {"type": DistributionType.UNIFORM, "params": {"low": 0, "high": 2}},
                    "bandwidth": {"type": DistributionType.UNIFORM, "params": {"low": 9, "high": 12}}
                },
                "inter_domain": {
                    "keyRate": {"type": DistributionType.NORMAL, "params": {"mu": 2.5, "sigma": 0.5, "min": 2.0, "max": 4.0}},
                    "proDelay": {"type": DistributionType.UNIFORM, "params": {"low": 3, "high": 10}},
                    "bandwidth": {"type": DistributionType.UNIFORM, "params": {"low": 4, "high": 6}}
                }
            },
            NetworkConfigPreset.LOW_LATENCY: {
                "intra_domain": {
                    "keyRate": {"type": DistributionType.UNIFORM, "params": {"low": 3.0, "high": 5.0}},
                    "proDelay": {"type": DistributionType.UNIFORM, "params": {"low": 0, "high": 1}},
                    "bandwidth": {"type": DistributionType.UNIFORM, "params": {"low": 6, "high": 8}}
                },
                "inter_domain": {
                    "keyRate": {"type": DistributionType.UNIFORM, "params": {"low": 2.0, "high": 3.0}},
                    "proDelay": {"type": DistributionType.UNIFORM, "params": {"low": 1, "high": 5}},
                    "bandwidth": {"type": DistributionType.UNIFORM, "params": {"low": 3, "high": 5}}
                }
            },
            NetworkConfigPreset.HIGH_CAPACITY: {
                "intra_domain": {
                    "keyRate": {"type": DistributionType.UNIFORM, "params": {"low": 2.0, "high": 4.0}},
                    "proDelay": {"type": DistributionType.UNIFORM, "params": {"low": 1, "high": 5}},
                    "bandwidth": {"type": DistributionType.UNIFORM, "params": {"low": 12, "high": 15}}
                },
                "inter_domain": {
                    "keyRate": {"type": DistributionType.UNIFORM, "params": {"low": 1.0, "high": 2.5}},
                    "proDelay": {"type": DistributionType.UNIFORM, "params": {"low": 5, "high": 15}},
                    "bandwidth": {"type": DistributionType.UNIFORM, "params": {"low": 6, "high": 8}}
                }
            }
        }
        
        return preset_configs.get(preset or NetworkConfigPreset.DEFAULT, preset_configs[NetworkConfigPreset.DEFAULT])

    def _get_demand_config(self, preset, custom_config):
        """根据预设和自定义配置获取需求配置"""
        if custom_config:
            return custom_config
        
        preset_configs = {
            DemandConfigPreset.UNIFORM: {
                "pattern": DemandPattern.UNIFORM,
                "params": {
                    "demandVolume": {"type": DistributionType.UNIFORM, "params": {"low": 50, "high": 100, "step": 5}},
                    "arriveTime": {"type": DistributionType.UNIFORM, "params": {"low": 2, "high": 50}}
                }
            },
            DemandConfigPreset.BURSTY: {
                "pattern": DemandPattern.BURST,
                "params": {
                    "demandVolume": {"type": DistributionType.NORMAL, "params": {"mu": 80, "sigma": 20, "min": 50, "max": 100}},
                    "arriveTime": {
                        "burst_prob": 0.8,
                        "burst_start": 0,
                        "burst_duration": 15,
                        "normal_range": {"low": 30, "high": 60}
                    }
                }
            },
            DemandConfigPreset.PERIODIC: {
                "pattern": DemandPattern.PERIODIC,
                "params": {
                    "demandVolume": {"type": DistributionType.UNIFORM, "params": {"low": 60, "high": 90, "step": 5}},
                    "arriveTime": {
                        "period": 12,
                        "duration": 4,
                        "base_time": 0,
                        "cycles": 6
                    }
                }
            },
            DemandConfigPreset.MIXED: {
                "pattern": DemandPattern.PULSE,
                "params": {
                    "demandVolume": {"type": DistributionType.UNIFORM, "params": {"low": 40, "high": 120, "step": 10}},
                    "arriveTime": {
                        "pulse_times": [0, 15, 30, 45],
                        "pulse_duration": 5,
                        "pulse_prob": 0.7,
                        "between_pulse_range": {"low": 10, "high": 20}
                    }
                }
            }
        }
        
        return preset_configs.get(preset or DemandConfigPreset.UNIFORM, preset_configs[DemandConfigPreset.UNIFORM])

    def _generate_from_distribution(self, dist_config):
        """根据分布配置生成随机值"""
        dist_type = dist_config["type"]
        params = dist_config["params"]
        
        if dist_type == DistributionType.UNIFORM:
            # 保持与原代码一致的选择逻辑
            if "step" in params:
                # 与原代码一致：从指定步长的范围内选择
                choices = [x for x in range(int(params["low"]), int(params["high"]) + 1, params["step"])]
                return self.rng.choice(choices)
            else:
                value = self.rng.uniform(params["low"], params["high"])
                # 如果是整数类型的参数，返回整数
                if "low" in params and isinstance(params["low"], int) and "high" in params and isinstance(params["high"], int):
                    return int(value)
                return value
        elif dist_type == DistributionType.NORMAL:
            # 使用截断正态分布，避免极端值
            value = self.rng.gauss(params["mu"], params["sigma"])
            if "min" in params and value < params["min"]:
                value = params["min"]
            if "max" in params and value > params["max"]:
                value = params["max"]
            return value
        elif dist_type == DistributionType.EXPONENTIAL:
            value = self.rng.expovariate(params["lambda"])
            if "max" in params and value > params["max"]:
                value = params["max"]
            return value
        elif dist_type == DistributionType.CONSTANT:
            return params["value"]
        else:
            raise ValueError(f"不支持的分布类型: {dist_type}")

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
          ▪ a,b 是排序后的小id和大id，用于去重

          ▪ u,v 保留原端点

          ▪ du,dv 是域id

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
        根据链路属性配置生成链路属性
        """
        # 选择域内或跨域配置
        config_key = "inter_domain" if is_cross else "intra_domain"
        config = self.link_attr_config[config_key]
        
        # 生成各属性
        keyRate = round(self._generate_from_distribution(config["keyRate"]), 1)
        proDelay = int(self._generate_from_distribution(config["proDelay"]))
        bandwidth = float(self._generate_from_distribution(config["bandwidth"]))
        
        # 计算权重
        weight = round(
            3 * ((keyRate / 5.0) * 1.5 + (bandwidth / 10.0) * 0.5 - (proDelay / 20.0) * 1),
            2,
        )
        return keyRate, proDelay, bandwidth, weight, -1  # faultTime=-1

    def generate_network_csv(self, out_dir, f):
        """
        生成网络拓扑CSV文件
        """
        if not os.path.exists(out_dir):
            os.makedirs(out_dir)
        
        # 确保networks子目录存在
        netdir = os.path.join(out_dir, "networks")
        if not os.path.exists(netdir):
            os.makedirs(netdir)
            
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
        tmpdict = {}
        for (u, v, du, dv) in all_edges_full:
            key = (u, v)
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

    def _generate_arrive_time(self, demand_index, total_demands):
        """根据需求模式生成到达时间"""
        pattern = self.demand_config["pattern"]
        params = self.demand_config.get("params", {}).get("arriveTime", {})
        
        if pattern == DemandPattern.UNIFORM:
            # 均匀分布 - 使用原代码的逻辑
            if not params:
                params = {"low": 2, "high": 50}
            return int(self.rng.uniform(params.get("low", 2), params.get("high", 50)))
        
        elif pattern == DemandPattern.BURST:
            # 短时突发：大部分需求在短时间内到达
            burst_prob = params.get("burst_prob", 0.7)  # 突发概率
            burst_start = params.get("burst_start", 0)
            burst_duration = params.get("burst_duration", 10)
            normal_range = params.get("normal_range", {"low": 20, "high": 50})
            
            if self.rng.random() < burst_prob:
                # 在突发时间段内到达
                return int(self.rng.uniform(burst_start, burst_start + burst_duration))
            else:
                # 在正常时间段内到达
                return int(self.rng.uniform(normal_range["low"], normal_range["high"]))
        
        elif pattern == DemandPattern.PERIODIC:
            # 周期性到达
            period = params.get("period", 10)
            duration = params.get("duration", 3)  # 每个周期的持续时间
            base_time = params.get("base_time", 0)
            cycles = params.get("cycles", max(1, total_demands // 10))  # 默认周期数
            
            # 计算当前需求所在的周期
            cycle = demand_index % cycles
            cycle_start = base_time + cycle * period
            return int(self.rng.uniform(cycle_start, cycle_start + duration))
        
        elif pattern == DemandPattern.PULSE:
            # 脉冲式到达：在特定时间点集中到达
            pulse_times = params.get("pulse_times", [0, 20, 40])
            pulse_duration = params.get("pulse_duration", 2)
            between_pulse_range = params.get("between_pulse_range", {"low": 10, "high": 15})
            pulse_prob = params.get("pulse_prob", 0.8)
            
            # 随机选择一个脉冲时间段
            if pulse_times and self.rng.random() < pulse_prob:
                pulse_time = self.rng.choice(pulse_times)
                return int(self.rng.uniform(pulse_time, pulse_time + pulse_duration))
            else:
                # 在脉冲之间到达
                return int(self.rng.uniform(between_pulse_range["low"], between_pulse_range["high"]))
        
        else:
            # 默认使用均匀分布（与原代码一致）
            return int(self.rng.uniform(2, 50))

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

    def generate_demand_csv(self, out_dir, f, G):
        """生成需求CSV文件，支持不同的到达模式"""
        # 确保demands子目录存在
        demdir = os.path.join(out_dir, 'demands')
        if not os.path.exists(demdir):
            os.makedirs(demdir)
            
        filename = os.path.join(demdir, f)
        
        with open(filename, "w", newline="") as csvfile:
            fieldnames = ["demandId", "sourceId", "sinkId", "demandVolume", "arriveTime"]
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()

            pairs = self._gen_pairs(G)
            for demandId, (src, dst) in enumerate(pairs):
                # 生成需求流量
                demand_config = self.demand_config.get("params", {}).get("demandVolume", {})
                if demand_config.get("type") == DistributionType.UNIFORM and "step" in demand_config.get("params", {}):
                    # 与原代码一致：从指定步长的范围内选择
                    params = demand_config["params"]
                    choices = [x for x in range(params["low"], params["high"] + 1, params["step"])]
                    demandVolume = self.rng.choice(choices)
                else:
                    demandVolume = int(self._generate_from_distribution(demand_config))
                
                # 生成到达时间（根据配置的模式）
                arriveTime = self._generate_arrive_time(demandId, len(pairs))
                
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

    def plot_topology(self, out_dir, all_edges_full):
        """绘制拓扑图（大网络时跳过）"""
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

    def generate_all(self, out_dir, filename):
        """生成完整的网络和需求数据"""
        all_edges_full = self.generate_network_csv(out_dir, filename)

        # 用这些边构建图
        G = nx.Graph()
        for (u, v, du, dv) in all_edges_full:
            G.add_edge(u, v)

        self.generate_demand_csv(out_dir, filename, G)
        # self.plot_topology(out_dir, all_edges_full)


def parse_args():
    """解析命令行参数，支持配置预设和自定义配置"""
    parser = argparse.ArgumentParser(description="异构多域网络拓扑生成器")

    parser.add_argument("--filename", type=str, required=True, help='输出文件名')
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
        help="输出目录",
    )
    
    # 网络配置预设
    parser.add_argument(
        "--network-preset",
        type=str,
        choices=[p.value for p in NetworkConfigPreset],
        default="default",
        help='网络配置预设: default, high_performance, low_latency, high_capacity'
    )
    
    # 需求配置预设
    parser.add_argument(
        "--demand-preset", 
        type=str,
        choices=[p.value for p in DemandConfigPreset],
        default="uniform",
        help='需求配置预设: uniform, bursty, periodic, mixed'
    )
    
    # 自定义配置（覆盖预设）
    parser.add_argument(
        "--custom-link-config",
        type=str,
        default=None,
        help='JSON格式的自定义链路配置（覆盖预设）'
    )
    
    parser.add_argument(
        "--custom-demand-config", 
        type=str, 
        default=None,
        help='JSON格式的自定义需求配置（覆盖预设）'
    )

    return parser.parse_args()


def main():
    """主函数，处理参数并生成网络"""
    args = parse_args()
    domain_specs = json.loads(args.domains)
    inter_edges_matrix = json.loads(args.inter_edges)
    
    # 转换预设参数为枚举
    network_preset = NetworkConfigPreset(args.network_preset)
    demand_preset = DemandConfigPreset(args.demand_preset)
    
    # 解析自定义配置
    custom_link_config = None
    if args.custom_link_config:
        try:
            custom_link_config = json.loads(args.custom_link_config)
            # 将字符串类型转换为DistributionType枚举
            for domain_type in ["intra_domain", "inter_domain"]:
                if domain_type in custom_link_config:
                    for attr in custom_link_config[domain_type]:
                        if isinstance(custom_link_config[domain_type][attr]["type"], str):
                            custom_link_config[domain_type][attr]["type"] = DistributionType(custom_link_config[domain_type][attr]["type"])
        except json.JSONDecodeError as e:
            print(f"警告：自定义链路配置JSON解析错误，使用预设配置: {e}")
            custom_link_config = None
    
    # 解析自定义需求配置
    custom_demand_config = None
    if args.custom_demand_config:
        try:
            custom_demand_config = json.loads(args.custom_demand_config)
            # 将字符串类型转换为枚举
            if "pattern" in custom_demand_config and isinstance(custom_demand_config["pattern"], str):
                custom_demand_config["pattern"] = DemandPattern(custom_demand_config["pattern"])
        except json.JSONDecodeError as e:
            print(f"警告：自定义需求配置JSON解析错误，使用预设配置: {e}")
            custom_demand_config = None

    # 创建生成器
    gen = HeteroNetworkGenerator(
        domain_specs=domain_specs,
        inter_edges_matrix=inter_edges_matrix,
        seed=args.seed,
        num_pairs=args.num_pairs,
        intra_pair_ratio=args.intra_pair_ratio,
        max_pair_hops=args.max_pair_hops,
        plot_limit=args.plot_limit,
        network_config_preset=network_preset,
        demand_config_preset=demand_preset,
        custom_link_config=custom_link_config,
        custom_demand_config=custom_demand_config,
    )

    # 生成数据
    gen.generate_all(args.out_dir, args.filename)
    
    # 保存配置信息
    config_info = {
        "generated_at": datetime.now().isoformat(),
        "domain_specs": domain_specs,
        "inter_edges_matrix": inter_edges_matrix,
        "seed": args.seed,
        "num_pairs": args.num_pairs,
        "intra_pair_ratio": args.intra_pair_ratio,
        "max_pair_hops": args.max_pair_hops,
        "network_preset": network_preset.value,
        "demand_preset": demand_preset.value,
        "custom_link_config_used": custom_link_config is not None,
        "custom_demand_config_used": custom_demand_config is not None
    }
    
    config_file = os.path.join(args.out_dir, f"config_{args.filename}.json")
    with open(config_file, 'w') as f:
        json.dump(config_info, f, indent=2)
    
    print(f"[OK] 配置信息已保存到 {config_file}")


if __name__ == "__main__":
    main()




#     使用示例
# 示例1：使用预设配置生成数据
# python script.py \
#     --domains '[{"nodes":100,"edges":200},{"nodes":80,"edges":150}]' \
#     --inter-edges '[[0,10],[10,0]]' \
#     --filename "high_perf_bursty.csv" \
#     --network-preset high_performance \
#     --demand-preset bursty
# 示例2：混合配置
# python script.py \
#     --domains '[{"nodes":200,"edges":500},{"nodes":150,"edges":300}]' \
#     --inter-edges '[[0,20],[20,0]]' \
#     --filename "low_latency_periodic.csv" \
#     --network-preset low_latency \
#     --demand-preset periodic
# 示例3：自定义配置覆盖预设
# python script.py \
#     --domains '[{"nodes":100,"edges":200}]' \
#     --inter-edges '[[0]]' \
#     --filename "custom_network.csv" \
#     --network-preset default \
#     --custom-link-config '{"intra_domain": {"keyRate": {"type": "normal", "params": {"mu": 4.0, "sigma": 0.5}}}}'
# 5. 预设配置说明
# 网络配置预设:
# default: 默认配置，与原代码行为一致
# high_performance: 高性能网络（高密钥速率、低延迟）
# low_latency: 低延迟网络（极低延迟，适中带宽）
# high_capacity: 高容量网络（高带宽，适中延迟）
# 需求配置预设:
# uniform: 均匀到达（默认）
# bursty: 突发需求（80%需求在短时间内到达）
# periodic: 周期性需求（按周期到达）
# mixed: 混合模式（脉冲式到达）
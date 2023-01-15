import random
import streamlit as st
from streamlit_agraph import agraph, Node, Edge, Config

color_byte_random = lambda: random.randint(0, 255)

def generate_color() -> str:
    return '#%02X%02X%02X' % (color_byte_random(), color_byte_random(), color_byte_random());

def read_graph(graph_file: list[str]) -> dict:
    vertices = 0
    edges = 0

    graph = dict()

    for line in graph_file:
        if line[0] == 'c':
            continue
        elif line[0] == 'p':
            # Command example:
            # p edge 11 20
            parts = line.split()

            vertices = int(parts[2])
            edges = int(parts[3])
        elif line[0] == 'e':
            # Command example:
            # e 1 2
            parts = line.split()
            start = int(parts[1]) - 1
            finish = int(parts[2]) - 1

            if start not in graph:
                graph[start] = set()

            if finish not in graph:
                graph[finish] = set()

            graph[start].add(finish)
            graph[finish].add(start)
        else:
            raise RuntimeError(f"Unknown command: {line}")

    # Add disconnected nodes to the graph
    for vertex in range(0, vertices):
        if vertex not in graph:
            graph[vertex] = set()

    return graph

def read_colors(colors: str) -> dict:
    colors_ids = [int(x) for x in colors.split()]

    colors_mapping = dict()
    result_coloring = dict()
    # to avoid collisions in color
    seen_colors = set()

    for vertex, color_id in enumerate(colors_ids):
        if color_id not in colors_mapping:
            generated_color = generate_color()
            while generate_color in seen_colors:
                generated_color = generate_color()
            seen_colors.add(generated_color)
            colors_mapping[color_id] = generated_color

        hex_color = colors_mapping[color_id]
        result_coloring[vertex] = hex_color

    return result_coloring

graph_file = st.file_uploader("Select a graph model")
coloring_file = st.file_uploader("Select a coloring model")

graph = dict()
if graph_file is not None:
    file_lines = graph_file.getvalue().decode("utf-8").splitlines()
    graph = read_graph(file_lines)

colors_mapping = dict()
if coloring_file is not None:
    file_lines = coloring_file.getvalue().decode("utf-8")
    colors_mapping = read_colors(file_lines)

nodes_to_draw = list()
edges_to_draw = list()

for vertex, edges in graph.items():
    color = "#DBEBC2"
    if len(colors_mapping) > 0:
        color = colors_mapping[vertex]

    node = Node(id=vertex, 
                label=vertex, 
                size=15, 
                shape="dot",
                color=color)

    nodes_to_draw.append(node)

    for edge in edges:
        edge = Edge(source=vertex, 
                    target=edge) 
        edges_to_draw.append(edge)

config = Config(width=1000, 
                height=1000,
                directed=False,
                graphviz_layout="twopi",
                nodeHighlightBehavior=False) 

return_value = agraph(nodes=nodes_to_draw, 
                      edges=edges_to_draw, 
                      config=config)

// This RTree is adapted from the JavaScript implementation at https://github.com/mourner/rbush

#ifndef RTREE_HPP
#define RTREE_HPP

#include <vector>
#include <iostream>
#include <cmath>
#include <limits>
#include <algorithm>
#include <queue>

template<typename T>
struct BBox;

template<typename T>
struct RTreeNode;

// Bounding box
template<typename T>
struct BBox {
    BBox() {
        min_x = std::numeric_limits<float>::max();
        min_y = std::numeric_limits<float>::max();
        max_x = std::numeric_limits<float>::min();
        max_y = std::numeric_limits<float>::min();
    }
    
    BBox(float a, float b, float c, float d) : min_x(a), min_y(b), max_x(c), max_y(d) {}

    BBox(T s) {
        min_x = s.location.x;
        max_x = s.location.x;
        min_y = s.location.y;
        max_y = s.location.y;
    }
    
    BBox(RTreeNode<T> p) {
        min_x = p.record.location.x;
        max_x = p.record.location.x;
        min_y = p.record.location.y;
        max_y = p.record.location.y;
    }

    float min_x;
    float min_y;
    float max_x;
    float max_y;
};

template<typename T>
struct RTreeNode {
    // Constructor for non-leaf node
    RTreeNode(float height = 1, bool leaf = true): height(height), leaf(leaf) {}

    // Constructor for leaf node (node with Spotted_Enemy record)
    RTreeNode(T record, float height = 0): record(record), height(height) {
        leaf = true;
        bbox.min_x = record.location.x;
        bbox.max_x = record.location.x;
        bbox.min_y = record.location.y;
        bbox.max_y = record.location.y;
    }

    // Copy constructor
    RTreeNode(const RTreeNode<T>& r) : height(r.height), leaf(r.leaf), bbox(r.bbox),
        record(r.record), children(r.children) {}
    
    void print() const {
        std::cout << "['(" << bbox.min_x << ", " << 
                               bbox.min_y << ", " << 
                               bbox.max_x << ", " << 
                               bbox.max_y << ")'" << (height==0 ? "" : "-> ");
        for (auto& child : children) child.print();
        std::cout << "]";
    }

    T record;
    std::vector<RTreeNode<T>> children;
    int height;
    bool leaf;
    BBox<T> bbox;
};


template<typename T>
class RTree {
public:
    RTree() {}

    void insert(const T& item) {
        BBox<T> bbox(item);
        float level = data.height - 1;
        std::vector<RTreeNode<T>*> insert_path;

        RTreeNode<T>* node_ptr = choose_subtree(bbox, data, level, insert_path);
        // Push item into node
        node_ptr->children.push_back(RTreeNode<T>(item));
        extend(*node_ptr, bbox);

        // Handle node overflow
        while (level >= 0 && insert_path[level]->children.size() > 9) {
            split(insert_path, level--);
        }

        // adjust bbox along insertion path
        for (int i = level; i >= 0; --i) extend(*insert_path[i], bbox);
    }

    std::vector<T> search(Point2D point, float radius) {
        // Use point and radius to compute bounding box which can be used to search the RTree
        bool all = radius == std::numeric_limits<float>::max();
        BBox<T> bbox(
            all ? std::numeric_limits<float>::min() : point.x - radius,
            all ? std::numeric_limits<float>::min() : point.y - radius,
            all ? std::numeric_limits<float>::max() : point.x + radius,
            all ? std::numeric_limits<float>::max() : point.y + radius
        );

        RTreeNode<T> node = data;
        std::vector<T> ret;
        if (!intersects(bbox, node.bbox)) return ret;
        
        // BFS for leaf nodes
        std::queue<RTreeNode<T>> nodes_to_search({node});
        while (!nodes_to_search.empty()) {
            node = nodes_to_search.front(); nodes_to_search.pop();
            for (auto& child : node.children) {
                if (intersects(bbox, child.bbox)) {
                    if (node.leaf) {
                        ret.push_back(child.record);
                    } else if (contains(bbox, child.bbox)) {
                        // every descendant of this node is in bounds
                        RTreeNode<T> n = child;
                        std::queue<RTreeNode<T>> q({n});
                        while (!q.empty()) {
                            n = q.front(); q.pop();
                            for (auto& c : n.children) {
                                if (n.leaf) ret.push_back(c.record);
                                else q.push(c);
                            }
                        }
                    } else {
                        nodes_to_search.push(child);
                    }
                }
            }
        }
        return ret;
    }

private:
    RTreeNode<T> data;
    
    bool intersects(BBox<T> a, BBox<T> b) const {
        return b.min_x <= a.max_x && b.min_y <= a.max_y && b.max_x >= a.min_x && b.max_y >= a.min_y;
    }

    bool contains(BBox<T> a, BBox<T> b) const {
        return a.min_x <= b.min_x && a.min_y <= b.min_y && b.max_x <= a.max_x && b.max_y <= a.max_y;
    }
    
    void extend(BBox<T>& a, BBox<T> b) const {
        a.min_x = std::min(a.min_x, b.min_x);
        a.min_y = std::min(a.min_y, b.min_y);
        a.max_x = std::max(a.max_x, b.max_x);
        a.max_y = std::max(a.max_y, b.max_y);
    }

    void extend(RTreeNode<T>& a, BBox<T> b) const {
        a.bbox.min_x = std::min(a.bbox.min_x, b.min_x);
        a.bbox.min_y = std::min(a.bbox.min_y, b.min_y);
        a.bbox.max_x = std::max(a.bbox.max_x, b.max_x);
        a.bbox.max_y = std::max(a.bbox.max_y, b.max_y);
    }

    int choose_split_index(RTreeNode<T>& node, const int m, const int M) const {
        int index = -1;
        float min_overlap = std::numeric_limits<float>::max();
        float min_area = std::numeric_limits<float>::max();
        
        for (int i = m; i <= M; ++i) {
            BBox<T> bbox1, bbox2;
            for (int i = 0; i < index; ++i) {
                extend(bbox1, node.children[i].bbox);
            }
            for (int i = index; i < M; ++i) {
                extend(bbox2, node.children[i].bbox);
            }
            float overlap = std::max(0.0f, std::min(bbox1.max_x, bbox2.max_x) - std::max(bbox1.min_x, bbox2.min_x)) -
                            std::max(0.0f, std::min(bbox1.max_y, bbox2.max_y) - std::max(bbox1.min_y, bbox2.min_y));
            float area = ((bbox1.max_x - bbox1.min_x) * (bbox1.max_y - bbox1.min_y)) +
                         ((bbox1.max_x - bbox1.min_x) * (bbox1.max_y - bbox1.min_y));
        
            // choose distribution with minimum overlap or minimum area
            if (overlap < min_overlap) {
                min_overlap = overlap;
                index = i;
                min_area = std::min(area, min_area);
            } else if (overlap == min_overlap && area < min_area) {
                min_area = area;
                index = i;
            }
        }
        
        return index == -1 ? M - m : index;
    }

    // total margin of all possible split distributions where each node is at least m full
    float all_dist_margin(RTreeNode<T>& node, const int m, const int M, bool do_x) {
        std::sort(node.children.begin(), node.children.end(), [do_x](const RTreeNode<T>& a, const RTreeNode<T>& b) {
            return do_x ? a.bbox.min_x - b.bbox.min_x : a.bbox.min_y - b.bbox.min_y;
        });
        BBox<T> left_bbox, right_bbox;
        for (int i = 0; i < m; ++i) extend(left_bbox, node.children[i].bbox);
        for (int i = M-m; i < M; ++i) extend(right_bbox, node.children[i].bbox);
        float margin = (left_bbox.max_x - left_bbox.min_x) + (left_bbox.max_y - left_bbox.min_y) +
                     (right_bbox.max_x - right_bbox.min_x) + (right_bbox.max_y - right_bbox.min_y);
        for (int i = m; i < M - m; ++i) {
            extend(left_bbox, node.children[i].bbox);
            margin += (left_bbox.max_x - left_bbox.min_x) + (left_bbox.max_y - left_bbox.min_y);
        }
        for (int i = M - m - 1; i >= m; --i) {
            extend(right_bbox, node.children[i].bbox);
            margin += (right_bbox.max_x - right_bbox.min_x) + (right_bbox.max_y - right_bbox.min_y);
        }
        return margin;
    }

    // sorts node children by the best axis for split
    void choose_split_axis(RTreeNode<T>& node, const int m, const int M) {
        float x_margin = all_dist_margin(node, m, M, true);
        float y_margin = all_dist_margin(node, m, M, false);
        if (x_margin < y_margin) {
            std::sort(node.children.begin(), node.children.end(),
                [](const RTreeNode<T>& a, const RTreeNode<T>& b) { return a.bbox.min_x - b.bbox.min_x; }
            );
        }
    }

    // Split overflowed node into two
    void split(std::vector<RTreeNode<T>*>& insertPath, const int level) {
        RTreeNode<T>* node_ptr = insertPath[level];
        choose_split_axis(*node_ptr, 4, node_ptr->children.size());
        int split_index = choose_split_index(*node_ptr, 4, node_ptr->children.size());
        RTreeNode<T> new_node(node_ptr->height, node_ptr->leaf);
        for (int i = split_index; i < node_ptr->children.size(); ++i) {
            new_node.children.push_back(node_ptr->children[i]);
        }
        for (auto& child : node_ptr->children) extend(*node_ptr, child.bbox);
        for (auto& child : new_node.children) extend(new_node, child.bbox);

        if (level) {
            insertPath[level - 1]->children.push_back(new_node);
        } else {
            data = RTreeNode<T>(node_ptr->height + 1, false);
            data.children.push_back(*node_ptr); data.children.push_back(new_node);
            for (auto& child : data.children) extend(data, child.bbox);
        }
    }

    RTreeNode<T>* choose_subtree(BBox<T>& bbox, RTreeNode<T>& node, int level, std::vector<RTreeNode<T>*>& path) {
        RTreeNode<T>* node_ptr = &node;

        while (true) {
            path.push_back(node_ptr);

            if (node_ptr->leaf || path.size() - 1 == level) break;

            RTreeNode<T>* target_node_ptr;
            bool targetUpdated = false;
            float minArea = std::numeric_limits<float>::max();
            float minEnlargement = std::numeric_limits<float>::max();

            for (auto& child : node_ptr->children) {
                float area = (child.bbox.max_x - child.bbox.min_x) * (child.bbox.max_y - child.bbox.min_y);
                float enlargement = (
                    (std::max(child.bbox.max_x, bbox.max_x) - std::min(child.bbox.min_x, bbox.min_x)) *
                    (std::max(child.bbox.max_y, bbox.max_y) - std::min(child.bbox.min_y, bbox.min_y))
                ) - area;

                // choose entry with the least enlargement else smallest area
                if (enlargement < minEnlargement) {
                    minEnlargement = enlargement;
                    minArea = std::min(area, minArea);
                    target_node_ptr = &child;
                    targetUpdated = true;
                } else if (enlargement == minEnlargement && area < minArea) {
                    minArea = area;
                    target_node_ptr = &child;
                    targetUpdated = true;
                }
            }
            node_ptr = targetUpdated ? target_node_ptr : &node.children[0];
        }
        return node_ptr;
    }
};

#endif // RTREE_HPP
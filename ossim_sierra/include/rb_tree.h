#ifndef RB_TREE_H
#define RB_TREE_H

#include "common.h"

// Define colors for red-black tree nodes
enum rb_color {
    RB_RED,
    RB_BLACK
};

// Red-black tree node structure
struct rb_node {
    struct rb_node *parent;
    struct rb_node *left;
    struct rb_node *right;
    enum rb_color color;
    struct pcb_t *proc;  // Process associated with this node
};

// Red-black tree structure
struct rb_tree {
    struct rb_node *root;
    struct rb_node *nil;  // Sentinel node
};

// Initialize a new red-black tree
void rb_init(struct rb_tree *tree);

// Insert a process into the red-black tree (ordered by vruntime)
void rb_insert(struct rb_tree *tree, struct pcb_t *proc);

// Find and remove the leftmost node (minimum vruntime)
struct pcb_t *rb_extract_min(struct rb_tree *tree);

// Remove a specific process from the tree
void rb_remove(struct rb_tree *tree, struct pcb_t *proc);

// Check if the tree is empty
int rb_empty(struct rb_tree *tree);

#endif
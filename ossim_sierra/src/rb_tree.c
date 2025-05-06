#include "rb_tree.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef CFS_SCHED
// Left rotation around node x
static void rb_rotate_left(struct rb_tree *tree, struct rb_node *x) {
    struct rb_node *y = x->right;
    x->right = y->left;
    
    if (y->left != tree->nil)
        y->left->parent = x;
    
    y->parent = x->parent;
    
    if (x->parent == tree->nil)
        tree->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    
    y->left = x;
    x->parent = y;
}

// Right rotation around node y
static void rb_rotate_right(struct rb_tree *tree, struct rb_node *y) {
    struct rb_node *x = y->left;
    y->left = x->right;
    
    if (x->right != tree->nil)
        x->right->parent = y;
    
    x->parent = y->parent;
    
    if (y->parent == tree->nil)
        tree->root = x;
    else if (y == y->parent->left)
        y->parent->left = x;
    else
        y->parent->right = x;
    
    x->right = y;
    y->parent = x;
}

// Fix red-black tree properties after insertion
static void rb_insert_fixup(struct rb_tree *tree, struct rb_node *z) {
    while (z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            struct rb_node *y = z->parent->parent->right;
            
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rb_rotate_left(tree, z);
                }
                
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rb_rotate_right(tree, z->parent->parent);
            }
        } else {
            struct rb_node *y = z->parent->parent->left;
            
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rb_rotate_right(tree, z);
                }
                
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rb_rotate_left(tree, z->parent->parent);
            }
        }
    }
    
    tree->root->color = RB_BLACK;
}

static void rb_delete_fixup(struct rb_tree *tree, struct rb_node *x) {
    while (x != tree->root && x->color == RB_BLACK) {
        if (x == x->parent->left) {
            struct rb_node *w = x->parent->right;
            
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                rb_rotate_left(tree, x->parent);
                w = x->parent->right;
            }
            
            if (w->left->color == RB_BLACK && w->right->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->right->color == RB_BLACK) {
                    w->left->color = RB_BLACK;
                    w->color = RB_RED;
                    rb_rotate_right(tree, w);
                    w = x->parent->right;
                }
                
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->right->color = RB_BLACK;
                rb_rotate_left(tree, x->parent);
                x = tree->root;
            }
        } else {
            struct rb_node *w = x->parent->left;
            
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                rb_rotate_right(tree, x->parent);
                w = x->parent->left;
            }
            
            if (w->right->color == RB_BLACK && w->left->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->left->color == RB_BLACK) {
                    w->right->color = RB_BLACK;
                    w->color = RB_RED;
                    rb_rotate_left(tree, w);
                    w = x->parent->left;
                }
                
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->left->color = RB_BLACK;
                rb_rotate_right(tree, x->parent);
                x = tree->root;
            }
        }
    }
    
    x->color = RB_BLACK;
}

// Initialize a new red-black tree
void rb_init(struct rb_tree *tree) {
    tree->nil = (struct rb_node*)malloc(sizeof(struct rb_node));
    tree->nil->color = RB_BLACK;
    tree->nil->left = NULL;
    tree->nil->right = NULL;
    tree->nil->parent = NULL;
    tree->nil->proc = NULL;
    tree->root = tree->nil;
}

// Insert a process into the red-black tree (ordered by vruntime)
void rb_insert(struct rb_tree *tree, struct pcb_t *proc) {
    struct rb_node *z = (struct rb_node*)malloc(sizeof(struct rb_node));
    z->proc = proc;
    
    struct rb_node *y = tree->nil;
    struct rb_node *x = tree->root;
    
    // Find the insertion point based on vruntime
    while (x != tree->nil) {
        y = x;
        if (proc->vruntime < x->proc->vruntime)
            x = x->left;
        else
            x = x->right;
    }
    
    z->parent = y;
    
    if (y == tree->nil)
        tree->root = z;
    else if (proc->vruntime < y->proc->vruntime)
        y->left = z;
    else
        y->right = z;
    
    z->left = tree->nil;
    z->right = tree->nil;
    z->color = RB_RED;
    
    rb_insert_fixup(tree, z);
}

// Find the node with minimum vruntime
static struct rb_node *rb_minimum(struct rb_tree *tree, struct rb_node *x) {
    while (x->left != tree->nil)
        x = x->left;
    return x;
}

// Extract the process with the minimum vruntime
struct pcb_t *rb_extract_min(struct rb_tree *tree) {
    if (tree->root == tree->nil)
        return NULL;
    
    struct rb_node *z = rb_minimum(tree, tree->root);
    struct pcb_t *proc = z->proc;
    
    struct rb_node *y = z;
    enum rb_color y_original_color = y->color;
    struct rb_node *x;
    
    if (z->left == tree->nil) {
        x = z->right;
        
        if (z->parent == tree->nil)
            tree->root = z->right;
        else if (z == z->parent->left)
            z->parent->left = z->right;
        else
            z->parent->right = z->right;
        
        z->right->parent = z->parent;
    }
    else if (z->right == tree->nil) {
        x = z->left;
        
        if (z->parent == tree->nil)
            tree->root = z->left;
        else if (z == z->parent->left)
            z->parent->left = z->left;
        else
            z->parent->right = z->left;
        
        z->left->parent = z->parent;
    }
    else {
        y = rb_minimum(tree, z->right);
        y_original_color = y->color;
        x = y->right;
        
        if (y->parent == z)
            x->parent = y;
        else {
            if (y->parent == tree->nil)
                tree->root = y;
            else if (y == y->parent->left)
                y->parent->left = y->right;
            else
                y->parent->right = y->right;
            
            y->right->parent = y->parent;
            y->right = z->right;
            z->right->parent = y;
        }
        
        if (z->parent == tree->nil)
            tree->root = y;
        else if (z == z->parent->left)
            z->parent->left = y;
        else
            z->parent->right = y;
        
        y->parent = z->parent;
        y->left = z->left;
        z->left->parent = y;
        y->color = z->color;
    }
    
    if (y_original_color == RB_BLACK)
        rb_delete_fixup(tree, x);
    
    free(z);
    return proc;
}

// Remove a specific process from the tree
void rb_remove(struct rb_tree *tree, struct pcb_t *proc) {
    // Implementation omitted for brevity
    // This would work similarly to extract_min but find the specific node first
}

// Check if the tree is empty
int rb_empty(struct rb_tree *tree) {
    return tree->root == tree->nil;
}

#else  // MLQ_SCHED

// Provide empty stub implementations for MLQ mode

void rb_init(struct rb_tree *tree) {
    // Empty implementation for MLQ
}

void rb_insert(struct rb_tree *tree, struct pcb_t *proc) {
    // Empty implementation for MLQ
}

struct pcb_t *rb_extract_min(struct rb_tree *tree) {
    // Empty implementation for MLQ
    return NULL;
}

void rb_remove(struct rb_tree *tree, struct pcb_t *proc) {
    // Empty implementation for MLQ
}

int rb_empty(struct rb_tree *tree) {
    // Empty implementation for MLQ
    return 1;
}

#endif // CFS_SCHED